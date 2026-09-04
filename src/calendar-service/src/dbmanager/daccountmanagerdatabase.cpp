// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagerdatabase.h"

#include "units.h"

#include "commondef.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavprofile.h"
#include <QSet>
#include <QSqlQuery>
#include <QtDebug>
#include <QSqlError>
#include <QFile>
#include <QUrl>

DAccountManagerDataBase::DAccountManagerDataBase(QObject *parent)
    : DDataBase(parent)
{
    qCDebug(ServiceLogger) << "DAccountManagerDataBase constructor";
    setConnectionName(NameAccountManager);
}

void DAccountManagerDataBase::initDBData()
{
    qCDebug(ServiceLogger) << "Initializing account manager database";
    createDB();
    initAccountManagerDB();
}

void DAccountManagerDataBase::ensureSchema()
{
    createDB();
}

DAccount::List DAccountManagerDataBase::getAccountList()
{
    qCDebug(ServiceLogger) << "Getting account list";
    DAccount::List accountList;
    QString strSql("SELECT accountID,accountName, displayName, accountState, accountAvatar,               \
                   accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate, expandStatus, dtDelete, dtUpdate, isDeleted         \
                   FROM accountManager");
    SqliteQuery query(m_database);
    if (query.prepare(strSql) && query.exec()) {
        while (query.next()) {
            DAccount::Type type = static_cast<DAccount::Type>(query.value("accountType").toInt());
            DAccount::Ptr account(new DAccount(type));
            account->setAccountID(query.value("accountID").toString());
            account->setAccountName(query.value("accountName").toString());
            account->setDisplayName(query.value("displayName").toString());
            account->setAccountState(static_cast<DAccount::AccountState>(query.value("accountState").toInt()));
            account->setAvatar(query.value("accountAvatar").toString());
            account->setDescription(query.value("accountDescription").toString());
            account->setDbName(query.value("dbName").toString());
            account->setDbusPath(query.value("dBusPath").toString());
            account->setDbusInterface(query.value("dBusInterface").toString());
            account->setIsExpandDisplay(query.value("expandStatus").toBool());
            account->setDtCreate(QDateTime::fromString(query.value("dtCreate").toString(), Qt::ISODate));
            accountList.append(account);
        }
    } else {
        qCWarning(ServiceLogger) << "Failed to get account list:" << query.lastError().text();
    }
    return accountList;
}

DAccount::Ptr DAccountManagerDataBase::getAccountByID(const QString &accountID)
{
    qCDebug(ServiceLogger) << "Getting account by ID:" << accountID;
    QString strSql("SELECT accountName, displayName, accountState, accountAvatar,               \
                   accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate, dtDelete, dtUpdate, expandStatus, isDeleted         \
                   FROM accountManager WHERE accountID = ?");
    SqliteQuery query(m_database);
    if (query.prepare(strSql)) {
        qCDebug(ServiceLogger) << "Preparing query for account ID:" << accountID;
        query.addBindValue(accountID);
        if (query.exec() && query.next()) {
            DAccount::Type type = static_cast<DAccount::Type>(query.value("accountType").toInt());
            DAccount::Ptr account(new DAccount(type));
            account->setAccountID(accountID);
            account->setAccountName(query.value("accountName").toString());
            account->setDisplayName(query.value("displayName").toString());
            account->setAccountState(static_cast<DAccount::AccountState>(query.value("accountState").toInt()));
            account->setAvatar(query.value("accountAvatar").toString());
            account->setDescription(query.value("accountDescription").toString());
            account->setDbName(query.value("dbName").toString());
            account->setDbusPath(query.value("dBusPath").toString());
            account->setDbusInterface(query.value("dBusInterface").toString());
            account->setIsExpandDisplay(query.value("expandStatus").toBool());
            account->setDtCreate(QDateTime::fromString(query.value("dtCreate").toString(), Qt::ISODate));
            return account;
        } else {
            qCWarning(ServiceLogger) << "Account not found or query failed:" << query.lastError().text();
        }
    } else {
        qCWarning(ServiceLogger) << "Failed to prepare account query:" << query.lastError().text();
    }

    return nullptr;
}

QString DAccountManagerDataBase::addAccountInfo(const DAccount::Ptr &accountInfo)
{
    qCDebug(ServiceLogger) << "Adding new account:" << accountInfo->accountName();
    SqliteQuery query(m_database);
    //生成唯一标识
    if (accountInfo->accountID().isEmpty()) {
        qCDebug(ServiceLogger) << "Generating new account ID";
        accountInfo->setAccountID(DDataBase::createUuid());
        qCDebug(ServiceLogger) << "Generated new account ID:" << accountInfo->accountID();
    }
    QString strSql("INSERT INTO accountManager                                          \
                   (accountID, accountName, displayName, accountState, accountAvatar,  \
                    accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate,         \
                     expandStatus, isDeleted)                                                \
                   VALUES(?,?, ?, ?,?,?,?,?,?,?,?,?,?)");
    if (query.prepare(strSql)) {
        query.addBindValue(accountInfo->accountID());
        query.addBindValue(accountInfo->accountName());
        query.addBindValue(accountInfo->displayName());
        query.addBindValue(int(accountInfo->accountState()));
        query.addBindValue(accountInfo->avatar());
        query.addBindValue(accountInfo->description());
        query.addBindValue(accountInfo->accountType());
        query.addBindValue(accountInfo->dbName());
        query.addBindValue(accountInfo->dbusPath());
        query.addBindValue(accountInfo->dbusInterface());
        query.addBindValue(dtToString(accountInfo->dtCreate()));
        query.addBindValue(accountInfo->isExpandDisplay());
        query.addBindValue(0);
        if (!query.exec()) {
            qCWarning(ServiceLogger) << "Failed to add account:" << query.lastError().text();
            accountInfo->setAccountID("");
        }
    } else {
        qCWarning(ServiceLogger) << "Failed to prepare account insertion query:" << query.lastError().text();
        accountInfo->setAccountID("");
    }

    return accountInfo->accountID();
}

bool DAccountManagerDataBase::updateAccountInfo(const DAccount::Ptr &accountInfo)
{
    qCDebug(ServiceLogger) << "Updating account:" << accountInfo->accountName() << "ID:" << accountInfo->accountID();
    QString strSql("UPDATE accountManager                                                           \
                   SET accountName=?, displayName=?, accountState= ?,                   \
                   accountAvatar=?, accountDescription=?, accountType=?, dbName=?,               \
                   dBusPath = ? ,dBusInterface = ?, expandStatus = ? WHERE accountID=?");
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(accountInfo->accountName());
        query.addBindValue(accountInfo->displayName());
        query.addBindValue(int(accountInfo->accountState()));
        query.addBindValue(accountInfo->avatar());
        query.addBindValue(accountInfo->description());
        query.addBindValue(accountInfo->accountType());
        query.addBindValue(accountInfo->dbName());
        query.addBindValue(accountInfo->dbusPath());
        query.addBindValue(accountInfo->dbusInterface());
        query.addBindValue(accountInfo->isExpandDisplay());
        query.addBindValue(accountInfo->accountID());
        res = query.exec();
    }
    if (!res) {
         qCWarning(ServiceLogger) << "Failed to update account:" << query.lastError().text();
    }

    return res;
}

bool DAccountManagerDataBase::deleteAccountInfo(const QString &accountID)
{
    qCDebug(ServiceLogger) << "Deleting account with ID:" << accountID;
    QString strSql("DELETE FROM accountManager      \
                   WHERE accountID=?");
    SqliteQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(accountID);
        res = query.exec();
    }

    if (!res) {
        qCWarning(ServiceLogger) << "Failed to delete account:" << query.lastError().text();
    }
    return res;
}
// 保存通用设置
DCalendarGeneralSettings::Ptr DAccountManagerDataBase::getCalendarGeneralSettings()
{
    qCDebug(ServiceLogger) << "Getting calendar general settings";
    DCalendarGeneralSettings::Ptr cgSet(new DCalendarGeneralSettings);
    SqliteQuery query(m_database);
    query.exec("select vch_value from calendargeneralsettings where vch_key = 'firstDayOfWeek' ");
    if (query.next()) {
        cgSet->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(query.value(0).toInt()));
        qCDebug(ServiceLogger) << "Retrieved first day of week:" << cgSet->firstDayOfWeek();
    }

    query.exec("select vch_value from calendargeneralsettings where vch_key = 'timeShowType' ");
    if (query.next()) {
        cgSet->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(query.value(0).toInt()));
        qCDebug(ServiceLogger) << "Retrieved time show type:" << cgSet->timeShowType();
    }

    return cgSet;
}
// 获取通用设置
void DAccountManagerDataBase::setCalendarGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet)
{
    qCDebug(ServiceLogger) << "Updating calendar general settings";
    SqliteQuery query(m_database);
    query.prepare("update calendargeneralsettings set vch_value = ? where vch_key = 'firstDayOfWeek' ");
    query.addBindValue(cgSet->firstDayOfWeek());
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to update first day of week:" << query.lastError().text();
    }

    query.prepare("update calendargeneralsettings set vch_value = ? where vch_key = 'timeShowType' ");
    query.addBindValue(cgSet->timeShowType());
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to update time show type:" << query.lastError().text();
    }
}

