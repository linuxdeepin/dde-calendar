// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccount.h"

#include "units.h"
#include "commondef.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DAccount::DAccount(DAccount::Type type)
    : m_displayName("")
    , m_accountID("")
    , m_accountName("")
    , m_dbName("")
    , m_dbusPath("")
    , m_accountType(type)
    , m_avatar("")
    , m_description("")
    , m_syncTag(0)
    , m_accountState(Account_Invalid)
    , m_syncState(Sync_Normal)
    , m_syncFreq(SyncFreq_15Mins)
    , m_intervalTime(0)
    , m_isExpandDisplay(true)
{
    qCDebug(CommonLogger) << "DAccount::DAccount, type:" << type;
}

QString DAccount::displayName() const
{
    // qCDebug(CommonLogger) << "DAccount::displayName";
    return m_displayName;
}

void DAccount::setDisplayName(const QString &displayName)
{
    // qCDebug(CommonLogger) << "Setting display name for account" << m_accountID << "from" << m_displayName << "to" << displayName;
    m_displayName = displayName;
}

QString DAccount::accountID() const
{
    // qCDebug(CommonLogger) << "DAccount::accountID";
    return m_accountID;
}

void DAccount::setAccountID(const QString &accountID)
{
    qCDebug(CommonLogger) << "Setting account ID from" << m_accountID << "to" << accountID;
    m_accountID = accountID;
}

QString DAccount::accountName() const
{
    // qCDebug(CommonLogger) << "DAccount::accountName";
    return m_accountName;
}

void DAccount::setAccountName(const QString &accountName)
{
    // qCDebug(CommonLogger) << "Setting account name for" << m_accountID << "from" << m_accountName << "to" << accountName;
    m_accountName = accountName;
}

QString DAccount::dbusPath() const
{
    // qCDebug(CommonLogger) << "DAccount::dbusPath";
    return m_dbusPath;
}

void DAccount::setDbusPath(const QString &dbusPath)
{
    // qCDebug(CommonLogger) << "Setting DBus path for account" << m_accountID << "from" << m_dbusPath << "to" << dbusPath;
    m_dbusPath = dbusPath;
}

DAccount::Type DAccount::accountType() const
{
    // qCDebug(CommonLogger) << "DAccount::accountType";
    return m_accountType;
}

void DAccount::setAccountType(const Type &accountType)
{
    // qCDebug(CommonLogger) << "DAccount::setAccountType, accountType:" << accountType;
    m_accountType = accountType;
}

bool DAccount::isExpandDisplay() const
{
    // qCDebug(CommonLogger) << "DAccount::isExpandDisplay";
    return m_isExpandDisplay;
}

void DAccount::setIsExpandDisplay(bool isExpandDisplay)
{
    // qCDebug(CommonLogger) << "DAccount::setIsExpandDisplay, isExpandDisplay:" << isExpandDisplay;
    m_isExpandDisplay = isExpandDisplay;
}

bool DAccount::isNetWorkAccount()
{
    // qCDebug(CommonLogger) << "DAccount::isNetWorkAccount";
    return m_accountType != Type::Account_Local;
}

int DAccount::syncTag() const
{
    // qCDebug(CommonLogger) << "DAccount::syncTag";
    return m_syncTag;
}

void DAccount::setSyncTag(int syncTag)
{
    // qCDebug(CommonLogger) << "DAccount::setSyncTag, syncTag:" << syncTag;
    m_syncTag = syncTag;
}

DAccount::AccountSyncState DAccount::syncState() const
{
    // qCDebug(CommonLogger) << "DAccount::syncState";
    return m_syncState;
}

void DAccount::setSyncState(AccountSyncState syncState)
{
    // qCDebug(CommonLogger) << "DAccount::setSyncState, syncState:" << syncState;
    m_syncState = syncState;
}

QString DAccount::avatar() const
{
    // qCDebug(CommonLogger) << "DAccount::avatar";
    return m_avatar;
}

void DAccount::setAvatar(const QString &avatar)
{
    // qCDebug(CommonLogger) << "Setting avatar for account" << m_accountID;
    m_avatar = avatar;
}

QString DAccount::description() const
{
    // qCDebug(CommonLogger) << "DAccount::description";
    return m_description;
}

void DAccount::setDescription(const QString &description)
{
    // qCDebug(CommonLogger) << "Setting description for account" << m_accountID;
    m_description = description;
}

QDateTime DAccount::dtCreate() const
{
    // qCDebug(CommonLogger) << "DAccount::dtCreate";
    return m_dtCreate;
}

void DAccount::setDtCreate(const QDateTime &dtCreate)
{
    // qCDebug(CommonLogger) << "Setting creation time for account" << m_accountID << "to" << dtCreate.toString();
    m_dtCreate = dtCreate;
}

