// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "repeatinfo.h"

#include <QSet>

#include <algorithm>
#include <memory>
#include <utility>

using namespace CalendarCore;

namespace {

template<typename T>
void addValid(QSet<T> &values, const T &value)
{
    if (value.isValid()) {
        values.insert(value);
    }
}

template<typename T>
QList<T> sortedValues(const QSet<T> &values)
{
    auto sorted = values.values();
    std::sort(sorted.begin(), sorted.end());
    return sorted;
}

} // namespace

RepeatInfo::RepeatInfo(const RepeatInfo &other)
    : m_start(other.m_start), m_allDay(other.m_allDay), m_timeZoneId(other.m_timeZoneId),
      m_timeZoneComponent(other.m_timeZoneComponent), m_dates(other.m_dates),
      m_exceptionDates(other.m_exceptionDates), m_times(other.m_times), m_exceptionTimes(other.m_exceptionTimes)
{
    for (const auto &rule : other.m_rules) {
        m_rules.append(QSharedPointer<RRule>::create(*rule));
    }
    for (const auto &rule : other.m_exceptionRules) {
        m_exceptionRules.append(QSharedPointer<RRule>::create(*rule));
    }
}

RepeatInfo &RepeatInfo::operator=(RepeatInfo other)
{
    std::swap(m_start, other.m_start);
    std::swap(m_allDay, other.m_allDay);
    std::swap(m_timeZoneId, other.m_timeZoneId);
    std::swap(m_timeZoneComponent, other.m_timeZoneComponent);
    m_rules.swap(other.m_rules);
    m_exceptionRules.swap(other.m_exceptionRules);
    m_dates.swap(other.m_dates);
    m_exceptionDates.swap(other.m_exceptionDates);
    m_times.swap(other.m_times);
    m_exceptionTimes.swap(other.m_exceptionTimes);
    return *this;
}

bool RepeatInfo::operator!=(const RepeatInfo &other) const
{
    const auto sameRules = [](const QList<QSharedPointer<RRule>> &first,
                              const QList<QSharedPointer<RRule>> &second) {
        return first.size() == second.size()
            && std::equal(first.cbegin(), first.cend(), second.cbegin(),
                          [](const QSharedPointer<RRule> &a, const QSharedPointer<RRule> &b) {
                              return a->rrule() == b->rrule();
                          });
    };
    return m_start != other.m_start || m_allDay != other.m_allDay
        || m_timeZoneId != other.m_timeZoneId || m_timeZoneComponent != other.m_timeZoneComponent
        || m_dates != other.m_dates
        || m_times != other.m_times || m_exceptionDates != other.m_exceptionDates
        || m_exceptionTimes != other.m_exceptionTimes || !sameRules(m_rules, other.m_rules)
        || !sameRules(m_exceptionRules, other.m_exceptionRules);
}

void RepeatInfo::setAnchor(const QDateTime &start, bool allDay)
{
    m_start = start;
    m_allDay = allDay;
    if (allDay) {
        m_start.setTime(QTime(0, 0));
    }
    for (const auto *group : { &m_rules, &m_exceptionRules }) {
        for (const auto &rule : *group) {
            rule->setStartDt(m_start);
            rule->setAllDay(allDay);
            rule->setCustomTimeZone(m_timeZoneId, m_timeZoneComponent);
        }
    }
}

void RepeatInfo::setCustomTimeZone(const QByteArray &id, const QByteArray &component)
{
    m_timeZoneId = id;
    m_timeZoneComponent = component;
    for (const auto *group : { &m_rules, &m_exceptionRules }) {
        for (const auto &rule : *group) {
            rule->setCustomTimeZone(id, component);
        }
    }
}

void RepeatInfo::clear()
{
    m_rules.clear();
    m_exceptionRules.clear();
    m_dates.clear();
    m_exceptionDates.clear();
    m_times.clear();
    m_exceptionTimes.clear();
}

bool RepeatInfo::recurs() const
{
    return !m_rules.isEmpty() || !m_dates.isEmpty() || !m_times.isEmpty();
}

