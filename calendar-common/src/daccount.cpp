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

#include "units.h"

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
    , m_syncState(0)
    , m_syncFreq(0)
    , m_intervalTime(0)
    , m_isExpandDisplay(true)
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
    return m_accountType != Type::Account_Local;
}

int DAccount::syncTag() const
{
    return m_syncTag;
}

void DAccount::setSyncTag(int syncTag)
{
    m_syncTag = syncTag;
}

int DAccount::syncState() const
{
    return m_syncState;
}

void DAccount::setSyncState(int syncState)
{
    m_syncState = syncState;
}

QString DAccount::avatar() const
{
    return m_avatar;
}

void DAccount::setAvatar(const QString &avatar)
{
    m_avatar = avatar;
}

QString DAccount::description() const
{
    return m_description;
}

void DAccount::setDescription(const QString &description)
{
    m_description = description;
}

QDateTime DAccount::dtCreate() const
{
    return m_dtCreate;
}

void DAccount::setDtCreate(const QDateTime &dtCreate)
{
    m_dtCreate = dtCreate;
}

QDateTime DAccount::dtDelete() const
{
    return m_dtDelete;
}

void DAccount::setDtDelete(const QDateTime &dtDelete)
{
    m_dtDelete = dtDelete;
}

QDateTime DAccount::dtUpdate() const
{
    return m_dtUpdate;
}

void DAccount::setDtUpdate(const QDateTime &dtUpdate)
{
    m_dtUpdate = dtUpdate;
}

bool DAccount::toJsonString(const DAccount::Ptr &account, QString &jsonStr)
{
    QJsonObject rootObj;
    rootObj.insert("accountID", account->accountID());
    rootObj.insert("displayName", account->displayName());
    rootObj.insert("accountName", account->accountName());
    rootObj.insert("dbusPath", account->dbusPath());
    rootObj.insert("type", account->accountType());
    rootObj.insert("avatar", account->avatar());
    rootObj.insert("description", account->description());
    rootObj.insert("syncTag", account->syncTag());
    rootObj.insert("syncState", account->syncState());
    rootObj.insert("dtCreate", dtToString(account->dtCreate()));
    rootObj.insert("dbName", account->dbName());
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObj);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

bool DAccount::fromJsonString(const DAccount::Ptr &account, const QString &jsonStr)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "error:" << jsonError.errorString();
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
    if (rootObj.contains("syncState")) {
        account->setSyncState(rootObj.value("syncState").toInt());
    }
    if (rootObj.contains("dtCreate")) {
        account->setDtCreate(dtFromString(rootObj.value("dtCreate").toString()));
    }
    if (rootObj.contains("dbName")) {
        account->setDbName(rootObj.value("dbName").toString());
    }
    return true;
}

QString DAccount::dbName() const
{
    return m_dbName;
}

void DAccount::setDbName(const QString &dbName)
{
    m_dbName = dbName;
}

QString DAccount::cloudPath() const
{
    return m_cloudPath;
}

void DAccount::setCloudPath(const QString &cloudPath)
{
    m_cloudPath = cloudPath;
}

int DAccount::syncFreq() const
{
    return m_syncFreq;
}

void DAccount::setSyncFreq(int syncFreq)
{
    m_syncFreq = syncFreq;
}

int DAccount::intervalTime() const
{
    return m_intervalTime;
}

void DAccount::setIntervalTime(int intervalTime)
{
    m_intervalTime = intervalTime;
}
