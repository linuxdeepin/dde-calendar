// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUERYSCHEDULEPROXY_H
#define QUERYSCHEDULEPROXY_H

#include "../data/schedulestructs.h"
#include "../data/jsondata.h"
#include "../dbus/schedulesdbus.h"

class queryScheduleProxy
{
public:
    queryScheduleProxy(JsonData *jsonData, CSchedulesDBus *dbus);
    QVector<ScheduleDtailInfo> querySchedule();
    bool getTimeIsExpired() const;

private:
    struct DateTimeLimit {
        QDateTime beginTime;
        QDateTime endTime;
    };
    struct TimeLimit {
        QTime beginTime;
        QTime endTime;
        bool isInvalid {false};
    };
    QVector<ScheduleDtailInfo> queryWeeklySchedule(QDateTime &beginTime, QDateTime &endTime, int beginW = 0, int endW = 0);
    QVector<ScheduleDtailInfo> queryMonthlySchedule(QDateTime &beginTime, QDateTime &endTime, int beginM = 0, int endM = 0);
    QVector<ScheduleDtailInfo> queryEveryDaySchedule(QDateTime &beginTime, QDateTime &endTime);
    QVector<ScheduleDtailInfo> queryEveryYearSchedule(QDateTime &beginTime, QDateTime &endTime);
    QVector<ScheduleDtailInfo> queryWorkingDaySchedule(QDateTime &beginTime, QDateTime &endTime);
    QVector<ScheduleDtailInfo> queryNonRepeatingSchedule();
    QVector<ScheduleDtailInfo> queryNextNumSchedule(QDateTime &beginTime, QDateTime &endTime, int NextNum);
    QVector<ScheduleDtailInfo> queryAllSchedule(QString key, QDateTime &beginTime, QDateTime &endTime);

    QVector<ScheduleDtailInfo> sortAndFilterSchedule(QVector<ScheduleDateRangeInfo> &out);
    QVector<ScheduleDtailInfo> WeeklyScheduleFileter(QVector<ScheduleDateRangeInfo> &out, QSet<int> &weeklySet);
    QVector<ScheduleDtailInfo> MonthlyScheduleFileter(QVector<ScheduleDateRangeInfo> &out, int beginM = 0, int endM = 0);
    bool monthlyIsIntersections(QDateTime &beginTime, QDateTime &endTime, int beginM = 0, int endM = 0);

    bool checkedTimeIsIntersection(QTime &beginTime, QTime &endTime, QTime &fixbeginTime, QTime &fixendTime);

    QVector<ScheduleDtailInfo> scheduleFileterByTime(QVector<ScheduleDtailInfo> &scheduleInfo, QTime &fileterBeginTime, QTime &fileterEndTime);
    QVector<ScheduleDtailInfo> scheduleFileterByDate(QVector<ScheduleDtailInfo> &scheduleInfo, QDate &fileterBeginDate, QDate &fileterEndDate);
    QVector<ScheduleDtailInfo> scheduleFileterByTitleName(QVector<ScheduleDtailInfo> &scheduleInfo, const QString &strName);
    bool weeklyIsIntersections(QDateTime &beginTime, QDateTime &endTime, QSet<int> &weeklySet);

    SemanticsDateTime getQueryDateTime(JsonData *jsonData);

    DateTimeLimit getTimeLimitByTimeInfo(const SemanticsDateTime &timeInfoVect);
    TimeLimit getTimeFileterByTimeInfo(const SemanticsDateTime &timeInfoVect);
    void setTimeIsExpired(const bool timeisExp);
    /**
     * @brief timeFrameIsValid    时间范围是否有效，开始时间不能大于结束时间
     * @param timeInfoVect  时间范围
     * @return      返回false表示开始时间大于结束时间
     */
    bool timeFrameIsValid(const SemanticsDateTime &timeInfoVect);
private:
    JsonData *m_queryJsonData;
    CSchedulesDBus *m_dbus {nullptr};
    bool m_TimeIsExpired {false};
};

#endif // QUERYSCHEDULEPROXY_H
