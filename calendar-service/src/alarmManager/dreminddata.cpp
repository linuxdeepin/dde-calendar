// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dreminddata.h"
#include <QLoggingCategory>

// Add logging category
Q_LOGGING_CATEGORY(remindLog, "dde.calendar.remind")

DRemindData::DRemindData()
    : m_alarmID("")
    , m_accountID("")
    , m_scheduleID("")
    , m_remindCount(0)
    , m_notifyid(-1)
{
    qCDebug(remindLog) << "Creating new DRemindData instance";
}

QString DRemindData::accountID() const
{
    return m_accountID;
}

void DRemindData::setAccountID(const QString &accountID)
{
    qCDebug(remindLog) << "Setting account ID:" << accountID;
    m_accountID = accountID;
}

QString DRemindData::scheduleID() const
{
    return m_scheduleID;
}

void DRemindData::setScheduleID(const QString &scheduleID)
{
    qCDebug(remindLog) << "Setting schedule ID:" << scheduleID;
    m_scheduleID = scheduleID;
}

QDateTime DRemindData::recurrenceId() const
{
    return m_recurrenceId;
}

void DRemindData::setRecurrenceId(const QDateTime &recurrenceId)
{
    qCDebug(remindLog) << "Setting recurrence ID:" << recurrenceId.toString();
    m_recurrenceId = recurrenceId;
}

int DRemindData::remindCount() const
{
    return m_remindCount;
}

void DRemindData::setRemindCount(int remindCount)
{
    qCDebug(remindLog) << "Setting remind count:" << remindCount;
    m_remindCount = remindCount;
}

int DRemindData::notifyid() const
{
    return m_notifyid;
}

void DRemindData::setNotifyid(int notifyid)
{
    qCDebug(remindLog) << "Setting notify ID:" << notifyid;
    m_notifyid = notifyid;
}

QDateTime DRemindData::dtRemind() const
{
    return m_dtRemind;
}

void DRemindData::setDtRemind(const QDateTime &dtRemind)
{
    qCDebug(remindLog) << "Setting remind datetime:" << dtRemind.toString();
    m_dtRemind = dtRemind;
}

QDateTime DRemindData::dtStart() const
{
    return m_dtStart;
}

void DRemindData::setDtStart(const QDateTime &dtStart)
{
    qCDebug(remindLog) << "Setting start datetime:" << dtStart.toString();
    m_dtStart = dtStart;
}

QDateTime DRemindData::dtEnd() const
{
    return m_dtEnd;
}

void DRemindData::setDtEnd(const QDateTime &dtEnd)
{
    qCDebug(remindLog) << "Setting end datetime:" << dtEnd.toString();
    m_dtEnd = dtEnd;
}

QString DRemindData::alarmID() const
{
    return m_alarmID;
}

void DRemindData::setAlarmID(const QString &alarmID)
{
    qCDebug(remindLog) << "Setting alarm ID:" << alarmID;
    m_alarmID = alarmID;
}

void DRemindData::updateRemindTimeByCount()
{
    qCDebug(remindLog) << "Updating remind time by count for alarm:" << m_alarmID << "count:" << m_remindCount;
    qint64 Minute = 60 * 1000;
    qint64 Hour = Minute * 60;
    qint64 duration = (10 + ((m_remindCount - 1) * 5)) * Minute; //下一次提醒距离现在的时间间隔，单位毫秒
    if (duration >= Hour) {
        qCDebug(remindLog) << "Duration exceeded one hour, setting to maximum (1 hour)";
        duration = Hour;
    }
    setDtRemind(getRemindTimeByMesc(duration));
}

void DRemindData::updateRemindTimeByMesc(qint64 duration)
{
    qCDebug(remindLog) << "Updating remind time by milliseconds for alarm:" << m_alarmID << "duration:" << duration;
    setDtRemind(getRemindTimeByMesc(duration));
}

QDateTime DRemindData::getRemindTimeByMesc(qint64 duration)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime remindTime = currentTime.addMSecs(duration);
    qCDebug(remindLog) << "Calculated remind time:" << remindTime.toString() << "from current time:" << currentTime.toString();
    return remindTime;
}
