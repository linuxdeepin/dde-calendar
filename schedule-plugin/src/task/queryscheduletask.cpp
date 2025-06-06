// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "queryscheduletask.h"
#include "../globaldef.h"
#include "dscheduledatamanager.h"
#include "queryscheduleproxy.h"
#include "commondef.h"

queryScheduleTask::queryScheduleTask()
    : scheduleBaseTask()
{
}

Reply queryScheduleTask::SchedulePress(semanticAnalysisTask &semanticTask)
{
    QueryJsonData *queryJsonData = dynamic_cast<QueryJsonData *>(semanticTask.getJsonData());
    //如果转换失败则返回错误消息
    if (queryJsonData == nullptr) {
        qCWarning(CommonLogger) << "Failed to cast json data to QueryJsonData";
        return errorMessage();
    }
    //如果时间无效
    if (queryJsonData->getDateTimeInvalid()) {
        qCWarning(CommonLogger) << "Invalid date time in query request";
        Reply m_reply;
        REPLY_ONLY_TTS(m_reply, DATETIME_ERR_TTS, DATETIME_ERR_TTS, true);
        return m_reply;
    }
    //查询日程
    if (queryJsonData->offset() > -1
            && queryJsonData->getPropertyStatus() == JsonData::PRO_NONE) {
        qCWarning(CommonLogger) << "Invalid offset or property status - Offset:" << queryJsonData->offset() 
                               << "Status:" << queryJsonData->getPropertyStatus();
        Reply m_reply;
        REPLY_ONLY_TTS(m_reply, CANCEL_ERR_TTS, CANCEL_ERR_TTS, true);
        return m_reply;
    }
    viewWidget = new viewschedulewidget();

    queryScheduleProxy m_querySchedule(queryJsonData);
    DSchedule::Map showdate = m_querySchedule.querySchedule();

    setDateTime(queryJsonData);

    viewWidget->viewScheduleInfoShow(showdate);

    Reply m_reply;

    if (queryJsonData->ShouldEndSession()) {
        qCDebug(CommonLogger) << "Processing end session response";
        //不进行多轮
        if (queryOverDueDate(queryJsonData)) {
            //过期时间
            qCInfo(CommonLogger) << "Query date is overdue";
            REPLY_ONLY_TTS(m_reply, VIEW_DATE_IS_OVERDUE_TTS, VIEW_DATE_IS_OVERDUE_TTS, true);
        } else if (queryJsonData->getDateTime().suggestDatetime.size() > 0
                   && queryJsonData->getDateTime().suggestDatetime.at(0).datetime > QDateTime::currentDateTime().addMonths(6)) {
            //超过半年的时间
            qCInfo(CommonLogger) << "Query date is beyond half year";
            REPLY_ONLY_TTS(m_reply, VIEW_DATETIME_OUT_TTS, VIEW_DATETIME_OUT_TTS, true);
        } else {
            int scheduleCount = viewWidget->getScheduleNum(showdate);
            if (scheduleCount == 0) {
                //没有查询的日程
                qCInfo(CommonLogger) << "No schedules found for query";
                REPLY_ONLY_TTS(m_reply, NO_SCHEDULE_VIEWED_TTS, NO_SCHEDULE_VIEWED_TTS, true);
            } else {
                //查询到日程
                qCInfo(CommonLogger) << "Found" << scheduleCount << "schedules";
                QString str = QString(VIEW_SCHEDULE_TTS).arg(scheduleCount);
                REPLY_WIDGET_TTS(m_reply, viewWidget, str, str, true);
            }
        }
    } else {
        //多轮的情况
        qCDebug(CommonLogger) << "Starting multi-round query";
        REPLY_ONLY_TTS(m_reply, queryJsonData->SuggestMsg(), queryJsonData->SuggestMsg(), false);
    }

    return m_reply;
}

