// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "queryscheduleproxy.h"

#include "../globaldef.h"
#include "../data/changejsondata.h"

bool scheduleSort(const ScheduleDtailInfo &s1, const ScheduleDtailInfo &s2)
{
    if (s1.beginDateTime < s2.beginDateTime) {
        return true;
    } else if (s1.beginDateTime == s2.beginDateTime) {
        return s1.endDateTime < s2.endDateTime;
    } else {
        return false;
    }
}

queryScheduleProxy::queryScheduleProxy(JsonData *jsonData, CSchedulesDBus *dbus)
    : m_queryJsonData(jsonData)
    , m_dbus(dbus)
{
}

QVector<ScheduleDtailInfo> queryScheduleProxy::querySchedule()
{
    QVector<ScheduleDtailInfo> scheduleInfo {};
    scheduleInfo.clear();
    switch (m_queryJsonData->getRepeatStatus()) {
    case JsonData::RepeatStatus::NONE:
        scheduleInfo = queryNonRepeatingSchedule();
        break;
    case JsonData::RepeatStatus::EVED: {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        scheduleInfo = queryEveryDaySchedule(beginTime, endTime);
        if (!m_queryJsonData->TitleName().isEmpty()) {
            scheduleInfo = scheduleFileterByTitleName(scheduleInfo, m_queryJsonData->TitleName());
        }
    } break;
    case JsonData::RepeatStatus::EVEW: {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        int beginW = 0;
        int endW = 0;
        if (m_queryJsonData->getRepeatNum().size() == 0) {
        } else if (m_queryJsonData->getRepeatNum().size() == 1) {
            beginW = m_queryJsonData->getRepeatNum().at(0);
            endW = m_queryJsonData->getRepeatNum().at(0);
        } else {
            beginW = m_queryJsonData->getRepeatNum().at(0);
            endW = m_queryJsonData->getRepeatNum().at(1);
        }
        scheduleInfo = queryWeeklySchedule(beginTime, endTime, beginW, endW);
        SemanticsDateTime queryDatetime = getQueryDateTime(m_queryJsonData);
        TimeLimit fileterTime = getTimeFileterByTimeInfo(queryDatetime);
        if (!fileterTime.isInvalid) {
            scheduleInfo = scheduleFileterByTime(scheduleInfo, fileterTime.beginTime, fileterTime.endTime);
        }
        if (!m_queryJsonData->TitleName().isEmpty()) {
            scheduleInfo = scheduleFileterByTitleName(scheduleInfo, m_queryJsonData->TitleName());
        }
    } break;
    case JsonData::RepeatStatus::EVEM: {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        int beginM = 0;
        int endM = 0;
        if (m_queryJsonData->getRepeatNum().size() == 0) {
        } else if (m_queryJsonData->getRepeatNum().size() == 1) {
            beginM = m_queryJsonData->getRepeatNum().at(0);
            endM = m_queryJsonData->getRepeatNum().at(0);
        } else {
            beginM = m_queryJsonData->getRepeatNum().at(0);
            endM = m_queryJsonData->getRepeatNum().at(1);
        }
        scheduleInfo = queryMonthlySchedule(beginTime, endTime, beginM, endM);
        if (!m_queryJsonData->TitleName().isEmpty()) {
            scheduleInfo = scheduleFileterByTitleName(scheduleInfo, m_queryJsonData->TitleName());
        }
    } break;
    case JsonData::RepeatStatus::EVEY: {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        scheduleInfo = queryEveryYearSchedule(beginTime, endTime);
        SemanticsDateTime queryDatetime = getQueryDateTime(m_queryJsonData);
        //查询每年的日程中包含日期
        if (queryDatetime.suggestDatetime.size() == 1) {
            //查询的日期
            QDate beginD = queryDatetime.suggestDatetime.at(0).datetime.date();
            QDate endD = beginD;
            //过滤包含查询日期的日程
            scheduleInfo = scheduleFileterByDate(scheduleInfo, beginD, endD);
        }
        //获取查询的具体时间
        TimeLimit fileterTime = getTimeFileterByTimeInfo(queryDatetime);
        if (!fileterTime.isInvalid) {
            //过滤包含查询时间的日程
            scheduleInfo = scheduleFileterByTime(scheduleInfo, fileterTime.beginTime, fileterTime.endTime);
        }
        if (!m_queryJsonData->TitleName().isEmpty()) {
            scheduleInfo = scheduleFileterByTitleName(scheduleInfo, m_queryJsonData->TitleName());
        }
    } break;
    case JsonData::RepeatStatus::RESTD: {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        scheduleInfo = queryWeeklySchedule(beginTime, endTime, 6, 7);
        if (!m_queryJsonData->TitleName().isEmpty()) {
            scheduleInfo = scheduleFileterByTitleName(scheduleInfo, m_queryJsonData->TitleName());
        }
    } break;
    case JsonData::RepeatStatus::WORKD: {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        scheduleInfo = queryWorkingDaySchedule(beginTime, endTime);
        if (!m_queryJsonData->TitleName().isEmpty()) {
            scheduleInfo = scheduleFileterByTitleName(scheduleInfo, m_queryJsonData->TitleName());
        }
    } break;
    }
    return scheduleInfo;
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryWeeklySchedule(QDateTime &beginTime, QDateTime &endTime, int beginW, int endW)
{
    QSet<int> weeklySet;
    if (beginW == endW) {
        weeklySet.insert(beginW);
    } else if (beginW < endW) {
        if (beginW == 1) {
            if (endW == 5) {
                //WORK
                return queryWorkingDaySchedule(beginTime, endTime);
            } else if (endW == 7) {
                //EVED
                return queryEveryDaySchedule(beginTime, endTime);
            }
        }
        for (int i = beginW; i <= endW; ++i) {
            weeklySet.insert(i);
        }
    } else {
        if (beginW - endW == 1) {
            //EVED
            return queryEveryDaySchedule(beginTime, endTime);
        }
        int w;
        for (int i = beginW; i <= endW + 7; ++i) {
            w = i % 7;
            w = w ? w : 7;
            weeklySet.insert(w);
        }
    }
    QVector<ScheduleDateRangeInfo> out {};
    m_dbus->QueryJobsWithRule(beginTime, endTime, DBUS_RRUL_EVEW, out);

    if (beginW == 0 || endW == 0) {
        weeklySet.clear();
    }

    return WeeklyScheduleFileter(out, weeklySet);
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryMonthlySchedule(QDateTime &beginTime, QDateTime &endTime, int beginM, int endM)
{
    QVector<ScheduleDateRangeInfo> out {};
    m_dbus->QueryJobsWithRule(beginTime, endTime, DBUS_RRUL_EVEM, out);
    return MonthlyScheduleFileter(out, beginM, endM);
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryEveryDaySchedule(QDateTime &beginTime, QDateTime &endTime)
{
    QVector<ScheduleDateRangeInfo> out {};
    m_dbus->QueryJobsWithRule(beginTime, endTime, DBUS_RRUL_EVED, out);
    return sortAndFilterSchedule(out);
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryEveryYearSchedule(QDateTime &beginTime, QDateTime &endTime)
{
    QVector<ScheduleDateRangeInfo> out {};
    m_dbus->QueryJobsWithRule(beginTime, endTime, DBUS_RRUL_EVEY, out);
    return sortAndFilterSchedule(out);
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryWorkingDaySchedule(QDateTime &beginTime, QDateTime &endTime)
{
    QVector<ScheduleDateRangeInfo> out {};
    m_dbus->QueryJobsWithRule(beginTime, endTime, DBUS_RRUL_WORK, out);
    return sortAndFilterSchedule(out);
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryNonRepeatingSchedule()
{
    QVector<ScheduleDtailInfo> mScheduleInfoVector {};
    mScheduleInfoVector.clear();
    SemanticsDateTime queryDatetime = getQueryDateTime(m_queryJsonData);
    //如果开始时间大于结束时间则退出
    if (!timeFrameIsValid(queryDatetime)) {
        return mScheduleInfoVector;
    }
    switch (m_queryJsonData->getPropertyStatus()) {
    case JsonData::PropertyStatus::ALL: {
        DateTimeLimit timeLimit = getTimeLimitByTimeInfo(queryDatetime);
        if (!getTimeIsExpired()) {
            mScheduleInfoVector = queryAllSchedule(m_queryJsonData->TitleName(), timeLimit.beginTime, timeLimit.endTime);
        }
    } break;
    case JsonData::PropertyStatus::NEXT: {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        mScheduleInfoVector = queryNextNumSchedule(beginTime, endTime, 1);
    } break;
    case JsonData::PropertyStatus::LAST: {
    } break;
    default: {
        //NONE
        DateTimeLimit timeLimit = getTimeLimitByTimeInfo(queryDatetime);
        if (!getTimeIsExpired()) {
            mScheduleInfoVector = queryAllSchedule(m_queryJsonData->TitleName(), timeLimit.beginTime, timeLimit.endTime);
        }
    } break;
    }
    return mScheduleInfoVector;
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryNextNumSchedule(QDateTime &beginTime, QDateTime &endTime, int NextNum)
{
    QVector<ScheduleDtailInfo> mScheduleInfoVector {};
    QVector<ScheduleDateRangeInfo> out {};
    m_dbus->QueryJobsWithLimit(beginTime, endTime, NextNum, out);
    return sortAndFilterSchedule(out);
}

QVector<ScheduleDtailInfo> queryScheduleProxy::queryAllSchedule(QString key, QDateTime &beginTime, QDateTime &endTime)
{
    QVector<ScheduleDateRangeInfo> out {};
    m_dbus->QueryJobs(key, beginTime, endTime, out);
    return sortAndFilterSchedule(out);
}

QVector<ScheduleDtailInfo> queryScheduleProxy::sortAndFilterSchedule(QVector<ScheduleDateRangeInfo> &out)
{
    QVector<ScheduleDtailInfo> scheduleInfo {};
    for (int i = 0; i < out.size(); ++i) {
        for (int j = 0; j < out[i].vData.size(); ++j) {
            if (!(scheduleInfo.contains(out[i].vData[j]) || out[i].vData[j].type.ID == 4)) {
                scheduleInfo.append(out[i].vData[j]);
            }
        }
    }
    std::sort(scheduleInfo.begin(), scheduleInfo.end(), scheduleSort);
    return scheduleInfo;
}

QVector<ScheduleDtailInfo> queryScheduleProxy::WeeklyScheduleFileter(QVector<ScheduleDateRangeInfo> &out, QSet<int> &weeklySet)
{
    QVector<ScheduleDtailInfo> scheduleInfo {};
    if (weeklySet.size() == 0) {
        return sortAndFilterSchedule(out);
    } else {
        for (int i = 0; i < out.size(); ++i) {
            for (int j = 0; j < out[i].vData.size(); ++j) {
                if (!(scheduleInfo.contains(out[i].vData[j])) && weeklyIsIntersections(out[i].vData[j].beginDateTime, out[i].vData[j].endDateTime, weeklySet)) {
                    scheduleInfo.append(out[i].vData[j]);
                }
            }
        }
    }
    return scheduleInfo;
}

QVector<ScheduleDtailInfo> queryScheduleProxy::MonthlyScheduleFileter(QVector<ScheduleDateRangeInfo> &out, int beginM, int endM)
{
    if (beginM == 0 || endM == 0) {
        return sortAndFilterSchedule(out);
    }
    QVector<ScheduleDtailInfo> scheduleInfo {};
    for (int i = 0; i < out.size(); ++i) {
        for (int j = 0; j < out[i].vData.size(); ++j) {
            if (!(scheduleInfo.contains(out[i].vData[j])) && monthlyIsIntersections(out[i].vData[j].beginDateTime, out[i].vData[j].endDateTime, beginM, endM)) {
                scheduleInfo.append(out[i].vData[j]);
            }
        }
    }
    return scheduleInfo;
}

bool queryScheduleProxy::monthlyIsIntersections(QDateTime &beginTime, QDateTime &endTime, int beginM, int endM)
{
    bool b_monthly = false;
    int beginDay = beginTime.date().day();
    int endDay = endTime.date().day();
    if (beginM > endM) {
        b_monthly = beginDay > endM && endDay < beginM ? false : true;
    } else {
        b_monthly = (beginM <= beginDay && endM >= beginDay) || (beginM <= endDay && endM >= endDay)
                    || (beginDay <= beginM && endDay >= endM);
    }
    return b_monthly;
}

bool queryScheduleProxy::checkedTimeIsIntersection(QTime &beginTime, QTime &endTime, QTime &fixbeginTime, QTime &fixendTime)
{
    bool b_checked = false;
    if (beginTime.secsTo(endTime) < 0) {
        if (fixbeginTime.secsTo(fixendTime) < 0) {
            b_checked = true;
        } else {
            b_checked = endTime.secsTo(fixbeginTime) > 0 && fixendTime.secsTo(beginTime) > 0 ? false : true;
        }
    } else {
        if (fixbeginTime.secsTo(fixendTime) < 0) {
            b_checked = fixendTime.secsTo(beginTime) > 0 && endTime.secsTo(fixbeginTime) > 0 ? false : true;
        } else {
            b_checked = (beginTime.secsTo(fixbeginTime) >= 0 && fixbeginTime.secsTo(endTime) >= 0)
                        || (beginTime.secsTo(fixendTime) >= 0 && fixendTime.secsTo(endTime) >= 0);
        }
    }
    return b_checked;
}

QVector<ScheduleDtailInfo> queryScheduleProxy::scheduleFileterByTime(QVector<ScheduleDtailInfo> &scheduleInfo, QTime &fileterBeginTime, QTime &fileterEndTime)
{
    QVector<ScheduleDtailInfo> mScheduleFileter {};
    qint64 timeoffset_Secs = 0;
    bool isApppend = false;
    for (int i = 0; i < scheduleInfo.size(); ++i) {
        timeoffset_Secs = scheduleInfo.at(i).beginDateTime.secsTo(scheduleInfo.at(i).endDateTime);
        if (timeoffset_Secs < ONE_DAY_SECS) {
            QTime begTime = scheduleInfo.at(i).beginDateTime.time();
            QTime endTime = scheduleInfo.at(i).endDateTime.time();
            isApppend = checkedTimeIsIntersection(begTime, endTime, fileterBeginTime, fileterEndTime);
        } else {
            isApppend = true;
        }
        if (isApppend) {
            mScheduleFileter.append(scheduleInfo.at(i));
        }
    }
    return mScheduleFileter;
}

/**
 * @brief queryScheduleProxy::scheduleFileterByDate 过滤包含查询日期的日程
 * @param scheduleInfo 日程信息
 * @param fileterBeginDate 查询的开始时间
 * @param fileterEndDate 查询的结束时间
 * @return 过滤后的日程信息
 */
QVector<ScheduleDtailInfo> queryScheduleProxy::scheduleFileterByDate(QVector<ScheduleDtailInfo> &scheduleInfo, QDate &fileterBeginDate, QDate &fileterEndDate)
{
    QVector<ScheduleDtailInfo> mScheduleFileter {};
    //遍历查询到的日程
    for (int i = 0; i < scheduleInfo.size(); i++) {
        //查询到的日程的开始结束日期
        QDate beginD = scheduleInfo.at(i).beginDateTime.date();
        QDate endD = scheduleInfo.at(i).endDateTime.date();
        //过滤添加包含查询日期的日程
        if ((fileterBeginDate <= beginD && fileterEndDate >= beginD)
                || (fileterBeginDate >= beginD && fileterBeginDate <= endD)) {
            mScheduleFileter.append(scheduleInfo.at(i));
        }
    }
    return mScheduleFileter;
}

QVector<ScheduleDtailInfo> queryScheduleProxy::scheduleFileterByTitleName(QVector<ScheduleDtailInfo> &scheduleInfo, const QString &strName)
{
    QVector<ScheduleDtailInfo> mScheduleFileter {};
    for (int i = 0; i < scheduleInfo.size(); ++i) {
        if (scheduleInfo.at(i).titleName.contains(strName)) {
            mScheduleFileter.append(scheduleInfo.at(i));
        }
    }
    return mScheduleFileter;
}

bool queryScheduleProxy::weeklyIsIntersections(QDateTime &beginTime, QDateTime &endTime, QSet<int> &weeklySet)
{
    QSet<int> scheduleWeekSet;
    bool returnValue = false;
    scheduleWeekSet.clear();
    qint64 dayoffset = beginTime.daysTo(endTime);
    if (dayoffset > 5) {
        returnValue = true;
    } else {
        int beginW = beginTime.date().dayOfWeek();
        int w;
        for (int i = beginW; i <= beginW + dayoffset; ++i) {
            w = i % 7;
            w = w ? w : 7;
            scheduleWeekSet.insert(w);
        }
        returnValue = scheduleWeekSet.intersects(weeklySet);
    }
    return returnValue;
}

SemanticsDateTime queryScheduleProxy::getQueryDateTime(JsonData *jsonData)
{
    changejsondata *changedata = dynamic_cast<changejsondata *>(jsonData);
    if (changedata != nullptr && changedata->fromDateTime().suggestDatetime.size() > 0) {
        return changedata->fromDateTime();
    }
    return jsonData->getDateTime();
}

queryScheduleProxy::DateTimeLimit queryScheduleProxy::getTimeLimitByTimeInfo(const SemanticsDateTime &timeInfoVect)
{
    DateTimeLimit timeLimit;
    setTimeIsExpired(false);
    if (timeInfoVect.suggestDatetime.size() == 0) {
        TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
        timeLimit.beginTime = beginTime;
        timeLimit.endTime = endTime;
    } else if (timeInfoVect.suggestDatetime.size() == 1) {
        if (timeInfoVect.suggestDatetime.at(0).datetime.date() < QDateTime::currentDateTime().date()
                || timeInfoVect.suggestDatetime.at(0).datetime.date() > QDateTime::currentDateTime().addDays(MAXIMUM_DAYS_IN_THE_FUTURE).date()) {
            setTimeIsExpired(true);
            return timeLimit;
        }
        if (timeInfoVect.suggestDatetime.at(0).hasTime) {
            if (timeInfoVect.suggestDatetime.at(0).datetime < QDateTime::currentDateTime()) {
                setTimeIsExpired(true);
            } else {
                timeLimit.beginTime = timeInfoVect.suggestDatetime.at(0).datetime;
                timeLimit.endTime = timeInfoVect.suggestDatetime.at(0).datetime;
            }
        } else {
            if (timeInfoVect.suggestDatetime.at(0).datetime.date() == QDateTime::currentDateTime().date()) {
                timeLimit.beginTime = QDateTime::currentDateTime();
                timeLimit.endTime.setDate(timeLimit.beginTime.date());
                timeLimit.endTime.setTime(QTime(23, 59, 59));
            } else {
                timeLimit.beginTime = timeInfoVect.suggestDatetime.at(0).datetime;
                timeLimit.endTime.setDate(timeLimit.beginTime.date());
                timeLimit.endTime.setTime(QTime(23, 59, 59));
            }
        }
    } else {
        QDateTime maxDay = QDateTime::currentDateTime().addDays(MAXIMUM_DAYS_IN_THE_FUTURE);
        if (timeInfoVect.suggestDatetime.at(1).datetime.date() < QDateTime::currentDateTime().date()
                || timeInfoVect.suggestDatetime.at(0).datetime.date() > maxDay.date()) {
            setTimeIsExpired(true);
            return timeLimit;
        }
        if (timeInfoVect.suggestDatetime.at(0).datetime < QDateTime::currentDateTime()) {
            timeLimit.beginTime = QDateTime::currentDateTime();
        } else {
            timeLimit.beginTime = timeInfoVect.suggestDatetime.at(0).datetime;
        }
        if (timeInfoVect.suggestDatetime.at(1).hasTime) {
            timeLimit.endTime = timeInfoVect.suggestDatetime.at(1).datetime;
        } else {
            timeLimit.endTime.setDate(timeInfoVect.suggestDatetime.at(1).datetime.date());
            timeLimit.endTime.setTime(QTime(23, 59, 59));
        }
        if (timeLimit.endTime.date() > maxDay.date()) {
            timeLimit.endTime.setDate(maxDay.date());
            timeLimit.endTime.setTime(QTime(23, 59, 59));
        }
    }
    return timeLimit;
}

queryScheduleProxy::TimeLimit queryScheduleProxy::getTimeFileterByTimeInfo(const SemanticsDateTime &timeInfoVect)
{
    TimeLimit fileterTime;
    fileterTime.isInvalid = true;
    if (timeInfoVect.suggestDatetime.size() == 0) {
    } else if (timeInfoVect.suggestDatetime.size() == 1) {
        if (timeInfoVect.suggestDatetime.at(0).hasTime) {
            fileterTime.beginTime = timeInfoVect.suggestDatetime.at(0).datetime.time();
            fileterTime.endTime = fileterTime.beginTime;
            fileterTime.isInvalid = false;
        }
    } else {
        fileterTime.beginTime = timeInfoVect.suggestDatetime.at(0).datetime.time();
        fileterTime.endTime = timeInfoVect.suggestDatetime.at(1).datetime.time();
        fileterTime.isInvalid = false;
    }
    return fileterTime;
}

bool queryScheduleProxy::getTimeIsExpired() const
{
    return m_TimeIsExpired;
}

void queryScheduleProxy::setTimeIsExpired(const bool timeisExp)
{
    m_TimeIsExpired = timeisExp;
}

bool queryScheduleProxy::timeFrameIsValid(const SemanticsDateTime &timeInfoVect)
{
    //如果开始时间大于结束时间则返回false
    if (timeInfoVect.suggestDatetime.size() > 1 && timeInfoVect.suggestDatetime.at(0).datetime > timeInfoVect.suggestDatetime.at(1).datetime) {
        return false;
    }
    return true;
}
