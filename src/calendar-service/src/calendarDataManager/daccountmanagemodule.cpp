// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagemodule.h"
#include "commondef.h"
#include "units.h"
#include "calendarprogramexitcontrol.h"
#include "dcaldavaccountregistrar.h"
#include "dcaldavaccountdeletioncleanup.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavcredentialstore.h"
#include "dcalendareventlog.h"
#include "dcaldavprofile.h"
#include "dcaldavreadonlysync.h"
#include "dcaldavretrypolicy.h"
#include "dcaldavsyncstatusmapper.h"
#include "dcaldavtransport.h"
#include <qstandardpaths.h>
#include <QUrl>
#include <QCoreApplication>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <DSysInfo>

#include <limits>

const QString firstDayOfWeek_key = "firstDayOfWeek";
const QString shortTimeFormat_key = "shortTimeFormat";
const QString firstDayOfWeekSource_key = "firstDayOfWeekSource";
const QString shortTimeFormatSource_key = "shortTimeFormatSource";

namespace {

QString localTypeIDForCategories(DAccountDataBase *localDatabase,
                                const QStringList &categories,
                                QString *unmatchedOriginalType)
{
    if (unmatchedOriginalType != nullptr) {
        unmatchedOriginalType->clear();
    }

    QStringList originalTypes;
    for (const QString &category : categories) {
        const QString normalized = category.trimmed();
        if (!normalized.isEmpty()) {
            originalTypes.append(normalized);
        }
    }
    QHash<QString, QString> categoryTypeMap;
    const auto addCategoryAliases = [&categoryTypeMap](const QString &targetName,
                                                        const QStringList &aliases) {
        for (const QString &alias : aliases) {
            categoryTypeMap.insert(alias.toLower(), targetName);
        }
    };
    addCategoryAliases(QStringLiteral("Work"),
                       {QStringLiteral("Work"), QStringLiteral("Business"),
                        QCoreApplication::translate("DAccountDataBase", "Work")});
    addCategoryAliases(QStringLiteral("Life"),
                       {QStringLiteral("Life"), QStringLiteral("Personal"),
                        QCoreApplication::translate("DAccountDataBase", "Life")});
    addCategoryAliases(QStringLiteral("Other"),
                       {QStringLiteral("Other"),
                        QCoreApplication::translate("DAccountDataBase", "Other")});

    QString targetName = QStringLiteral("Other");
    bool matched = false;
    for (const QString &typeName : originalTypes) {
        const auto target = categoryTypeMap.constFind(typeName.toLower());
        if (target == categoryTypeMap.constEnd()) {
            continue;
        }
        targetName = target.value();
        matched = true;
        break;
    }

    for (const DScheduleType::Ptr &type : localDatabase->getScheduleTypeList()) {
        if (!type.isNull()
            && (type->typeName().compare(targetName, Qt::CaseInsensitive) == 0
                || type->displayName().compare(targetName, Qt::CaseInsensitive) == 0)) {
            if (!matched && unmatchedOriginalType != nullptr) {
                *unmatchedOriginalType = originalTypes.join(QStringLiteral(", "));
            }
            return type->typeID();
        }
    }
    return QString();
}


QString defaultCalDavAccountColor(const QString &accountID)
{
    static const QStringList colors = {
        QStringLiteral("#5B8FF9"), QStringLiteral("#61DDAA"), QStringLiteral("#65789B"),
        QStringLiteral("#F6BD16"), QStringLiteral("#7262FD"), QStringLiteral("#78D3F8"),
        QStringLiteral("#9661BC"), QStringLiteral("#F6903D"), QStringLiteral("#008685")
    };
    return colors.at(static_cast<uint>(qHash(accountID)) % colors.size());
}

bool restoreCalDavConflictSnapshot(const DAccountModule::Ptr &module,
                                   const DCalDavOutboxItem &item)
{
    if (module.isNull() || item.localScheduleID.isEmpty() || item.conflictIcs.isEmpty()) {
        return true;
    }
    DAccountDataBase *database = module->accountDatabase();
    if (database == nullptr) {
        return false;
    }
    const DSchedule::Ptr currentSchedule = database->getScheduleByScheduleID(item.localScheduleID);
    DSchedule::Ptr snapshot;
    if (!DSchedule::fromIcsString(snapshot, item.conflictIcs) || snapshot.isNull()
        || currentSchedule.isNull()) {
        return false;
    }
    snapshot->setUid(item.localScheduleID);
    snapshot->setScheduleTypeID(currentSchedule->scheduleTypeID());
    return database->updateSchedule(snapshot);
}

void appendOriginalTypeToSummary(const DSchedule::Ptr &schedule, const QString &originalType)
{
    if (schedule.isNull() || originalType.trimmed().isEmpty()) {
        return;
    }
    const QString suffix = QCoreApplication::translate(
        "DAccountManageModule", "[Original Type: %1]").arg(originalType.trimmed());
    if (!schedule->summary().contains(suffix)) {
        schedule->setSummary(schedule->summary() + suffix);
    }
}

bool applyServerSnapshotToLocal(const DAccountModule::Ptr &module,
                                const DCalDavOutboxItem &item)
{
    if (module.isNull() || item.localScheduleID.isEmpty() || item.serverIcs.isEmpty()) {
        return false;
    }

    DAccountDataBase *database = module->accountDatabase();
    if (database == nullptr) {
        return false;
    }
    const DSchedule::Ptr currentSchedule = database->getScheduleByScheduleID(item.localScheduleID);
    DSchedule::Ptr serverSchedule;
    if (currentSchedule.isNull() || !DSchedule::fromIcsString(serverSchedule, item.serverIcs)
        || serverSchedule.isNull()) {
        return false;
    }
    serverSchedule->setUid(item.localScheduleID);
    serverSchedule->setScheduleTypeID(currentSchedule->scheduleTypeID());
    serverSchedule->setCreated(currentSchedule->created());
    return database->updateSchedule(serverSchedule);
}

/**
 * @brief Prepares a conflicted Outbox item for a local-version retry.
 * @param item Persisted conflict item whose server state must be discarded.
 * @return The retry item, using Modify for an already-created remote resource.
 */
DCalDavOutboxItem resetConflictOutboxItem(const DCalDavOutboxItem &item)
{
    DCalDavOutboxItem retryItem = item;
    retryItem.baseEtag.clear();
    retryItem.retryCount = 0;
    retryItem.nextRetryAt = QDateTime();
    retryItem.failureType = DCalDavOutboxItem::NoFailure;
    retryItem.conflictIcs.clear();
    retryItem.serverIcs.clear();
    if (retryItem.operationType == DCalDavOutboxItem::CreateOperation) {
        retryItem.operationType = DCalDavOutboxItem::ModifyOperation;
    }
    return retryItem;
}

bool applyServerCalDavConflict(const DAccountModule::Ptr &module,
                               DAccountManagerDataBase *accountManagerDatabase,
                               const DCalDavOutboxItem &item)
{
    if (module.isNull() || accountManagerDatabase == nullptr || item.operationID.isEmpty()
        || item.serverIcs.isEmpty()) {
        return false;
    }

    const DCalDavEventMappingInfo mapping = accountManagerDatabase
        ->getCalDavEventMappingByLocalScheduleID(item.accountID, item.localScheduleID);
    if (mapping.href.isEmpty() || !applyServerSnapshotToLocal(module, item)) {
        return false;
    }

    DCalDavEventMappingInfo updatedMapping = mapping;
    updatedMapping.originalIcs = item.serverIcs;
    // The conflict response may not include a reliable ETag. Fetch it before
    // the next write instead of reusing the stale pre-conflict value.
    updatedMapping.etag.clear();
    return accountManagerDatabase->upsertCalDavEventMapping(updatedMapping)
        && accountManagerDatabase->deleteCalDavOutboxItemIfCurrent(item);
}

} // namespace

