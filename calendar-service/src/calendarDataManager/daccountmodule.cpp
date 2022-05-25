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
#include "daccountmodule.h"
#include "units.h"
#include "dscheduletype.h"

DAccountModule::DAccountModule(const DAccount::Ptr &account, QObject *parent)
    : QObject(parent)
    , m_account(account)
    , m_accountDB(new DAccountDataBase(account))
{
    QString newDbPatch = getDBPath();
    m_accountDB->setDBPath(newDbPatch + "/" + account->dbName());
    m_accountDB->dbOpen();
    m_accountDB->initDBData();
    m_account = m_accountDB->getAccountInfo();
}

QString DAccountModule::getAccountInfo()
{
    QString accountInfo;
    m_account = m_accountDB->getAccountInfo();
    DAccount::toJsonString(m_account, accountInfo);
    return accountInfo;
}

QString DAccountModule::getScheduleTypeList()
{
    DScheduleType::List typeList = m_accountDB->getScheduleTypeList();
    QString typeListStr;
    DScheduleType::toJsonListString(typeList, typeListStr);
    return typeListStr;
}

QString DAccountModule::getScheduleTypeByID(const QString &typeID)
{
    DScheduleType::Ptr scheduleType = m_accountDB->getScheduleTypeByID(typeID);
    QString typeStr;
    DScheduleType::toJsonString(scheduleType, typeStr);
    return typeStr;
}

QString DAccountModule::createScheduleType(const QString &typeInfo)
{
    DScheduleType::Ptr scheduleType;
    DScheduleType::fromJsonString(scheduleType, typeInfo);
    //如果颜色为用户自定义则需要在数据库中记录
    if (scheduleType->typeColor().privilege() == DTypeColor::PriUser) {
        m_accountDB->addTypeColor(scheduleType->typeColor().colorID(), scheduleType->typeColor().colorCode(), 7);
    }
    return m_accountDB->createScheduleType(scheduleType);
}

DAccount::Ptr DAccountModule::account() const
{
    return m_account;
}
