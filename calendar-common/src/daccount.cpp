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
#include "daccount.h"

DAccount::DAccount(DAccount::Type type)
    : m_accountType(type)
{
}

QString DAccount::displayName() const
{
    return m_displayName;
}

void DAccount::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

QString DAccount::accountID() const
{
    return m_accountID;
}

void DAccount::setAccountID(const QString &accountID)
{
    m_accountID = accountID;
}

QString DAccount::accountName() const
{
    return m_accountName;
}

void DAccount::setAccountName(const QString &accountName)
{
    m_accountName = accountName;
}

QString DAccount::dbusPath() const
{
    return m_dbusPath;
}

void DAccount::setDbusPath(const QString &dbusPath)
{
    m_dbusPath = dbusPath;
}

DAccount::Type DAccount::accountType() const
{
    return m_accountType;
}

void DAccount::setAccountType(const Type &accountType)
{
    m_accountType = accountType;
}

bool DAccount::isExpandDisplay() const
{
    return m_isExpandDisplay;
}

void DAccount::setIsExpandDisplay(bool isExpandDisplay)
{
    m_isExpandDisplay = isExpandDisplay;
}

bool DAccount::isNetWorkAccount()
{
    return m_accountType != Type::Local_Account;
}
