// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dreminddata.h"
#include "commondef.h"

DRemindData::DRemindData()
    : m_alarmID("")
    , m_accountID("")
    , m_scheduleID("")
    , m_remindCount(0)
    , m_notifyid(-1)
{
    // qCDebug(ServiceLogger) << "DRemindData default constructor called.";
}

QString DRemindData::accountID() const
{
    // qCDebug(ServiceLogger) << "Getting accountID:" << m_accountID;
    return m_accountID;
}

void DRemindData::setAccountID(const QString &accountID)
{
    // qCDebug(ServiceLogger) << "Setting accountID to:" << accountID;
    m_accountID = accountID;
}

QString DRemindData::scheduleID() const
{
    // qCDebug(ServiceLogger) << "Getting scheduleID:" << m_scheduleID;
    return m_scheduleID;
}

void DRemindData::setScheduleID(const QString &scheduleID)
{
    // qCDebug(ServiceLogger) << "Setting scheduleID to:" << scheduleID;
    m_scheduleID = scheduleID;
}

QDateTime DRemindData::recurrenceId() const
{
    // qCDebug(ServiceLogger) << "Getting recurrenceId:" << m_recurrenceId;
    return m_recurrenceId;
}

void DRemindData::setRecurrenceId(const QDateTime &recurrenceId)
{
    // qCDebug(ServiceLogger) << "Setting recurrenceId to:" << recurrenceId;
    m_recurrenceId = recurrenceId;
}

int DRemindData::remindCount() const
{
    // qCDebug(ServiceLogger) << "Getting remindCount:" << m_remindCount;
    return m_remindCount;
}

void DRemindData::setRemindCount(int remindCount)
{
    // qCDebug(ServiceLogger) << "Setting remindCount to:" << remindCount;
    m_remindCount = remindCount;
}

int DRemindData::notifyid() const
{
    // qCDebug(ServiceLogger) << "Getting notifyid:" << m_notifyid;
    return m_notifyid;
}

void DRemindData::setNotifyid(int notifyid)
{
    // qCDebug(ServiceLogger) << "Setting notifyid to:" << notifyid;
    m_notifyid = notifyid;
}

QDateTime DRemindData::dtRemind() const
{
    // qCDebug(ServiceLogger) << "Getting dtRemind:" << m_dtRemind;
    return m_dtRemind;
}

void DRemindData::setDtRemind(const QDateTime &dtRemind)
{
    // qCDebug(ServiceLogger) << "Setting dtRemind to:" << dtRemind;
    m_dtRemind = dtRemind;
}

QDateTime DRemindData::dtStart() const
{
    // qCDebug(ServiceLogger) << "Getting dtStart:" << m_dtStart;
    return m_dtStart;
}

void DRemindData::setDtStart(const QDateTime &dtStart)
{
    // qCDebug(ServiceLogger) << "Setting dtStart to:" << dtStart;
    m_dtStart = dtStart;
}

QDateTime DRemindData::dtEnd() const
{
    // qCDebug(ServiceLogger) << "Getting dtEnd:" << m_dtEnd;
    return m_dtEnd;
}

void DRemindData::setDtEnd(const QDateTime &dtEnd)
{
    // qCDebug(ServiceLogger) << "Setting dtEnd to:" << dtEnd;
    m_dtEnd = dtEnd;
}

QString DRemindData::alarmID() const
{
    // qCDebug(ServiceLogger) << "Getting alarmID:" << m_alarmID;
    return m_alarmID;
}

void DRemindData::setAlarmID(const QString &alarmID)
{
    // qCDebug(ServiceLogger) << "Setting alarmID to:" << alarmID;
    m_alarmID = alarmID;
}

void DRemindData::updateRemindTimeByCount()
{
    qCDebug(ServiceLogger) << "Updating remind time by count. Current count:" << m_remindCount;
    qint64 Minute = 60 * 1000;
    qint64 Hour = Minute * 60;
    qint64 duration = (10 + ((m_remindCount - 1) * 5)) * Minute; //下一次提醒距离现在的时间间隔，单位毫秒
    if (duration >= Hour) {
        qCDebug(ServiceLogger) << "Duration is greater than or equal to one hour, setting to one hour";
        duration = Hour;
    }
    qCDebug(ServiceLogger) << "Calculated duration:" << duration << "ms.";
    setDtRemind(getRemindTimeByMesc(duration));
}

void DRemindData::updateRemindTimeByMesc(qint64 duration)
{
    qCDebug(ServiceLogger) << "Updating remind time by milliseconds:" << duration;
    setDtRemind(getRemindTimeByMesc(duration));
}

QDateTime DRemindData::getRemindTimeByMesc(qint64 duration)
{
    qCDebug(ServiceLogger) << "Getting remind time by milliseconds from now. Duration:" << duration;
    QDateTime currentTime = QDateTime::currentDateTime();
    currentTime = currentTime.addMSecs(duration);
    qCDebug(ServiceLogger) << "New remind time:" << currentTime;
    return currentTime;
}
