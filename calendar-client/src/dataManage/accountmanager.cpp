// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountmanager.h"
#include "commondef.h"

AccountManager *AccountManager::m_accountManager = nullptr;
AccountManager::AccountManager(QObject *parent)
    : QObject(parent)
    , m_dbusRequest(new DbusAccountManagerRequest(this))
{
    qCDebug(ClientLogger) << "Creating AccountManager";
    initConnect();
    m_dbusRequest->clientIsShow(true);
    m_isSupportUid  = m_dbusRequest->getIsSupportUid();
}

void AccountManager::initConnect()
{
    qCDebug(ClientLogger) << "Initializing connections";
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetAccountListFinish, this, &AccountManager::slotGetAccountListFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish, this, &AccountManager::slotGetGeneralSettingsFinish);
}

bool AccountManager::getIsSupportUid() const
{
    qCDebug(ClientLogger) << "Getting isSupportUid:" << m_isSupportUid;
    return m_isSupportUid;
}

AccountManager::~AccountManager()
{
    qCDebug(ClientLogger) << "Destroying AccountManager";
    m_dbusRequest->clientIsShow(false);
}

AccountManager *AccountManager::getInstance()
{
    // qCDebug(ClientLogger) << "Getting AccountManager instance";
    static AccountManager m_accountManager;
    return &m_accountManager;
}

/**
 * @brief AccountManager::getAccountList
 * 获取帐户列表
 * @return 帐户列表
 */
QList<AccountItem::Ptr> AccountManager::getAccountList()
{
    qCDebug(ClientLogger) << "Getting account list";
    QList<QSharedPointer<AccountItem>> accountList;
    if (nullptr != m_localAccountItem.data()) {
        qCDebug(ClientLogger) << "Adding local account to list";
        accountList.append(m_localAccountItem);
    }

    if (nullptr != m_unionAccountItem.data()) {
        qCDebug(ClientLogger) << "Adding union account to list";
        accountList.append(m_unionAccountItem);
    }
    qCDebug(ClientLogger) << "Returning" << accountList.size() << "accounts";
    return accountList;
}

/**
 * @brief AccountManager::getLocalAccountItem
 * 获取本地帐户
 * @return
 */
AccountItem::Ptr AccountManager::getLocalAccountItem()
{
    // qCDebug(ClientLogger) << "Getting local account item";
    return m_localAccountItem;
}

/**
 * @brief AccountManager::getUnionAccountItem
 * 获取unionID帐户
 * @return
 */
AccountItem::Ptr AccountManager::getUnionAccountItem()
{
    // qCDebug(ClientLogger) << "Getting union account item";
    return m_unionAccountItem;
}

DScheduleType::Ptr AccountManager::getScheduleTypeByScheduleTypeId(const QString &schduleTypeId)
{
    qCDebug(ClientLogger) << "Getting schedule type by ID:" << schduleTypeId;
    DScheduleType::Ptr type = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        type = p->getScheduleTypeByID(schduleTypeId);
        if (nullptr != type) {
            qCDebug(ClientLogger) << "Found schedule type:" << type->displayName() << "in account:" << p->getAccount()->accountName();
            break;
        }
    }
    if (type == nullptr) {
        qCDebug(ClientLogger) << "Schedule type not found for ID:" << schduleTypeId;
    }
    return type;
}

AccountItem::Ptr AccountManager::getAccountItemByScheduleTypeId(const QString &schduleTypeId)
{
    qCDebug(ClientLogger) << "Getting account item by schedule type ID:" << schduleTypeId;
    DScheduleType::Ptr type = getScheduleTypeByScheduleTypeId(schduleTypeId);
    if (nullptr == type) {
        qCDebug(ClientLogger) << "Schedule type not found for ID:" << schduleTypeId;
        return nullptr;
    }
    qCDebug(ClientLogger) << "Getting account by account ID:" << type->accountID();
    return getAccountItemByAccountId(type->accountID());
}

AccountItem::Ptr AccountManager::getAccountItemByAccountId(const QString &accountId)
{
    qCDebug(ClientLogger) << "Getting account item by account ID:" << accountId;
    AccountItem::Ptr account = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        if (p->getAccount()->accountID() == accountId) {
            qCDebug(ClientLogger) << "Found account:" << p->getAccount()->accountName();
            account = p;
            break;
        }
    }
    if (account == nullptr) {
        qCDebug(ClientLogger) << "Account not found for ID:" << accountId;
    }
    return account;
}

AccountItem::Ptr AccountManager::getAccountItemByAccountName(const QString &accountName)
{
    qCDebug(ClientLogger) << "Getting account item by account name:" << accountName;
    AccountItem::Ptr account = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        if (p->getAccount()->accountName() == accountName) {
            qCDebug(ClientLogger) << "Found account with ID:" << p->getAccount()->accountID();
            account = p;
            break;
        }
    }
    if (account == nullptr) {
        qCDebug(ClientLogger) << "Account not found for name:" << accountName;
    }
    return account;
}

DCalendarGeneralSettings::Ptr AccountManager::getGeneralSettings()
{
    qCDebug(ClientLogger) << "Getting general settings";
    return m_settings;
}

/**
 * @brief AccountManager::resetAccount
 * 重置帐户信息
 */
void AccountManager::resetAccount()
{
    qCDebug(ClientLogger) << "Resetting account information";
    m_localAccountItem.clear();
    m_unionAccountItem.clear();
    m_dbusRequest->getAccountList();
    m_dbusRequest->getCalendarGeneralSettings();
}

/**
 * @brief AccountManager::downloadByAccountID
 * 根据帐户ID下拉数据
 * @param accountID 帐户id
 * @param callback 回调函数
 */