int RepeatInfo::periodType() const
{
    auto rule = defaultRRuleConst();
    if (!rule) {
        return rNone;
    }
    const QString text = rule->rrule();
    const auto hasFilter = [&](const char *filter) { return text.contains(QLatin1String(filter)); };
    const auto period = rule->periodType();
    const bool byDay = hasFilter("BYDAY=");
    const bool byMonthDay = hasFilter("BYMONTHDAY=");
    const bool byYearDay = hasFilter("BYYEARDAY=");
    const bool byMonth = hasFilter("BYMONTH=");
    // Keep the historical recurrence.type classification exposed over D-Bus.
    if (hasFilter("BYSETPOS=") || hasFilter("BYWEEKNO=") || hasFilter("BYSECOND=")
        || hasFilter("BYMINUTE=") || hasFilter("BYHOUR=")
        || ((byYearDay || byMonth) && period != RRule::rYearly)
        || (byDay && period != RRule::rWeekly && period != RRule::rMonthly && period != RRule::rYearly)) {
        return rOther;
    }
    switch (period) {
    case RRule::rMinutely: return rMinutely;
    case RRule::rHourly: return rHourly;
    case RRule::rDaily: return rDaily;
    case RRule::rWeekly: return rWeekly;
    case RRule::rMonthly:
        if (byDay && byMonthDay) {
            return rOther;
        }
        return byDay ? rMonthlyPos : rMonthlyDay;
    case RRule::rYearly:
        if (byDay) {
            return byMonthDay || byYearDay ? rOther : rYearlyPos;
        }
        if (byYearDay) {
            return byMonth || byMonthDay ? rOther : rYearlyDay;
        }
        return rYearlyMonth;
    case RRule::rNone: return rNone;
    default: return rOther;
    }
}

int RepeatInfo::duration() const
{
    const auto rule = defaultRRuleConst();
    return rule ? rule->duration() : 0;
}

int RepeatInfo::frequency() const
{
    const auto rule = defaultRRuleConst();
    return rule ? rule->frequency() : 0;
}

int RepeatInfo::durationTo(const QDateTime &through) const
{
    const auto rule = defaultRRuleConst();
    return rule ? rule->durationTo(through) : 0;
}

QDateTime RepeatInfo::endDateTime() const
{
    auto end = m_start;
    for (const auto &rule : m_rules) {
        const auto last = rule->endDt();
        if (!last.isValid()) {
            return {};
        }
        end = qMax(end, last);
    }
    if (!m_dates.isEmpty()) {
        end = qMax(end, QDateTime(*std::max_element(m_dates.cbegin(), m_dates.cend()), QTime(0, 0), m_start.timeZone()));
    }
    if (!m_times.isEmpty()) {
        end = qMax(end, *std::max_element(m_times.cbegin(), m_times.cend()));
    }
    return end;
}

void RepeatInfo::setDuration(int count)
{
    if (!m_rules.isEmpty()) {
        m_rules.constFirst()->setDuration(count);
    }
}

void RepeatInfo::setEndDate(const QDate &day)
{
    setEndDateTime(QDateTime(day, m_allDay ? QTime(23, 59, 59) : m_start.time(), m_start.timeZone()));
}

void RepeatInfo::setEndDateTime(const QDateTime &time)
{
    if (!m_rules.isEmpty()) {
        m_rules.constFirst()->setEndDt(time);
    }
}

void RepeatInfo::setDaily(int interval)
{
    auto rule = std::unique_ptr<RRule>(new RRule);
    if (rule->parse(QStringLiteral("FREQ=DAILY;INTERVAL=%1").arg(qMax(1, interval)))) {
        if (!m_rules.isEmpty()) {
            m_rules.removeFirst();
        }
        rule->setStartDt(m_start);
        rule->setAllDay(m_allDay);
        rule->setCustomTimeZone(m_timeZoneId, m_timeZoneComponent);
        m_rules.prepend(QSharedPointer<RRule>(rule.release()));
    }
}

void RepeatInfo::addRRule(RRule *rule)
{
    if (rule) {
        rule->setStartDt(m_start);
        rule->setAllDay(m_allDay);
        rule->setCustomTimeZone(m_timeZoneId, m_timeZoneComponent);
        m_rules.append(QSharedPointer<RRule>(rule));
    }
}

void RepeatInfo::addExRule(RRule *rule)
{
    if (rule) {
        rule->setStartDt(m_start);
        rule->setAllDay(m_allDay);
        rule->setCustomTimeZone(m_timeZoneId, m_timeZoneComponent);
        m_exceptionRules.append(QSharedPointer<RRule>(rule));
    }
}

const RRule *RepeatInfo::defaultRRuleConst() const
{
    return m_rules.isEmpty() ? nullptr : m_rules.constFirst().data();
}

QList<QDate> RepeatInfo::rDates() const { return sortedValues(m_dates); }
QList<QDate> RepeatInfo::exDates() const { return sortedValues(m_exceptionDates); }
QList<QDateTime> RepeatInfo::rDateTimes() const { return sortedValues(m_times); }
QList<QDateTime> RepeatInfo::exDateTimes() const { return sortedValues(m_exceptionTimes); }

void RepeatInfo::addRDate(const QDate &date) { addValid(m_dates, date); }
void RepeatInfo::addExDate(const QDate &date) { addValid(m_exceptionDates, date); }
void RepeatInfo::addRDateTime(const QDateTime &time) { addValid(m_times, time); }
void RepeatInfo::addExDateTime(const QDateTime &time) { addValid(m_exceptionTimes, time); }

void RepeatInfo::setRDateTimes(const QList<QDateTime> &times)
{
    m_times.clear();
    for (const auto &time : times) {
        addRDateTime(time);
    }
}

