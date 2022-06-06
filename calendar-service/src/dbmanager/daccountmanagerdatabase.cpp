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
#include "daccountmanagerdatabase.h"

#include "units.h"

#include <QSqlQuery>
#include <QtDebug>
#include <QSqlError>
#include <QFile>

DAccountManagerDataBase::DAccountManagerDataBase(QObject *parent)
    : DDataBase(parent)
{
    setConnectionName("accountmanager");
}

void DAccountManagerDataBase::initDBData()
{
    createDB();
    initAccountManagerDB();
}

DAccount::List DAccountManagerDataBase::getAccountList()
{
    DAccount::List accountList;
    QString strSql("SELECT accountID,accountName, displayName, syncState, accountAvatar,               \
                   accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate, dtDelete, dtUpdate, isDeleted         \
                   FROM accountManager");
    QSqlQuery query(m_database);
    if (query.prepare(strSql) && query.exec()) {
        while (query.next()) {
            DAccount::Type type = static_cast<DAccount::Type>(query.value("accountType").toInt());
            DAccount::Ptr account(new DAccount(type));
            account->setAccountID(query.value("accountID").toString());
            account->setAccountName(query.value("accountName").toString());
            account->setDisplayName(query.value("displayName").toString());
            account->setSyncState(static_cast<DAccount::AccountSyncState>(query.value("syncState").toInt()));
            account->setAvatar(query.value("accountAvatar").toString());
            account->setDescription(query.value("accountDescription").toString());
            account->setDbName(query.value("dbName").toString());
            account->setDbusPath(query.value("dBusPath").toString());
            account->setDbusInterface(query.value("dBusInterface").toString());
            account->setDtCreate(QDateTime::fromString(query.value("dtCreate").toString(), Qt::ISODate));
            accountList.append(account);
        }
    } else {
        qWarning() << "getAccountList error:" << query.lastError();
    }
    return accountList;
}

DAccount::Ptr DAccountManagerDataBase::getAccountByID(const QString &accountID)
{
    QString strSql("SELECT accountName, displayName, syncState, accountAvatar,               \
                   accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate, dtDelete, dtUpdate, isDeleted         \
                   FROM accountManager WHERE accountID = ?");
    QSqlQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(accountID);
        if (query.exec() && query.next()) {
            DAccount::Type type = static_cast<DAccount::Type>(query.value("accountType").toInt());
            DAccount::Ptr account(new DAccount(type));
            account->setAccountID(accountID);
            account->setAccountName(query.value("accountName").toString());
            account->setDisplayName(query.value("displayName").toString());
            account->setSyncState(static_cast<DAccount::AccountSyncState>(query.value("syncState").toInt()));
            account->setAvatar(query.value("accountAvatar").toString());
            account->setDescription(query.value("accountDescription").toString());
            account->setDbName(query.value("dbName").toString());
            account->setDbusPath(query.value("dBusPath").toString());
            account->setDbusInterface(query.value("dBusInterface").toString());
            account->setDtCreate(QDateTime::fromString(query.value("dtCreate").toString(), Qt::ISODate));
            return account;
        } else {
            qWarning() << "getAccountByID error:" << query.lastError();
        }
    } else {
        qWarning() << "getAccountByID error:" << query.lastError();
    }

    return nullptr;
}

QString DAccountManagerDataBase::addAccountInfo(const DAccount::Ptr &accountInfo)
{
    QSqlQuery query(m_database);
    //生成唯一标识
    if (accountInfo->accountID().isEmpty()) {
        accountInfo->setAccountID(DDataBase::createUuid());
    }
    QString strSql("INSERT INTO accountManager                                          \
                   (accountID, accountName, displayName, syncState, accountAvatar,  \
                    accountDescription, accountType, dbName,dBusPath,dBusInterface, dtCreate,         \
                     isDeleted)                                                \
                   VALUES(?,?, ?, ?,?,?,?,?,?,?,?,?)");
    if (query.prepare(strSql)) {
        query.addBindValue(accountInfo->accountID());
        query.addBindValue(accountInfo->accountName());
        query.addBindValue(accountInfo->displayName());
        query.addBindValue(accountInfo->syncState());
        query.addBindValue(accountInfo->avatar());
        query.addBindValue(accountInfo->description());
        query.addBindValue(accountInfo->accountType());
        query.addBindValue(accountInfo->dbName());
        query.addBindValue(accountInfo->dbusPath());
        query.addBindValue(accountInfo->dbusInterface());
        query.addBindValue(dtToString(accountInfo->dtCreate()));
        query.addBindValue(0);
        if (!query.exec()) {
            qWarning() << "addAccountInfo error:" << query.lastError();
            accountInfo->setAccountID("");
        }
    } else {
        qWarning() << "addAccountInfo error:" << query.lastError();
        accountInfo->setAccountID("");
    }

    return accountInfo->accountID();
}