void queryScheduleTask::setDateTime(QueryJsonData *queryJsonData)
{
    switch (queryJsonData->getDateTime().suggestDatetime.size()) {
    case 1: {
        qCDebug(CommonLogger) << "Processing single date time suggestion";
        m_BeginDateTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_EndDateTime = m_BeginDateTime;
        //时间处理
        if (!queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
            if (m_BeginDateTime.date() == QDate::currentDate()) {
                qCDebug(CommonLogger) << "Setting begin time to current time for today";
                m_BeginDateTime.setTime(QTime::currentTime());
            } else {
                qCDebug(CommonLogger) << "Setting begin time to start of day";
                m_BeginDateTime.setTime(QTime(0, 0, 0));
            }
            m_EndDateTime.setTime(QTime(23, 59, 59));
        }
    }
    break;
    case 2: {
        qCDebug(CommonLogger) << "Processing date time range suggestion";
        m_BeginDateTime = queryJsonData->getDateTime().suggestDatetime.at(0).datetime;
        m_EndDateTime = queryJsonData->getDateTime().suggestDatetime.at(1).datetime;
        //查询时间为过期时间或者是超过半年的时间，则返回一个无效的时间
        if (queryJsonData->getDateTime().suggestDatetime.at(1).datetime.date() < QDateTime::currentDateTime().date()
            || queryJsonData->getDateTime().suggestDatetime.at(0).datetime.date() > QDate::currentDate().addMonths(6)) {
            //如果查询结束时间小于当前时间，设置开始结束时间为无效时间
            qCWarning(CommonLogger) << "Date range is invalid - setting to invalid time";
            m_BeginDateTime.setDate(QDate(0, 0, 0));
            m_BeginDateTime.setTime(QTime(0, 0, 0));
            m_EndDateTime.setDate(QDate(0, 0, 0));
            m_EndDateTime.setTime(QTime(0, 0, 0));
            break;
        }
        //对查询的开始时间进行处理
        if (queryJsonData->getDateTime().suggestDatetime.at(0).datetime < QDateTime::currentDateTime()) {
            //开始时间小于当前时间，设置当前时间
            qCDebug(CommonLogger) << "Begin time is in past, adjusting to current time";
            m_BeginDateTime = QDateTime::currentDateTime();
        } else {
            if (!queryJsonData->getDateTime().suggestDatetime.at(0).hasTime) {
                //没有time
                if (queryJsonData->getDateTime().suggestDatetime.at(0).datetime.date() == QDate::currentDate()) {
                    //如果是今天，设置当前时间
                    qCDebug(CommonLogger) << "Begin time is today, setting to current time";
                    m_BeginDateTime.setTime(QTime::currentTime());
                } else {
                    //不是今天，设置一天最初的时间
                    qCDebug(CommonLogger) << "Begin time is not today, setting to start of day";
                    m_BeginDateTime.setTime(QTime(0, 0, 0));
                }
            }
        }
        //对查询的结束时间进行处理
        if (queryJsonData->getDateTime().suggestDatetime.at(1).datetime.date() > QDate::currentDate().addMonths(6)) {
            //如果查询的结束时间超过了半年，则设置为半年以后的时间
            qCDebug(CommonLogger) << "End time is beyond half year, adjusting to 6 months from now";
            m_EndDateTime.setDate(QDate::currentDate().addMonths(6));
            m_EndDateTime.setTime(QTime(23, 59, 59));
        } else {
            //如果查询的结束时间没有超过半年，并且没有具体时间，则设置为一天最晚的时间
            if (!queryJsonData->getDateTime().suggestDatetime.at(1).hasTime) {
                qCDebug(CommonLogger) << "End time has no specific time, setting to end of day";
                m_EndDateTime.setTime(QTime(23, 59, 59));
            }
        }
    }
    break;
    default: {
        //如果没有时间，设置开始结束时间为无效时间
        qCWarning(CommonLogger) << "No valid date time suggestions, setting to invalid time";
        m_BeginDateTime.setDate(QDate(0, 0, 0));
        m_BeginDateTime.setTime(QTime(0, 0, 0));
        m_EndDateTime.setDate(QDate(0, 0, 0));
        m_EndDateTime.setTime(QTime(0, 0, 0));
    }
    break;
    }
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
            qCDebug(CommonLogger) << "Query date is in the past";
            overduedate = true;
        } else if (queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.date() == QDate::currentDate()
                   && queryJsonData->getDateTime().suggestDatetime.at(datenum).hasTime
                   && queryJsonData->getDateTime().suggestDatetime.at(datenum).datetime.time() < QTime::currentTime()) {
            qCDebug(CommonLogger) << "Query time is in the past for today";
            overduedate = true;
        } else {
            qCDebug(CommonLogger) << "Query date is valid";
            overduedate = false;
        }
    } else {
        qCDebug(CommonLogger) << "No date suggestions, query is not overdue";
        overduedate = false;
    }
    return overduedate;
}
