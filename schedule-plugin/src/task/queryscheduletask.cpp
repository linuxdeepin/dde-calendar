// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "queryscheduletask.h"
#include "../globaldef.h"

queryScheduleTask::queryScheduleTask(CSchedulesDBus *dbus)
    : scheduleBaseTask(dbus)
{
}

Reply queryScheduleTask::SchedulePress(semanticAnalysisTask &semanticTask)
{
    QueryJsonData *queryJsonData = dynamic_cast<QueryJsonData *>(semanticTask.getJsonData());
    //如果转换失败则返回错误消息
    if (queryJsonData == nullptr)
        return errorMessage();
    //如果时间无效
    if (queryJsonData->getDateTimeInvalid()) {
        Reply m_reply;
        REPLY_ONLY_TTS(m_reply, DATETIME_ERR_TTS, DATETIME_ERR_TTS, true);
        return m_reply;
    }
    //查询日程
    if (queryJsonData->offset() > -1
            && queryJsonData->getPropertyStatus() == JsonData::PRO_NONE) {
        Reply m_reply;
        REPLY_ONLY_TTS(m_reply, CANCEL_ERR_TTS, CANCEL_ERR_TTS, true);
        return m_reply;
    }

    QString m_queryTitleName = queryJsonData->TitleName();
    QVector<int> getDayNum = queryJsonData->getRepeatNum();
    viewWidget = new viewschedulewidget();

    QVector<ScheduleDateRangeInfo> showdate;
    TIME_FRAME_IN_THE_NEXT_SIX_MONTHT
    QVector<ScheduleDateRangeInfo> schedule = getSchedule(m_queryTitleName, beginTime, endTime);

    setDateTime(queryJsonData);

    switch (queryJsonData->getRepeatStatus()) {
    case QueryJsonData::RepeatStatus::EVED: {
        showdate = getEveryDayOrWorkDaySchedule(queryJsonData, 1);
    }
    break;
    case QueryJsonData::RepeatStatus::WORKD: {
        showdate = getEveryDayOrWorkDaySchedule(queryJsonData, 2);
    }
    break;
    case QueryJsonData::RepeatStatus::RESTD: {
        QVector<int> queryRest;
        queryRest.append(6);
        queryRest.append(7);

        showdate = getRestDaySchedule(queryJsonData, queryRest);
    }
    break;
    case QueryJsonData::RepeatStatus::EVEW: {
        showdate = getEveryWeekSchedule(queryJsonData, getDayNum);
    }
    break;
    case QueryJsonData::RepeatStatus::EVEM: {
        showdate = getEveryMonthSchedule(queryJsonData, getDayNum);
    }
    break;
    case QueryJsonData::RepeatStatus::EVEY: {
        showdate = getEveryYearSchedule(queryJsonData);
    }
    break;
    default: {
        showdate = getNonePropertyStatusSchedule(queryJsonData, schedule);
    }
    break;
    }

    viewWidget->viewScheduleInfoShow(showdate);

    Reply m_reply;

    if (queryJsonData->ShouldEndSession()) {
        //不进行多轮
        if (queryOverDueDate(queryJsonData)) {
            //过期时间
            REPLY_ONLY_TTS(m_reply, VIEW_DATE_IS_OVERDUE_TTS, VIEW_DATE_IS_OVERDUE_TTS, true);
        } else if (queryJsonData->getDateTime().suggestDatetime.size() > 0
                   && queryJsonData->getDateTime().suggestDatetime.at(0).datetime > QDateTime::currentDateTime().addMonths(6)) {
            //超过半年的时间
            REPLY_ONLY_TTS(m_reply, VIEW_DATETIME_OUT_TTS, VIEW_DATETIME_OUT_TTS, true);
        } else {
            if (viewWidget->getScheduleNum(showdate) == 0) {
                //没有查询的日程
                REPLY_ONLY_TTS(m_reply, NO_SCHEDULE_VIEWED_TTS, NO_SCHEDULE_VIEWED_TTS, true);
            } else {
                //查询到日程
                QString str = QString(VIEW_SCHEDULE_TTS).arg(viewWidget->getScheduleNum(showdate));
                REPLY_WIDGET_TTS(m_reply, viewWidget, str, str, true);
            }
        }
    } else {
        //多轮的情况
        REPLY_ONLY_TTS(m_reply, queryJsonData->SuggestMsg(), queryJsonData->SuggestMsg(), false);
    }

    return m_reply;
}