bool DAccountManagerDataBase::updateAccountInfo(const DAccount::Ptr &accountInfo)
{
    QString strSql("UPDATE accountManager                                                           \
                   SET accountName=?, displayName=?, syncState= ?,                   \
                   accountAvatar=?, accountDescription=?, accountType=?, dbName=?,               \
                   dBusPath = ? ,dBusInterface = ? WHERE accountID=?");
    QSqlQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(accountInfo->accountName());
        query.addBindValue(accountInfo->displayName());
        query.addBindValue(accountInfo->syncState());
        query.addBindValue(accountInfo->avatar());
        query.addBindValue(accountInfo->description());
        query.addBindValue(accountInfo->accountType());
        query.addBindValue(accountInfo->dbName());
        query.addBindValue(accountInfo->dbusPath());
        query.addBindValue(accountInfo->dbusInterface());
        query.addBindValue(accountInfo->accountID());
        res = query.exec();
    }
    if (!res) {
        qWarning() << "updateAccountInfo error:" << query.lastError();
    }

    return res;
}

bool DAccountManagerDataBase::deleteAccountInfo(const QString &accountID)
{
    QString strSql("DELETE FROM accountManager      \
                   WHERE accountID=?");
    QSqlQuery query(m_database);
    bool res = false;
    if (query.prepare(strSql)) {
        query.addBindValue(accountID);

        res = query.exec();
    }
    query.addBindValue(accountID);

    if (!res) {
        qWarning() << "deleteAccountInfo error:" << query.lastError();
    }
    return res;
}

DCalendarGeneralSettings::Ptr DAccountManagerDataBase::getCalendarGeneralSettings()
{
    DCalendarGeneralSettings::Ptr cgSet(new DCalendarGeneralSettings);
    QString strSql("SELECT  firstDayOfWeek, timeShowType        \
                   FROM calendargeneralsettings WHERE id = 1");
    QSqlQuery query(m_database);
    if (query.prepare(strSql) && query.exec() && query.next()) {
        cgSet->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(query.value("firstDayOfWeek").toInt()));
        cgSet->setTimeShowType(static_cast<DCalendarGeneralSettings::TimeShowType>(query.value("timeShowType").toInt()));
    }
    return cgSet;
}

void DAccountManagerDataBase::setCalendarGeneralSettings(const DCalendarGeneralSettings::Ptr &cgSet)
{
    QString strSql("UPDATE calendargeneralsettings              \
                   SET firstDayOfWeek=?, timeShowType=?         \
                   WHERE id=1;");
    QSqlQuery query(m_database);
    if (query.prepare(strSql)) {
        query.addBindValue(cgSet->firstDayOfWeek());
        query.addBindValue(cgSet->timeShowType());
        if (!query.exec()) {
            qWarning() << "UPDATE calendargeneralsettings error," << query.lastError();
        }
    } else {
        qWarning() << "UPDATE calendargeneralsettings error," << query.lastError();
    }
}