DAccountManageModule::DAccountManageModule(QObject *parent)
    : QObject(parent)
    , m_syncFileManage(new SyncFileManage())
    , m_accountManagerDB(new DAccountManagerDataBase)
    , m_reginFormatConfig(DTK_CORE_NAMESPACE::DConfig::createGeneric("org.deepin.region-format", QString(), this))
    , m_settings( getAppConfigDir().filePath( "config.ini"), QSettings::IniFormat)
{
    qCDebug(ServiceLogger) << "DAccountManageModule constructor called.";
    if (m_reginFormatConfig->isValid()) {
        connect(m_reginFormatConfig,
                &DTK_CORE_NAMESPACE::DConfig::valueChanged,
                this,
                &DAccountManageModule::slotSettingChange);
    } else {
        connect(&m_timeDateDbus,
                &DBusTimedate::ShortTimeFormatChanged,
                this,
                &DAccountManageModule::slotSettingChange);
        connect(&m_timeDateDbus,
                &DBusTimedate::WeekBeginsChanged,
                this,
                &DAccountManageModule::slotSettingChange);
    }
    m_isSupportUid = m_syncFileManage->getSyncoperation()->hasAvailable();
    //新文件路径
    QString newDbPath = getDBPath();
    QString newDB(newDbPath + "/" + "accountmanager.db");
    qCDebug(ServiceLogger) << "Setting account manager DB path to:" << newDB;
    m_accountManagerDB->setDBPath(newDB);
    m_accountManagerDB->ensureSchema();
    resumeCalDavAccountDeletionCleanups();

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    //将云端帐户信息基本数据与本地数据合并
    qCDebug(ServiceLogger) << "Merging UnionID data.";
    unionIDDataMerging();

    //根据获取到的帐户信息创建对应的帐户服务
    qCDebug(ServiceLogger) << "Creating account services for" << m_accountList.size() << "accounts.";
    foreach (auto account, m_accountList) {
        //如果不支持云同步且帐户类型为UID则过滤
        if (!m_isSupportUid && account->accountType() == DAccount::Account_UnionID) {
            continue;
        }
        DAccountModule::Ptr accountModule = DAccountModule::Ptr(new DAccountModule(account, m_accountManagerDB.data()));
        QObject::connect(accountModule.data(), &DAccountModule::signalSettingChange, this, &DAccountManageModule::slotSettingChange);
        const QString accountID = account->accountID();
        QObject::connect(accountModule.data(), &DAccountModule::signalCalDavScheduleCreateFailed,
                         this, [this, accountID](int) {
            emit calDavAccountStatusChanged(accountID);
        });
        QObject::connect(accountModule.data(), &DAccountModule::signalCalDavLocalChange,
                         this, [this, accountID]() {
            qCDebug(ServiceLogger) << "Requesting immediate CalDAV sync after local change"
                                   << "accountID:" << accountID;
            if (!m_calDavSyncJobManager.requestSync(
                    accountID, DCalDavSyncStateMachine::LocalChangeTrigger)) {
                qCWarning(ServiceLogger) << "Failed to request immediate CalDAV sync"
                                         << "accountID:" << accountID;
            }
        });
        m_accountModuleMap[account->accountID()] = accountModule;
        DAccountService::Ptr accountService = DAccountService::Ptr(new DAccountService(account->dbusPath(), account->dbusInterface(), accountModule, this));
        if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(), accountService.data(), options)) {
            qCWarning(ServiceLogger) << "Failed to register account service - Account:" << account->accountID() 
                                     << "Error:" << sessionBus.lastError().message();
        } else {
            m_AccountServiceMap[account->accountType()].insert(account->accountID(), accountService);
            //如果是网络帐户则开启定时下载任务
            if (account->isNetWorkAccount() && account->accountType() != DAccount::Account_CalDav
                && account->accountState().testFlag(DAccount::Account_Open)) {
                qCDebug(ServiceLogger) << "Starting download task for network account:" << account->accountID();
                accountModule->downloadTaskhanding(0);
            }
        }
    }
    m_generalSetting = getGeneralSettings();

    connect(&m_timer, &QTimer::timeout, this, &DAccountManageModule::slotClientIsOpen);
    m_timer.start(2000);
    QTimer::singleShot(0, this, &DAccountManageModule::registerCalDavAccounts);

    connect(&m_calDavDailyTimer, &QTimer::timeout,
            this, &DAccountManageModule::slotCalDavDailySync);
    scheduleNextCalDavDailySync();
    connect(&m_calDavRetryTimer, &QTimer::timeout,
            this, &DAccountManageModule::slotCalDavRetry);
    scheduleNextCalDavRetry();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (QNetworkInformation::loadDefaultBackend()) {
        if (QNetworkInformation *networkInformation = QNetworkInformation::instance()) {
            connect(networkInformation, &QNetworkInformation::reachabilityChanged, this,
                    [this](QNetworkInformation::Reachability reachability) {
                slotCalDavOnlineStateChanged(
                    reachability == QNetworkInformation::Reachability::Online);
            });
        }
    }
#else
    connect(&m_networkConfigurationManager, &QNetworkConfigurationManager::onlineStateChanged,
            this, &DAccountManageModule::slotCalDavOnlineStateChanged);
#endif

    if (m_isSupportUid) {
        qCDebug(ServiceLogger) << "UID is supported, connecting signals.";
        QObject::connect(m_syncFileManage->getSyncoperation(), &Syncoperation::signalLoginStatusChange, this, &DAccountManageModule::slotUidLoginStatueChange);
        QObject::connect(m_syncFileManage->getSyncoperation(), &Syncoperation::SwitcherChange, this, &DAccountManageModule::slotSwitcherChange);
    }

    //第一次启动加载完成后发送帐户改变信号
    // emit signalLoginStatusChange();
    connect(&m_calDavSyncJobManager, &DCalDavSyncJobManager::accountSyncStateChanged,
            this, [this](const QString &accountID, DCalDavSyncStateMachine::State) {
        emit calDavAccountStatusChanged(accountID);
    });
    connect(&m_calDavSyncJobManager, &DCalDavSyncJobManager::accountSyncDataChanged,
            this, [this](const QString &accountID) {
        const DAccountModule::Ptr accountModule = m_accountModuleMap.value(accountID);
        if (!accountModule.isNull()) {
            accountModule->notifyScheduleDataChanged();
        }
    });
    connect(&m_calDavSyncJobManager, &DCalDavSyncJobManager::accountScheduleCreateFailed,
            this, [this](const QString &accountID, int createFailure) {
        const DAccountModule::Ptr accountModule = m_accountModuleMap.value(accountID);
        if (!accountModule.isNull()) {
            accountModule->notifyCalDavScheduleCreateFailed(createFailure);
        }
    });
    connect(&m_calDavSyncJobManager, &DCalDavSyncJobManager::accountSyncFinished,
            this, [this](const QString &, bool, const QString &,
                        const DCalDavTransport::Response &) {
        scheduleNextCalDavRetry();
    });

    qCDebug(ServiceLogger) << "DAccountManageModule constructed.";
}

QString DAccountManageModule::getAccountList()
{
    qCDebug(ServiceLogger) << "Getting account list as JSON string.";
    QString accountStr;
    DAccount::toJsonListString(m_accountList, accountStr);
    return accountStr;
}

QString DAccountManageModule::getCalendarGeneralSettings()
{
    qCDebug(ServiceLogger) << "Getting calendar general settings as JSON string.";
    QString cgSetStr;
    m_generalSetting = getGeneralSettings();
    DCalendarGeneralSettings::toJsonString(m_generalSetting, cgSetStr);
    return cgSetStr;
}

void DAccountManageModule::setCalendarGeneralSettings(const QString &cgSet)
{
    qCDebug(ServiceLogger) << "Setting calendar general settings from JSON:" << cgSet;
    DCalendarGeneralSettings::Ptr cgSetPtr = DCalendarGeneralSettings::Ptr(new DCalendarGeneralSettings);
    DCalendarGeneralSettings::fromJsonString(cgSetPtr, cgSet);
    if (m_generalSetting != cgSetPtr) {
        setGeneralSettings(cgSetPtr);
        DCalendarGeneralSettings::Ptr tmpSetting = DCalendarGeneralSettings::Ptr(m_generalSetting->clone());
        m_generalSetting = cgSetPtr;
        if (tmpSetting->firstDayOfWeek() != m_generalSetting->firstDayOfWeek()) {
            qCDebug(ServiceLogger) << "First day of week changed from" << tmpSetting->firstDayOfWeek() 
                                   << "to" << m_generalSetting->firstDayOfWeek();
            emit firstDayOfWeekChange();
        }
        if (tmpSetting->timeShowType() != m_generalSetting->timeShowType()) {
            qCDebug(ServiceLogger) << "Time format type changed from" << tmpSetting->timeShowType() 
                                   << "to" << m_generalSetting->timeShowType();
            emit timeFormatTypeChange();
        }
    }
}

