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
#include "daccountmanagerservice.h"
#include "units.h"

#include <QMetaType>
#include <QDBusMetaType>

DAccountManagerService::DAccountManagerService(QObject *parent)
    : DServiceBase(serviceBasePath + "/AccountManager", serviceBaseName + ".AccountManager", parent)
    , m_accountManager(new DAccountManageModule(this))
{
}

QString DAccountManagerService::getAccountList()
{
    return m_accountManager->getAccountList();
}

void DAccountManagerService::remindJob(const QString &alarmID)
{
    m_accountManager->remindJob(alarmID);
}

void DAccountManagerService::updateRemindJob(bool isClear)
{
    m_accountManager->updateRemindSchedules(isClear);
}

void DAccountManagerService::notifyMsgHanding(const QString &alarmID, const qint32 operationNum)
{
    m_accountManager->notifyMsgHanding(alarmID, operationNum);
}

void DAccountManagerService::downloadByAccountID(const QString &accountID)
{
}

void DAccountManagerService::uploadNetWorkAccountData()
{
}

QString DAccountManagerService::getCalendarGeneralSettings()
{
    return m_accountManager->getCalendarGeneralSettings();
}

void DAccountManagerService::setCalendarGeneralSettings(const QString &cgSet)
{
    m_accountManager->setCalendarGeneralSettings(cgSet);
}

void DAccountManagerService::calendarOpen(const bool &isOpen)
{
    m_accountManager->calendarOpen(isOpen);
}

int DAccountManagerService::getfirstDayOfWeek()
{
    return m_accountManager->getfirstDayOfWeek();
}

void DAccountManagerService::setFirstDayOfWeek(const int firstday)
{
    m_accountManager->setFirstDayOfWeek(firstday);
}

int DAccountManagerService::getTimeFormatType()
{
    return m_accountManager->getTimeFormatType();
}

void DAccountManagerService::setTimeFormatType(const int timeType)
{
    m_accountManager->setTimeFormatType(timeType);
}
