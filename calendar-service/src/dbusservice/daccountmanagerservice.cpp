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
#include "calendarprogramexitcontrol.h"

#include <QMetaType>
#include <QDBusMetaType>
#include <QDebug>

DAccountManagerService::DAccountManagerService(QObject *parent)
    : DServiceBase(serviceBasePath + "/AccountManager", serviceBaseName + ".AccountManager", parent)
    , m_accountManager(new DAccountManageModule(this))
{
    //TODO:为了便于调试先注释,待开发完成取消注释
    //    DServiceExitControl exitControl;
}

QString DAccountManagerService::getAccountList()
{
    DServiceExitControl exitControl;
    if (!clientWhite(1)) {
        return QString();
    }
    return m_accountManager->getAccountList();
}

void DAccountManagerService::remindJob(const QString &accountID, const QString &alarmID)
{
    m_accountManager->remindJob(accountID, alarmID);
}

void DAccountManagerService::updateRemindJob(bool isClear)
{
    m_accountManager->updateRemindSchedules(isClear);
}

void DAccountManagerService::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
{
    m_accountManager->notifyMsgHanding(accountID, alarmID, operationNum);
}

void DAccountManagerService::downloadByAccountID(const QString &accountID)
{
}

void DAccountManagerService::uploadNetWorkAccountData()
{
}

QString DAccountManagerService::getCalendarGeneralSettings()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return QString();
    }
    return m_accountManager->getCalendarGeneralSettings();
}

void DAccountManagerService::setCalendarGeneralSettings(const QString &cgSet)
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    m_accountManager->setCalendarGeneralSettings(cgSet);
}

void DAccountManagerService::calendarIsShow(const bool &isShow)
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    exitControl.setClientIsOpen(isShow);
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