int DAccountManageModule::getfirstDayOfWeek()
{
    // qCDebug(ServiceLogger) << "Getting first day of week.";
    return static_cast<int>(m_generalSetting->firstDayOfWeek());
}

void DAccountManageModule::setFirstDayOfWeek(const int firstday)
{
    qCDebug(ServiceLogger) << "Setting first day of week to:" << firstday;
    if (m_generalSetting->firstDayOfWeek() != firstday) {
        m_generalSetting->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(firstday));
        setGeneralSettings(m_generalSetting);
        foreach (auto account, m_accountList) {
            if (account->accountType() == DAccount::Account_UnionID) {
                m_accountModuleMap[account->accountID()]->accountDownload();
            }
        }
    }
}

int DAccountManageModule::getTimeFormatType()
{
    // qCDebug(ServiceLogger) << "Getting time format type.";
    return static_cast<int>(m_generalSetting->timeShowType());
}

void DAccountManageModule::setTimeFormatType(const int timeType)
{
    qCDebug(ServiceLogger) << "Setting time format type to:" << timeType;
    if (m_generalSetting->timeShowType() != timeType) {
        m_generalSetting->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(timeType));
        setGeneralSettings(m_generalSetting);
        foreach (auto account, m_accountList) {
            if (account->accountType() == DAccount::Account_UnionID) {
                m_accountModuleMap[account->accountID()]->accountDownload();
            }
        }
    }
}

void DAccountManageModule::remindJob(const QString &accountID, const QString &alarmID)
{
    qCDebug(ServiceLogger) << "Executing remind job for account:" << accountID << "alarm:" << alarmID;
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->remindJob(alarmID);
    }
}

void DAccountManageModule::updateRemindSchedules(bool isClear)
{
    qCDebug(ServiceLogger) << "Updating remind schedules for all accounts. isClear:" << isClear;
    QMap<QString, DAccountModule::Ptr>::const_iterator iter = m_accountModuleMap.constBegin();
    for (; iter != m_accountModuleMap.constEnd(); ++iter) {
        iter.value()->updateRemindSchedules(isClear);
    }
}

void DAccountManageModule::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
{
    qCDebug(ServiceLogger) << "Handling notification message for account:" << accountID << "alarm:" << alarmID << "operation:" << operationNum;
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->notifyMsgHanding(alarmID, operationNum);
    }
}

void DAccountManageModule::downloadByAccountID(const QString &accountID)
{
    qCDebug(ServiceLogger) << "Triggering download for account:" << accountID;
    const DAccount::Ptr account = m_accountManagerDB->getAccountByID(accountID);
    if (account && account->accountType() == DAccount::Account_CalDav) {
        m_calDavSyncJobManager.requestSync(accountID, DCalDavSyncStateMachine::ManualTrigger);
        return;
    }
    if (m_accountModuleMap.contains(accountID)) {
        m_accountModuleMap[accountID]->accountDownload();
    }
}

QString DAccountManageModule::getCalDavAccountStatusList()
{
    return m_accountManagerDB->getCalDavAccountStatusList();
}

QString DAccountManageModule::getCalDavAccountConfig(const QString &accountID)
{
    DCalDavAccountInfo accountInfo;
    const DAccount::Ptr account = m_accountManagerDB->getAccountByID(accountID);
    if (account.isNull() || account->accountType() != DAccount::Account_CalDav
        || !m_accountManagerDB->getCalDavAccountInfo(accountID, accountInfo)) {
        return QString();
    }

    QJsonObject object;
    object.insert(QStringLiteral("accountID"), accountInfo.accountId);
    object.insert(QStringLiteral("providerType"), accountInfo.providerType);
    object.insert(QStringLiteral("serverUrl"), accountInfo.serverUrl);
    object.insert(QStringLiteral("username"), accountInfo.username);
    object.insert(QStringLiteral("accountColor"), accountInfo.accountColor);
    object.insert(QStringLiteral("displayName"), account->displayName());
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QString DAccountManageModule::validateCalDavAccountForUpdate(
    const QString &accountID, int providerType, const QString &serverUrl,
    const QString &username, const QString &credentialRef)
{
    DCalDavAccountInfo oldInfo;
    const DAccount::Ptr account = m_accountManagerDB->getAccountByID(accountID);
    if (account.isNull() || account->accountType() != DAccount::Account_CalDav
        || !m_accountManagerDB->getCalDavAccountInfo(accountID, oldInfo)) {
        return QString();
    }

    const QString effectiveCredentialRef = credentialRef.isEmpty()
        ? oldInfo.credentialRef : credentialRef;
    return validateCalDavAccount(providerType, serverUrl, username, effectiveCredentialRef);
}

QString DAccountManageModule::validateCalDavAccount(int providerType, const QString &serverUrl,
                                                     const QString &username, const QString &credentialRef)
{
    const QString requestID = DDataBase::createUuid();
    const auto finishValidationFailure = [this, requestID](
        DCalDavValidationError::Type validationError) {
        QTimer::singleShot(0, this, [this, requestID, validationError]() {
            emit calDavAccountValidationFinished(requestID, false,
                                                 static_cast<int>(validationError), QString(), QString());
        });
    };

    const QUrl normalizedServerUrl = DCalDavProviderProfile::normalizeServerUrl(serverUrl);
    if (providerType < DCalDavProviderProfile::Provider_DingTalk
        || providerType > DCalDavProviderProfile::Provider_Other
        || !normalizedServerUrl.isValid()
        || username.trimmed().isEmpty() || credentialRef.isEmpty()) {
        DCalendarEventLog::instance().reportLoginValidationFinished(
            false, DCalDavValidationError::Other);
        finishValidationFailure(DCalDavValidationError::Other);
        return requestID;
    }

    QString password;
    QString errorMessage;
    if (!DCalDavCredentialStore::readPassword(credentialRef, password, &errorMessage)) {
        qCWarning(ServiceLogger) << "Failed to read CalDAV validation credential"
                                 << "errorPresent:" << !errorMessage.isEmpty();
        DCalendarEventLog::instance().reportLoginValidationFinished(
            false, DCalDavValidationError::Other);
        finishValidationFailure(DCalDavValidationError::Other);
        return requestID;
    }

    DCalDavReadOnlySync *validator = new DCalDavReadOnlySync(this);
    m_calDavValidationJobs.insert(requestID, validator);

    DCalDavReadOnlySync::Request request;
    request.serverUrl = normalizedServerUrl;
    request.username = username.trimmed();
    request.password = password;
    password.clear();
    validator->start(request, [this, requestID, validator](const DCalDavReadOnlySync::Result &result) {
        DCalendarEventLog::instance().reportLoginValidationFinished(
            result.success, result.validationError);
        m_calDavValidationJobs.remove(requestID);
        validator->deleteLater();
        emit calDavAccountValidationFinished(requestID, result.success,
                                             static_cast<int>(result.validationError),
                                             result.errorMessage,
                                             result.discovery.principalDisplayName);
    });
    return requestID;
}

QString DAccountManageModule::createCalDavAccount(int providerType, const QString &serverUrl,
                                                   const QString &username, const QString &credentialRef,
                                                   const QString &displayName)
{
    const QUrl normalizedServerUrl = DCalDavProviderProfile::normalizeServerUrl(serverUrl);
    if (providerType < DCalDavProviderProfile::Provider_DingTalk
        || providerType > DCalDavProviderProfile::Provider_Other
        || !normalizedServerUrl.isValid()
        || username.trimmed().isEmpty() || credentialRef.isEmpty()) {
        return QString();
    }

    QString password;
    QString errorMessage;
    if (!DCalDavCredentialStore::readPassword(credentialRef, password, &errorMessage)) {
        qCWarning(ServiceLogger) << "Failed to read CalDAV account credential"
                                 << "errorPresent:" << !errorMessage.isEmpty();
        return QString();
    }
    password.clear();

    const DCalDavProviderProfile profile = DCalDavProviderProfile::forProvider(
        static_cast<DCalDavProviderProfile::ProviderType>(providerType));
    const QString accountDisplayName = displayName.trimmed().isEmpty()
        ? profile.displayName : displayName.trimmed();

    const QString normalizedUsername = username.trimmed();
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountName(normalizedUsername);
    account->setDisplayName(accountDisplayName);
    initAccountDBusInfo(account);
    SqlTransactionLocker accountTransaction({DDataBase::NameAccountManager});
    if (!accountTransaction.isValid() || m_accountManagerDB->addAccountInfo(account).isEmpty()) {
        return QString();
    }

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = providerType;
    accountInfo.serverUrl = normalizedServerUrl.toString();
    accountInfo.username = normalizedUsername;
    accountInfo.credentialRef = credentialRef;
    accountInfo.accountColor = defaultCalDavAccountColor(account->accountID());
    if (!m_accountManagerDB->upsertCalDavAccountInfo(accountInfo)
        || !accountTransaction.commit()) {
        return QString();
    }

    DAccountModule::Ptr accountModule(new DAccountModule(account, m_accountManagerDB.data()));
    const QString accountID = account->accountID();
    QObject::connect(accountModule.data(), &DAccountModule::signalCalDavScheduleCreateFailed,
                     this, [this, accountID](int) {
        emit calDavAccountStatusChanged(accountID);
    });
    QObject::connect(accountModule.data(), &DAccountModule::signalCalDavLocalChange,
                     this, [this, accountID]() {
        qCDebug(ServiceLogger) << "Requesting immediate CalDAV sync after local change"
                               << "accountID:" << accountID;
        if (!m_calDavSyncJobManager.requestSync(
                accountID, DCalDavSyncStateMachine::LocalChangeTrigger)) {
            qCWarning(ServiceLogger) << "Failed to request immediate CalDAV sync"
                                     << "accountID:" << accountID;
        }
    });
    DAccountService::Ptr accountService(new DAccountService(
        account->dbusPath(), account->dbusInterface(), accountModule, this));
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    const QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots
        | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(),
                                   accountService.data(), options)) {
        accountModule->removeDB();
        SqlTransactionLocker cleanupTransaction({DDataBase::NameAccountManager});
        if (cleanupTransaction.isValid()
            && m_accountManagerDB->deleteCalDavAccountInfo(account->accountID())
            && m_accountManagerDB->deleteAccountInfo(account->accountID())) {
            cleanupTransaction.commit();
        }
        return QString();
    }

    m_accountList.append(account);
    m_accountModuleMap.insert(account->accountID(), accountModule);
    m_AccountServiceMap[account->accountType()].insert(account->accountID(), accountService);
    registerCalDavAccount(account);
    emit signalLoginStatusChange();
    return account->accountID();
}

