// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountmanager.h"
#include "commondef.h"
#include "units.h"

AccountManager *AccountManager::m_accountManager = nullptr;
AccountManager::AccountManager(QObject *parent)
    : QObject(parent)
    , m_dbusRequest(new DbusAccountManagerRequest(this))
{
    qCDebug(ClientLogger) << "Creating AccountManager";
    initConnect();
    m_dbusRequest->getCalDavAccountStatusList();

    if (isCommunityEdition()) {
        m_isSupportUid = false;
        m_isSupportUidLoaded = true;
    } else {
        m_isSupportUid = true;
        m_isSupportUidLoaded = false;
        m_dbusRequest->getIsSupportUidAsync();
    }
}

void AccountManager::initConnect()
{
    qCDebug(ClientLogger) << "Initializing connections";
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetAccountListFinish, this, &AccountManager::slotGetAccountListFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish, this, &AccountManager::slotGetGeneralSettingsFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetIsSupportUidFinish, this, &AccountManager::slotGetIsSupportUidFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetCalDavAccountStatusListFinish,
            this, &AccountManager::slotGetCalDavAccountStatusListFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalCalDavAccountStatusRefreshRequested,
            this, [this](const QString &accountID) {
        if (!m_calDavStatusesInitialized && !accountID.isEmpty()) {
            m_pendingCalDavStatusChanges.insert(accountID);
        }
    });
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalValidateCalDavAccountStart,
            this, [this](const QString &requestID) {
        qCDebug(ClientLogger) << "Forwarding CalDAV validation start"
                                << "accountManager:" << this
                                << "requestIdPresent:" << !requestID.isEmpty()
                                << "receiverCount:"
                                << receivers(SIGNAL(signalCalDavAccountValidationStarted(QString)));
        emit signalCalDavAccountValidationStarted(requestID);
    });
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalValidateCalDavAccountForUpdateStart,
            this, [this](const QString &requestID) {
        qCDebug(ClientLogger) << "Forwarding CalDAV update validation start"
                                << "accountManager:" << this
                                << "requestIdPresent:" << !requestID.isEmpty()
                                << "receiverCount:"
                                << receivers(SIGNAL(signalCalDavAccountValidationForUpdateStarted(QString)));
        emit signalCalDavAccountValidationForUpdateStarted(requestID);
    });
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalGetCalDavAccountConfigFinish,
            this, &AccountManager::signalGetCalDavAccountConfigFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalUpdateCalDavAccountFinish,
            this, &AccountManager::signalUpdateCalDavAccountFinish);
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalCalDavAccountValidationFinished,
            this, [this](const QString &requestID, bool success, int validationError,
                         const QString &errorMessage, const QString &principalDisplayName) {
        qCDebug(ClientLogger) << "Forwarding CalDAV validation result"
                                << "accountManager:" << this
                                << "requestIdPresent:" << !requestID.isEmpty()
                                << "success:" << success
                                << "validationError:" << validationError
                                << "errorPresent:" << !errorMessage.isEmpty()
                                << "principalDisplayNamePresent:" << !principalDisplayName.isEmpty()
                                << "receiverCount:"
                                << receivers(SIGNAL(signalCalDavAccountValidationFinished(QString,bool,int,QString,QString)));
        emit signalCalDavAccountValidationFinished(requestID, success, validationError, errorMessage,
                                                    principalDisplayName);
    });
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalCreateCalDavAccountFinish,
            this, [this](const QString &accountID) {
        if (!accountID.isEmpty()) {
            if (m_pendingCalDavProviderType >= 0) {
                m_calDavProviderTypeOverrides.insert(accountID, m_pendingCalDavProviderType);
                DCalDavAccountStatus &status = m_calDavAccountStatuses[accountID];
                status.accountId = accountID;
                status.providerType = m_pendingCalDavProviderType;
            }
            m_pendingCalDavProviderType = -1;
            // Refresh immediately so the new account starts loading its schedule data
            // without waiting for a later settings refresh or application restart.
            m_dbusRequest->getAccountList();
        }
        emit signalCreateCalDavAccountFinish(accountID);
    });
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalDeleteCalDavAccountFinish,
            this, [this](bool success) {
        if (success && !m_pendingCalDavDeleteAccountID.isEmpty()) {
            m_calDavProviderTypeOverrides.remove(m_pendingCalDavDeleteAccountID);
            m_calDavAccountStatuses.remove(m_pendingCalDavDeleteAccountID);
            m_manualCalDavSyncAccounts.remove(m_pendingCalDavDeleteAccountID);
        }
        m_pendingCalDavDeleteAccountID.clear();
        emit signalDeleteCalDavAccountFinish(success);
    });
    connect(m_dbusRequest, &DbusAccountManagerRequest::signalCalDavAccountRequestFailed,
            this, &AccountManager::signalCalDavAccountRequestFailed);
}