void queryScheduleTask::setDateTime(QueryJsonData *queryJsonData)
{
    switch (queryJsonData->getDateTime().suggestDatetime.size()) {
    case 1: {
        m_BeginDateTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_EndDateTime = m_BeginDateTime;
        //时间处理
        if (!queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
            if (m_BeginDateTime.date() == QDate::currentDate()) {
                m_BeginDateTime.setTime(QTime::currentTime());
            } else {
                m_BeginDateTime.setTime(QTime(0, 0, 0));
            }
            m_EndDateTime.setTime(QTime(23, 59, 59));
        }
    }
    break;
    case 2: {
        m_BeginDateTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_EndDateTime = queryJsonData->getDateTime().suggestDatetime.at(1).datetime;
        //查询时间为过期时间或者是超过半年的时间，则返回一个无效的时间
        if (queryJsonData->getDateTime().suggestDatetime.at(1).datetime.date() < QDateTime::currentDateTime().date()
            || queryJsonData->getDateTime().suggestDatetime.at(0).datetime.date() > QDate::currentDate().addMonths(6)) {
            //如果查询结束时间小于当前时间，设置开始结束时间为无效时间
            m_BeginDateTime.setDate(QDate(0, 0, 0));
            m_BeginDateTime.setTime(QTime(0, 0, 0));
            m_EndDateTime.setDate(QDate(0, 0, 0));
            m_EndDateTime.setTime(QTime(0, 0, 0));
            break;
        }
        //对查询的开始时间进行处理
        if (queryJsonData->getDateTime().suggestDatetime.at(0).datetime < QDateTime::currentDateTime()) {
            //开始时间小于当前时间，设置当前时间
            m_BeginDateTime = QDateTime::currentDateTime();
        } else {
            if (!queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
                //没有time
                if (queryJsonData->getDateTime().suggestDatetime.at(0).datetime.date() == QDate::currentDate())
                    //如果是今天，设置当前时间
                    m_BeginDateTime.setTime(QTime::currentTime());
                else
                    //不是今天，设置一天最初的时间
                    m_BeginDateTime.setTime(QTime(0, 0, 0));
            }
        }
        //对查询的结束时间进行处理
        if (queryJsonData->getDateTime().suggestDatetime.at(1).datetime.date() > QDate::currentDate().addMonths(6)) {
            //如果查询的结束时间超过了半年，则设置为半年以后的时间
            m_EndDateTime.setDate(QDate::currentDate().addMonths(6));
            m_EndDateTime.setTime(QTime(23, 59, 59));
        } else {
            //如果查询的结束时间没有超过半年，并且没有具体时间，则设置为一天最晚的时间
            if (!queryJsonData->getDateTime().suggestDatetime.at(1).hasTime) {
                m_EndDateTime.setTime(QTime(23, 59, 59));
            }
        }
    }
    break;
    default: {
        //如果没有时间，设置开始结束时间为无效时间
        m_BeginDateTime.setDate(QDate(0, 0, 0));
        m_BeginDateTime.setTime(QTime(0, 0, 0));
        m_EndDateTime.setDate(QDate(0, 0, 0));
        m_EndDateTime.setTime(QTime(0, 0, 0));
    }
    break;
    }
}

QVector<ScheduleDateRangeInfo> queryScheduleTask::getSchedule(QString titleName, QDateTime beginDatetime, QDateTime endDateTime)
{
    QVector<ScheduleDateRangeInfo> schedule;
    //使用dbus查询日程
    m_dbus->QueryJobs(titleName, beginDatetime, endDateTime, schedule);
    //设置查询的日程
    viewWidget->setScheduleDateRangeInfo(schedule);
    //返回过滤后的日程
    return viewWidget->getAllScheduleInfo();
}

