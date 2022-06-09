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
#include "ddatabase.h"

#include <QDateTime>
#include <QUuid>
#include <QFile>


const QString DDataBase::NameAccountManager = "AccountManager";
const QString DDataBase::NameSync = "SyncManager";

const QString DDataBase::sql_create_account =
        " CREATE TABLE if not exists account(    "
        " id integer not null primary key,       "
        " syncState integer not null ,           "
        " accountState integer not null,         "
        " accountName text not null,             "
        " displayName text not null,             "
        " cloudPath text  ,                      "
        " accountType integer not null,          "
        " syncFreq integer not null,             "
        " intervalTime integer,                  "
        " syncTag    integer,                    "
        " expandStatus  integer,                 "
        " dtLastUpdate DATETIME                  "
        " )";

//日程表
const QString DDataBase::sql_create_schedules =
        " CREATE TABLE if not exists schedules ("
        " scheduleID TEXT not null primary key, "
        " scheduleTypeID TEXT not null,         "
        " summary TEXT not null,                "
        " description TEXT,                     "
        " allDay BOOL not null,                 "
        " dtStart DATETIME not null,            "
        " dtEnd DATETIME not null,              "
        " isAlarm   INTEGER  ,                  "
        " titlePinyin TEXT,                     "
        " isLunar INTEGER not null,             "
        " ics TEXT not null,                    "
        " fileName  TEXT,                       "
        " dtCreate DATETIME not null,           "
        " dtUpdate DATETIME ,                   "
        " dtDelete DATETIME,                    "
        " isDeleted INTEGER not null)";

//类型表
const QString DDataBase::sql_create_scheduleType =
        " CREATE TABLE if not exists scheduleType (            "
        " typeID TEXT not null PRIMARY KEY,                   "
        " typeName TEXT not null,                 "
        " typeDisplayName TEXT,                   "
        " typePath TEXT,                          "
        " typeColorID INTEGER not null,           "
        " description TEXT ,                      "
        " privilege INTEGER not null,             "
        " showState INTEGER not null,             "
        " syncTag INTEGER,                        "
        " dtCreate DATETIME not null,             "
        " dtUpdate DATETIME,                      "
        " dtDelete DATETIME,                      "
        " isDeleted INTEGER not null)";
//颜色表
const QString DDataBase::sql_create_typeColor =
        " CREATE TABLE if not exists typeColor (              "
        " ColorID INTEGER not null PRIMARY KEY,              "
        " ColorHex TEXT not null,                "
        " privilege INTEGER not null)";

//创建上传任务表
const QString DDataBase::sql_create_uploadTask =
        " CREATE TABLE if not exists uploadTask (                "
        " taskID TEXT NOT NULL PRIMARY KEY,                     "
        " uploadType integer NOT NULL,         "
        " uploadObject integer NOT NULL,            "
        " objectID TEXT NOT NULL,                   "
        " dtCreate DATETIME NOT NULL)";

//创建提醒任务表
const QString DDataBase::sql_create_remindTask =
        " CREATE TABLE if not exists remindTask (            "
        " alarmID TEXT NOT NULL PRIMARY KEY,                "
        " scheduleID TEXT NOT NULL,             "
        " recurID DATETIME ,                    "
        " remindCount INTEGER,                  "
        " notifyID INTEGER ,                    "
        " dtRemind DATETIME NOT NULL,           "
        " dtStart DATETIME NOT NULL,            "
        " dtEnd DATETIME NOT NULL)";


//创建帐户管理表
const QString DDataBase::sql_create_accountManager =
        " CREATE TABLE  if not exists accountManager (    "
        " accountID TEXT NOT NULL PRIMARY KEY,             "
        " accountName TEXT NOT NULL,           "
        " displayName TEXT NOT NULL,           "
        " syncState INTEGER not null,          "
        " accountAvatar TEXT,                  "
        " accountDescription TEXT ,            "
        " accountType INTEGER not null,        "
        " dbName TEXT not null,                "
        " dBusPath TEXT not null,              "
        " dBusInterface TEXT not null,         "
        " dtCreate DATETIME not null,    "
        " dtDelete DATETIME,             "
        " dtUpdate DATETIME,             "
        " expandStatus  integer,               "
        " isDeleted INTEGER not null)";

//日历通用设置
const QString DDataBase::sql_create_calendargeneralsettings =
        " CREATE TABLE  if not exists calendargeneralsettings(     "
        " vch_key TEXT NOT NULL PRIMARY KEY,           "
        " vch_value TEXT NOT NULL           "
        " )";


DDataBase::DDataBase(QObject *parent)
    : QObject(parent)
    , m_DBPath("")
    , m_connectionName("")
{
}

DDataBase::~DDataBase()
{
}

QString DDataBase::getDBPath() const
{
    return m_DBPath;
}

void DDataBase::setDBPath(const QString &DBPath)
{
    m_DBPath = DBPath;
}

QString DDataBase::createUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int DDataBase::createColorId()
{
    return (int(QDateTime().currentDateTime().toTime_t())%(60*60*24*365*10));
}

QString DDataBase::getConnectionName() const
{
    return m_connectionName;
}

void DDataBase::setConnectionName(const QString &connectionName)
{
    m_connectionName = connectionName;
}

void DDataBase::initDBData()
{
    createDB();
}

void DDataBase::dbOpen()
{
    QStringList cntNames = QSqlDatabase::connectionNames();
    if (cntNames.contains(getConnectionName())) {
        m_database = QSqlDatabase::database(getConnectionName());
    } else {
        m_database = QSqlDatabase::addDatabase("QSQLITE", getConnectionName());
        m_database.setDatabaseName(getDBPath());
        m_database.open();
    }
}

bool DDataBase::dbFileExists()
{
    QFile file;
    file.setFileName(getDBPath());
    return file.exists();
}
