// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QDateTime>
#include <QByteArray>
#include <QList>
#include <QStringList>

namespace CalendarCore {

class RRule
{
public:
    enum PeriodType : int { rNone, rSecondly, rMinutely, rHourly, rDaily, rWeekly, rMonthly, rYearly };

    bool parse(const QString &text);
    QString rrule() const;
    PeriodType periodType() const { return m_period; }
    void setPeriodType(PeriodType period);
    int frequency() const { return m_interval; }
    int duration() const { return m_count; }
    void setDuration(int count) { m_count = count; m_endComputed = false; }
    QDateTime startDt() const { return m_start; }
    void setStartDt(const QDateTime &start) { m_start = start; m_endComputed = false; }
    void setAllDay(bool allDay) { m_allDay = allDay; m_endComputed = false; }
    void setCustomTimeZone(const QByteArray &id, const QByteArray &component)
    {
        m_timeZoneId = id;
        m_timeZoneComponent = component;
        m_endComputed = false;
    }
    void setEndDt(const QDateTime &end);
    // A finite COUNT can have an unknown end if calculating it exceeds the scan limit.
    QDateTime endDt() const;
    // Counts occurrences without materializing them. Returns -1 if the scan limit is reached.
    int durationTo(const QDateTime &through) const;
    bool mayInclude(const QDateTime &time) const;
    // Expansion returns at most 10,000 occurrences and scans at most 100,000 candidates.
    QList<QDateTime> timesInInterval(const QDateTime &from, const QDateTime &through) const;

private:
    friend class IcsCodec;
    bool parse(const QString &text, bool allowInvalidUntil);
    void bindFloatingUntil(const QTimeZone &zone);
    QList<QDateTime> occurrences(const QDateTime &from, const QDateTime &through,
                                 bool lastOnly = false, int *count = nullptr) const;
    QStringList m_filters;
    QDateTime m_start;
    QDateTime m_until;
    PeriodType m_period = rNone;
    int m_interval = 1;
    int m_count = -1;
    bool m_allDay = false;
    bool m_untilDate = false;
    bool m_untilFloating = false;
    QByteArray m_timeZoneId;
    QByteArray m_timeZoneComponent;
    mutable bool m_endComputed = false;
    mutable QDateTime m_cachedEnd;
};

} // namespace CalendarCore