void RepeatInfo::setExDateTimes(const QList<QDateTime> &times)
{
    m_exceptionTimes.clear();
    for (const auto &time : times) {
        addExDateTime(time);
    }
}

QDateTime RepeatInfo::rebaseRecurrenceId(const QDateTime &id, const QDateTime &oldStart,
                                              const QDateTime &newStart, bool wasAllDay, bool allDay)
{
    if (oldStart == newStart && oldStart.timeZone() == newStart.timeZone() && wasAllDay == allDay) {
        return id;
    }
    const auto local = wasAllDay ? id : id.toTimeZone(oldStart.timeZone());
    const QDateTime wallTime(local.date().addDays(oldStart.date().daysTo(newStart.date())), local.time(), Qt::UTC);
    const auto shifted = wallTime.addMSecs(oldStart.time().msecsTo(newStart.time()));
    return QDateTime(shifted.date(), allDay ? QTime(0, 0) : shifted.time(), newStart.timeZone());
}

void RepeatInfo::partitionDatesAt(const QDateTime &cutoff, RepeatInfo *tail)
{
    if (!cutoff.isValid()) {
        return;
    }
    const auto anchor = m_allDay ? cutoff : cutoff.toTimeZone(m_start.timeZone());
    const auto split = [&](auto &dates, auto *destination, const auto &toTime, const auto &toValue) {
        if (destination) {
            destination->clear();
        }
        for (auto entry = dates.begin(); entry != dates.end();) {
            const auto time = toTime(*entry);
            if (time < cutoff) {
                ++entry;
                continue;
            }
            if (destination) {
                const auto shifted = rebaseRecurrenceId(time, anchor, tail->m_start, m_allDay, tail->m_allDay);
                addValid(*destination, toValue(shifted));
            }
            entry = dates.erase(entry);
        }
    };
    const auto asTime = [](const QDateTime &time) { return time; };
    const auto dateSlot = [&](const QDate &date) { return QDateTime(date, m_start.time(), m_start.timeZone()); };
    const auto asDate = [](const QDateTime &time) { return time.date(); };
    split(m_times, tail ? &tail->m_times : nullptr, asTime, asTime);
    split(m_exceptionTimes, tail ? &tail->m_exceptionTimes : nullptr, asTime, asTime);
    split(m_dates, tail ? &tail->m_dates : nullptr, dateSlot, asDate);
    split(m_exceptionDates, tail ? &tail->m_exceptionDates : nullptr, dateSlot, asDate);
}

bool RepeatInfo::hasAdditionalRecurrenceData() const
{
    return m_rules.size() > 1 || !m_exceptionRules.isEmpty() || !m_dates.isEmpty() || !m_times.isEmpty()
        || !m_exceptionDates.isEmpty() || !m_exceptionTimes.isEmpty();
}

bool RepeatInfo::mayIncludeRecurrenceId(const QDateTime &id) const
{
    if (!id.isValid() || !m_start.isValid()) {
        return false;
    }
    const auto time = m_allDay ? QDateTime(id.date(), m_start.time(), m_start.timeZone()) : id;
    const auto local = time.toTimeZone(m_start.timeZone());
    // An explicit detached event can replace a slot suppressed by EXDATE or EXRULE.
    if (m_times.contains(time) || (local.time() == m_start.time() && m_dates.contains(local.date()))) {
        return true;
    }
    return m_rules.isEmpty() ? time == m_start
                             : std::any_of(m_rules.cbegin(), m_rules.cend(), [&](const auto &rule) {
                                   return rule->mayInclude(time);
                               });
}

QList<QDateTime> RepeatInfo::timesInInterval(const QDateTime &from, const QDateTime &through) const
{
    if (!m_start.isValid() || !from.isValid() || !through.isValid() || from > through) {
        return {};
    }
    QSet<QDateTime> candidates;
    auto include = [&](const QDateTime &time) {
        if (time >= from && time <= through) {
            candidates.insert(time);
        }
    };
    if (m_rules.isEmpty() && (!m_dates.isEmpty() || !m_times.isEmpty())) {
        include(m_start);
    }
    for (const auto &rule : m_rules) {
        const auto matches = rule->timesInInterval(from, through);
        for (const auto &time : matches) {
            include(time);
        }
    }
    for (const auto &time : m_times) {
        include(time);
    }
    for (const auto &day : m_dates) {
        include(QDateTime(day, m_start.time(), m_start.timeZone()));
    }
    for (const auto &rule : m_exceptionRules) {
        const auto excluded = rule->timesInInterval(from, through);
        for (const auto &time : excluded) {
            candidates.remove(time);
        }
    }
    QList<QDateTime> result;
    for (const auto &time : candidates) {
        if (!m_exceptionTimes.contains(time)
            && !m_exceptionDates.contains(time.toTimeZone(m_start.timeZone()).date())) {
            result.append(time);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}
