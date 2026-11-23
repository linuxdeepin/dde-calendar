// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "rrule.h"

#include <QDebug>
#include <QHash>
#include <QTimeZone>
#include <libical/ical.h>

#include <limits>
#include <memory>

using namespace CalendarCore;

namespace {
struct TimeZoneDeleter {
    void operator()(icaltimezone *zone) const { if (zone) icaltimezone_free(zone, 1); }
};
}

namespace {

constexpr int MaxOccurrencesPerQuery = 10000;
// COUNT rules with calendar filters normally need to iterate from DTSTART.
constexpr int MaxIterationsPerQuery = 100000;

const char *const frequencies[] = { "", "SECONDLY", "MINUTELY", "HOURLY", "DAILY",
                                   "WEEKLY", "MONTHLY", "YEARLY" };

icaltimetype localFields(const QDateTime &time, bool dateOnly)
{
    auto value = icaltime_null_time();
    value.year = time.date().year();
    value.month = time.date().month();
    value.day = time.date().day();
    value.is_date = dateOnly;
    if (!dateOnly) {
        value.hour = time.time().hour();
        value.minute = time.time().minute();
        value.second = time.time().second();
    }
    return value;
}

qint64 fixedStepSeconds(RRule::PeriodType period)
{
    switch (period) {
    case RRule::rSecondly: return 1;
    case RRule::rMinutely: return 60;
    case RRule::rHourly: return 3600;
    case RRule::rDaily: return 86400;
    case RRule::rWeekly: return 7 * 86400;
    default: return 0;
    }
}

} // namespace

bool RRule::parse(const QString &text)
{
    return parse(text, false);
}

bool RRule::parse(const QString &text, bool allowInvalidUntil)
{
    const QByteArray bytes = text.trimmed().toUpper().toLatin1();
    icalerror_clear_errno();
    auto rule = icalrecurrencetype_from_string(bytes.constData());
    if (icalerrno != ICAL_NO_ERROR || rule.freq < ICAL_SECONDLY_RECURRENCE || rule.freq > ICAL_YEARLY_RECURRENCE) {
        return false;
    }

    QDateTime until;
    if (!icaltime_is_null_time(rule.until)) {
        const auto &end = rule.until;
        const QDate date(end.year, end.month, end.day);
        const QTime time = end.is_date ? QTime(0, 0) : QTime(end.hour, end.minute, end.second);
        // Preserve the literal fields of a floating end independently of the host zone.
        until = QDateTime(date, time, Qt::UTC);
    }
    if (!until.isValid() && (bytes.startsWith("UNTIL=") || bytes.contains(";UNTIL=")) && !allowInvalidUntil) {
        return false;
    }

    QStringList filters;
    const QString canonical = QString::fromLatin1(icalrecurrencetype_as_string(&rule));
    for (const QString &part : canonical.split(QLatin1Char(';'))) {
        const QString key = part.section(QLatin1Char('='), 0, 0);
        if (key != QLatin1String("FREQ") && key != QLatin1String("INTERVAL")
            && key != QLatin1String("COUNT") && key != QLatin1String("UNTIL")) {
            if (key.startsWith(QLatin1String("BY"))) {
                auto values = part.mid(key.size() + 1).split(QLatin1Char(','));
                // Legacy KCalendarCore can write duplicate BY values. Count each slot once.
                values.removeDuplicates();
                filters.append(key + QLatin1Char('=') + values.join(QLatin1Char(',')));
            } else {
                filters.append(part);
            }
        }
    }
    m_endComputed = false;
    m_period = static_cast<PeriodType>(rule.freq + 1);
    m_interval = qMax(1, int(rule.interval));
    m_filters = filters;
    m_untilDate = rule.until.is_date;
    m_untilFloating = !m_untilDate && !icaltime_is_utc(rule.until);
    m_until = until;
    m_count = rule.count > 0 ? rule.count : (m_until.isValid() ? 0 : -1);
    return true;
}