QDateTime DAccount::dtDelete() const
{
    // qCDebug(CommonLogger) << "DAccount::dtDelete";
    return m_dtDelete;
}

void DAccount::setDtDelete(const QDateTime &dtDelete)
{
    // qCDebug(CommonLogger) << "DAccount::setDtDelete, dtDelete:" << dtDelete;
    m_dtDelete = dtDelete;
}

QDateTime DAccount::dtUpdate() const
{
    // qCDebug(CommonLogger) << "DAccount::dtUpdate";
    return m_dtUpdate;
}

void DAccount::setDtUpdate(const QDateTime &dtUpdate)
{
    // qCDebug(CommonLogger) << "DAccount::setDtUpdate, dtUpdate:" << dtUpdate;
    m_dtUpdate = dtUpdate;
}

bool DAccount::toJsonString(const DAccount::Ptr &account, QString &jsonStr)
{
    // qCDebug(CommonLogger) << "DAccount::toJsonString";
    if (account.isNull()) {
        qCWarning(CommonLogger) << "Cannot convert null account to JSON";
        return false;
    }
    QJsonObject rootObj;
    rootObj.insert("accountID", account->accountID());
    rootObj.insert("displayName", account->displayName());
    rootObj.insert("accountName", account->accountName());
    rootObj.insert("dbusPath", account->dbusPath());
    rootObj.insert("dbusInterface", account->dbusInterface());
    rootObj.insert("type", account->accountType());
    rootObj.insert("avatar", account->avatar());
    rootObj.insert("description", account->description());
    rootObj.insert("syncTag", account->syncTag());
    rootObj.insert("accountState", int(account->accountState()));
    rootObj.insert("syncState", account->syncState());
    rootObj.insert("dtCreate", dtToString(account->dtCreate()));
    rootObj.insert("dbName", account->dbName());
    rootObj.insert("isExpandDisplay", account->isExpandDisplay());
    rootObj.insert("dtLastSync", dtToString(account->dtLastSync()));
    rootObj.insert("syncFreq", syncFreqToJsonString(account));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObj);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

bool DAccount::fromJsonString(Ptr &account, const QString &jsonStr)
{
    qCDebug(CommonLogger) << "DAccount::fromJsonString";
    if (account.isNull()) {
        qCDebug(CommonLogger) << "Creating new account instance for JSON parsing";
        account = DAccount::Ptr(new DAccount);
    }

    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse account JSON. Error:" << jsonError.errorString();
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("accountID")) {
        account->setAccountID(rootObj.value("accountID").toString());
    }
    if (rootObj.contains("displayName")) {
        account->setDisplayName(rootObj.value("displayName").toString());
    }
    if (rootObj.contains("accountName")) {
        account->setAccountName(rootObj.value("accountName").toString());
    }
    if (rootObj.contains("dbusPath")) {
        account->setDbusPath(rootObj.value("dbusPath").toString());
    }
    if (rootObj.contains("dbusInterface")) {
        account->setDbusInterface(rootObj.value("dbusInterface").toString());
    }

    if (rootObj.contains("type")) {
        account->setAccountType(static_cast<DAccount::Type>(rootObj.value("type").toInt()));
    }
    if (rootObj.contains("avatar")) {
        account->setAvatar(rootObj.value("avatar").toString());
    }
    if (rootObj.contains("description")) {
        account->setDescription(rootObj.value("description").toString());
    }
    if (rootObj.contains("syncTag")) {
        account->setSyncTag(rootObj.value("syncTag").toInt());
    }
    if (rootObj.contains("accountState")) {
        account->setAccountState(static_cast<AccountState>(rootObj.value("accountState").toInt()));
    }
    if (rootObj.contains("syncState")) {
        account->setSyncState(static_cast<AccountSyncState>(rootObj.value("syncState").toInt()));
    }
    if (rootObj.contains("dtCreate")) {
        account->setDtCreate(dtFromString(rootObj.value("dtCreate").toString()));
    }
    if (rootObj.contains("dbName")) {
        account->setDbName(rootObj.value("dbName").toString());
    }
    if (rootObj.contains("isExpandDisplay")) {
        account->setIsExpandDisplay(rootObj.value("isExpandDisplay").toBool());
    }
    if (rootObj.contains("dtLastSync")) {
        account->setDtLastSync(dtFromString(rootObj.value("dtLastSync").toString()));
    }

    if (rootObj.contains("syncFreq")) {
        syncFreqFromJsonString(account, rootObj.value("syncFreq").toString());
    }

    return true;
}

