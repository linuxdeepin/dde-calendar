// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "rrule.h"

#include <QSharedPointer>
#include <QSet>
#include <QTimeZone>

namespace CalendarCore {

class RepeatInfo
{
public:
    enum Type : int { rNone, rMinutely, rHourly, rDaily, rWeekly, rMonthlyPos, rMonthlyDay,
                rYearlyMonth, rYearlyDay, rYearlyPos, rOther };

    RepeatInfo() = default;
    RepeatInfo(const RepeatInfo &other);
    RepeatInfo &operator=(RepeatInfo other);
    bool operator!=(const RepeatInfo &other) const;
    void setAnchor(const QDateTime &start, bool allDay);
    void setCustomTimeZone(const QByteArray &id, const QByteArray &component);
    void clear();
    bool recurs() const;
    int periodType() const;
    int frequency() const;
    int duration() const;
    // Returns -1 if counting exceeds the scan limit.
    int durationTo(const QDateTime &through) const;
    // An invalid end can mean an infinite series or a finite end that could not be calculated.
    QDateTime endDateTime() const;
    QDate endDate() const { return endDateTime().date(); }
    void setDuration(int count);
    void setEndDate(const QDate &day);
    void setEndDateTime(const QDateTime &time);
    void setDaily(int interval);

    void addRRule(RRule *rule);
    void addExRule(RRule *rule);
    const RRule *defaultRRuleConst() const;
    QList<QDate> rDates() const;
    QList<QDate> exDates() const;
    QList<QDateTime> rDateTimes() const;
    QList<QDateTime> exDateTimes() const;
    void addRDate(const QDate &date);
    void addExDate(const QDate &date);
    void addRDateTime(const QDateTime &time);
    void addExDateTime(const QDateTime &time);
    void setRDateTimes(const QList<QDateTime> &times);
    void setExDateTimes(const QList<QDateTime> &times);
    void partitionDatesAt(const QDateTime &cutoff, RepeatInfo *tail = nullptr);
    bool hasAdditionalRecurrenceData() const;
    static QDateTime rebaseRecurrenceId(const QDateTime &id, const QDateTime &oldStart,
                                        const QDateTime &newStart, bool wasAllDay, bool allDay);
    bool mayIncludeRecurrenceId(const QDateTime &id) const;
    QList<QDateTime> timesInInterval(const QDateTime &from, const QDateTime &through) const;

private:
    friend class IcsCodec;
    QDateTime m_start;
    bool m_allDay = false;
    QByteArray m_timeZoneId, m_timeZoneComponent;
    QList<QSharedPointer<RRule>> m_rules, m_exceptionRules;
    QSet<QDate> m_dates, m_exceptionDates;
    QSet<QDateTime> m_times, m_exceptionTimes;
};

} // namespace CalendarCore
