/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#include "accountmanager.h"

AccountManager* AccountManager::m_accountManager = nullptr;
AccountManager::AccountManager(QObject *parent) : QObject(parent)
  , m_dbusRequest(new DbusAccountManagerRequest(this))
{
    initConnect();
}

void AccountManager::initConnect()
{
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetAccountListFinish, this, &AccountManager::slotGetAccountListFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish, this, &AccountManager::slotGetGeneralSettingsFinish);
}

AccountManager* AccountManager::getInstance()
{
    static AccountManager m_accountManager;
    return &m_accountManager;
}

/**
 * @brief AccountManager::getAccountList
 * 获取账户列表
 * @return 账户列表
 */
QList<AccountItem::Ptr> AccountManager::getAccountList()
{
    QList<QSharedPointer<AccountItem>> accountList;
    if (nullptr != m_localAccountItem.data()) {
        accountList.append(m_localAccountItem);
    }

    if (nullptr != m_unionAccountItem.data()) {
        accountList.append(m_unionAccountItem);
    }
    return accountList;
}

/**
 * @brief AccountManager::getLocalAccountItem
 * 获取本地账户
 * @return
 */
QSharedPointer<AccountItem> AccountManager::getLocalAccountItem()
{
    return m_localAccountItem;
}

/**
 * @brief AccountManager::getUnionAccountItem
 * 获取unionID账户
 * @return
 */
QSharedPointer<AccountItem> AccountManager::getUnionAccountItem()
{
    return m_unionAccountItem;
}

/**
 * @brief AccountManager::resetAccount
 * 重置账户信息
 */
void AccountManager::resetAccount()
{
    m_dataInitFinished = false;
    m_localAccountItem.clear();
    m_unionAccountItem.clear();
    m_dbusRequest->getAccountList();
    m_dbusRequest->getCalendarGeneralSettings();
}

/**
 * @brief AccountManager::downloadByAccountID
 * 根据帐户ID下拉数据
 * @param accountID 账户id
 * @param callback 回调函数
 */
void AccountManager::downloadByAccountID(const QString &accountID, CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->downloadByAccountID(accountID);
}

/**
 * @brief AccountManager::uploadNetWorkAccountData
 * 更新网络帐户数据
 * @param callback 回调函数
 */
void AccountManager::uploadNetWorkAccountData(CallbackFunc callback)
{
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->uploadNetWorkAccountData();
}

/**
 * @brief AccountManager::waitingData
 * 等待数据获取完成的事件，若数据已获取完成则直接执行回调函数，若数据还没有获取完成则将回调函数保存，待数据获取完成后执行
 * @param callback 回调函数
 */
void AccountManager::waitingData(CallbackFunc callback)
{
    if (m_dataInitFinished) {
        callback(true);
    } else {
        m_waitingCallList.append(callback);
    }
}

/**
 * @brief AccountManager::execWaitingCall
 * 执行回调函数
 */
void AccountManager::execWaitingCall()
{
    m_dataInitFinished = true;
    for (CallbackFunc call : m_waitingCallList) {
        call(true);
    }
    //回调函数只是用一次
    m_waitingCallList.clear();
}

/**
 * @brief AccountManager::slotGetAccountListFinish
 * 获取账户信息完成事件
 * @param accountList 账户列表
 */
void AccountManager::slotGetAccountListFinish(DAccount::List accountList)
{
    for (DAccount::Ptr account : accountList) {
        if (account->accountType() == DAccount::Account_Local) {
            m_localAccountItem.reset(new AccountItem(account, this));
            connect(m_localAccountItem.data(), &AccountItem::signalScheduleUpdate, this, &AccountManager::signalScheduleUpdate);
            m_localAccountItem->resetAccount();
        }

        if (account->accountType() == DAccount::Account_UnionID) {
            m_unionAccountItem.reset(new AccountItem(account, this));
            connect(m_unionAccountItem.data(), &AccountItem::signalScheduleUpdate, this, &AccountManager::signalScheduleUpdate);
            m_unionAccountItem->resetAccount();
        }
    }
    if (m_dataInitFinished) {
        emit signalAccountUpdate();
    }
}

/**
 * @brief AccountManager::slotGetGeneralSettingsFinish
 * 获取通用设置完成事件
 * @param ptr 通用设置数据
 */
void AccountManager::slotGetGeneralSettingsFinish(DCalendarGeneralSettings::Ptr ptr)
{
    m_settings = ptr;
    if (!m_dataInitFinished) {
        execWaitingCall();
        emit signalDataInitFinished();
    } else {
        emit signalGeneralSettingsUpdate();
    }
}