bool DAccount::toJsonListString(const DAccount::List &accountList, QString &jsonStr)
{
    qCDebug(CommonLogger) << "DAccount::toJsonListString, list size:" << accountList.size();
    QJsonArray jsArr;
    foreach (auto account, accountList) {
        QJsonObject jsonAccount;
        QString strAccount;
        toJsonString(account, strAccount);
        jsonAccount.insert("account", strAccount);
        jsArr.append(jsonAccount);
    }
    QJsonObject jsObj;
    jsObj.insert("accounts", jsArr);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsObj);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

bool DAccount::fromJsonListString(List &accountList, const QString &jsonStr)
{
    qCDebug(CommonLogger) << "DAccount::fromJsonListString";
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse account list JSON. Error:" << jsonError.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("accounts")) {
        QJsonArray jsArr = rootObj.value("accounts").toArray();
        foreach (auto ja, jsArr) {
            QJsonObject jsObj = ja.toObject();
            DAccount::Ptr account = DAccount::Ptr(new DAccount);
            QString strAcc = jsObj.value("account").toString();
            if (fromJsonString(account, strAcc)) {
                accountList.append(account);
            } else {
                qCWarning(CommonLogger) << "Failed to parse account JSON:" << strAcc;
            }
        }
    }
    return true;
}

QString DAccount::dbName() const
{
    // qCDebug(CommonLogger) << "DAccount::dbName";
    return m_dbName;
}

void DAccount::setDbName(const QString &dbName)
{
    // qCDebug(CommonLogger) << "Setting database name for account" << m_accountID << "from" << m_dbName << "to" << dbName;
    m_dbName = dbName;
}

QString DAccount::cloudPath() const
{
    // qCDebug(CommonLogger) << "DAccount::cloudPath";
    return m_cloudPath;
}

void DAccount::setCloudPath(const QString &cloudPath)
{
    // qCDebug(CommonLogger) << "Setting cloud path for account" << m_accountID << "from" << m_cloudPath << "to" << cloudPath;
    m_cloudPath = cloudPath;
}

DAccount::SyncFreqType DAccount::syncFreq() const
{
    // qCDebug(CommonLogger) << "DAccount::syncFreq";
    return m_syncFreq;
}

void DAccount::setSyncFreq(SyncFreqType syncFreq)
{
    // qCDebug(CommonLogger) << "DAccount::setSyncFreq, syncFreq:" << syncFreq;
    m_syncFreq = syncFreq;
}

int DAccount::intervalTime() const
{
    // qCDebug(CommonLogger) << "DAccount::intervalTime";
    return m_intervalTime;
}

void DAccount::setIntervalTime(int intervalTime)
{
    // qCDebug(CommonLogger) << "DAccount::setIntervalTime, intervalTime:" << intervalTime;
    m_intervalTime = intervalTime;
}

QString DAccount::dbusInterface() const
{
    // qCDebug(CommonLogger) << "DAccount::dbusInterface";
    return m_dbusInterface;
}

void DAccount::setDbusInterface(const QString &dbusInterface)
{
    qCDebug(CommonLogger) << "DAccount::setDbusInterface, dbusInterface:" << dbusInterface;
    m_dbusInterface = dbusInterface;
}

DAccount::AccountStates DAccount::accountState() const
{
    // qCDebug(CommonLogger) << "DAccount::accountState";
    return m_accountState;
}

void DAccount::setAccountState(const AccountStates &accountState)
{
    qCDebug(CommonLogger) << "DAccount::setAccountState, accountState:" << accountState;
    m_accountState = accountState;
}

QDateTime DAccount::dtLastSync() const
{
    // qCDebug(CommonLogger) << "DAccount::dtLastSync";
    return m_dtLastSync;
}

void DAccount::setDtLastSync(const QDateTime &dtLastSync)
{
    // qCDebug(CommonLogger) << "DAccount::setDtLastSync, dtLastSync:" << dtLastSync;
    m_dtLastSync = dtLastSync;
}

QString DAccount::syncFreqToJsonString(const DAccount::Ptr &account)
{
    qCDebug(CommonLogger) << "DAccount::syncFreqToJsonString";
    QJsonObject rootObj;
    rootObj.insert("syncFreq", account->syncFreq());
    rootObj.insert("m_intervalTime", account->intervalTime());
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObj);
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

void DAccount::syncFreqFromJsonString(const DAccount::Ptr &account, const QString &syncFreqStr)
{
    qCDebug(CommonLogger) << "DAccount::syncFreqFromJsonString";
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(syncFreqStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse sync frequency JSON. Error:" << jsonError.errorString();
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("syncFreq")) {
        account->setSyncFreq(SyncFreqType(rootObj.value("syncFreq").toInt()));
    }
    if (rootObj.contains("m_intervalTime")) {
        account->setIntervalTime(rootObj.value("m_intervalTime").toInt());
    }
}
