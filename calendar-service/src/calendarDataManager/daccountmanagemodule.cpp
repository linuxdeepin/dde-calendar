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
#include "dalarmmanager.h"

DAccountManageModule::DAccountManageModule(QObject *parent)
    : QObject(parent)
    , m_accountManagerDB(new DAccountManagerDataBase)
    , m_syncoperation(new Syncoperation)
{
    //新文件路径
    QString newDbPatch = getDBPath();
    QString newDB(newDbPatch + "/" + "accountmanager.db");
    m_accountManagerDB->setDBPath(newDB);
    m_accountManagerDB->dbOpen();

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    //将云端帐户信息基本数据与本地数据合并
    unionIDDataMerging();

    //根据获取到的帐户信息创建对应的帐户服务
    foreach (auto account, m_accountList) {
        DAccountModule::Ptr accountModule = DAccountModule::Ptr(new DAccountModule(account));
        m_accountModuleMap[account->accountID()] = accountModule;
        DAccountService::Ptr accountService = DAccountService::Ptr(new DAccountService(account->dbusPath(), account->dbusInterface(), accountModule, this));
        if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(), accountService.data(), options)) {
            qWarning() << "registerObject accountService failed:" << sessionBus.lastError();
        } else {
            m_AccountServiceMap[account->accountType()].insert(account->accountID(), accountService);
        }
    }
    m_generalSetting = m_accountManagerDB->getCalendarGeneralSettings();
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
    m_generalSetting = m_accountManagerDB->getCalendarGeneralSettings();
    DCalendarGeneralSettings::toJsonString(m_generalSetting, cgSetStr);
    return cgSetStr;
}

void DAccountManageModule::setCalendarGeneralSettings(const QString &cgSet)
{
    DCalendarGeneralSettings::Ptr cgSetPtr = DCalendarGeneralSettings::Ptr(new DCalendarGeneralSettings);
    DCalendarGeneralSettings::fromJsonString(cgSetPtr, cgSet);
    if (m_generalSetting != cgSetPtr) {
        m_accountManagerDB->setCalendarGeneralSettings(cgSetPtr);
        DCalendarGeneralSettings::Ptr tmpSetting = DCalendarGeneralSettings::Ptr(m_generalSetting->clone());
        m_generalSetting = cgSetPtr;
        if (tmpSetting->firstDayOfWeek() != m_generalSetting->firstDayOfWeek()) {
            emit firstDayOfWeekChange(m_generalSetting->firstDayOfWeek());
        }
        if (tmpSetting->timeShowType() != m_generalSetting->timeShowType()) {
            emit timeFormatTypeChange(m_generalSetting->timeShowType());
        }
    }
}

int DAccountManageModule::getfirstDayOfWeek()
{
    return static_cast<int>(m_generalSetting->firstDayOfWeek());
}

void DAccountManageModule::setFirstDayOfWeek(const int firstday)
{
    if (m_generalSetting->firstDayOfWeek() != firstday) {
        m_generalSetting->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(firstday));
        m_accountManagerDB->setCalendarGeneralSettings(m_generalSetting);
    }
}

int DAccountManageModule::getTimeFormatType()
{
    return static_cast<int>(m_generalSetting->timeShowType());
}

void DAccountManageModule::setTimeFormatType(const int timeType)
{
    if (m_generalSetting->timeShowType() != timeType) {
        m_generalSetting->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(timeType));
        m_accountManagerDB->setCalendarGeneralSettings(m_generalSetting);
    }
}

void DAccountManageModule::remindJob(const QString &accountID, const QString &alarmID)
{
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->remindJob(alarmID);
    }
}

void DAccountManageModule::updateRemindSchedules(bool isClear)
{
    QMap<QString, DAccountModule::Ptr>::const_iterator iter = m_accountModuleMap.constBegin();
    for (; iter != m_accountModuleMap.constEnd(); ++iter) {
        iter.value()->updateRemindSchedules(isClear);
    }
}

void DAccountManageModule::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
{
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->notifyMsgHanding(alarmID, operationNum);
    }
}

void DAccountManageModule::downloadByAccountID(const QString &accountID)
{
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->accountDownload();
    }
}

void DAccountManageModule::uploadNetWorkAccountData()
{
    QMap<QString, DAccountModule::Ptr>::const_iterator iter = m_accountModuleMap.constBegin();
    for (; iter != m_accountModuleMap.constEnd(); ++iter) {
        iter.value()->uploadNetWorkAccountData();
    }
}

void DAccountManageModule::unionIDDataMerging()
{
    m_accountList = m_accountManagerDB->getAccountList();
    DAccount::Ptr accountUnionid = m_syncoperation->optUserData();

    DAccount::Ptr unionidDB;
    auto hasUnionid = [=, &unionidDB](const DAccount::Ptr &account) {
        if (account->accountType() == DAccount::Account_UnionID) {
            unionidDB = account;
            return true;
        }
        return false;
    };
    //如果unionid帐户不存在，则判断数据库中是否有登陆前的信息
    //若有则移除
    if (accountUnionid.isNull()) {
        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            m_accountManagerDB->deleteAccountInfo(unionidDB->accountID());
        }
    } else {
        //如果unionID登陆了
        //设置DBus路径和数据库名

        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            //如果是一个帐户则判断信息是否一致，不一致需更新
            if (unionidDB->accountName() == accountUnionid->accountName()) {
                if (unionidDB->avatar() != accountUnionid->avatar() || unionidDB->displayName() != accountUnionid->displayName()) {
                    unionidDB->avatar() = accountUnionid->avatar();
                    unionidDB->displayName() = accountUnionid->displayName();
                    m_accountManagerDB->updateAccountInfo(unionidDB);
                }
            } else {
                m_accountManagerDB->deleteAccountInfo(unionidDB->accountID());
                //如果不是一个帐户则移除再设置
                initAccountDBusInfo(accountUnionid);
                m_accountManagerDB->addAccountInfo(accountUnionid);
            }
        } else {
            initAccountDBusInfo(accountUnionid);
            m_accountManagerDB->addAccountInfo(accountUnionid);
        }
    }
    //更新最新的帐户信息
    m_accountList = m_accountManagerDB->getAccountList();
}

void DAccountManageModule::initAccountDBusInfo(const DAccount::Ptr &account)
{
    QString typeStr = "";
    switch (account->accountType()) {
    case DAccount::Type::Account_UnionID:
        typeStr = "uid";
        break;
    case DAccount::Type::Account_CalDav:
        typeStr = "caldav";
        break;
    default:
        typeStr = "default";
        break;
    }
    account->setAccountID(DDataBase::createUuid());
    QString sortID = account->accountID().mid(0, 5);
    account->setAccountType(DAccount::Account_UnionID);
    account->setDtCreate(QDateTime::currentDateTime());
    account->setDbName(QString("account_%1_%2.db").arg(typeStr).arg(sortID));
    account->setDbusPath(QString("%1/account_%2_%3").arg(serviceBasePath).arg(typeStr).arg(sortID));
    account->setDbusInterface(accountServiceInterface);
}