bool DAccountManageModule::updateCalDavAccount(const QString &accountID, int providerType,
                                                 const QString &serverUrl, const QString &username,
                                                 const QString &credentialRef, const QString &displayName)
{
    const QUrl normalizedServerUrl = DCalDavProviderProfile::normalizeServerUrl(serverUrl);
    const QString normalizedUsername = username.trimmed();
    if (providerType < DCalDavProviderProfile::Provider_DingTalk
        || providerType > DCalDavProviderProfile::Provider_Other
        || !normalizedServerUrl.isValid() || normalizedUsername.isEmpty()
        || !m_accountModuleMap.contains(accountID) || m_calDavRegistrars.contains(accountID)) {
        return false;
    }
    const DAccountModule::Ptr accountModule = m_accountModuleMap.value(accountID);
    const DAccount::Ptr account = accountModule->account();
    if (account.isNull() || account->accountType() != DAccount::Account_CalDav) {
        return false;
    }

    DCalDavAccountInfo oldInfo;
    if (!m_accountManagerDB->getCalDavAccountInfo(accountID, oldInfo)) {
        return false;
    }
    if (DCalDavProviderProfile::normalizeServerUrl(oldInfo.serverUrl) != normalizedServerUrl) {
        qCWarning(ServiceLogger) << "Changing the CalDAV server requires removing and re-adding the account.";
        return false;
    }

    const DCalDavProviderProfile profile = DCalDavProviderProfile::forProvider(
        static_cast<DCalDavProviderProfile::ProviderType>(providerType));
    const QString newDisplayName = displayName.trimmed().isEmpty()
        ? profile.displayName : displayName.trimmed();
    DCalDavAccountInfo newInfo = oldInfo;
    newInfo.providerType = providerType;
    newInfo.serverUrl = normalizedServerUrl.toString();
    newInfo.username = normalizedUsername;

    if (!credentialRef.isEmpty()) {
        QString password;
        QString errorMessage;
        if (!DCalDavCredentialStore::readPassword(credentialRef, password, &errorMessage)) {
            qCWarning(ServiceLogger) << "Failed to read CalDAV update credential"
                                     << "errorPresent:" << !errorMessage.isEmpty();
            return false;
        }
        password.clear();
        newInfo.credentialRef = credentialRef;
    }

    SqlTransactionLocker accountTransaction({DDataBase::NameAccountManager});
    if (!accountTransaction.isValid() || !m_accountManagerDB->upsertCalDavAccountInfo(newInfo)) {
        return false;
    }

    const QString oldAccountName = account->accountName();
    const QString oldDisplayName = account->displayName();
    account->setAccountName(normalizedUsername);
    account->setDisplayName(newDisplayName);
    if (!m_accountManagerDB->updateAccountInfo(account) || !accountTransaction.commit()) {
        account->setAccountName(oldAccountName);
        account->setDisplayName(oldDisplayName);
        return false;
    }

    if (!credentialRef.isEmpty() && credentialRef != oldInfo.credentialRef
        && !oldInfo.credentialRef.isEmpty()
        && !DCalDavCredentialStore::deletePassword(oldInfo.credentialRef)) {
        qCWarning(ServiceLogger) << "CalDAV account was updated but its previous credential could not be removed.";
    }

    registerCalDavAccount(account);
    emit signalLoginStatusChange();
    emit calDavAccountStatusChanged(accountID);
    return true;
}

bool DAccountManageModule::deleteCalDavAccount(const QString &accountID)
{
    return deleteCalDavAccountInternal(accountID, true);
}

bool DAccountManageModule::deleteCalDavAccountWithLocalDataOption(const QString &accountID,
                                                                    bool deleteLocalData)
{
    return deleteCalDavAccountInternal(accountID, deleteLocalData);
}

bool DAccountManageModule::resolveAllCalDavConflicts(const QString &accountID, bool keepLocal)
{
    if (!m_accountModuleMap.contains(accountID)) {
        return false;
    }
    const DCalDavOutboxItem::List conflicts = m_accountManagerDB->getCalDavConflictItems(accountID);
    if (conflicts.isEmpty()) {
        return false;
    }
    const DAccountModule::Ptr module = m_accountModuleMap.value(accountID);
    if (module.isNull() || module->accountDatabase() == nullptr) {
        return false;
    }
    std::unique_ptr<SqlTransactionLocker> transaction(new SqlTransactionLocker(
        {module->accountDatabase()->getConnectionName(), DDataBase::NameAccountManager}));
    if (!transaction->isValid()) {
        return false;
    }

    for (const DCalDavOutboxItem &item : conflicts) {
        bool success = false;
        if (keepLocal) {
            if (!restoreCalDavConflictSnapshot(module, item)) {
                transaction->rollback();
                return false;
            }
            success = m_accountManagerDB->upsertCalDavOutboxItem(
                resetConflictOutboxItem(item));
        } else {
            success = applyServerCalDavConflict(module, m_accountManagerDB.data(), item);
        }
        if (!success) {
            // Abort all still-active database transactions immediately. The
            // locker destructor remains a safety net for other early returns.
            transaction->rollback();
            return false;
        }
    }
    if (!transaction->commit()) {
        if (!keepLocal) {
            for (const DCalDavOutboxItem &item : conflicts) {
                if (!applyServerSnapshotToLocal(module, item)) {
                    qCWarning(ServiceLogger)
                        << "Failed to restore server conflict snapshot after transaction failure.";
                }
            }
        }
        return false;
    }

    module->notifyScheduleDataChanged();
    m_calDavSyncJobManager.requestSync(accountID, DCalDavSyncStateMachine::ManualTrigger);
    emit calDavAccountStatusChanged(accountID);
    return true;
}