QVector<ScheduleDateRangeInfo> queryScheduleTask::getEveryDayOrWorkDaySchedule(QueryJsonData *queryJsonData, int repeat)
{
    QTime m_BTime;
    QTime m_ETime;
    QVector<ScheduleDateRangeInfo> schedule;
    schedule = viewWidget->getAllRpeatScheduleInfo(repeat);
    if (queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
        m_BTime = m_BeginDateTime.time();
        m_ETime = m_EndDateTime.time();
        schedule = viewWidget->queryScheduleWithTime(schedule, m_BTime, m_ETime);
    }
    return schedule;
}

QVector<ScheduleDateRangeInfo> queryScheduleTask::getRestDaySchedule(QueryJsonData *queryJsonData, QVector<int> queryWeek)
{
    QVector<ScheduleDateRangeInfo> schedule;
    QTime m_BTime;
    QTime m_ETime;
    //查询所有周重复的日程
    schedule = viewWidget->getAllRpeatScheduleInfo(3);
    //查询周几-周几的重复日程
    schedule = viewWidget->queryScheduleWithWeek(schedule, queryWeek);
    //如果有时间，则再按照时间进行过滤
    if (queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
        m_BTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.time();
        m_ETime = m_BTime;
        schedule = viewWidget->queryScheduleWithTime(schedule, m_BTime, m_ETime);
    }
    return  schedule;
}

QVector<ScheduleDateRangeInfo> queryScheduleTask::getEveryWeekSchedule(QueryJsonData *queryJsonData, QVector<int> repeatNum)
{
    QTime m_BTime;
    QTime m_ETime;
    QVector<ScheduleDateRangeInfo> schedule;
    //查询所有周重复的日程
    schedule = viewWidget->getAllRpeatScheduleInfo(3);

    if (repeatNum.size() == 1) {
        QVector<int> queryWeek;
        queryWeek.append(repeatNum[0]);

        schedule = viewWidget->queryScheduleWithWeek(schedule, queryWeek);
        if (queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
            m_BTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.time();
            m_ETime = m_BTime;
            schedule = viewWidget->queryScheduleWithTime(schedule, m_BTime, m_ETime);
        }
    } else if (repeatNum.size() == 2) {
        int start = repeatNum[0];
        int end = repeatNum[1];
        QVector<int> queryWeek;

        if (start == end) {
            return viewWidget->getAllRpeatScheduleInfo(1);
        } else if (start < end) {
            if (start == 1 && end == 5) {
                return viewWidget->getAllRpeatScheduleInfo(2);
            } else if (end - start == 6) {
                return viewWidget->getAllRpeatScheduleInfo(1);
            } else {
                for (int i = start; i <= end; i++) {
                    queryWeek.append(i);
                }
            }
        } else {
            if (start - end == 1) {
                return viewWidget->getAllRpeatScheduleInfo(1);
            } else {
                for (int i = 0; i <= 7 - start; i++) {
                    queryWeek.append(i + start);
                }
                for (int i = 0; i < end; i++) {
                    queryWeek.append(i + 1);
                }
            }
        }
        //查询周几-周几的重复日程
        schedule = viewWidget->queryScheduleWithWeek(schedule, queryWeek);
        if (queryJsonData->getDateTime().suggestDatetime.size() == 1) {
            if (queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
                m_BTime = QTime(0, 0, 0);
                m_ETime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.time();
                schedule = viewWidget->queryScheduleWithWeek(schedule, queryWeek, end, m_BTime, m_ETime);
            }
        }
    }
    return schedule;
}