void RRule::bindFloatingUntil(const QTimeZone &zone)
{
    if (m_count == 0 && m_untilFloating && m_until.isValid()) {
        const QDateTime end(m_until.date(), m_until.time(), zone);
        if (end.isValid()) {
            m_until = end;
            m_untilFloating = false;
            m_endComputed = false;
        }
    }
}

QString RRule::rrule() const
{
    if (m_period == rNone) {
        return {};
    }
    QStringList clauses{ QStringLiteral("FREQ=") + QLatin1String(frequencies[m_period]) };
    if (m_count > 0) {
        clauses << QStringLiteral("COUNT=%1").arg(m_count);
    } else if (m_count == 0 && m_until.isValid()) {
        QString end;
        if (m_allDay || m_untilDate) {
            end = m_until.date().toString(QStringLiteral("yyyyMMdd"));
        } else if (m_untilFloating) {
            const auto boundEnd = m_start.isValid() && m_start.timeSpec() != Qt::LocalTime
                    && m_timeZoneComponent.isEmpty() ? endDt() : QDateTime();
            end = boundEnd.isValid() ? boundEnd.toUTC().toString(QStringLiteral("yyyyMMdd'T'hhmmss'Z'"))
                                     : m_until.toString(QStringLiteral("yyyyMMdd'T'hhmmss"));
        } else {
            end = m_until.toUTC().toString(QStringLiteral("yyyyMMdd'T'hhmmss'Z'"));
        }
        clauses << QStringLiteral("UNTIL=") + end;
    }
    if (m_interval > 1) {
        clauses << QStringLiteral("INTERVAL=%1").arg(m_interval);
    }
    clauses << m_filters;
    return clauses.join(QLatin1Char(';'));
}

void RRule::setPeriodType(PeriodType period)
{
    m_period = period;
    m_endComputed = false;
}

void RRule::setEndDt(const QDateTime &end)
{
    m_endComputed = false;
    m_untilDate = m_allDay;
    m_untilFloating = !m_allDay && (m_start.isValid() ? m_start.timeSpec() == Qt::LocalTime
                                                     : end.timeSpec() == Qt::LocalTime);
    m_until = end;
    if (m_untilFloating && end.isValid()) {
        const auto localEnd = end.toLocalTime();
        m_until = QDateTime(localEnd.date(), localEnd.time(), Qt::UTC);
    }
    m_count = end.isValid() ? 0 : -1;
}