DCalDavAccountStatus AccountManager::getCalDavAccountStatus(const QString &accountID) const
{
    return m_calDavAccountStatuses.value(accountID);
}

bool AccountManager::canWriteCalDavAccount(const QString &accountID) const
{
    const DCalDavAccountStatus status = m_calDavAccountStatuses.value(accountID);
    return !status.accountId.isEmpty() && status.supportsWrite;
}

bool AccountManager::takeManualCalDavSyncRequest(const QString &accountID, int syncStatus)
{
    const bool completed = syncStatus == DCalDavSyncStatus::Succeeded
        || syncStatus == DCalDavSyncStatus::Failed
        || syncStatus == DCalDavSyncStatus::AuthenticationRequired
        || syncStatus == DCalDavSyncStatus::PermissionDenied
        || syncStatus == DCalDavSyncStatus::RetryScheduled;
    if (!completed || !m_manualCalDavSyncAccounts.contains(accountID)) {
        return false;
    }

    m_manualCalDavSyncAccounts.remove(accountID);
    return true;
}

bool AccountManager::getIsSupportUid() const
{
    qCDebug(ClientLogger) << "Getting isSupportUid:" << m_isSupportUid;
    return m_isSupportUid;
}

void AccountManager::slotGetCalDavAccountStatusListFinish(DCalDavAccountStatus::List statusList)
{
    QHash<QString, DCalDavAccountStatus> updatedStatuses;
    for (const DCalDavAccountStatus &status : statusList) {
        updatedStatuses.insert(status.accountId, status);
    }
    for (auto it = m_calDavProviderTypeOverrides.cbegin();
         it != m_calDavProviderTypeOverrides.cend(); ++it) {
        auto status = updatedStatuses.find(it.key());
        if (status != updatedStatuses.end()) {
            status->providerType = it.value();
        }
    }

    // The first status query is an initial snapshot, not a new failure event.
    // Do not notify consumers here, otherwise a previously persisted failure
    // would show a stale toast when the calendar starts.
    const bool shouldNotifyChanges = m_calDavStatusesInitialized;
    const QHash<QString, DCalDavAccountStatus> previousStatuses = m_calDavAccountStatuses;
    m_calDavAccountStatuses = updatedStatuses;
    m_calDavStatusesInitialized = true;
    if (!shouldNotifyChanges) {
        // The initial snapshot is intentionally not exposed as a status-change
        // event, otherwise persisted failures could trigger stale toasts. A
        // status-change signal received before this snapshot, however, belongs
        // to a live service event and must not be swallowed by initialization.
        const QSet<QString> pendingStatusChanges = m_pendingCalDavStatusChanges;
        m_pendingCalDavStatusChanges.clear();
        emit signalCalDavAccountStatusReady();
        for (const QString &accountID : pendingStatusChanges) {
            if (updatedStatuses.contains(accountID)) {
                emit signalCalDavAccountStatusChanged(accountID);
            }
        }
        maybeNotifyClientIsShow();
        return;
    }

    for (auto it = updatedStatuses.cbegin(); it != updatedStatuses.cend(); ++it) {
        const DCalDavAccountStatus previous = previousStatuses.value(it.key());
        const DCalDavAccountStatus &current = it.value();
        if (previous.accountId != current.accountId
            || previous.displayName != current.displayName
            || previous.providerType != current.providerType
            || previous.syncStatus != current.syncStatus
            || previous.lastSuccessfulSync != current.lastSuccessfulSync
            || previous.failureReason != current.failureReason
            || previous.failureCode != current.failureCode
            || previous.accountColor != current.accountColor
            || previous.supportsWrite != current.supportsWrite
            || previous.pendingOperationCount != current.pendingOperationCount
            || previous.pendingDeleteCount != current.pendingDeleteCount
            || previous.conflictCount != current.conflictCount
            || previous.nextRetryAt != current.nextRetryAt) {
            emit signalCalDavAccountStatusChanged(current.accountId);
        }
    }
    emit signalCalDavAccountStatusReady();
    maybeNotifyClientIsShow();
}