QString DAccountManagerDataBase::getCalDavAccountStatusList()
{
    DCalDavAccountStatus::List statusList;
    const QString sql = "SELECT caldavAccount.accountID, caldavAccount.providerType, caldavAccount.syncStatus, "
                        "caldavAccount.lastSuccessfulSync, caldavAccount.failureReason, caldavAccount.failureCode, caldavAccount.accountColor, "
                        "caldavAccount.serverUrl, accountManager.displayName, "
                        "(SELECT COUNT(*) FROM caldavOutbox outbox "
                        " WHERE outbox.accountID = caldavAccount.accountID) AS pendingOperationCount, "
                        "(SELECT COUNT(*) FROM caldavOutbox deleteBox "
                        " WHERE deleteBox.accountID = caldavAccount.accountID "
                        " AND deleteBox.operationType = 2) AS pendingDeleteCount, "
                        "(SELECT COUNT(*) FROM caldavOutbox conflictBox "
                        " WHERE conflictBox.accountID = caldavAccount.accountID AND conflictBox.failureType = 4) AS conflictCount, "
                        "(SELECT COUNT(*) FROM caldavCalendar calendarCount "
                        " WHERE calendarCount.accountID = caldavAccount.accountID) AS calendarCount, "
                        "(SELECT COUNT(*) FROM caldavCalendar writableCalendar "
                        " WHERE writableCalendar.accountID = caldavAccount.accountID "
                        " AND (writableCalendar.privileges & 2) != 0) AS writableCalendarCount, "
                        "CASE WHEN caldavAccount.nextRetryAt IS NULL OR caldavAccount.nextRetryAt = '' THEN "
                        " (SELECT MIN(retryBox.nextRetryAt) FROM caldavOutbox retryBox "
                        "  WHERE retryBox.accountID = caldavAccount.accountID AND retryBox.failureType = 1 "
                        "  AND retryBox.nextRetryAt IS NOT NULL AND retryBox.nextRetryAt != '') "
                        "WHEN (SELECT MIN(retryBox.nextRetryAt) FROM caldavOutbox retryBox "
                        "      WHERE retryBox.accountID = caldavAccount.accountID AND retryBox.failureType = 1 "
                        "      AND retryBox.nextRetryAt IS NOT NULL AND retryBox.nextRetryAt != '') IS NULL "
                        " THEN caldavAccount.nextRetryAt "
                        "WHEN caldavAccount.nextRetryAt < (SELECT MIN(retryBox.nextRetryAt) FROM caldavOutbox retryBox "
                        "      WHERE retryBox.accountID = caldavAccount.accountID AND retryBox.failureType = 1 "
                        "      AND retryBox.nextRetryAt IS NOT NULL AND retryBox.nextRetryAt != '') "
                        " THEN caldavAccount.nextRetryAt "
                        "ELSE (SELECT MIN(retryBox.nextRetryAt) FROM caldavOutbox retryBox "
                        "      WHERE retryBox.accountID = caldavAccount.accountID AND retryBox.failureType = 1 "
                        "      AND retryBox.nextRetryAt IS NOT NULL AND retryBox.nextRetryAt != '') END AS nextRetryAt "
                        "FROM caldavAccount LEFT JOIN accountManager ON caldavAccount.accountID = accountManager.accountID";
    SqliteQuery query(m_database);
    if (!query.prepare(sql) || !query.exec()) {
        qCWarning(ServiceLogger) << "Failed to get CalDAV account statuses:" << query.lastError().text();
        return QStringLiteral("[]");
    }

    while (query.next()) {
        DCalDavAccountStatus status;
        status.accountId = query.value("accountID").toString();
        status.displayName = query.value("displayName").toString();
        const auto storedProviderType = static_cast<DCalDavProviderProfile::ProviderType>(
            query.value("providerType").toInt());
        status.providerType = DCalDavProviderProfile::providerTypeForServerUrl(
            query.value("serverUrl").toString(), storedProviderType);
        status.syncStatus = query.value("syncStatus").toInt();
        status.lastSuccessfulSync = QDateTime::fromString(query.value("lastSuccessfulSync").toString(), Qt::ISODate);
        status.failureReason = query.value("failureReason").toString();
        status.failureCode = query.value("failureCode").toInt();
        status.accountColor = query.value("accountColor").toString();
        const bool providerSupportsWrite = DCalDavProviderProfile::forProvider(
            static_cast<DCalDavProviderProfile::ProviderType>(status.providerType)).supportsWrite;
        const int calendarCount = query.value("calendarCount").toInt();
        const int writableCalendarCount = query.value("writableCalendarCount").toInt();
        status.supportsWrite = providerSupportsWrite
            && (calendarCount == 0 || writableCalendarCount > 0);
        status.pendingOperationCount = query.value("pendingOperationCount").toInt();
        status.pendingDeleteCount = query.value("pendingDeleteCount").toInt();
        status.conflictCount = query.value("conflictCount").toInt();
        status.nextRetryAt = QDateTime::fromString(
            query.value("nextRetryAt").toString(), Qt::ISODate);
        statusList.append(status);
    }
    return DCalDavAccountStatus::toJsonListString(statusList);
}

bool DAccountManagerDataBase::getCalDavAccountInfo(const QString &accountID, DCalDavAccountInfo &accountInfo)
{
    accountInfo = DCalDavAccountInfo();
    if (accountID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT accountID, providerType, serverUrl, username, credentialRef, accountColor, "
                       "retryCount, nextRetryAt FROM caldavAccount WHERE accountID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV account query:" << query.lastError().text();
        return false;
    }
    query.addBindValue(accountID);
    if (!query.exec() || !query.next()) {
        if (query.lastError().isValid()) {
            qCWarning(ServiceLogger) << "Failed to query CalDAV account:" << query.lastError().text();
        }
        return false;
    }

    accountInfo.accountId = query.value("accountID").toString();
    accountInfo.serverUrl = query.value("serverUrl").toString();
    const auto storedProviderType = static_cast<DCalDavProviderProfile::ProviderType>(
        query.value("providerType").toInt());
    accountInfo.providerType = DCalDavProviderProfile::providerTypeForServerUrl(
        accountInfo.serverUrl, storedProviderType);
    accountInfo.username = query.value("username").toString();
    accountInfo.credentialRef = query.value("credentialRef").toString();
    accountInfo.accountColor = query.value("accountColor").toString();
    accountInfo.retryCount = query.value("retryCount").toInt();
    accountInfo.nextRetryAt = QDateTime::fromString(query.value("nextRetryAt").toString(), Qt::ISODate);
    return true;
}

