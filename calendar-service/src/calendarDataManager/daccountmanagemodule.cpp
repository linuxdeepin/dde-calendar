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
#include "daccountmanagemodule.h"
#include "units.h"

DAccountManageModule::DAccountManageModule(QObject *parent)
    : QObject(parent)
    , m_accountManagerDB(new DAccountManagerDataBase)
    , m_unionIDDbus(new DUnionIDDbus(unionid_service, unionid_path, QDBusConnection::sessionBus()))
{
    //新文件路径
    QString newDbPatch = getDBPath();
    QString newDB(newDbPatch + "/" + "accountmanager.db");
    m_accountManagerDB->setDBPath(newDB);
    m_accountManagerDB->dbOpen();
    m_accountList = m_accountManagerDB->getAccountList();
    //将云端帐户信息基本数据与本地数据合并
    unionIDDataMerging();

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    //根据获取到的帐户信息创建对应的帐户服务
    DAccount::List accountList = m_accountManagerDB->getAccountList();
    foreach (auto account, accountList) {
        DAccountService::Ptr accountService = DAccountService::Ptr(new DAccountService(account->dbusPath(), account->dbusInterface(), account, this));
        if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(), accountService.data(), options)) {
            qWarning() << "registerObject accountService failed:" << sessionBus.lastError();
        } else {
            m_AccountServiceMap[account->accountType()].insert(account->accountID(), accountService);
        }
    }
}

QString DAccountManageModule::getAccountList()
{
    QString accountStr;
    DAccount::toJsonListString(m_accountList, accountStr);
    return accountStr;
}

QString DAccountManageModule::getCalendarGeneralSettings()
{
    QString cgSetStr;
    DCalendarGeneralSettings::Ptr cgSet = m_accountManagerDB->getCalendarGeneralSettings();
    DCalendarGeneralSettings::toJsonString(cgSet, cgSetStr);
    return cgSetStr;
}

void DAccountManageModule::setCalendarGeneralSettings(const QString &cgSet)
{
    DCalendarGeneralSettings::Ptr cgSetPtr = DCalendarGeneralSettings::Ptr(new DCalendarGeneralSettings);
    DCalendarGeneralSettings::fromJsonString(cgSetPtr, cgSet);
    m_accountManagerDB->setCalendarGeneralSettings(cgSetPtr);
}

void DAccountManageModule::unionIDDataMerging()
{
    DAccount::Ptr accountUnionid = m_unionIDDbus->getUserData();
    //如果已登陆unionid
    if (!accountUnionid.isNull()) {
        DAccount::Ptr unionidDB;
        auto hasUnionid = [=, &unionidDB](const DAccount::Ptr &account) {
            if (account->accountType() == DAccount::Account_UnionID) {
                unionidDB = account;
                return true;
            }
            return false;
        };
        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            if (unionidDB->avatar() == accountUnionid->avatar() && unionidDB->accountName() == accountUnionid->accountName()
                && unionidDB->displayName() == accountUnionid->displayName()) {
            } else {
                unionidDB->setAvatar(accountUnionid->avatar());
                unionidDB->setAccountName(accountUnionid->accountName());
                unionidDB->setDisplayName(accountUnionid->displayName());
                m_accountManagerDB->updateAccountInfo(unionidDB);
            }
        }
    }
}