void AccountManager::maybeNotifyClientIsShow()
{
    if (!m_clientShowRequested || m_clientShowNotified || !m_calDavStatusesInitialized) {
        return;
    }

    qCDebug(ClientLogger) << "Notifying account service that calendar is shown";
    m_clientShowNotified = true;
    m_dbusRequest->clientIsShow(true);
}

void AccountManager::slotGetIsSupportUidFinish(bool supported)
{
    m_isSupportUid = supported;
    m_isSupportUidLoaded = true;

    if (m_localAccountItem) {
        auto account = m_localAccountItem->getAccount();
        if (account) {
            const QString expectedName = m_isSupportUid ? tr("Local account") : tr("Event types");
            if (account->accountName() != expectedName) {
                account->setAccountName(expectedName);
                emit signalAccountUpdate();
            }
        }
    }
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
    for (const AccountItem::Ptr &account : m_calDavAccountItems) {
        accountList.append(account);
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

void AccountManager::notifyClientIsShow()
{
    m_clientShowRequested = true;
    maybeNotifyClientIsShow();
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
    const AccountItem::Ptr account = getAccountItemByAccountId(accountID);
    if (account && account->getAccount()
        && account->getAccount()->accountType() == DAccount::Account_CalDav) {
        m_manualCalDavSyncAccounts.insert(accountID);
    } else {
        if (account && account->getAccount()
            && account->getAccount()->accountType() == DAccount::Account_UnionID) {
            m_manualUnionSyncAccounts.insert(accountID);
        }
        emit signalSyncNum();
    }
    m_dbusRequest->setCallbackFunc(callback);
    m_dbusRequest->downloadByAccountID(accountID);
}

/**
 * @brief AccountManager::uploadNetWorkAccountData
 * 更新网络帐户数据
 * @param callback 回调函数
 */
void AccountManager::validateCalDavAccount(int providerType, const QString &serverUrl,
                                             const QString &username, const QString &credentialRef)
{
    m_dbusRequest->validateCalDavAccount(providerType, serverUrl, username, credentialRef);
}

void AccountManager::deleteCalDavAccountWithLocalDataOption(const QString &accountID,
                                                             bool deleteLocalData)
{
    m_pendingCalDavDeleteAccountID = accountID;
    m_dbusRequest->deleteCalDavAccountWithLocalDataOption(accountID, deleteLocalData);
}

void AccountManager::getCalDavAccountConfig(const QString &accountID)
{
    m_dbusRequest->getCalDavAccountConfig(accountID);
}

void AccountManager::validateCalDavAccountForUpdate(
    const QString &accountID, int providerType, const QString &serverUrl,
    const QString &username, const QString &credentialRef)
{
    m_dbusRequest->validateCalDavAccountForUpdate(
        accountID, providerType, serverUrl, username, credentialRef);
}

void AccountManager::createCalDavAccount(int providerType, const QString &serverUrl,
                                           const QString &username, const QString &credentialRef,
                                           const QString &displayName)
{
    m_pendingCalDavProviderType = providerType;
    m_dbusRequest->createCalDavAccount(providerType, serverUrl, username, credentialRef, displayName);
}

void AccountManager::updateCalDavAccount(const QString &accountID, int providerType,
                                          const QString &serverUrl, const QString &username,
                                          const QString &credentialRef, const QString &displayName)
{
    m_calDavProviderTypeOverrides.insert(accountID, providerType);
    if (m_calDavAccountStatuses.contains(accountID)) {
        m_calDavAccountStatuses[accountID].providerType = providerType;
        emit signalCalDavAccountStatusChanged(accountID);
    }
    m_dbusRequest->updateCalDavAccount(accountID, providerType, serverUrl, username, credentialRef,
                                       displayName);
}

void AccountManager::resolveAllCalDavConflicts(const QString &accountID, bool keepLocal)
{
    m_dbusRequest->resolveAllCalDavConflicts(accountID, keepLocal);
}

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
    QHash<QString, AccountItem::Ptr> existingCalDavAccounts;
    for (const AccountItem::Ptr &item : m_calDavAccountItems) {
        existingCalDavAccounts.insert(item->getAccount()->accountID(), item);
    }
    QList<AccountItem::Ptr> updatedCalDavAccounts;
    QStringList currentCalDavAccountIDs;
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


        } else if (account->accountType() == DAccount::Account_UnionID && !isCommunityEdition()) {
            qCDebug(ClientLogger) << "Processing UnionID account:" << account->accountName();
            hasUnionAccount = true;
            bool unionAccountChanged = false;
            if (!m_unionAccountItem) {
                qCDebug(ClientLogger) << "Creating new union account item";
                m_unionAccountItem.reset(new AccountItem(account, this));
                unionAccountChanged = true;
            } else if (m_unionAccountItem->getAccount()->accountID() != account->accountID()) {
                qCDebug(ClientLogger) << "Union account ID changed, creating new union account item";
                emit m_unionAccountItem->signalLogout(m_unionAccountItem->getAccount()->accountType());
                m_manualUnionSyncAccounts.remove(m_unionAccountItem->getAccount()->accountID());
                m_unionAccountItem.reset(new AccountItem(account, this));
                unionAccountChanged = true;
            }
            if (unionAccountChanged) {
                const QString unionAccountID = m_unionAccountItem->getAccount()->accountID();
                connect(m_unionAccountItem.data(), &AccountItem::signalSyncStateChange,
                        this, [this, unionAccountID](DAccount::AccountSyncState state) {
                    // Automatic syncs stay silent on success, while failures
                    // always notify. Manual sync results also notify.
                    const bool manuallyRequested =
                        m_manualUnionSyncAccounts.remove(unionAccountID) > 0;
                    if (state == DAccount::Sync_Normal && !manuallyRequested) {
                        return;
                    }
                    emit signalSyncNum(static_cast<int>(state));
                });
            }
        } else if (account->accountType() == DAccount::Account_CalDav) {
            currentCalDavAccountIDs.append(account->accountID());
            AccountItem::Ptr item = existingCalDavAccounts.value(account->accountID());
            if (!item) {
                item.reset(new AccountItem(account, this));
            } else {
                item->updateAccount(account);
            }
            updatedCalDavAccounts.append(item);
        }
    }
    m_calDavAccountItems = updatedCalDavAccounts;
    for (auto it = m_calDavProviderTypeOverrides.begin();
         it != m_calDavProviderTypeOverrides.end();) {
        if (!currentCalDavAccountIDs.contains(it.key())) {
            it = m_calDavProviderTypeOverrides.erase(it);
        } else {
            ++it;
        }
    }

    if (!hasUnionAccount && m_unionAccountItem) {
        qCDebug(ClientLogger) << "No union account in list but union account item exists, clearing it";
        emit m_unionAccountItem->signalLogout(m_unionAccountItem->getAccount()->accountType());
        m_manualUnionSyncAccounts.clear();
        m_unionAccountItem.reset(nullptr);
    }

    // Install all account signal connections before starting asynchronous
    // data queries. A local CalDAV database can answer quickly, so starting a
    // query before these connections are in place may lose the result and
    // leave the calendar view without the remote schedules.
    for (AccountItem::Ptr p : getAccountList()) {
        qCDebug(ClientLogger) << "Setting up account data connections"
                                << "accountId:" << p->getAccount()->accountID()
                                << "accountType:" << p->getAccount()->accountType();
        connect(p.data(), &AccountItem::signalScheduleUpdate, this, &AccountManager::signalScheduleUpdate,
                Qt::UniqueConnection);
        connect(p.data(), &AccountItem::signalSearchScheduleUpdate, this, &AccountManager::signalSearchScheduleUpdate,
                Qt::UniqueConnection);
        connect(p.data(), &AccountItem::signalScheduleTypeUpdate, this, &AccountManager::signalScheduleTypeUpdate,
                Qt::UniqueConnection);
        connect(p.data(), &AccountItem::signalLogout, this, &AccountManager::signalLogout,
                Qt::UniqueConnection);
        connect(p.data(), &AccountItem::signalAccountStateChange, this, &AccountManager::signalAccountStateChange,
                Qt::UniqueConnection);
        connect(p.data(), &AccountItem::signalCalDavScheduleCreateFailed, this,
                &AccountManager::signalCalDavScheduleCreateFailed, Qt::UniqueConnection);
    }

    // Start loading only after the account list and all forwarding connections
    // are ready.
    for (AccountItem::Ptr p : getAccountList()) {
        p->resetAccount();
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
