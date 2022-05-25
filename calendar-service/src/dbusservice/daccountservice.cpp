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
#include "daccountservice.h"

DAccountService::DAccountService(const QString &path, const QString &interface, const DAccount::Ptr &account, QObject *parent)
    : DServiceBase(path, interface, parent)
    , m_accountModel(new DAccountModule(account))
{
}

QString DAccountService::getAccountInfo()
{
    return m_accountModel->getAccountInfo();
}

QString DAccountService::getScheduleTypeList()
{
    return m_accountModel->getScheduleTypeList();
}

QString DAccountService::getScheduleTypeByID(const QString &typeID)
{
    return m_accountModel->getScheduleTypeByID(typeID);
}

QString DAccountService::createScheduleType(const QString &typeInfo)
{
    return m_accountModel->createScheduleType(typeInfo);
}

bool DAccountService::updateScheduleType(const QString &typeInfo)
{
}

bool DAccountService::deleteScheduleTypeByID(const QString &typeID)
{
    //TODO:删除类型(弱删除)和颜色
}

bool DAccountService::scheduleTypeByUsed(const QString &typeID)
{
}

QString DAccountService::createSchedule(const QString &scheduleInfo)
{
}

bool DAccountService::updateSchedule(const QString &scheduleInfo)
{
}

bool DAccountService::deleteScheduleByScheduleID(const QString &scheduleID)
{
    //(弱删除)
}

bool DAccountService::deleteSchedulesByScheduleTypeID(const QString &typeID)
{
    //(弱删除)
}

QString DAccountService::querySchedulesWithParameter(const QString &params)
{
}

void DAccountService::updateAccountInfo(const QString &accountInfo)
{
}