bool DAccountManageModule::resolveCalDavConflict(const QString &accountID,
                                                   const QString &localScheduleID,
                                                   bool keepLocal)
{
    if (!m_accountModuleMap.contains(accountID) || localScheduleID.isEmpty()) {
        return false;
    }
    const DCalDavOutboxItem item = m_accountManagerDB->getCalDavOutboxItem(
        accountID, localScheduleID);
    if (item.operationID.isEmpty() || item.failureType != DCalDavOutboxItem::ConflictFailure) {
        return false;
    }
    const DAccountModule::Ptr module = m_accountModuleMap.value(accountID);
    if (module.isNull() || module->accountDatabase() == nullptr) {
        return false;
    }
    std::unique_ptr<SqlTransactionLocker> transaction(new SqlTransactionLocker(
        {module->accountDatabase()->getConnectionName(), DDataBase::NameAccountManager}));
    if (!transaction->isValid()) {
        return false;
    }

    bool success = false;
    if (keepLocal) {
        if (!restoreCalDavConflictSnapshot(module, item)) {
            return false;
        }
        success = m_accountManagerDB->upsertCalDavOutboxItem(
            resetConflictOutboxItem(item));
    } else {
        success = applyServerCalDavConflict(module, m_accountManagerDB.data(), item);
    }
    if (success && !transaction->commit()) {
        if (!keepLocal) {
            success = applyServerSnapshotToLocal(module, item);
            if (!success) {
                qCWarning(ServiceLogger)
                    << "Failed to restore server conflict snapshot after transaction failure.";
            }
        } else {
            success = false;
        }
    }
    if (success) {
        module->notifyScheduleDataChanged();
        m_calDavSyncJobManager.requestSync(accountID, DCalDavSyncStateMachine::ManualTrigger);
        emit calDavAccountStatusChanged(accountID);
    }
    return success;
}

bool DAccountManageModule::migrateCalDavSchedulesToLocal(
    const DAccountModule::Ptr &calDavModule, const DAccountModule::Ptr &localModule,
    QStringList *createdScheduleIDs, QStringList *createdScheduleTypeIDs)
{
    if (calDavModule.isNull() || localModule.isNull() || createdScheduleIDs == nullptr
        || createdScheduleTypeIDs == nullptr) {
        return false;
    }

    DAccountDataBase *sourceDatabase = calDavModule->accountDatabase();
    DAccountDataBase *targetDatabase = localModule->accountDatabase();
    if (sourceDatabase == nullptr || targetDatabase == nullptr) {
        return false;
    }

    Q_UNUSED(createdScheduleTypeIDs);
    const DScheduleType::List sourceTypes = sourceDatabase->getScheduleTypeList();
    for (const DScheduleType::Ptr &sourceType : sourceTypes) {
        if (sourceType.isNull()) {
            continue;
        }

        const DSchedule::List sourceSchedules = sourceDatabase->getScheduleListByTypeID(
            sourceType->typeID());
        for (const DSchedule::Ptr &sourceSchedule : sourceSchedules) {
            if (sourceSchedule.isNull()) {
                continue;
            }

            QString unmatchedOriginalType;
            const QString targetTypeID = localTypeIDForCategories(
                targetDatabase, sourceSchedule->categories(), &unmatchedOriginalType);
            if (targetTypeID.isEmpty()) {
                return false;
            }

            DSchedule::Ptr targetSchedule(new DSchedule(*sourceSchedule));
            targetSchedule->setScheduleTypeID(targetTypeID);
            // Keep the remote UID as the local copy's stable identity. This
            // allows a later remove-and-readd cycle to recognize the same
            // event and avoid creating a second local copy.
            const QString localScheduleID = sourceSchedule->uid();
            if (localScheduleID.isEmpty()) {
                return false;
            }
            if (targetDatabase->scheduleExistsByScheduleID(localScheduleID)) {
                continue;
            }
            targetSchedule->setSchedulingID(localScheduleID, localScheduleID);
            appendOriginalTypeToSummary(targetSchedule, unmatchedOriginalType);
            const QString targetScheduleID = targetDatabase->createSchedule(targetSchedule);
            if (targetScheduleID.isEmpty()) {
                return false;
            }
            createdScheduleIDs->append(targetScheduleID);
        }
    }
    return true;
}

bool DAccountManageModule::deleteCalDavAccountInternal(const QString &accountID,
                                                         bool deleteLocalData)
{
    if (!m_accountModuleMap.contains(accountID) || m_calDavRegistrars.contains(accountID)) {
        return false;
    }
    const DAccountModule::Ptr accountModule = m_accountModuleMap.value(accountID);
    const DAccount::Ptr account = accountModule->account();
    DCalDavAccountInfo accountInfo;
    if (account.isNull() || account->accountType() != DAccount::Account_CalDav
        || !m_accountManagerDB->getCalDavAccountInfo(accountID, accountInfo)) {
        return false;
    }
    if (!m_calDavSyncJobManager.cancelAccount(accountID)) {
        return false;
    }
    const auto restoreSyncJob = [&]() {
        registerCalDavAccount(account);
    };

    QStringList createdScheduleIDs;
    QStringList createdScheduleTypeIDs;
    DAccountModule::Ptr localModule;
    const auto rollbackLocalMigration = [&]() {
        if (localModule.isNull()) {
            return;
        }
        DAccountDataBase *database = localModule->accountDatabase();
        for (const QString &scheduleID : createdScheduleIDs) {
            database->deleteScheduleByScheduleID(scheduleID, 1);
        }
        for (const QString &typeID : createdScheduleTypeIDs) {
            database->deleteScheduleTypeByID(typeID);
        }
    };

    if (!deleteLocalData) {
        for (const DAccount::Ptr &candidate : m_accountList) {
            if (candidate && candidate->accountType() == DAccount::Account_Local) {
                localModule = m_accountModuleMap.value(candidate->accountID());
                break;
            }
        }
        if (localModule.isNull()
            || !migrateCalDavSchedulesToLocal(accountModule, localModule, &createdScheduleIDs,
                                              &createdScheduleTypeIDs)) {
            rollbackLocalMigration();
            restoreSyncJob();
            return false;
        }
    }

    if (!m_accountManagerDB->upsertCalDavAccountDeletionCleanup(accountID, account->dbName())) {
        rollbackLocalMigration();
        restoreSyncJob();
        return false;
    }
    if (!m_accountManagerDB->deleteCalDavAccountData(accountID)) {
        m_accountManagerDB->deleteCalDavAccountDeletionCleanup(accountID);
        rollbackLocalMigration();
        restoreSyncJob();
        return false;
    }

    if (!accountInfo.credentialRef.isEmpty()
        && !DCalDavCredentialStore::deletePassword(accountInfo.credentialRef)) {
        qCWarning(ServiceLogger) << "CalDAV account was deleted but its credential could not be removed.";
    }

    QDBusConnection::sessionBus().unregisterObject(account->dbusPath());
    m_AccountServiceMap[account->accountType()].remove(accountID);
    m_accountModuleMap.remove(accountID);
    m_accountList.removeOne(account);
    const bool databaseRemoved = accountModule->removeDB();
    if (databaseRemoved) {
        m_accountManagerDB->deleteCalDavAccountDeletionCleanup(accountID);
    } else {
        qCWarning(ServiceLogger) << "CalDAV account data database removal is pending"
                                 << "accountID:" << accountID
                                 << "database:" << account->dbName();
    }
    if (!deleteLocalData && !localModule.isNull()) {
        localModule->notifyScheduleDataChanged();
    }
    emit signalLoginStatusChange();
    return true;
}

void DAccountManageModule::resumeCalDavAccountDeletionCleanups()
{
    DCalDavAccountDeletionCleanup::resume(m_accountManagerDB.data(), getDBPath());
}