QList<QDateTime> RRule::occurrences(const QDateTime &from, const QDateTime &through,
                                   bool lastOnly, int *count) const
{
    if (count) {
        *count = 0;
    }
    if (m_period == rNone || !m_start.isValid() || (through.isValid() && through < m_start)) {
        return {};
    }

    const auto startZone = m_start.timeZone();
    const qint64 step = fixedStepSeconds(m_period) * m_interval;
    // Simple sub-daily rules use elapsed time, including across DST transitions.
    const bool elapsedRecurrence = !m_allDay && m_period <= rHourly;
    const bool elapsedUntil = elapsedRecurrence && m_count == 0 && !m_untilDate && !m_untilFloating;
    if ((m_count > 0 || (m_count < 0 && (elapsedRecurrence || count)) || elapsedUntil)
        && step > 0 && m_filters.isEmpty() && m_timeZoneComponent.isEmpty()
        && (elapsedRecurrence || !startZone.hasTransitions())
        && (!m_allDay || (m_period >= rDaily && m_start.time() == QTime(0, 0)))) {
        // These occurrences are equally spaced, so even large counts can be
        // queried directly and the final occurrence does not require a scan.
        const qint64 stepMs = step * 1000;
        if (lastOnly) {
            const qint64 available = (std::numeric_limits<qint64>::max)() - qMax<qint64>(0, m_start.toMSecsSinceEpoch());
            if (m_count - 1 > available / stepMs) {
                return {};
            }
            return { m_start.addMSecs((m_count - 1) * stepMs) };
        }
        const qint64 elapsed = qMax<qint64>(0, m_start.msecsTo(from));
        const qint64 first = elapsed / stepMs + (elapsed % stepMs != 0);
        const auto windowEnd = elapsedUntil ? qMin(through, m_until) : through;
        if (windowEnd < m_start) {
            return {};
        }
        const qint64 lastWithinWindow = m_start.msecsTo(windowEnd) / stepMs;
        const qint64 last = m_count > 0 ? qMin<qint64>(m_count - 1, lastWithinWindow) : lastWithinWindow;
        if (count) {
            const qint64 total = qMax<qint64>(0, last - first + 1);
            *count = total <= (std::numeric_limits<int>::max)() ? int(total) : -1;
            return {};
        }
        const qint64 limit = qMin(last + 1, first + MaxOccurrencesPerQuery);
        QList<QDateTime> found;
        for (qint64 index = first; index < limit; ++index) {
            found.append(m_start.addMSecs(index * stepMs));
        }
        return found;
    }

    auto recurrence = icalrecurrencetype_from_string(rrule().toLatin1().constData());
    // Run libical's iterator in wall-clock time. Qt applies the event's time
    // zone when we materialize each result, including historical DST offsets.
    recurrence.until = icaltime_null_time();
    // Count valid occurrences after resolving wall times so DST gaps do not consume COUNT.
    recurrence.count = 0;
    std::unique_ptr<icalrecur_iterator, decltype(&icalrecur_iterator_free)> iterator(
        icalrecur_iterator_new(recurrence, localFields(m_start, m_allDay)), icalrecur_iterator_free);
    if (!iterator) {
        return {};
    }
    std::unique_ptr<icaltimezone, TimeZoneDeleter> customZone;
    if (!m_timeZoneComponent.isEmpty()) {
        icalcomponent *component = icalcomponent_new_from_string(m_timeZoneComponent.constData());
        if (component) {
            customZone.reset(icaltimezone_new());
            if (!icaltimezone_set_component(customZone.get(), component)) {
                icalcomponent_free(component);
                customZone.reset();
            }
        }
    }
    if (m_count <= 0 && from > m_start) {
        QDateTime localStart = from.toTimeZone(startZone);
        if (customZone) {
            auto utcFields = localFields(from.toUTC(), false);
            auto nextDayFields = localFields(from.toUTC().addDays(1), false);
            int daylight = 0;
            const int offset = icaltimezone_get_utc_offset_of_utc_time(customZone.get(), &utcFields, &daylight);
            const int nextOffset = icaltimezone_get_utc_offset_of_utc_time(customZone.get(), &nextDayFields, &daylight);
            // Seek early enough to include wall times repeated by a backward
            // transition. The UTC window still filters the resulting instances.
            localStart = from.toUTC().addSecs(qMin(offset, nextOffset));
        }
        if (localStart.date() > m_start.date()
            || (localStart.date() == m_start.date() && localStart.time() > m_start.time())) {
            icalrecur_iterator_set_start(iterator.get(), localFields(localStart, m_allDay));
        }
    }

    const QString customZoneName = QString::fromUtf8(m_timeZoneId);
    QHash<int, QTimeZone> customOffsets;
    QList<QDateTime> found;
    int visited = 0;
    int validOccurrences = 0;
    const auto dateOnlyEnd = m_count == 0 && m_untilDate && !m_allDay ? endDt() : QDateTime();
    for (auto next = icalrecur_iterator_next(iterator.get()); !icaltime_is_null_time(next);
         next = icalrecur_iterator_next(iterator.get())) {
        const QDate date(next.year, next.month, next.day);
        const QTime time = m_allDay ? QTime(0, 0) : QTime(next.hour, next.minute, next.second);
        QTimeZone zone = startZone;
        if (customZone) {
            int daylight = 0;
            const int seconds = icaltimezone_get_utc_offset(customZone.get(), &next, &daylight);
            auto zoneForOffset = customOffsets.constFind(seconds);
            if (zoneForOffset == customOffsets.cend()) {
                zone = QTimeZone(m_timeZoneId, seconds, customZoneName, customZoneName);
                if (!zone.isValid()) {
                    zone = QTimeZone(seconds);
                }
                customOffsets.insert(seconds, zone);
            } else {
                zone = zoneForOffset.value();
            }
        }
        const QDateTime candidate = m_start.timeSpec() == Qt::LocalTime && !customZone
            ? QDateTime(date, time) : QDateTime(date, time, zone);
        // Qt may normalize a nonexistent wall time to the other side of a DST gap.
        bool wallTimeExists = candidate.isValid() && (m_allDay || (candidate.date() == date && candidate.time() == time));
        if (wallTimeExists && customZone) {
            auto utcFields = localFields(candidate.toUTC(), false);
            int daylight = 0;
            wallTimeExists = icaltimezone_get_utc_offset_of_utc_time(customZone.get(), &utcFields, &daylight) == candidate.offsetFromUtc();
        }
        if (wallTimeExists) {
            if (m_count == 0 && m_until.isValid()) {
                bool pastEnd = false;
                if (m_allDay) {
                    pastEnd = date > m_until.date();
                } else if (m_untilDate) {
                    // KCalendarCore treated a date-only UNTIL on a timed event as local midnight.
                    pastEnd = candidate > dateOnlyEnd;
                } else if (m_untilFloating) {
                    pastEnd = QDateTime(date, time, Qt::UTC) > m_until;
                } else {
                    pastEnd = candidate > m_until;
                }
                if (pastEnd) {
                    break;
                }
            }
            if (through.isValid() && candidate > through) {
                break;
            }
        }
        if (visited++ == MaxIterationsPerQuery) {
            if (count) {
                *count = -1;
            }
            return lastOnly ? QList<QDateTime>() : found;
        }
        if (!wallTimeExists) {
            continue;
        }
        if (!from.isValid() || candidate >= from) {
            if (count) {
                ++*count;
            } else {
                if (lastOnly) {
                    found.clear();
                }
                found.append(candidate);
                if (!lastOnly && found.size() == MaxOccurrencesPerQuery) {
                    break;
                }
            }
        }
        if (m_count > 0 && ++validOccurrences == m_count) {
            break;
        }
    }
    return found;
}