void DAccountManagerDataBase::createDB()
{
    dbOpen();
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(getDBPath());
    //如果不存在该文件则创建
    if (!file.exists()) {
        m_database.open();
        m_database.close();
    }
    //将权限修改为600（对文件的所有者可以读写，其他用户不可读不可写）
    if (!file.setPermissions(QFile::WriteOwner | QFile::ReadOwner)) {
        qWarning() << "permissions cannot be modified，error:" << file.errorString();
    }

    if (m_database.open()) {
        QSqlQuery query(m_database);
        bool res = true;
        //创建帐户管理表
        QString accountManagerSql("CREATE TABLE accountManager (    \
                               id INTEGER NOT NULL PRIMARY KEY,     \
                               accountID TEXT NOT NULL,             \
                               accountName TEXT NOT NULL,           \
                               displayName TEXT NOT NULL,           \
                               syncState INTEGER not null,          \
                               accountAvatar TEXT,                  \
                               accountDescription TEXT ,            \
                               accountType INTEGER not null,        \
                               dbName TEXT not null,                \
                               dBusPath TEXT not null,              \
                               dBusInterface TEXT not null,         \
                               dtCreate DATETIME not null,    \
                               dtDelete DATETIME,             \
                               dtUpdate DATETIME,             \
                               isDeleted INTEGER not null)");
        res = query.exec(accountManagerSql);
        if (!res) {
            qWarning() << "accountManager create failed.error:" << query.lastError();
        }


        //日历通用设置
        QString cgSetSql("CREATE TABLE calendargeneralsettings(     \
                         id INTEGER NOT NULL PRIMARY KEY,           \
                         firstDayOfWeek INTEGER NOT NULL,           \
                         timeShowType INTEGER NOT NULL              \
                         )");
        res = query.exec(cgSetSql);
        if (!res) {
            qWarning() << "uploadTask create failed.error:" << query.lastError();
        }

        if (query.isActive()) {
            query.finish();
        }
    }
}

void DAccountManagerDataBase::initAccountManagerDB()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    currentDateTime.setOffsetFromUtc(currentDateTime.offsetFromUtc());
    m_database = QSqlDatabase::database("accountmanager");
    m_database.setDatabaseName(getDBPath());
    //帐户管理表
    {
        QSqlQuery query(m_database);
        QString strsql("INSERT INTO accountManager                              \
                       (accountID, accountName, displayName,                    \
                       syncState, accountAvatar, accountDescription,            \
                       accountType, dbName,dBusPath,dBusInterface,dtCreate, dtUpdate,      \
                       isDeleted)                                \
                   VALUES(:accountID,:accountName,:displayName,:syncState,:accountAvatar,   \
                   :accountDescription,:accountType,:dbName,:dBusPath,:dBusInterface,:dtCreate,:dtUpdate,            \
                    :isDeleted);");
        if (query.prepare(strsql)) {
            query.bindValue(":accountID", DDataBase::createUuid());
            query.bindValue(":accountName", "localAccount");
            query.bindValue(":displayName", "localAccount");
            query.bindValue(":syncState", 0);
            query.bindValue(":accountAvatar", "");
            query.bindValue(":accountDescription", "");
            query.bindValue(":accountType", 0);
            query.bindValue(":dbName", m_loaclDB);
            query.bindValue(":dBusPath", serviceBasePath + "/account_local");
            query.bindValue(":dBusInterface", accountServiceInterface);
            query.bindValue(":dtCreate", dtToString(currentDateTime));
            query.bindValue(":isDeleted", 0);

            if (query.exec()) {
                if (query.isActive()) {
                    query.finish();
                }
            } else {
                qWarning() << __FUNCTION__ << query.lastError();
            }
        } else {
            qWarning() << __FUNCTION__ << query.lastError();
        }
    }

    //通用设置
    {
        QSqlQuery query(m_database);
        QString strsql("INSERT INTO calendargeneralsettings     \
                       (firstDayOfWeek, timeShowType)       \
                       VALUES(?, ?)");
        if (query.prepare(strsql)) {
            query.addBindValue(7);
            query.addBindValue(2);
            if (query.exec()) {
                if (query.isActive()) {
                    query.finish();
                }
                m_database.commit();
            } else {
                qWarning() << __FUNCTION__ << query.lastError();
            }
        } else {
            qWarning() << __FUNCTION__ << query.lastError();
        }
    }
}

void DAccountManagerDataBase::setLoaclDB(const QString &loaclDB)
{
    m_loaclDB = loaclDB;
}