bool DAccountManageModule::updateCalDavCredentialReference(const QString &accountID, const QString &credentialRef)
{
    const bool updated = m_accountManagerDB->updateCalDavCredentialReference(accountID, credentialRef);
    if (updated) {
        const DAccount::Ptr account = m_accountManagerDB->getAccountByID(accountID);
        if (account && account->accountType() == DAccount::Account_CalDav
            && m_calDavSyncJobManager.stateFor(accountID) != DCalDavSyncStateMachine::Running) {
            registerCalDavAccount(account);
        }
        emit calDavAccountStatusChanged(accountID);
    }
    return updated;
}

void DAccountManageModule::scheduleNextCalDavDailySync()
{
    const QDateTime now = QDateTime::currentDateTime();
    QDateTime next(now.date(), QTime(9, 0));
    if (next <= now) {
        next = next.addDays(1);
    }
    m_calDavDailyTimer.start(static_cast<int>(now.msecsTo(next)));
}

void DAccountManageModule::slotCalDavDailySync()
{
    m_calDavSyncJobManager.requestSyncForAll(DCalDavSyncStateMachine::DailyTrigger);
    scheduleNextCalDavDailySync();
}

void DAccountManageModule::scheduleNextCalDavRetry()
{
    const QDateTime retryAt = m_accountManagerDB->earliestCalDavRetryAt();
    if (!retryAt.isValid()) {
        m_calDavRetryTimer.stop();
        return;
    }

    const qint64 delay = QDateTime::currentDateTimeUtc().msecsTo(retryAt.toUTC());
    const qint64 boundedDelay = qBound<qint64>(
        1, delay, static_cast<qint64>(std::numeric_limits<int>::max()));
    m_calDavRetryTimer.start(static_cast<int>(boundedDelay));
}

void DAccountManageModule::slotCalDavRetry()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    QStringList accountIDs = m_accountManagerDB->dueCalDavAccountRetryIDs(now);
    const QStringList outboxAccountIDs = m_accountManagerDB->dueCalDavOutboxAccountIDs(now);
    for (const QString &accountID : outboxAccountIDs) {
        if (!accountIDs.contains(accountID)) {
            accountIDs.append(accountID);
        }
    }
    for (const QString &accountID : accountIDs) {
        if (m_calDavSyncJobManager.containsAccount(accountID)) {
            m_calDavSyncJobManager.requestSync(accountID, DCalDavSyncStateMachine::RetryTrigger);
            continue;
        }
        for (const DAccount::Ptr &account : m_accountList) {
            if (!account.isNull() && account->accountID() == accountID) {
                registerCalDavAccount(account);
                break;
            }
        }
    }
    scheduleNextCalDavRetry();
}

void DAccountManageModule::slotCalDavOnlineStateChanged(bool isOnline)
{
    if (isOnline) {
        m_calDavSyncJobManager.requestSyncForAll(DCalDavSyncStateMachine::NetworkRestoredTrigger);
    }
}

void DAccountManageModule::registerCalDavAccounts()
{
    for (const DAccount::Ptr &account : m_accountList) {
        if (account->accountType() == DAccount::Account_CalDav) {
            registerCalDavAccount(account);
        }
    }
}

void DAccountManageModule::registerCalDavAccount(const DAccount::Ptr &account)
{
    if (account.isNull() || account->accountID().isEmpty()
        || !m_accountModuleMap.contains(account->accountID())
        || m_calDavRegistrars.contains(account->accountID())) {
        return;
    }

    DCalDavAccountInfo accountInfo;
    if (!m_accountManagerDB->getCalDavAccountInfo(account->accountID(), accountInfo)) {
        m_accountManagerDB->updateCalDavSyncStatus(
            account->accountID(), DCalDavSyncStatus::Failed, QDateTime(),
            QStringLiteral("CalDAV account configuration is missing."),
            DCalDavErrorCode::StorageError);
        emit calDavAccountStatusChanged(account->accountID());
        return;
    }
    if (accountInfo.nextRetryAt.isValid()
        && accountInfo.nextRetryAt > QDateTime::currentDateTimeUtc()) {
        return;
    }

    const DAccountModule::Ptr accountModule = m_accountModuleMap.value(account->accountID());

    DCalDavAccountRegistrar::Request request;
    request.account = accountInfo;
    request.localDatabase = accountModule->accountDatabase();
    request.accountManagerDatabase = m_accountManagerDB.data();
    request.jobManager = &m_calDavSyncJobManager;

    const QString accountID = account->accountID();
    DCalDavAccountRegistrar *registrar = new DCalDavAccountRegistrar(this);
    m_calDavRegistrars.insert(accountID, registrar);
    registrar->start(request, [this, accountID, registrar](
                         const DCalDavAccountRegistrar::Result &result) {
        m_calDavRegistrars.remove(accountID);
        if (result.success) {
            const DAccountModule::Ptr accountModule = m_accountModuleMap.value(accountID);
            if (!accountModule.isNull()) {
                accountModule->notifyScheduleDataChanged();
            }
        } else {
            DCalDavAccountInfo accountInfo;
            if (m_accountManagerDB->getCalDavAccountInfo(accountID, accountInfo)
                && result.failureResponse.error != DCalDavTransport::NoError) {
                const DCalDavRetryPolicy::Decision retry = DCalDavRetryPolicy::decide(
                    result.failureResponse, accountInfo.retryCount);
                if (retry.retry) {
                    m_accountManagerDB->updateCalDavRetryState(
                        accountID, accountInfo.retryCount + 1,
                        QDateTime::currentDateTimeUtc().addSecs(retry.delaySeconds),
                        result.errorMessage, result.failureCode);
                } else {
                    m_accountManagerDB->updateCalDavSyncStatus(
                        accountID, DCalDavSyncStatus::fromErrorCode(result.failureCode),
                        QDateTime(), result.errorMessage, result.failureCode);
                }
            } else {
                m_accountManagerDB->updateCalDavSyncStatus(
                    accountID, DCalDavSyncStatus::fromErrorCode(result.failureCode),
                    QDateTime(), result.errorMessage, result.failureCode);
            }
        }
        scheduleNextCalDavRetry();
        emit calDavAccountStatusChanged(accountID);
        registrar->deleteLater();
    });
}

void DAccountManageModule::uploadNetWorkAccountData()
{
    qCDebug(ServiceLogger) << "Uploading network account data for all accounts.";
    QMap<QString, DAccountModule::Ptr>::const_iterator iter = m_accountModuleMap.constBegin();
    for (; iter != m_accountModuleMap.constEnd(); ++iter) {
        if (iter.value()->account()->accountType() == DAccount::Account_CalDav) {
            continue;
        }
        iter.value()->uploadNetWorkAccountData();
    }
}

//账户登录
void DAccountManageModule::login()
{
    qCDebug(ServiceLogger) << "Login requested.";
    m_syncFileManage->getSyncoperation()->optlogin();
}
//账户登出
void DAccountManageModule::logout()
{
    qCDebug(ServiceLogger) << "Logout requested.";
    m_syncFileManage->getSyncoperation()->optlogout();
}

bool DAccountManageModule::isSupportUid()
{
    // qCDebug(ServiceLogger) << "Checking UID support. Supported:" << m_isSupportUid;
    return m_isSupportUid;
}

void DAccountManageModule::calendarOpen(bool isOpen)
{
    qCDebug(ServiceLogger) << "Calendar open status changed:" << isOpen;
    //每次开启日历时需要同步数据
    if (isOpen) {
        QMap<QString, DAccountModule::Ptr>::iterator iter = m_accountModuleMap.begin();
        for (; iter != m_accountModuleMap.end(); ++iter) {
            if (iter.value()->account()->accountType() == DAccount::Account_CalDav) {
                continue;
            }
            iter.value()->accountDownload();
        }
        m_calDavSyncJobManager.requestSyncForAll(DCalDavSyncStateMachine::ForegroundTrigger);
    }
}