bool DAccountManagerDataBase::upsertCalDavAccountInfo(const DCalDavAccountInfo &accountInfo)
{
    const QUrl serverUrl(accountInfo.serverUrl);
    if (accountInfo.accountId.isEmpty() || accountInfo.username.isEmpty()
        || accountInfo.credentialRef.isEmpty() || serverUrl.scheme() != QStringLiteral("https")
        || serverUrl.host().isEmpty() || !DCalDavCredentialReference::isValid(accountInfo.credentialRef)) {
        qCWarning(ServiceLogger) << "Rejected invalid CalDAV account configuration.";
        return false;
    }

    SqliteQuery updateQuery(m_database);
    if (!updateQuery.prepare("UPDATE caldavAccount SET providerType = ?, serverUrl = ?, username = ?, "
                             "credentialRef = ?, accountColor = ?, retryCount = 0, nextRetryAt = NULL WHERE accountID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV account update:" << updateQuery.lastError().text();
        return false;
    }
    updateQuery.addBindValue(accountInfo.providerType);
    updateQuery.addBindValue(accountInfo.serverUrl);
    updateQuery.addBindValue(accountInfo.username);
    updateQuery.addBindValue(accountInfo.credentialRef);
    updateQuery.addBindValue(accountInfo.accountColor);
    updateQuery.addBindValue(accountInfo.accountId);
    if (!updateQuery.exec()) {
        qCWarning(ServiceLogger) << "Failed to update CalDAV account configuration:" << updateQuery.lastError().text();
        return false;
    }
    if (updateQuery.numRowsAffected() == 1) {
        return true;
    }

    SqliteQuery insertQuery(m_database);
    if (!insertQuery.prepare("INSERT INTO caldavAccount "
                             "(accountID, providerType, serverUrl, username, credentialRef, accountColor) "
                             "VALUES (?, ?, ?, ?, ?, ?)")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV account insert:" << insertQuery.lastError().text();
        return false;
    }
    insertQuery.addBindValue(accountInfo.accountId);
    insertQuery.addBindValue(accountInfo.providerType);
    insertQuery.addBindValue(accountInfo.serverUrl);
    insertQuery.addBindValue(accountInfo.username);
    insertQuery.addBindValue(accountInfo.credentialRef);
    insertQuery.addBindValue(accountInfo.accountColor);
    if (!insertQuery.exec()) {
        qCWarning(ServiceLogger) << "Failed to insert CalDAV account configuration:" << insertQuery.lastError().text();
        return false;
    }
    return true;
}

bool DAccountManagerDataBase::deleteCalDavAccountInfo(const QString &accountID)
{
    if (accountID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("DELETE FROM caldavAccount WHERE accountID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV account deletion:" << query.lastError().text();
        return false;
    }
    query.addBindValue(accountID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to delete CalDAV account configuration:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}

DCalDavOutboxItem DAccountManagerDataBase::getCalDavOutboxItem(const QString &accountID,
                                                                  const QString &localScheduleID)
{
    DCalDavOutboxItem item;
    if (accountID.isEmpty() || localScheduleID.isEmpty()) {
        return item;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT operationID, accountID, localScheduleID, operationType, baseEtag, conflictIcs, serverIcs, retryCount, "
                       "nextRetryAt, failureType FROM caldavOutbox "
                       "WHERE accountID = ? AND localScheduleID = ? ORDER BY operationID LIMIT 1")) {
        return item;
    }
    query.addBindValue(accountID);
    query.addBindValue(localScheduleID);
    if (!query.exec() || !query.next()) {
        return item;
    }
    item.operationID = query.value("operationID").toString();
    item.accountID = query.value("accountID").toString();
    item.localScheduleID = query.value("localScheduleID").toString();
    item.operationType = static_cast<DCalDavOutboxItem::OperationType>(
        query.value("operationType").toInt());
    item.baseEtag = query.value("baseEtag").toString();
    item.conflictIcs = query.value("conflictIcs").toString();
        item.serverIcs = query.value("serverIcs").toString();
    item.retryCount = query.value("retryCount").toInt();
    item.nextRetryAt = QDateTime::fromString(query.value("nextRetryAt").toString(), Qt::ISODate);
    item.failureType = static_cast<DCalDavOutboxItem::FailureType>(
        query.value("failureType").toInt());
    return item;
}

bool DAccountManagerDataBase::hasCalDavOutboxItems(const QString &accountID) const
{
    if (accountID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "SELECT 1 FROM caldavOutbox WHERE accountID = ? LIMIT 1"))) {
        return false;
    }
    query.addBindValue(accountID);
    return query.exec() && query.next();
}

DCalDavOutboxItem::List DAccountManagerDataBase::getCalDavConflictItems(
    const QString &accountID)
{
    DCalDavOutboxItem::List items;
    if (accountID.isEmpty()) {
        return items;
    }
    SqliteQuery query(m_database);
    if (!query.prepare("SELECT operationID, accountID, localScheduleID, operationType, baseEtag, conflictIcs, serverIcs, "
                       "retryCount, nextRetryAt, failureType FROM caldavOutbox "
                       "WHERE accountID = ? AND failureType = 4 ORDER BY operationID")) {
        return items;
    }
    query.addBindValue(accountID);
    if (!query.exec()) {
        return items;
    }
    while (query.next()) {
        DCalDavOutboxItem item;
        item.operationID = query.value("operationID").toString();
        item.accountID = query.value("accountID").toString();
        item.localScheduleID = query.value("localScheduleID").toString();
        item.operationType = static_cast<DCalDavOutboxItem::OperationType>(
            query.value("operationType").toInt());
        item.baseEtag = query.value("baseEtag").toString();
    item.conflictIcs = query.value("conflictIcs").toString();
    item.serverIcs = query.value("serverIcs").toString();
        item.retryCount = query.value("retryCount").toInt();
        item.nextRetryAt = QDateTime::fromString(query.value("nextRetryAt").toString(), Qt::ISODate);
        item.failureType = DCalDavOutboxItem::ConflictFailure;
        items.append(item);
    }
    return items;
}

DCalDavOutboxItem::List DAccountManagerDataBase::getCalDavBlockedOutboxItems(
    const QString &accountID)
{
    DCalDavOutboxItem::List items;
    if (accountID.isEmpty()) {
        return items;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(
            "SELECT operationID, accountID, localScheduleID, operationType, baseEtag, conflictIcs, serverIcs, "
            "retryCount, nextRetryAt, failureType FROM caldavOutbox "
            "WHERE accountID = ? AND failureType NOT IN (?, ?) ORDER BY operationID")) {
        return items;
    }
    query.addBindValue(accountID);
    query.addBindValue(static_cast<int>(DCalDavOutboxItem::NoFailure));
    query.addBindValue(static_cast<int>(DCalDavOutboxItem::NetworkFailure));
    if (!query.exec()) {
        return items;
    }
    while (query.next()) {
        DCalDavOutboxItem item;
        item.operationID = query.value("operationID").toString();
        item.accountID = query.value("accountID").toString();
        item.localScheduleID = query.value("localScheduleID").toString();
        item.operationType = static_cast<DCalDavOutboxItem::OperationType>(
            query.value("operationType").toInt());
        item.baseEtag = query.value("baseEtag").toString();
    item.conflictIcs = query.value("conflictIcs").toString();
    item.serverIcs = query.value("serverIcs").toString();
        item.retryCount = query.value("retryCount").toInt();
        item.nextRetryAt = QDateTime::fromString(query.value("nextRetryAt").toString(), Qt::ISODate);
        item.failureType = static_cast<DCalDavOutboxItem::FailureType>(
            query.value("failureType").toInt());
        items.append(item);
    }
    return items;
}

DCalDavOutboxItem::List DAccountManagerDataBase::getCalDavRetryScheduledOutboxItems(
    const QString &accountID, const QDateTime &now) const
{
    DCalDavOutboxItem::List items;
    if (accountID.isEmpty() || !now.isValid()) {
        return items;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(
            "SELECT operationID, accountID, localScheduleID, operationType, baseEtag, conflictIcs, serverIcs, "
            "retryCount, nextRetryAt, failureType FROM caldavOutbox "
            "WHERE accountID = ? AND failureType = ? AND nextRetryAt IS NOT NULL "
            "AND nextRetryAt != '' AND nextRetryAt > ? ORDER BY operationID")) {
        return items;
    }
    query.addBindValue(accountID);
    query.addBindValue(static_cast<int>(DCalDavOutboxItem::NetworkFailure));
    query.addBindValue(now.toUTC().toString(Qt::ISODate));
    if (!query.exec()) {
        return items;
    }
    while (query.next()) {
        DCalDavOutboxItem item;
        item.operationID = query.value("operationID").toString();
        item.accountID = query.value("accountID").toString();
        item.localScheduleID = query.value("localScheduleID").toString();
        item.operationType = static_cast<DCalDavOutboxItem::OperationType>(
            query.value("operationType").toInt());
        item.baseEtag = query.value("baseEtag").toString();
    item.conflictIcs = query.value("conflictIcs").toString();
    item.serverIcs = query.value("serverIcs").toString();
        item.retryCount = query.value("retryCount").toInt();
        item.nextRetryAt = QDateTime::fromString(query.value("nextRetryAt").toString(), Qt::ISODate);
        item.failureType = DCalDavOutboxItem::NetworkFailure;
        items.append(item);
    }
    return items;
}

