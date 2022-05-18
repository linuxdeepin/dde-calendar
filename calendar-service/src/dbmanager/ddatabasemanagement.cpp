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
#include "ddatabasemanagement.h"

#include "ddatabase.h"
#include "vcalformat.h"
#include "daccountdatabase.h"

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

const QString localDBName = "localAccount.db";

DDataBaseManagement::DDataBaseManagement()
    : m_newDatabaseName("accountmanager.db")
    , m_oldDatabaseName("scheduler.db")
{
    //旧文件路径
    QString oldDbPatch = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/.config/deepin/dde-daemon/calendar");
    setOldDatabasePath(oldDbPatch);
    //新文件路径
    QString newDbPatch = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/.config/deepin/dde-calendar-service");
    setNewDatabasePath(newDbPatch);
    QString newDB(newDatabasePath() + "/" + m_newDatabaseName);
    //如果新数据库不存在
    if (!databaseExists(newDatabasePath())) {
        QString localAccountDB(newDatabasePath() + "/" + localDBName);
        //创建数据库并初始化数据
        createAccountManagerDB(newDB);
        initAccountManagerDB(newDB);
        createLocalAccountDB(localAccountDB);
        initLocalAccountDB(localAccountDB);

        //判断是否存在旧的数据库
        QString oldDB(oldDatabasePath() + "/" + m_oldDatabaseName);
        if (databaseExists(oldDB)) {
            //TODO:对数据进行迁移
            KCalendarCore::VCalFormat vcal;
        }
    }
}

QString DDataBaseManagement::newDatabasePath() const
{
    return m_newDatabasePath;
}

void DDataBaseManagement::setNewDatabasePath(const QString &newDatabasePath)
{
    m_newDatabasePath = newDatabasePath;
}

QString DDataBaseManagement::oldDatabasePath() const
{
    return m_oldDatabasePath;
}

void DDataBaseManagement::setOldDatabasePath(const QString &oldDatabasePath)
{
    m_oldDatabasePath = oldDatabasePath;
}

bool DDataBaseManagement::databaseExists(const QString &databasePath)
{
    QDir dir;
    bool exist = false;
    if (dir.exists(databasePath)) {
        exist = true;
    } else {
        dir.mkpath(databasePath);
    }
    return exist;
}

void DDataBaseManagement::createAccountManagerDB(const QString &databasePath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE", "accountmanager");
    m_database.setDatabaseName(databasePath);
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(databasePath);
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
                               dtCreate DATETIME not null,    \
                               dtDelete DATETIME,             \
                               dtUpdate DATETIME,             \
                               isDeleted INTEGER not null)");
        res = query.exec(accountManagerSql);
        if (!res) {
            qWarning() << "accountManager create failed.error:" << query.lastError();
        }
        //创建上传任务表
        QString uploadSql("CREATE TABLE uploadTask (                \
                          id INTEGER NOT NULL PRIMARY KEY,          \
                          accountID TEXT NOT NULL,                  \
                          updateOperation integer NOT NULL,         \
                          updateObject integer NOT NULL,            \
                          objectID TEXT NOT NULL,                   \
                          dtCreate DATETIME NOT NULL)");
        res = query.exec(uploadSql);
        if (!res) {
            qWarning() << "uploadTask create failed.error:" << query.lastError();
        }

        //创建提醒任务表
        QString remindTaskSql("CREATE TABLE remindTask (            \
                              id INTEGER NOT NULL PRIMARY KEY,      \
                              accountID TEXT NOT NULL,              \
                              scheduleID TEXT NOT NULL,             \
                              recurID DATETIME ,                    \
                              notifyID integer ,                    \
                              dtRemind DATETIME NOT NULL,           \
                              dtStart DATETIME NOT NULL,            \
                              dtEnd DATETIME NOT NULL)");
        res = query.exec(remindTaskSql);
        if (!res) {
            qWarning() << "uploadTask create failed.error:" << query.lastError();
        }
        if (query.isActive()) {
            query.finish();
        }

        m_database.commit();
    }
}