void DAccountManageModule::unionIDDataMerging()
{
    qCDebug(ServiceLogger) << "Starting UnionID data merging process.";
    m_accountList = m_accountManagerDB->getAccountList();

    //如果不支持云同步
    if (!m_isSupportUid) {
        DAccount::Ptr unionidDB;
        auto hasUnionid = [ =, &unionidDB](const DAccount::Ptr & account) {
            if (account->accountType() == DAccount::Account_UnionID) {
                unionidDB = account;
                return true;
            }
            return false;
        };
        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            //如果包含则移除
            removeUIdAccount(unionidDB);
        }
        return;
    }

    DAccount::Ptr accountUnionid = m_syncFileManage->getuserInfo();
    qCDebug(ServiceLogger) << "Fetched UnionID user info. Account ID:" << (accountUnionid.isNull() ? "null" : accountUnionid->accountID());

    DAccount::Ptr unionidDB;
    auto hasUnionid = [ =, &unionidDB](const DAccount::Ptr & account) {
        if (account->accountType() == DAccount::Account_UnionID) {
            unionidDB = account;
            return true;
        }
        return false;
    };
    //如果unionid帐户不存在，则判断数据库中是否有登陆前的信息
    //若有则移除
    if (accountUnionid.isNull() || accountUnionid->accountID().isEmpty()) {
        qCDebug(ServiceLogger) << "No active UnionID session. Checking for stale UID account in DB.";
        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            qCDebug(ServiceLogger) << "Removing existing UID account:" << unionidDB->accountID();
            removeUIdAccount(unionidDB);
        }
    } else {
        //如果unionID登陆了

        //如果数据库中有unionid帐户
        if (std::any_of(m_accountList.begin(), m_accountList.end(), hasUnionid)) {
            //如果是一个帐户则判断信息是否一致，不一致需更新
            if (unionidDB->accountName() == accountUnionid->accountName()) {
                qCDebug(ServiceLogger) << "Updating existing UID account:" << unionidDB->accountID();
                updateUIdAccount(unionidDB, accountUnionid);
            } else {
                qCDebug(ServiceLogger) << "Replacing UID account - Old:" << unionidDB->accountID() 
                                      << "New:" << accountUnionid->accountID();
                removeUIdAccount(unionidDB);
                addUIdAccount(accountUnionid);
            }
        } else {
            qCDebug(ServiceLogger) << "Adding new UID account:" << accountUnionid->accountID();
            addUIdAccount(accountUnionid);
        }
    }
}

void DAccountManageModule::initAccountDBusInfo(const DAccount::Ptr &account)
{
    qCDebug(ServiceLogger) << "Initializing DBus info for account:" << account->accountID();
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
    QString sortID = DDataBase::createUuid().mid(0, 5);
    if (account->accountType() == DAccount::Account_UnionID) {
        account->setAccountState(DAccount::Account_Setting | DAccount::Account_Calendar);
        setUidSwitchStatus(account);
    } else {
        account->setAccountState(DAccount::Account_Open | DAccount::Account_Calendar);
    }

    //设置DBus路径和数据库名
    account->setDtCreate(QDateTime::currentDateTime());
    account->setDbName(QString("account_%1_%2.db").arg(typeStr).arg(sortID));
    account->setDbusPath(QString("%1/account_%2_%3").arg(serviceBasePath).arg(typeStr).arg(sortID));
    account->setDbusInterface(accountServiceInterface);
}

void DAccountManageModule::removeUIdAccount(const DAccount::Ptr &uidAccount)
{
    qCDebug(ServiceLogger) << "Removing UID account:" << uidAccount->accountID();
    //帐户列表移除uid帐户
    m_accountList.removeOne(uidAccount);
    //移除对应的数据库 ，停止对应的定时器
    DAccountModule::Ptr accountModule(new DAccountModule(uidAccount));
    accountModule->removeDB();
    accountModule->downloadTaskhanding(2);
    //帐户管理数据库中删除相关数据
    m_accountManagerDB->deleteAccountInfo(uidAccount->accountID());
}

void DAccountManageModule::addUIdAccount(const DAccount::Ptr &uidAccount)
{
    qCDebug(ServiceLogger) << "Adding new UID account to DB:" << uidAccount->accountID();
    //帐户管理数据库中添加uid帐户
    initAccountDBusInfo(uidAccount);
    m_accountManagerDB->addAccountInfo(uidAccount);
    m_accountList.append(uidAccount);
}

void DAccountManageModule::updateUIdAccount(const DAccount::Ptr &oldAccount, const DAccount::Ptr &uidAccount)
{
    qCDebug(ServiceLogger) << "Updating UID account info for:" << oldAccount->accountID();
    oldAccount->avatar() = uidAccount->avatar();
    oldAccount->displayName() = uidAccount->displayName();
    setUidSwitchStatus(oldAccount);
    m_accountManagerDB->updateAccountInfo(oldAccount);
}

void DAccountManageModule::setUidSwitchStatus(const DAccount::Ptr &account)
{
    qCDebug(ServiceLogger) << "Setting UID switch status for account:" << account->accountID();
    //获取控制中心开关状态
    bool calendarSwitch = m_syncFileManage->getSyncoperation()->optGetCalendarSwitcher().switch_state;
    //获取帐户信息状态
    DAccount::AccountStates accountState = account->accountState();
    accountState.setFlag(DAccount::Account_Open, calendarSwitch);
    account->setAccountState(accountState);
}

// 获取通用配置
DCalendarGeneralSettings::Ptr DAccountManageModule::getGeneralSettings()
{
    qCDebug(ServiceLogger) << "Getting general settings.";
    auto cg = m_accountManagerDB->getCalendarGeneralSettings();
    if (getFirstDayOfWeekSource() == DCalendarGeneralSettings::Source_System) {
        qCDebug(ServiceLogger) << "First day of week source is System.";
        // deepin23使用dconfig存储系统配置
        if (m_reginFormatConfig->isValid()) {
            bool ok;
            auto dayofWeek =
                Qt::DayOfWeek(m_reginFormatConfig->value(firstDayOfWeek_key).toInt(&ok));
            if (ok) {
                cg->setFirstDayOfWeek(dayofWeek);
            } else {
                qWarning() << "Unable to get first day of week from control center config file";
            }
        } else {
            // 在deepin20.9使用dbus获取系统配置
            qCDebug(ServiceLogger) << "Using DBus to get first day of week.";
            cg->setFirstDayOfWeek(m_timeDateDbus.weekBegins());
        }
    }
    if (getTimeFormatTypeSource() == DCalendarGeneralSettings::Source_System) {
        qCDebug(ServiceLogger) << "Time format source is System.";
        // 在deepin23使用dconfig获取系统配置
        if (m_reginFormatConfig->isValid()) {
            auto shortTimeFormat = m_reginFormatConfig->value(shortTimeFormat_key).toString();
            if (shortTimeFormat.isEmpty()) {
                qWarning() << "Unable to short time format from control center config file";
            } else if (shortTimeFormat.contains("ap")) {
                cg->setTimeShowType(DCalendarGeneralSettings::Twelve);
            } else {
                cg->setTimeShowType(DCalendarGeneralSettings::TwentyFour);
            }
        } else {
            // 在deepin20.9使用dbus获取系统配置
            qCDebug(ServiceLogger) << "Using DBus to get time format.";
            if (m_timeDateDbus.shortTimeFormat() == 0) {
                cg->setTimeShowType(DCalendarGeneralSettings::Twelve);
            } else {
                cg->setTimeShowType(DCalendarGeneralSettings::TwentyFour);
            }
        }
    }
    return cg;
}

// 更改通用配置
void DAccountManageModule::setGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet)
{
    qCDebug(ServiceLogger) << "Setting general settings in DB.";
    m_accountManagerDB->setCalendarGeneralSettings(cgSet);
};

void DAccountManageModule::slotFirstDayOfWeek(const int firstDay)
{
    qCDebug(ServiceLogger) << "Slot: First day of week changed to:" << firstDay;
    if (getfirstDayOfWeek() != firstDay) {
        setFirstDayOfWeek(firstDay);
        emit firstDayOfWeekChange();
    }
}

void DAccountManageModule::slotTimeFormatType(const int timeType)
{
    qCDebug(ServiceLogger) << "Slot: Time format type changed to:" << timeType;
    if (getTimeFormatType() != timeType) {
        setTimeFormatType(timeType);
        emit timeFormatTypeChange();
    }
}