DCalDavOutboxItem::List DAccountManagerDataBase::getDueCalDavOutboxItems(
    const QString &accountID, const QDateTime &now, bool ignoreRetryAt)
{
    DCalDavOutboxItem::List items;
    if (accountID.isEmpty()) {
        return items;
    }

    SqliteQuery query(m_database);
    const QString sql = ignoreRetryAt
        ? QStringLiteral("SELECT operationID, accountID, localScheduleID, operationType, baseEtag, conflictIcs, serverIcs, retryCount, "
                        "nextRetryAt, failureType FROM caldavOutbox "
                        "WHERE accountID = ? AND failureType IN (0, 1, 2, 3, 5) ORDER BY operationID")
        : QStringLiteral("SELECT operationID, accountID, localScheduleID, operationType, baseEtag, conflictIcs, serverIcs, retryCount, "
                        "nextRetryAt, failureType FROM caldavOutbox "
                        "WHERE accountID = ? AND failureType IN (0, 1) "
                        "AND (nextRetryAt IS NULL OR nextRetryAt = '' OR nextRetryAt <= ?) "
                        "ORDER BY operationID");
    if (!query.prepare(sql)) {
        return items;
    }
    query.addBindValue(accountID);
    if (!ignoreRetryAt) {
        query.addBindValue(now.toString(Qt::ISODate));
    }
    if (!query.exec()) {
        return items;
    }
    while (query.next()) {
        DCalDavOutboxItem item;
        item.operationID = query.value("operationID").toString();
        item.accountID = query.value("accountID").toString();
        item.localScheduleID = query.value("localScheduleID").toString();
        item.operationType = static_cast<DCalDavOutboxItem::OperationType>(
            query.value("operationType").toInt());
            item.baseEtag = query.value("baseEtag").toString();
    item.conflictIcs = query.value("conflictIcs").toString();
    item.serverIcs = query.value("serverIcs").toString();
        item.retryCount = query.value("retryCount").toInt();
        item.nextRetryAt = QDateTime::fromString(query.value("nextRetryAt").toString(), Qt::ISODate);
        item.failureType = static_cast<DCalDavOutboxItem::FailureType>(
            query.value("failureType").toInt());
        items.append(item);
    }
    return items;
}

QDateTime DAccountManagerDataBase::earliestCalDavOutboxRetryAt() const
{
    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "SELECT MIN(nextRetryAt) FROM caldavOutbox "
            "WHERE failureType = ? AND nextRetryAt IS NOT NULL AND nextRetryAt != ''"))) {
        return QDateTime();
    }
    query.addBindValue(static_cast<int>(DCalDavOutboxItem::NetworkFailure));
    if (!query.exec() || !query.next()) {
        return QDateTime();
    }
    return QDateTime::fromString(query.value(0).toString(), Qt::ISODate);
}

bool DAccountManagerDataBase::upsertCalDavOutboxItem(const DCalDavOutboxItem &item)
{
    if (item.operationID.isEmpty() || item.accountID.isEmpty() || item.localScheduleID.isEmpty()) {
        return false;
    }

    SqliteQuery updateQuery(m_database);
    if (!updateQuery.prepare("UPDATE caldavOutbox SET operationType = ?, baseEtag = ?, conflictIcs = ?, serverIcs = ?, retryCount = ?, "
                             "nextRetryAt = ?, failureType = ? WHERE operationID = ?")) {
        return false;
    }
    updateQuery.addBindValue(static_cast<int>(item.operationType));
    updateQuery.addBindValue(item.baseEtag);
    updateQuery.addBindValue(item.conflictIcs);
    updateQuery.addBindValue(item.serverIcs);
    updateQuery.addBindValue(item.retryCount);
    updateQuery.addBindValue(item.nextRetryAt.isValid() ? item.nextRetryAt.toString(Qt::ISODate) : QString());
    updateQuery.addBindValue(static_cast<int>(item.failureType));
    updateQuery.addBindValue(item.operationID);
    if (!updateQuery.exec()) {
        return false;
    }
    if (updateQuery.numRowsAffected() == 1) {
        return true;
    }

    SqliteQuery insertQuery(m_database);
    if (!insertQuery.prepare("INSERT INTO caldavOutbox "
                             "(operationID, accountID, localScheduleID, operationType, baseEtag, conflictIcs, serverIcs, retryCount, "
                             "nextRetryAt, failureType) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")) {
        return false;
    }
    insertQuery.addBindValue(item.operationID);
    insertQuery.addBindValue(item.accountID);
    insertQuery.addBindValue(item.localScheduleID);
    insertQuery.addBindValue(static_cast<int>(item.operationType));
    insertQuery.addBindValue(item.baseEtag);
    insertQuery.addBindValue(item.conflictIcs);
    insertQuery.addBindValue(item.serverIcs);
    insertQuery.addBindValue(item.retryCount);
    insertQuery.addBindValue(item.nextRetryAt.isValid() ? item.nextRetryAt.toString(Qt::ISODate) : QString());
    insertQuery.addBindValue(static_cast<int>(item.failureType));
    return insertQuery.exec();
}

bool DAccountManagerDataBase::deleteCalDavOutboxItem(const QString &accountID,
                                                      const QString &localScheduleID)
{
    if (accountID.isEmpty() || localScheduleID.isEmpty()) {
        return false;
    }
    SqliteQuery query(m_database);
    if (!query.prepare("DELETE FROM caldavOutbox WHERE accountID = ? AND localScheduleID = ?")) {
        return false;
    }
    query.addBindValue(accountID);
    query.addBindValue(localScheduleID);
    return query.exec();
}

bool DAccountManagerDataBase::deleteCalDavOutboxItemIfCurrent(const DCalDavOutboxItem &item)
{
    if (item.operationID.isEmpty() || item.accountID.isEmpty() || item.localScheduleID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("DELETE FROM caldavOutbox WHERE operationID = ? AND accountID = ? "
                       "AND localScheduleID = ?")) {
        return false;
    }
    query.addBindValue(item.operationID);
    query.addBindValue(item.accountID);
    query.addBindValue(item.localScheduleID);
    return query.exec() && query.numRowsAffected() == 1;
}

bool DAccountManagerDataBase::deleteCalDavAccountData(const QString &accountID)
{
    if (accountID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.transaction()) {
        return false;
    }
    const QStringList statements = {
        QStringLiteral("DELETE FROM caldavEventMapping WHERE accountID = ?"),
        QStringLiteral("DELETE FROM caldavCategoryMapping WHERE accountID = ?"),
        QStringLiteral("DELETE FROM caldavOutbox WHERE accountID = ?"),
        QStringLiteral("DELETE FROM caldavCalendar WHERE accountID = ?"),
        QStringLiteral("DELETE FROM caldavAccount WHERE accountID = ?"),
        QStringLiteral("DELETE FROM accountManager WHERE accountID = ?"),
    };
    for (const QString &statement : statements) {
        if (!query.prepare(statement)) {
            query.rollback();
            return false;
        }
        query.addBindValue(accountID);
        if (!query.exec()) {
            query.rollback();
            return false;
        }
    }
    if (!query.commit()) {
        query.rollback();
        return false;
    }
    return true;
}

bool DAccountManagerDataBase::upsertCalDavAccountDeletionCleanup(
    const QString &accountID, const QString &sourceDbName)
{
    if (accountID.isEmpty() || sourceDbName.isEmpty()) {
        return false;
    }
    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO caldavAccountDeletionCleanup (accountID, sourceDbName) VALUES (?, ?)"))) {
        return false;
    }
    query.addBindValue(accountID);
    query.addBindValue(sourceDbName);
    return query.exec();
}

QMap<QString, QString> DAccountManagerDataBase::getCalDavAccountDeletionCleanups() const
{
    QMap<QString, QString> cleanups;
    SqliteQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT accountID, sourceDbName FROM caldavAccountDeletionCleanup"))) {
        return cleanups;
    }
    while (query.next()) {
        cleanups.insert(query.value(0).toString(), query.value(1).toString());
    }
    return cleanups;
}