QVector<ScheduleDateRangeInfo> queryScheduleTask::getEveryMonthSchedule(QueryJsonData *queryJsonData, QVector<int> repeatNum)
{
    QTime m_BTime;
    QTime m_ETime;
    QVector<ScheduleDateRangeInfo> schedule;
    //查询所有月重复的日程
    schedule = viewWidget->getAllRpeatScheduleInfo(4);

    if (repeatNum.size() == 1) {
        QVector<int> queryMonth;
        queryMonth.append(repeatNum[0]);

        schedule = viewWidget->queryScheduleWithMonth(schedule, queryMonth);
        if (queryJsonData->getDateTime().suggestDatetime.size() == 1) {
            if (queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
                m_BTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.time();
                m_ETime = m_BTime;
                schedule = viewWidget->queryScheduleWithTime(schedule, m_BTime, m_ETime);
            }
        } else if (queryJsonData->getDateTime().suggestDatetime.size() == 2) {
            m_BTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.time();
            m_ETime = queryJsonData->getDateTime().suggestDatetime.at(1).datetime.time();
            schedule = viewWidget->queryScheduleWithTime(schedule, m_BTime, m_ETime);
        }
    } else if (repeatNum.size() == 2) {
        int start = repeatNum[0];
        int end = repeatNum[1];
        QVector<int> queryMonth;

        if (start == end) {
            return viewWidget->getAllRpeatScheduleInfo(1);
        } else if (start < end) {
            for (int i = start; i <= end; i++) {
                queryMonth.append(i);
            }
        }

        schedule = viewWidget->queryScheduleWithMonth(schedule, queryMonth);
    } else {
        if (queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
            m_BTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.time();
            m_ETime = m_BTime;
            schedule = viewWidget->queryScheduleWithTime(schedule, m_BTime, m_ETime);
        }
    }
    return schedule;
}

QVector<ScheduleDateRangeInfo> queryScheduleTask::getEveryYearSchedule(QueryJsonData *queryJsonData)
{
    Q_UNUSED(queryJsonData)
    QVector<ScheduleDateRangeInfo> schedule;

    if (queryJsonData->getDateTime().suggestDatetime.size() == 1) {
        //查询所有年重复的日程
        schedule = viewWidget->getAllRpeatScheduleInfo(5);
        //设置查询的开始结束日期
        QDate m_beginD = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.date();
        QDate m_endD = m_beginD;
        //按照日期查询日程
        schedule = viewWidget->queryScheduleWithDate(schedule, m_beginD, m_endD);
        //如果有时间，则按照时间查询
        if (queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
            QTime m_beginT = queryJsonData->getDateTime().suggestDatetime.at(0).datetime.time();
            QTime m_endT = m_beginT;
            schedule = viewWidget->queryScheduleWithTime(schedule, m_beginT, m_endT);
        }
    }

    return schedule;
}

QVector<ScheduleDateRangeInfo> queryScheduleTask::getNonePropertyStatusSchedule(QueryJsonData *queryJsonData, QVector<ScheduleDateRangeInfo> schedule)
{
    QVector<ScheduleDateRangeInfo> scheduleInfo;
    scheduleInfo.clear();

    switch (queryJsonData->getPropertyStatus()) {
    case QueryJsonData::PropertyStatus::ALL: {
        //查询所有的日程，因此不需要对已查询到的日程进行过滤操作
        return schedule;
    }
    case QueryJsonData::PropertyStatus::NEXT: {
        if (schedule.isEmpty()) {
            //如果半年内没有日程，返回空容器
            return scheduleInfo;
        } else {
            schedule.clear();
            scheduleInfo = viewWidget->getNextScheduleInfo();
        }
        //返回下一个日程信息
        return scheduleInfo;
    }
    case QueryJsonData::PropertyStatus::LAST:
        break;
    default: {
        if (m_BeginDateTime.isValid()) {
            qDebug() << m_BeginDateTime << m_EndDateTime;
            //返回过滤的日程
            return getSchedule(queryJsonData->TitleName(), m_BeginDateTime, m_EndDateTime);
        }
    }
    }
    //如果以上情况都没有返回半年所有日程
    return schedule;
}

bool queryScheduleTask::queryOverDueDate(QueryJsonData *queryJsonData)
{
    bool overduedate = false;
    int datenum;
    if (queryJsonData->getDateTime().suggestDatetime.size() > 0) {
        if (queryJsonData->getDateTime().suggestDatetime.size() == 1)
            datenum = 0;
        else
            datenum = 1;

        if (queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.date() < QDate::currentDate()) {
            overduedate = true;
        } else if (queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.date() == QDate::currentDate()
                   && queryJsonData->getDateTime().suggestDatetime.at(datenum).hasTime
                   && queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.time() < QTime::currentTime()) {
            overduedate = true;
        } else {
            overduedate = false;
        }
    } else {
        overduedate = false;
    }
    return overduedate;
}