QList<QDateTime> RRule::timesInInterval(const QDateTime &from, const QDateTime &through) const
{
    if (!from.isValid() || !through.isValid() || through < from) {
        return {};
    }
    return occurrences(from, through);
}

bool RRule::mayInclude(const QDateTime &time) const
{
    if (!time.isValid() || (m_count > 0 && m_endComputed && m_cachedEnd.isValid() && time > m_cachedEnd)) {
        return false;
    }
    int count = 0;
    occurrences(time, time, false, &count);
    // A scan limit cannot prove that an occurrence is absent.
    return count != 0;
}

QDateTime RRule::endDt() const
{
    if (m_count == 0) {
        if (!m_until.isValid() || (!m_untilDate && !m_untilFloating)) {
            return m_until;
        }
        if (!m_start.isValid() || m_start.timeSpec() == Qt::LocalTime || m_untilDate) {
            return QDateTime(m_until.date(), m_until.time());
        }
        return QDateTime(m_until.date(), m_until.time(), m_start.timeZone());
    }
    if (m_count < 0) {
        return {};
    }
    if (!m_endComputed) {
        const auto last = occurrences({}, {}, true);
        m_cachedEnd = last.isEmpty() ? QDateTime() : last.constFirst();
        m_endComputed = true;
    }
    return m_cachedEnd;
}

int RRule::durationTo(const QDateTime &through) const
{
    if (!through.isValid() || through < m_start) {
        return 0;
    }
    if (m_count > 0 && m_endComputed && m_cachedEnd.isValid() && through >= m_cachedEnd) {
        return m_count;
    }
    int count = 0;
    occurrences(m_start, through, false, &count);
    return count;
}