bool DAccountManagerDataBase::deleteCalDavAccountDeletionCleanup(const QString &accountID)
{
    if (accountID.isEmpty()) {
        return false;
    }
    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "DELETE FROM caldavAccountDeletionCleanup WHERE accountID = ?"))) {
        return false;
    }
    query.addBindValue(accountID);
    return query.exec();
}

DCalDavCategoryInfo DAccountManagerDataBase::getCalDavCategoryMapping(
    const QString &accountID, const QString &calendarID, const QString &categoryKey) const
{
    DCalDavCategoryInfo mapping;
    if (accountID.isEmpty() || calendarID.isEmpty() || categoryKey.isEmpty()) {
        return mapping;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "SELECT accountID, calendarID, categoryKey, scheduleTypeID "
            "FROM caldavCategoryMapping WHERE accountID = ? AND calendarID = ? AND categoryKey = ?"))) {
        return mapping;
    }
    query.addBindValue(accountID);
    query.addBindValue(calendarID);
    query.addBindValue(categoryKey);
    if (!query.exec() || !query.next()) {
        return mapping;
    }
    mapping.accountId = query.value("accountID").toString();
    mapping.calendarId = query.value("calendarID").toString();
    mapping.categoryKey = query.value("categoryKey").toString();
    mapping.scheduleTypeId = query.value("scheduleTypeID").toString();
    return mapping;
}

DCalDavCategoryInfo::List DAccountManagerDataBase::getCalDavCategoryMappings(
    const QString &accountID, const QString &calendarID) const
{
    DCalDavCategoryInfo::List mappings;
    if (accountID.isEmpty() || calendarID.isEmpty()) {
        return mappings;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "SELECT accountID, calendarID, categoryKey, scheduleTypeID "
            "FROM caldavCategoryMapping WHERE accountID = ? AND calendarID = ? "
            "ORDER BY categoryKey"))) {
        return mappings;
    }
    query.addBindValue(accountID);
    query.addBindValue(calendarID);
    if (!query.exec()) {
        return mappings;
    }
    while (query.next()) {
        DCalDavCategoryInfo mapping;
        mapping.accountId = query.value("accountID").toString();
        mapping.calendarId = query.value("calendarID").toString();
        mapping.categoryKey = query.value("categoryKey").toString();
        mapping.scheduleTypeId = query.value("scheduleTypeID").toString();
        mappings.append(mapping);
    }
    return mappings;
}

DCalDavCategoryInfo DAccountManagerDataBase::getCalDavCategoryMappingByScheduleTypeID(
    const QString &accountID, const QString &scheduleTypeID) const
{
    DCalDavCategoryInfo mapping;
    if (accountID.isEmpty() || scheduleTypeID.isEmpty()) {
        return mapping;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "SELECT accountID, calendarID, categoryKey, scheduleTypeID "
            "FROM caldavCategoryMapping WHERE accountID = ? AND scheduleTypeID = ?"))) {
        return mapping;
    }
    query.addBindValue(accountID);
    query.addBindValue(scheduleTypeID);
    if (!query.exec() || !query.next()) {
        return mapping;
    }
    mapping.accountId = query.value("accountID").toString();
    mapping.calendarId = query.value("calendarID").toString();
    mapping.categoryKey = query.value("categoryKey").toString();
    mapping.scheduleTypeId = query.value("scheduleTypeID").toString();
    return mapping;
}

bool DAccountManagerDataBase::deleteCalDavCategoryMappingByScheduleTypeID(
    const QString &accountID, const QString &scheduleTypeID)
{
    if (accountID.isEmpty() || scheduleTypeID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "DELETE FROM caldavCategoryMapping WHERE accountID = ? AND scheduleTypeID = ?"))) {
        return false;
    }
    query.addBindValue(accountID);
    query.addBindValue(scheduleTypeID);
    return query.exec();
}

bool DAccountManagerDataBase::upsertCalDavCategoryMapping(
    const DCalDavCategoryInfo &mapping)
{
    if (mapping.accountId.isEmpty() || mapping.calendarId.isEmpty()
        || mapping.categoryKey.isEmpty() || mapping.scheduleTypeId.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO caldavCategoryMapping "
            "(accountID, calendarID, categoryKey, scheduleTypeID) VALUES (?, ?, ?, ?)"))) {
        return false;
    }
    query.addBindValue(mapping.accountId);
    query.addBindValue(mapping.calendarId);
    query.addBindValue(mapping.categoryKey);
    query.addBindValue(mapping.scheduleTypeId);
    return query.exec();
}

bool DAccountManagerDataBase::updateCalDavCredentialReference(const QString &accountID, const QString &credentialRef)
{
    if (accountID.isEmpty() || !DCalDavCredentialReference::isValid(credentialRef)) {
        qCWarning(ServiceLogger) << "Rejected invalid CalDAV credential reference update";
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("UPDATE caldavAccount SET credentialRef = ? WHERE accountID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV credential reference update:" << query.lastError().text();
        return false;
    }
    query.addBindValue(credentialRef);
    query.addBindValue(accountID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to update CalDAV credential reference:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}


DCalDavCalendarInfo::List DAccountManagerDataBase::getCalDavCalendarList(const QString &accountID)
{
    DCalDavCalendarInfo::List calendars;
    if (accountID.isEmpty()) {
        return calendars;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT calendarID, accountID, href, displayName, color, scheduleTypeID, privileges, syncToken, "
                       "initialSyncCompleted, enabled "
                       "FROM caldavCalendar WHERE accountID = ? ORDER BY calendarID")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV calendar query:" << query.lastError().text();
        return calendars;
    }
    query.addBindValue(accountID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to query CalDAV calendars:" << query.lastError().text();
        return calendars;
    }

    while (query.next()) {
        DCalDavCalendarInfo calendar;
        calendar.calendarId = query.value("calendarID").toString();
        calendar.accountId = query.value("accountID").toString();
        calendar.href = query.value("href").toString();
        calendar.displayName = query.value("displayName").toString();
        calendar.color = query.value("color").toString();
        calendar.scheduleTypeID = query.value("scheduleTypeID").toString();
        calendar.privileges = query.value("privileges").toInt();
        calendar.syncToken = query.value("syncToken").toString();
        calendar.initialSyncCompleted = query.value("initialSyncCompleted").toBool();
        calendar.enabled = query.value("enabled").toBool();
        calendars.append(calendar);
    }
    return calendars;
}

DCalDavCalendarInfo DAccountManagerDataBase::getCalDavCalendarByScheduleTypeID(
    const QString &accountID, const QString &scheduleTypeID)
{
    DCalDavCalendarInfo calendar;
    if (accountID.isEmpty() || scheduleTypeID.isEmpty()) {
        return calendar;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT calendarID, accountID, href, displayName, color, scheduleTypeID, privileges, "
                       "syncToken, initialSyncCompleted, enabled FROM caldavCalendar "
                       "WHERE accountID = ? AND scheduleTypeID = ? AND enabled = 1")) {
        return calendar;
    }
    query.addBindValue(accountID);
    query.addBindValue(scheduleTypeID);
    if (!query.exec() || !query.next()) {
        return calendar;
    }
    calendar.calendarId = query.value("calendarID").toString();
    calendar.accountId = query.value("accountID").toString();
    calendar.href = query.value("href").toString();
    calendar.displayName = query.value("displayName").toString();
    calendar.color = query.value("color").toString();
    calendar.scheduleTypeID = query.value("scheduleTypeID").toString();
    calendar.privileges = query.value("privileges").toInt();
    calendar.syncToken = query.value("syncToken").toString();
    calendar.initialSyncCompleted = query.value("initialSyncCompleted").toBool();
    calendar.enabled = query.value("enabled").toBool();
    return calendar;
}