void DDataBaseManagement::createLocalAccountDB(const QString &databasePath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE", "accountLocal");
    m_database.setDatabaseName(databasePath);
    //这里用QFile来修改日历数据库文件的权限
    QFile file;
    file.setFileName(databasePath);
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
        //帐户信息表
        QString createAccountSql("CREATE table account(                 \
                                 id integer not null primary key,       \
                                 accountID text not null,               \
                                 syncState integer not null,            \
                                 accountName text not null,             \
                                 displayName text not null,             \
                                 path text not null,                    \
                                 accountType integer not null,          \
                                 dtCreate datetime not null,            \
                                 syncFreq integer not null,             \
                                 intervalTime integer)");
        res = query.exec(createAccountSql);
        if (!res) {
            qWarning() << "account create failed.error:" << query.lastError();
        }

        //日程表
        QString createScheduleSql("CREATE TABLE schedules (             \
                                  id INTEGER NOT NULL PRIMARY KEY,      \
                                  scheduleID TEXT not null,             \
                                  scheduleTypeID TEXT not null,         \
                                  summary TEXT not null,                \
                                  description TEXT,                     \
                                  all_day BOOL not null,                \
                                  dtStart DATETIME not null,            \
                                  dtEnd DATETIME not null,              \
                                  rrule TEXT,                           \
                                  remind TEXT,                          \
                                  ignoreList TEXT,                      \
                                  title_pinyin TEXT,                    \
                                  is_Lunar INTEGER not null,            \
                                  ics TEXT not null,                    \
                                  dtCreate DATETIME not null,           \
                                  dtUpdate DATETIME not null,           \
                                  dtDelete DATETIME,                    \
                                  isDeleted INTEGER not null)");
        res = query.exec(createScheduleSql);
        if (!res) {
            qWarning() << "schedules create failed.error:" << query.lastError();
        }

        //类型表
        QString scheduleTypeSql("CREATE TABLE scheduleType (            \
                                id INTEGER NOT NULL PRIMARY KEY,        \
                                typeID TEXT not null,                   \
                                typeName TEXT not null,                 \
                                typeDisplayName TEXT,                   \
                                typePath TEXT,                          \
                                typeColorID INTEGER not null,           \
                                description TEXT ,                      \
                                privilege INTEGER not null,             \
                                showState INTEGER not null,             \
                                dtCreate DATETIME not null,             \
                                dtUpdate DATETIME,                      \
                                dtDelete DATETIME,                      \
                                isDeleted INTEGER not null)");
        res = query.exec(scheduleTypeSql);
        if (!res) {
            qWarning() << "scheduleType create failed.error:" << query.lastError();
        }

        //颜色表
        QString typeColorSql("CREATE TABLE typeColor (              \
                             id INTEGER NOT NULL PRIMARY KEY,       \
                             ColorID INTEGER not null,              \
                             ColorHex TEXT not null,                \
                             privilege INTEGER not null)");
        res = query.exec(typeColorSql);
        if (!res) {
            qWarning() << "typeColorSql create failed.error:" << query.lastError();
        }

        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    }
}

void DDataBaseManagement::initAccountManagerDB(const QString &databasePath)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    currentDateTime.setOffsetFromUtc(currentDateTime.offsetFromUtc());
    m_database = QSqlDatabase::database("accountmanager");
    m_database.setDatabaseName(databasePath);

    QSqlQuery query(m_database);
    QString strsql("INSERT INTO accountManager                              \
                   (accountID, accountName, displayName,                    \
                   syncState, accountAvatar, accountDescription,            \
                   accountType, dbName,dtCreate, dtUpdate,      \
                   isDeleted)                                \
               VALUES(:accountID,:accountName,:displayName,:syncState,:accountAvatar,   \
               :accountDescription,:accountType,:dbName,:dtCreate,:dtUpdate,            \
                :isDeleted);");
    query.prepare(strsql);
    query.bindValue(":accountID", DDataBase::createUuid());
    query.bindValue(":accountName", "localAccount");
    query.bindValue(":displayName", "localAccount");
    query.bindValue(":syncState", 0);
    query.bindValue(":accountAvatar", "");
    query.bindValue(":accountDescription", "");
    query.bindValue(":accountType", 0);
    query.bindValue(":dbName", localDBName);
    query.bindValue(":dtCreate", currentDateTime);
    query.bindValue(":dtUpdate", currentDateTime);
    query.bindValue(":isDeleted", 0);

    if (query.exec()) {
        if (query.isActive()) {
            query.finish();
        }
        m_database.commit();
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
    }
}

void DDataBaseManagement::initLocalAccountDB(const QString &databasePath)
{
    DAccountDataBase localDB;
    localDB.setDBPath(databasePath);
    localDB.dbOpen("accountLocal");
    localDB.initDBData();
}
