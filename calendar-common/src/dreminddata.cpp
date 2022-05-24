/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "dreminddata.h"

DRemindData::DRemindData()
    : m_accountID("")
    , m_scheduleID("")
    , m_remindCount(0)
    , m_notifyid(-1)
{
}

QString DRemindData::accountID() const
{
    return m_accountID;
}

void DRemindData::setAccountID(const QString &accountID)
{
    m_accountID = accountID;
}

QString DRemindData::scheduleID() const
{
    return m_scheduleID;
}

void DRemindData::setScheduleID(const QString &scheduleID)
{
    m_scheduleID = scheduleID;
}

QDateTime DRemindData::recurrenceId() const
{
    return m_recurrenceId;
}

void DRemindData::setRecurrenceId(const QDateTime &recurrenceId)
{
    m_recurrenceId = recurrenceId;
}

int DRemindData::remindCount() const
{
    return m_remindCount;
}

void DRemindData::setRemindCount(int remindCount)
{
    m_remindCount = remindCount;
}

int DRemindData::notifyid() const
{
    return m_notifyid;
}

void DRemindData::setNotifyid(int notifyid)
{
    m_notifyid = notifyid;
}

QDateTime DRemindData::dtRemind() const
{
    return m_dtRemind;
}

void DRemindData::setDtRemind(const QDateTime &dtRemind)
{
    m_dtRemind = dtRemind;
}

QDateTime DRemindData::dtStart() const
{
    return m_dtStart;
}

void DRemindData::setDtStart(const QDateTime &dtStart)
{
    m_dtStart = dtStart;
}

QDateTime DRemindData::dtEnd() const
{
    return m_dtEnd;
}

void DRemindData::setDtEnd(const QDateTime &dtEnd)
{
    m_dtEnd = dtEnd;
}