void AccountManager::downloadByAccountID(const QString &accountID, CallbackFunc callback)
{
    qCDebug(ClientLogger) << "Downloading data for account ID:" << accountID;
    emit signalSyncNum();
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
    qCDebug(ClientLogger) << "Uploading network account data";
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->uploadNetWorkAccountData();
}

void AccountManager::setFirstDayofWeek(int value)
{
    qCDebug(ClientLogger) << "Setting first day of week to:" << value;
    m_settings->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(value));
    m_dbusRequest->setFirstDayofWeek(value);
}

void AccountManager::setTimeFormatType(int value)
{
    qCDebug(ClientLogger) << "Setting time format type to:" << value;
    m_settings->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(value));
    m_dbusRequest->setTimeFormatType(value);
}

// 设置一周首日来源
void AccountManager::setFirstDayofWeekSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    // qCDebug(ClientLogger) << "Setting first day of week source to:" << value;
    m_dbusRequest->setFirstDayofWeekSource(value);
}

// 设置时间显示格式来源
void AccountManager::setTimeFormatTypeSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    // qCDebug(ClientLogger) << "Setting time format type source to:" << value;
    m_dbusRequest->setTimeFormatTypeSource(value);
}

// 获取一周首日来源
DCalendarGeneralSettings::GeneralSettingSource AccountManager::getFirstDayofWeekSource()
{
    DCalendarGeneralSettings::GeneralSettingSource source = m_dbusRequest->getFirstDayofWeekSource();
    // qCDebug(ClientLogger) << "Getting first day of week source:" << source;
    return source;
}

// 获取时间显示格式来源
DCalendarGeneralSettings::GeneralSettingSource AccountManager::getTimeFormatTypeSource()
{
    DCalendarGeneralSettings::GeneralSettingSource source = m_dbusRequest->getTimeFormatTypeSource();
    // qCDebug(ClientLogger) << "Getting time format type source:" << source;
    return source;
}

/**
 * @brief login
 * 帐户登录
 */
void AccountManager::login()
{
    qCDebug(ClientLogger) << "Logging in";
    m_dbusRequest->login();
}

/**
 * @brief loginout
 * 帐户登出
 */
void AccountManager::loginout()
{
    qCDebug(ClientLogger) << "Logging out";
    m_dbusRequest->logout();
}

/**
 * @brief AccountManager::slotGetAccountListFinish
 * 获取帐户信息完成事件
 * @param accountList 帐户列表
 */
void AccountManager::slotGetAccountListFinish(DAccount::List accountList)
{
    qCDebug(ClientLogger) << "Received account list with" << accountList.size() << "accounts";
    bool hasUnionAccount = false;
    for (DAccount::Ptr account : accountList) {
        if (account->accountType() == DAccount::Account_Local) {
            qCDebug(ClientLogger) << "Processing local account";
            QString localName = tr("Local account");
            if (!gAccountManager->getIsSupportUid()) {
                qCDebug(ClientLogger) << "UID not supported, using 'Event types' as local name";
                localName = tr("Event types");
            }
            account->setAccountName(localName);
            if (!m_localAccountItem) {
                qCDebug(ClientLogger) << "Creating new local account item";
                m_localAccountItem.reset(new AccountItem(account, this));
            }
            m_localAccountItem->resetAccount();

        } else if (account->accountType() == DAccount::Account_UnionID) {
            qCDebug(ClientLogger) << "Processing UnionID account:" << account->accountName();
            hasUnionAccount = true;
            if (!m_unionAccountItem) {
                qCDebug(ClientLogger) << "Creating new union account item";
                m_unionAccountItem.reset(new AccountItem(account, this));
                m_unionAccountItem->resetAccount();
            } else if (m_unionAccountItem && m_unionAccountItem->getAccount()->accountID() != account->accountID()) {
                qCDebug(ClientLogger) << "Union account ID changed, creating new union account item";
                emit m_unionAccountItem->signalLogout(m_unionAccountItem->getAccount()->accountType());
                m_unionAccountItem.reset(new AccountItem(account, this));
                m_unionAccountItem->resetAccount();
            }
        }
    }
    if (!hasUnionAccount && m_unionAccountItem) {
        qCDebug(ClientLogger) << "No union account in list but union account item exists, clearing it";
        emit m_unionAccountItem->signalLogout(m_unionAccountItem->getAccount()->accountType());
        m_unionAccountItem.reset(nullptr);
    }

    for (AccountItem::Ptr p : getAccountList()) {
        qCDebug(ClientLogger) << "Setting up connections for account:" << p->getAccount()->accountName();
        connect(p.data(), &AccountItem::signalScheduleUpdate, this, &AccountManager::signalScheduleUpdate);
        connect(p.data(), &AccountItem::signalSearchScheduleUpdate, this, &AccountManager::signalSearchScheduleUpdate);
        connect(p.data(), &AccountItem::signalScheduleTypeUpdate, this, &AccountManager::signalScheduleTypeUpdate);
        connect(p.data(), &AccountItem::signalLogout, this, &AccountManager::signalLogout);
        connect(p.data(), &AccountItem::signalAccountStateChange, this, &AccountManager::signalAccountStateChange);
    }

    emit signalAccountUpdate();
}

/**
 * @brief AccountManager::slotGetGeneralSettingsFinish
 * 获取通用设置完成事件
 * @param ptr 通用设置数据
 */
void AccountManager::slotGetGeneralSettingsFinish(DCalendarGeneralSettings::Ptr ptr)
{
    qCDebug(ClientLogger) << "Received general settings";
    m_settings = ptr;
    emit signalDataInitFinished();
    emit signalGeneralSettingsUpdate();
}