DCalendarGeneralSettings::GeneralSettingSource DAccountManageModule::getFirstDayOfWeekSource()
{
    // qCDebug(ServiceLogger) << "Getting first day of week source.";
    auto val = m_settings.value(firstDayOfWeekSource_key, DCalendarGeneralSettings::Source_Database);
    return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val.toInt());
}

void DAccountManageModule::setFirstDayOfWeekSource(const DCalendarGeneralSettings::GeneralSettingSource source)
{
    qCDebug(ServiceLogger) << "Setting first day of week source to:" << source;
    m_settings.setValue(firstDayOfWeekSource_key, source);
    emit firstDayOfWeekChange();
}

DCalendarGeneralSettings::GeneralSettingSource DAccountManageModule::getTimeFormatTypeSource()
{
    // qCDebug(ServiceLogger) << "Getting time format type source.";
    auto val = m_settings.value(shortTimeFormatSource_key, DCalendarGeneralSettings::GeneralSettingSource::Source_Database);
    return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val.toInt());
}

void DAccountManageModule::setTimeFormatTypeSource(const DCalendarGeneralSettings::GeneralSettingSource source)
{
    qCDebug(ServiceLogger) << "Setting time format type source to:" << source;
    m_settings.setValue(shortTimeFormatSource_key, source);
    emit timeFormatTypeChange();
}

void DAccountManageModule::slotUidLoginStatueChange(const int status)
{
    qCDebug(ServiceLogger) << "Slot: UID login status changed to:" << status;
    //因为有时登录成功会触发2次
    static QList<int> oldStatus{};
    //登录成功后会触发多次，状态也不一致。比如登录后会连续触发 1 -- 4 -- 1 信号
    if (!oldStatus.contains(status)) {
        oldStatus.append(status);
        qCDebug(ServiceLogger) << "New status added to history:" << status;
    } else {
        //如果当前状态和上次状态一直，则退出
        qCDebug(ServiceLogger) << "Duplicate status received, ignoring:" << status;
        return;
    }
    //1：登陆成功 2：登陆取消 3：登出 4：获取服务端配置的应用数据成功
    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    switch (status) {
    case 1: {
        qCDebug(ServiceLogger) << "Processing login success";
        //移除登出状态
        if (oldStatus.contains(3)) {
            oldStatus.removeAt(oldStatus.indexOf(3));
            qCDebug(ServiceLogger) << "Removed logout status from history";
        }

        //登陆成功
        DAccount::Ptr accountUnionid = m_syncFileManage->getuserInfo();
        if (accountUnionid.isNull() || accountUnionid->accountName().isEmpty()) {
            qCWarning(ServiceLogger) << "Failed to get account information after login";
            oldStatus.removeAt(oldStatus.indexOf(1));
            return;
        }
        qCDebug(ServiceLogger) << "Adding UID account after login:" << accountUnionid->accountID();
        addUIdAccount(accountUnionid);

        DAccountModule::Ptr accountModule = DAccountModule::Ptr(new DAccountModule(accountUnionid));
        QObject::connect(accountModule.data(), &DAccountModule::signalSettingChange, this, &DAccountManageModule::slotSettingChange);
        m_accountModuleMap[accountUnionid->accountID()] = accountModule;
        DAccountService::Ptr accountService = DAccountService::Ptr(new DAccountService(accountUnionid->dbusPath(), accountUnionid->dbusInterface(), accountModule, this));
        if (!sessionBus.registerObject(accountService->getPath(), accountService->getInterface(), accountService.data(), options)) {
            qCWarning(ServiceLogger) << "Failed to register account service - Error:" << sessionBus.lastError().message();
        } else {
            qCDebug(ServiceLogger) << "Successfully registered account service";
            m_AccountServiceMap[accountUnionid->accountType()].insert(accountUnionid->accountID(), accountService);
            if (accountUnionid->accountState().testFlag(DAccount::Account_Open)) {
                qCDebug(ServiceLogger) << "Starting download task for new account";
                accountModule->downloadTaskhanding(0);
            }
        }
    } break;
    case 3: {
        qCDebug(ServiceLogger) << "Processing logout";
        //移除登录状态
        if (oldStatus.contains(1)) {
            oldStatus.removeAt(oldStatus.indexOf(1));
            qCDebug(ServiceLogger) << "Removed login status from history";
        }
        //登出
        if (m_AccountServiceMap[DAccount::Type::Account_UnionID].size() > 0) {
            qCDebug(ServiceLogger) << "Removing UID account services";
            //如果存在UID帐户则移除相关信息
            //移除服务并注销
            QString accountID = m_AccountServiceMap[DAccount::Type::Account_UnionID].firstKey();
            DAccountService::Ptr accountService = m_AccountServiceMap[DAccount::Type::Account_UnionID].first();
            m_AccountServiceMap[DAccount::Type::Account_UnionID].clear();
            sessionBus.unregisterObject(accountService->getPath());
            //移除uid帐户信息
            //删除对应数据库
            m_accountModuleMap[accountID]->removeDB();
            m_accountModuleMap[accountID]->downloadTaskhanding(2);
            m_accountList.removeOne(m_accountModuleMap[accountID]->account());
            m_accountModuleMap.remove(accountID);
            m_accountManagerDB->deleteAccountInfo(accountID);
        }
    } break;
    default:
        //其它状态当前不做处理
        return;
    }
    emit signalLoginStatusChange();
}

void DAccountManageModule::slotSwitcherChange(const bool state)
{
    qCDebug(ServiceLogger) << "Calendar switcher changed to:" << state;
    foreach (auto schedule, m_accountList) {
        if (schedule->accountType() == DAccount::Account_UnionID) {
            qCDebug(ServiceLogger) << "Updating UID account state for account:" << schedule->accountID();
            if (state) {
                schedule->setAccountState(schedule->accountState() | DAccount::Account_Open);
                qCDebug(ServiceLogger) << "Starting download task for enabled account";
                m_accountModuleMap[schedule->accountID()]->downloadTaskhanding(1);
            } else {
                schedule->setAccountState(schedule->accountState() & ~DAccount::Account_Open);
                qCDebug(ServiceLogger) << "Stopping tasks for disabled account";
                m_accountModuleMap[schedule->accountID()]->downloadTaskhanding(2);
                m_accountModuleMap[schedule->accountID()]->uploadTaskHanding(0);
            }
            emit m_accountModuleMap[schedule->accountID()]->signalAccountState();
            return;
        }
    }
}

void DAccountManageModule::slotSettingChange()
{
    qCDebug(ServiceLogger) << "Slot: Settings changed, updating general settings.";
    DCalendarGeneralSettings::Ptr newSetting = getGeneralSettings();
    if (newSetting->firstDayOfWeek() != m_generalSetting->firstDayOfWeek()) {
        qCDebug(ServiceLogger) << "First day of week changed from" << m_generalSetting->firstDayOfWeek() 
                              << "to" << newSetting->firstDayOfWeek();
        m_generalSetting->setFirstDayOfWeek(newSetting->firstDayOfWeek());
        emit firstDayOfWeekChange();
    }

    if (newSetting->timeShowType() != m_generalSetting->timeShowType()) {
        qCDebug(ServiceLogger) << "Time format type changed from" << m_generalSetting->timeShowType() 
                              << "to" << newSetting->timeShowType();
        m_generalSetting->setTimeShowType(m_generalSetting->timeShowType());
        emit timeFormatTypeChange();
    }
}

void DAccountManageModule::slotClientIsOpen()
{
    // qCDebug(ServiceLogger) << "Checking if client is open...";
    //如果日历界面不存在则退出
    QProcess process;
    process.start("pidof", QStringList() << "dde-calendar");

    if (!process.waitForFinished(5000)) {
        qCWarning(ServiceLogger) << "pidof command timeout";
        return;
    }

    QString strResult = QString::fromUtf8(process.readAllStandardOutput()).trimmed();

    static QString preResult = "";

    if (preResult == strResult) {
        qCDebug(ServiceLogger) << "Calendar client status unchanged";
        return;
    } else {
        qCDebug(ServiceLogger) << "Calendar client status changed - Running:" << !strResult.isEmpty();
        preResult = strResult;
        DServiceExitControl exitControl;
        exitControl.setClientIsOpen(!strResult.isEmpty());
    }
}
