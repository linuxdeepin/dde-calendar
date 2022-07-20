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
    //自动退出
    DServiceExitControl exitControl;
    connect(m_accountManager.data(), &DAccountManageModule::signalLoginStatusChange, this, &DAccountManagerService::accountUpdate);

    connect(m_accountManager.data(), &DAccountManageModule::firstDayOfWeekChange, this, [&]() {
        notifyPropertyChanged(getInterface(), "firstDayOfWeek");
    });
    connect(m_accountManager.data(), &DAccountManageModule::timeFormatTypeChange, this, [&]() {
        notifyPropertyChanged(getInterface(), "timeFormatType");
    });
}

QString DAccountManagerService::getAccountList()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return QString();
    }
    return m_accountManager->getAccountList();
}

void DAccountManagerService::remindJob(const QString &accountID, const QString &alarmID)
{
    DServiceExitControl exitControl;
    m_accountManager->remindJob(accountID, alarmID);
}

void DAccountManagerService::updateRemindJob(bool isClear)
{
    DServiceExitControl exitControl;
    m_accountManager->updateRemindSchedules(isClear);
}

void DAccountManagerService::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
{
    DServiceExitControl exitControl;
    m_accountManager->notifyMsgHanding(accountID, alarmID, operationNum);
}

void DAccountManagerService::downloadByAccountID(const QString &accountID)
{
    //TODO:云同步获取数据
    DServiceExitControl exitControl;
    m_accountManager->downloadByAccountID(accountID);
}

void DAccountManagerService::uploadNetWorkAccountData()
{
    //TODO:云同步上传数据
    DServiceExitControl exitControl;
    m_accountManager->uploadNetWorkAccountData();
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
    m_accountManager->calendarOpen(isShow);
}

void DAccountManagerService::login()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    m_accountManager->login();
}

void DAccountManagerService::logout()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return;
    }
    m_accountManager->logout();
}

bool DAccountManagerService::isSupportUid()
{
    DServiceExitControl exitControl;
    if (!clientWhite(0)) {
        return false;
    }
    return m_accountManager->isSupportUid();
}

int DAccountManagerService::getfirstDayOfWeek() const
{
    DServiceExitControl exitControl;
    return m_accountManager->getfirstDayOfWeek();
}

void DAccountManagerService::setFirstDayOfWeek(const int firstday)
{
    DServiceExitControl exitControl;
    m_accountManager->setFirstDayOfWeek(firstday);
}

int DAccountManagerService::getTimeFormatType() const
{
    DServiceExitControl exitControl;
    return m_accountManager->getTimeFormatType();
}

void DAccountManagerService::setTimeFormatType(const int timeType)
{
    DServiceExitControl exitControl;
    m_accountManager->setTimeFormatType(timeType);
}