bool DAccountManagerDataBase::upsertCalDavCalendar(const DCalDavCalendarInfo &calendar)
{
    if (calendar.calendarId.isEmpty() || calendar.accountId.isEmpty() || calendar.href.isEmpty()) {
        return false;
    }

    SqliteQuery updateQuery(m_database);
    if (!updateQuery.prepare("UPDATE caldavCalendar SET accountID = ?, href = ?, displayName = ?, color = ?, scheduleTypeID = ?, "
                             "privileges = ?, syncToken = ?, initialSyncCompleted = ?, enabled = ? WHERE calendarID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV calendar update:" << updateQuery.lastError().text();
        return false;
    }
    updateQuery.addBindValue(calendar.accountId);
    updateQuery.addBindValue(calendar.href);
    updateQuery.addBindValue(calendar.displayName);
    updateQuery.addBindValue(calendar.color);
    updateQuery.addBindValue(calendar.scheduleTypeID);
    updateQuery.addBindValue(calendar.privileges);
    updateQuery.addBindValue(calendar.syncToken);
    updateQuery.addBindValue(calendar.initialSyncCompleted ? 1 : 0);
    updateQuery.addBindValue(calendar.enabled ? 1 : 0);
    updateQuery.addBindValue(calendar.calendarId);
    if (!updateQuery.exec()) {
        qCWarning(ServiceLogger) << "Failed to update CalDAV calendar:" << updateQuery.lastError().text();
        return false;
    }
    if (updateQuery.numRowsAffected() == 1) {
        return true;
    }

    SqliteQuery insertQuery(m_database);
    if (!insertQuery.prepare("INSERT INTO caldavCalendar "
                             "(calendarID, accountID, href, displayName, color, scheduleTypeID, privileges, syncToken, "
                             "initialSyncCompleted, enabled) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV calendar insert:" << insertQuery.lastError().text();
        return false;
    }
    insertQuery.addBindValue(calendar.calendarId);
    insertQuery.addBindValue(calendar.accountId);
    insertQuery.addBindValue(calendar.href);
    insertQuery.addBindValue(calendar.displayName);
    insertQuery.addBindValue(calendar.color);
    insertQuery.addBindValue(calendar.scheduleTypeID);
    insertQuery.addBindValue(calendar.privileges);
    insertQuery.addBindValue(calendar.syncToken);
    insertQuery.addBindValue(calendar.initialSyncCompleted ? 1 : 0);
    insertQuery.addBindValue(calendar.enabled ? 1 : 0);
    if (!insertQuery.exec()) {
        qCWarning(ServiceLogger) << "Failed to insert CalDAV calendar:" << insertQuery.lastError().text();
        return false;
    }
    return true;
}

bool DAccountManagerDataBase::updateCalDavCalendarSyncToken(const QString &calendarID, const QString &syncToken)
{
    if (calendarID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("UPDATE caldavCalendar SET syncToken = ? WHERE calendarID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV sync token update:" << query.lastError().text();
        return false;
    }
    query.addBindValue(syncToken);
    query.addBindValue(calendarID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to update CalDAV sync token:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}

bool DAccountManagerDataBase::updateCalDavRetryState(const QString &accountID, int retryCount,
                                                      const QDateTime &nextRetryAt,
                                                      const QString &failureReason,
                                                      DCalDavErrorCode failureCode)
{
    if (accountID.isEmpty() || retryCount < 0 || !nextRetryAt.isValid()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("UPDATE caldavAccount SET retryCount = ?, nextRetryAt = ?, "
                      "syncStatus = ?, failureReason = ?, failureCode = ? WHERE accountID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV retry state update:" << query.lastError().text();
        return false;
    }
    query.addBindValue(retryCount);
    query.addBindValue(nextRetryAt.toUTC().toString(Qt::ISODate));
    query.addBindValue(DCalDavSyncStatus::RetryScheduled);
    query.addBindValue(failureReason);
    query.addBindValue(static_cast<int>(failureCode));
    query.addBindValue(accountID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to update CalDAV retry state:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}

bool DAccountManagerDataBase::clearCalDavRetryState(const QString &accountID)
{
    if (accountID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("UPDATE caldavAccount SET retryCount = 0, nextRetryAt = NULL WHERE accountID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV retry state clear:" << query.lastError().text();
        return false;
    }
    query.addBindValue(accountID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to clear CalDAV retry state:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}

QStringList DAccountManagerDataBase::dueCalDavAccountRetryIDs(const QDateTime &now) const
{
    QStringList accountIDs;
    if (!now.isValid()) {
        return accountIDs;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT accountID FROM caldavAccount WHERE nextRetryAt IS NOT NULL "
                      "AND nextRetryAt != '' AND nextRetryAt <= ?")) {
        return accountIDs;
    }
    query.addBindValue(now.toUTC().toString(Qt::ISODate));
    if (!query.exec()) {
        return accountIDs;
    }
    while (query.next()) {
        accountIDs.append(query.value(0).toString());
    }
    return accountIDs;
}

QStringList DAccountManagerDataBase::dueCalDavOutboxAccountIDs(const QDateTime &now) const
{
    QStringList accountIDs;
    if (!now.isValid()) {
        return accountIDs;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT DISTINCT accountID FROM caldavOutbox WHERE failureType = ? "
                      "AND nextRetryAt IS NOT NULL AND nextRetryAt != '' AND nextRetryAt <= ?")) {
        return accountIDs;
    }
    query.addBindValue(static_cast<int>(DCalDavOutboxItem::NetworkFailure));
    query.addBindValue(now.toUTC().toString(Qt::ISODate));
    if (!query.exec()) {
        return accountIDs;
    }
    while (query.next()) {
        accountIDs.append(query.value(0).toString());
    }
    return accountIDs;
}

QDateTime DAccountManagerDataBase::earliestCalDavRetryAt() const
{
    SqliteQuery query(m_database);
    if (!query.prepare("SELECT MIN(retryAt) FROM ("
                      "SELECT nextRetryAt AS retryAt FROM caldavAccount "
                      "WHERE nextRetryAt IS NOT NULL AND nextRetryAt != '' "
                      "UNION ALL "
                      "SELECT nextRetryAt AS retryAt FROM caldavOutbox "
                      "WHERE failureType = ? AND nextRetryAt IS NOT NULL AND nextRetryAt != '')")) {
        return QDateTime();
    }
    query.addBindValue(static_cast<int>(DCalDavOutboxItem::NetworkFailure));
    if (!query.exec() || !query.next()) {
        return QDateTime();
    }
    return QDateTime::fromString(query.value(0).toString(), Qt::ISODate);
}

bool DAccountManagerDataBase::updateCalDavCalendarInitialSyncCompleted(
    const QString &calendarID, bool completed)
{
    if (calendarID.isEmpty()) {
        return false;
    }
    SqliteQuery query(m_database);
    if (!query.prepare(QStringLiteral(
            "UPDATE caldavCalendar SET initialSyncCompleted = ? WHERE calendarID = ?"))) {
        return false;
    }
    query.addBindValue(completed ? 1 : 0);
    query.addBindValue(calendarID);
    return query.exec();
}

bool DAccountManagerDataBase::updateCalDavSyncStatus(const QString &accountID, int syncStatus,
                                                     const QDateTime &lastSuccessfulSync,
                                                     const QString &failureReason,
                                                     DCalDavErrorCode failureCode)
{
    if (accountID.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("UPDATE caldavAccount SET syncStatus = ?, lastSuccessfulSync = ?, failureReason = ?, failureCode = ? "
                       "WHERE accountID = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV status update:" << query.lastError().text();
        return false;
    }
    query.addBindValue(syncStatus);
    query.addBindValue(lastSuccessfulSync.isValid() ? lastSuccessfulSync.toString(Qt::ISODate) : QString());
    query.addBindValue(failureReason);
    query.addBindValue(static_cast<int>(failureCode));
    query.addBindValue(accountID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to update CalDAV sync status:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}


DCalDavEventMappingInfo DAccountManagerDataBase::getCalDavEventMapping(const QString &accountID,
                                                                       const QString &href)
{
    DCalDavEventMappingInfo mapping;
    if (accountID.isEmpty() || href.isEmpty()) {
        return mapping;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT localScheduleID, accountID, calendarID, uid, href, etag, originalIcs "
                       "FROM caldavEventMapping WHERE accountID = ? AND href = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV event mapping query:" << query.lastError().text();
        return mapping;
    }
    query.addBindValue(accountID);
    query.addBindValue(href);
    if (query.exec() && query.next()) {
        mapping.localScheduleID = query.value("localScheduleID").toString();
        mapping.accountID = query.value("accountID").toString();
        mapping.calendarID = query.value("calendarID").toString();
        mapping.uid = query.value("uid").toString();
        mapping.href = query.value("href").toString();
        mapping.etag = query.value("etag").toString();
        mapping.originalIcs = query.value("originalIcs").toString();
    }
    return mapping;
}

DCalDavEventMappingInfo DAccountManagerDataBase::getCalDavEventMappingByLocalScheduleID(
    const QString &accountID, const QString &localScheduleID)
{
    DCalDavEventMappingInfo mapping;
    if (accountID.isEmpty() || localScheduleID.isEmpty()) {
        return mapping;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT localScheduleID, accountID, calendarID, uid, href, etag, originalIcs "
                       "FROM caldavEventMapping WHERE accountID = ? AND localScheduleID = ?")) {
        return mapping;
    }
    query.addBindValue(accountID);
    query.addBindValue(localScheduleID);
    if (!query.exec() || !query.next()) {
        return mapping;
    }
    mapping.localScheduleID = query.value("localScheduleID").toString();
    mapping.accountID = query.value("accountID").toString();
    mapping.calendarID = query.value("calendarID").toString();
    mapping.uid = query.value("uid").toString();
    mapping.href = query.value("href").toString();
    mapping.etag = query.value("etag").toString();
    mapping.originalIcs = query.value("originalIcs").toString();
    return mapping;
}

DCalDavEventMappingInfo::List DAccountManagerDataBase::getCalDavEventMappingList(const QString &accountID,
                                                                                       const QString &calendarID)
{
    DCalDavEventMappingInfo::List mappings;
    if (accountID.isEmpty() || calendarID.isEmpty()) {
        return mappings;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("SELECT localScheduleID, accountID, calendarID, uid, href, etag, originalIcs "
                       "FROM caldavEventMapping WHERE accountID = ? AND calendarID = ? ORDER BY href")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV event mapping list query:" << query.lastError().text();
        return mappings;
    }
    query.addBindValue(accountID);
    query.addBindValue(calendarID);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to query CalDAV event mappings:" << query.lastError().text();
        return mappings;
    }

    while (query.next()) {
        DCalDavEventMappingInfo mapping;
        mapping.localScheduleID = query.value("localScheduleID").toString();
        mapping.accountID = query.value("accountID").toString();
        mapping.calendarID = query.value("calendarID").toString();
        mapping.uid = query.value("uid").toString();
        mapping.href = query.value("href").toString();
        mapping.etag = query.value("etag").toString();
        mapping.originalIcs = query.value("originalIcs").toString();
        mappings.append(mapping);
    }
    return mappings;
}

bool DAccountManagerDataBase::upsertCalDavEventMapping(const DCalDavEventMappingInfo &mapping)
{
    if (mapping.localScheduleID.isEmpty() || mapping.accountID.isEmpty() || mapping.calendarID.isEmpty()
        || mapping.uid.isEmpty() || mapping.href.isEmpty()) {
        return false;
    }

    SqliteQuery staleQuery(m_database);
    if (!staleQuery.prepare("DELETE FROM caldavEventMapping WHERE accountID = ? AND localScheduleID = ? AND href != ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare stale CalDAV event mapping cleanup:" << staleQuery.lastError().text();
        return false;
    }
    staleQuery.addBindValue(mapping.accountID);
    staleQuery.addBindValue(mapping.localScheduleID);
    staleQuery.addBindValue(mapping.href);
    if (!staleQuery.exec()) {
        qCWarning(ServiceLogger) << "Failed to clean stale CalDAV event mappings:" << staleQuery.lastError().text();
        return false;
    }

    SqliteQuery updateQuery(m_database);
    if (!updateQuery.prepare("UPDATE caldavEventMapping SET localScheduleID = ?, calendarID = ?, uid = ?, "
                             "etag = ?, originalIcs = ? WHERE accountID = ? AND href = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV event mapping update:" << updateQuery.lastError().text();
        return false;
    }
    updateQuery.addBindValue(mapping.localScheduleID);
    updateQuery.addBindValue(mapping.calendarID);
    updateQuery.addBindValue(mapping.uid);
    updateQuery.addBindValue(mapping.etag);
    updateQuery.addBindValue(mapping.originalIcs);
    updateQuery.addBindValue(mapping.accountID);
    updateQuery.addBindValue(mapping.href);
    if (!updateQuery.exec()) {
        qCWarning(ServiceLogger) << "Failed to update CalDAV event mapping:" << updateQuery.lastError().text();
        return false;
    }
    if (updateQuery.numRowsAffected() == 1) {
        return true;
    }

    SqliteQuery insertQuery(m_database);
    if (!insertQuery.prepare("INSERT INTO caldavEventMapping "
                             "(localScheduleID, accountID, calendarID, uid, href, etag, originalIcs) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?)")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV event mapping insert:" << insertQuery.lastError().text();
        return false;
    }
    insertQuery.addBindValue(mapping.localScheduleID);
    insertQuery.addBindValue(mapping.accountID);
    insertQuery.addBindValue(mapping.calendarID);
    insertQuery.addBindValue(mapping.uid);
    insertQuery.addBindValue(mapping.href);
    insertQuery.addBindValue(mapping.etag);
    insertQuery.addBindValue(mapping.originalIcs);
    if (!insertQuery.exec()) {
        qCWarning(ServiceLogger) << "Failed to insert CalDAV event mapping:" << insertQuery.lastError().text();
        return false;
    }
    return true;
}

bool DAccountManagerDataBase::deleteCalDavEventMapping(const QString &accountID, const QString &href)
{
    if (accountID.isEmpty() || href.isEmpty()) {
        return false;
    }

    SqliteQuery query(m_database);
    if (!query.prepare("DELETE FROM caldavEventMapping WHERE accountID = ? AND href = ?")) {
        qCWarning(ServiceLogger) << "Failed to prepare CalDAV event mapping deletion:" << query.lastError().text();
        return false;
    }
    query.addBindValue(accountID);
    query.addBindValue(href);
    if (!query.exec()) {
        qCWarning(ServiceLogger) << "Failed to delete CalDAV event mapping:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() == 1;
}

void DAccountManagerDataBase::createDB()
{
    qCDebug(ServiceLogger) << "Creating account manager database";
    dbOpen();
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(getDBPath());
    //如果不存在该文件则创建
    if (!file.exists()) {
        m_database.open();
        m_database.close();
        qCDebug(ServiceLogger) << "Created new database file:" << getDBPath();
    }
    //将权限修改为600（对文件的所有者可以读写，其他用户不可读不可写）
    if (!file.setPermissions(QFile::WriteOwner | QFile::ReadOwner)) {
        qCWarning(ServiceLogger) << "Failed to set database file permissions:" << file.errorString();
    }

    if (m_database.open()) {
        SqliteQuery query(m_database);
        bool res = true;
        //创建帐户管理表
        res = query.exec(sql_create_accountManager);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create accountManager table:" << query.lastError().text();
        }

        res = query.exec(sql_create_caldavAccount);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create caldavAccount table:" << query.lastError().text();
        } else {
            QSet<QString> calDavAccountColumns;
            SqliteQuery columnQuery(m_database);
            if (columnQuery.exec("PRAGMA table_info(caldavAccount)")) {
                while (columnQuery.next()) {
                    calDavAccountColumns.insert(columnQuery.value("name").toString());
                }
            }
            if (!calDavAccountColumns.contains(QStringLiteral("retryCount"))
                && !columnQuery.exec(
                    "ALTER TABLE caldavAccount ADD COLUMN retryCount INTEGER NOT NULL DEFAULT 0")) {
                qCWarning(ServiceLogger) << "Failed to migrate CalDAV account retry count column:"
                                          << columnQuery.lastError().text();
            }
            if (!calDavAccountColumns.contains(QStringLiteral("nextRetryAt"))
                && !columnQuery.exec("ALTER TABLE caldavAccount ADD COLUMN nextRetryAt DATETIME")) {
                qCWarning(ServiceLogger) << "Failed to migrate CalDAV account retry time column:"
                                          << columnQuery.lastError().text();
            }
            if (!calDavAccountColumns.contains(QStringLiteral("accountColor"))
                && !columnQuery.exec("ALTER TABLE caldavAccount ADD COLUMN accountColor TEXT")) {
                qCWarning(ServiceLogger) << "Failed to migrate CalDAV account color column:"
                                          << columnQuery.lastError().text();
            }
            if (!calDavAccountColumns.contains(QStringLiteral("failureCode"))
                && !columnQuery.exec(
                    "ALTER TABLE caldavAccount ADD COLUMN failureCode INTEGER NOT NULL DEFAULT 0")) {
                qCWarning(ServiceLogger) << "Failed to migrate CalDAV account failure code column:"
                                          << columnQuery.lastError().text();
            }
        }

        res = query.exec(sql_create_caldavCalendar);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create caldavCalendar table:" << query.lastError().text();
        } else {
            bool hasScheduleTypeID = false;
            SqliteQuery columnQuery(m_database);
            if (columnQuery.exec("PRAGMA table_info(caldavCalendar)")) {
                while (columnQuery.next()) {
                    if (columnQuery.value("name").toString() == QStringLiteral("scheduleTypeID")) {
                        hasScheduleTypeID = true;
                        break;
                    }
                }
            }
            if (!hasScheduleTypeID && !columnQuery.exec(
                    "ALTER TABLE caldavCalendar ADD COLUMN scheduleTypeID TEXT")) {
                qCWarning(ServiceLogger) << "Failed to migrate caldavCalendar schedule type column:"
                                          << columnQuery.lastError().text();
            }
            bool hasInitialSyncCompleted = false;
            SqliteQuery initialSyncColumnQuery(m_database);
            if (initialSyncColumnQuery.exec("PRAGMA table_info(caldavCalendar)")) {
                while (initialSyncColumnQuery.next()) {
                    if (initialSyncColumnQuery.value("name").toString()
                        == QStringLiteral("initialSyncCompleted")) {
                        hasInitialSyncCompleted = true;
                        break;
                    }
                }
            }
            if (!hasInitialSyncCompleted && !initialSyncColumnQuery.exec(
                    "ALTER TABLE caldavCalendar ADD COLUMN initialSyncCompleted INTEGER NOT NULL DEFAULT 0")) {
                qCWarning(ServiceLogger) << "Failed to migrate CalDAV initial sync column:"
                                          << initialSyncColumnQuery.lastError().text();
            }
        }

        res = query.exec(sql_create_caldavEventMapping);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create caldavEventMapping table:" << query.lastError().text();
        }

        res = query.exec(sql_create_caldavCategoryMapping);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create caldavCategoryMapping table:" << query.lastError().text();
        }

        res = query.exec(sql_create_caldavOutbox);
        if (res) {
            QSet<QString> outboxColumns;
            SqliteQuery outboxColumnQuery(m_database);
            if (outboxColumnQuery.exec("PRAGMA table_info(caldavOutbox)")) {
                while (outboxColumnQuery.next()) {
                    outboxColumns.insert(outboxColumnQuery.value("name").toString());
                }
            }
            if (!outboxColumns.contains(QStringLiteral("conflictIcs"))
                && !outboxColumnQuery.exec("ALTER TABLE caldavOutbox ADD COLUMN conflictIcs TEXT")) {
                qCWarning(ServiceLogger) << "Failed to migrate CalDAV conflict snapshot column:"
                                          << outboxColumnQuery.lastError().text();
            }
            if (!outboxColumns.contains(QStringLiteral("serverIcs"))
                && !outboxColumnQuery.exec("ALTER TABLE caldavOutbox ADD COLUMN serverIcs TEXT")) {
                qCWarning(ServiceLogger) << "Failed to migrate CalDAV server snapshot column:"
                                          << outboxColumnQuery.lastError().text();
            }
        }
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create caldavOutbox table:" << query.lastError().text();
        }

        res = query.exec(sql_create_caldavAccountDeletionCleanup);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create CalDAV deletion cleanup table:"
                                     << query.lastError().text();
        }

        //日历通用设置
        res = query.exec(sql_create_calendargeneralsettings);
        if (!res) {
            qCWarning(ServiceLogger) << "Failed to create calendargeneralsettings table:" << query.lastError().text();
        }

        //创建calendargeneralsettings的触发器，数据有变动时，更新dt_update
        query.exec("SELECT name FROM sqlite_master WHERE type = 'trigger' and name = 'trigger_sync_calendargeneralsettings_datetime_when_insert'");
        if (!query.next()) {
            query.exec("CREATE  TRIGGER  trigger_sync_calendargeneralsettings_datetime_when_insert AFTER INSERT "
                       "ON calendargeneralsettings  "
                       "BEGIN  "
                       "    replace into calendargeneralsettings (vch_key, vch_value) values('dt_update', datetime(CURRENT_TIMESTAMP,'localtime')); "
                       "END;");
        }
        query.exec("SELECT name FROM sqlite_master WHERE type = 'trigger' and name = 'trigger_sync_calendargeneralsettings_datetime_when_update'");
        if (!query.next()) {
            query.exec("CREATE  TRIGGER  trigger_sync_calendargeneralsettings_datetime_when_update AFTER UPDATE "
                       "ON calendargeneralsettings  "
                       "BEGIN  "
                       "    replace into calendargeneralsettings (vch_key, vch_value) values('dt_update', datetime(CURRENT_TIMESTAMP,'localtime')); "
                       "END;");
        }
        query.exec("SELECT name FROM sqlite_master WHERE type = 'trigger' and name = 'trigger_sync_calendargeneralsettings_datetime_when_delete'");
        if (!query.next()) {
            query.exec("CREATE  TRIGGER  trigger_sync_calendargeneralsettings_datetime_when_delete AFTER DELETE "
                       "ON calendargeneralsettings  "
                       "BEGIN  "
                       "    replace into calendargeneralsettings (vch_key, vch_value) values('dt_update', datetime(CURRENT_TIMESTAMP,'localtime')); "
                       "END;");
        }
        if (query.isActive()) {
            query.finish();
        }
    }
}

void DAccountManagerDataBase::initAccountManagerDB()
{
    qCDebug(ServiceLogger) << "Initializing account manager database tables";
    QDateTime currentDateTime = QDateTime::currentDateTime();
    currentDateTime.setOffsetFromUtc(currentDateTime.offsetFromUtc());
    m_database = QSqlDatabase::database(NameAccountManager);
    m_database.setDatabaseName(getDBPath());
    //帐户管理表
    {
        SqliteQuery query(m_database);
        QString strsql("INSERT INTO accountManager                              \
                       (accountID, accountName, displayName,                    \
                       accountState, accountAvatar, accountDescription,            \
                       accountType, dbName,dBusPath,dBusInterface,dtCreate, dtUpdate,      \
                       expandStatus, isDeleted)                                \
                   VALUES(:accountID,:accountName,:displayName,:accountState,:accountAvatar,   \
                   :accountDescription,:accountType,:dbName,:dBusPath,:dBusInterface,:dtCreate,:dtUpdate,            \
                   :expandStatus, :isDeleted);");
        if (query.prepare(strsql)) {
            query.bindValue(":accountID", DDataBase::createUuid());
            query.bindValue(":accountName", "localAccount");
            query.bindValue(":displayName", "localAccount");
            query.bindValue(":accountState", 0);
            query.bindValue(":accountAvatar", "");
            query.bindValue(":accountDescription", "");
            query.bindValue(":accountType", 0);
            query.bindValue(":dbName", m_loaclDB);
            query.bindValue(":dBusPath", serviceBasePath + "/account_local");
            query.bindValue(":dBusInterface", accountServiceInterface);
            query.bindValue(":dtCreate", dtToString(currentDateTime));
            query.bindValue(":expandStatus", 1);
            query.bindValue(":isDeleted", 0);

            if (query.exec()) {
                if (query.isActive()) {
                    query.finish();
                }
            } else {
                qCWarning(ServiceLogger) << "Failed to create local account:" << query.lastError().text();
            }
        } else {
            qCWarning(ServiceLogger) << "Failed to prepare local account creation query:" << query.lastError().text();
        }
    }

    //通用设置
    {
        SqliteQuery query(m_database);
        if (query.exec("insert into calendargeneralsettings values"
                       "('firstDayOfWeek',  '7'),"
                       "('timeShowType',    '0')")) {
            qCDebug(ServiceLogger) << "Initialized calendar general settings";
            m_database.commit();
        } else {
            qCWarning(ServiceLogger) << "Failed to initialize calendar general settings:" << query.lastError().text();
        }
    }
}

void DAccountManagerDataBase::setLoaclDB(const QString &loaclDB)
{
    qCDebug(ServiceLogger) << "Setting local database name:" << loaclDB;
    m_loaclDB = loaclDB;
}
