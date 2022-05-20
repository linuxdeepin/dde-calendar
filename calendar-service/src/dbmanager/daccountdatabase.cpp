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
#include "daccountdatabase.h"

#include "units.h"

#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QSqlError>
#include <QFile>

DAccountDataBase::DAccountDataBase(const DAccount::Ptr &account, QObject *parent)
    : DDataBase(parent)
    , m_account(account)
{
    setConnectionName("accountLocal");
}

QString DAccountDataBase::createSchedule(const DSchedule::Ptr &schedule)
{
    if (!schedule.isNull()) {
        QSqlQuery query(m_database);
        schedule->setUid(DDataBase::createUuid());
        schedule->setCreated(QDateTime::currentDateTime());

        QString strSql("INSERT INTO schedules                                                   \
                       (scheduleID, scheduleTypeID, summary, description, allDay, dtStart   \
                       , dtEnd, isAlarm, isLunar, ics, fileName, dtCreate, isDeleted)   \
                       VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");

        query.prepare(strSql);
        query.addBindValue(schedule->schedulingID());
        query.addBindValue(schedule->scheduleTypeID());
        query.addBindValue(schedule->summary());
        query.addBindValue(schedule->description());
        query.addBindValue(schedule->allDay());
        query.addBindValue(dtToString(schedule->dtStart()));
        query.addBindValue(dtToString(schedule->dtEnd()));
        query.addBindValue(schedule->hasEnabledAlarms());
        query.addBindValue(schedule->lunnar());
        query.addBindValue(schedule->toIcsString(schedule));
        query.addBindValue(schedule->fileName());
        query.addBindValue(dtToString(schedule->created()));
        query.addBindValue(0);
        if (!query.exec()) {
            schedule->setUid("");
            qWarning() << "createSchedule error:" << query.lastError();
        }
    } else {
        schedule->setUid("");
        qWarning() << "schedule is null";
    }
    return schedule->uid();
}

bool DAccountDataBase::updateSchedule(const DSchedule::Ptr &schedule)
{
    bool resbool = false;
    if (!schedule.isNull()) {
        QSqlQuery query(m_database);
        QString strSql("UPDATE schedules                                                  \
                       SET scheduleTypeID=?, summary=?, description=?, allDay=?           \
                , dtStart=?, dtEnd=?, isAlarm=?, isLunar=?, ics=?, fileName=?             \
                , dtUpdate=? WHERE scheduleID= ?;");
        query.prepare(strSql);
        query.addBindValue(schedule->scheduleTypeID());
        query.addBindValue(schedule->summary());
        query.addBindValue(schedule->description());
        query.addBindValue(schedule->allDay());
        query.addBindValue(dtToString(schedule->dtStart()));
        query.addBindValue(dtToString(schedule->dtEnd()));
        query.addBindValue(schedule->hasEnabledAlarms());
        query.addBindValue(schedule->lunnar());
        query.addBindValue(DSchedule::toIcsString(schedule));
        query.addBindValue(schedule->fileName());
        query.addBindValue(dtToString(schedule->lastModified()));
        query.addBindValue(schedule->schedulingID());
        if (query.exec()) {
            resbool = true;
        } else {
            qWarning() << Q_FUNC_INFO << query.lastError();
        }
    }
    return resbool;
}

bool DAccountDataBase::deleteScheduleByScheduleID(const QString &scheduleID, const int isDeleted)
{
    QString strSql;
    if (isDeleted) {
        strSql = "DELETE FROM schedules WHERE scheduleID=?;";
    } else {
        strSql = QString("UPDATE schedules SET dtDelete = %1 , isDeleted = 1  WHERE scheduleID=?").arg(dtToString(QDateTime::currentDateTime()));
    }
    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.addBindValue(scheduleID);
    return query.exec();
}

bool DAccountDataBase::deleteSchedulesByScheduleTypeID(const QString &typeID, const int isDeleted)
{
    QString strSql;
    if (isDeleted) {
        strSql = "DELETE FROM schedules WHERE scheduleTypeID=?;";
    } else {
        strSql = QString("UPDATE schedules SET dtDelete = %1 , isDeleted = 1  WHERE scheduleTypeID=?").arg(dtToString(QDateTime::currentDateTime()));
    }
    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.addBindValue(typeID);
    return query.exec();
}

QString DAccountDataBase::querySchedulesWithParameter(const QString &params, const int isDeleted)
{
    //TODO:根据条件获取数据
}

void DAccountDataBase::initDBData()
{
    createDB();
    initTypeColor();
    initScheduleDB();
    initScheduleType();
    initAccountDB();
}

QString DAccountDataBase::createScheduleType(const DScheduleType::Ptr &scheduleType)
{
    QString res("");
    QString strSql("INSERT INTO scheduleType                                                                \
                           (typeID, typeName, typeDisplayName, typePath, typeColorID, description, privilege,       \
                            showState, dtCreate, dtUpdate, dtDelete, isDeleted)                                     \
                           VALUES(:typeID, :typeName, :typeDisplayName, :typePath, :typeColorID, :description       \
                            ,:privilege, :showState, :dtCreate, :dtUpdate, :dtDelete, :isDeleted)");
    QSqlQuery query(m_database);
    query.prepare(strSql);
    res = DDataBase::createUuid();
    query.bindValue(":typeID", res);
    query.bindValue(":typeName", scheduleType->typeName());
    query.bindValue(":typeDisplayName", scheduleType->displayName());
    query.bindValue(":typePath", scheduleType->typePath());
    query.bindValue(":typeColorID", scheduleType->typeColor().colorID());
    query.bindValue(":description", scheduleType->description());
    query.bindValue(":privilege", scheduleType->privilege());
    query.bindValue(":showState", scheduleType->showState());
    query.bindValue(":dtCreate", dtConvert(scheduleType->dtCreate()));
    query.bindValue(":dtUpdate", dtConvert(scheduleType->dtUpdate()));
    query.bindValue(":dtDelete", dtConvert(scheduleType->dtDelete()));
    query.bindValue("isDeleted", 0);

    if (!query.exec()) {
        qWarning() << __FUNCTION__ << query.lastError();
        res = "";
    }
    return res;
}

DScheduleType::Ptr DAccountDataBase::getScheduleTypeByID(const QString &typeID, const int isDeleted)
{
    QString strSql("SELECT                              \
                   st.typeID , st.typeName ,st.typeDisplayName ,        \
                   st.typePath ,st.typeColorID ,tc.ColorHex ,           \
                   tc.privilege as colorPri,st.description,             \
                   st.privilege as typePri,st.showState ,st.syncTag ,   \
                   st.dtCreate ,st.dtUpdate ,st.dtDelete ,st.isDeleted  \
               FROM                                                     \
                   scheduleType st                                      \
               inner join typeColor tc on                               \
                   st.typeColorID = tc.ColorID                          \
               WHERE                                                    \
                   st.typeID = ? AND st.isDeleteD = ?");
    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.addBindValue(typeID);
    query.addBindValue(isDeleted);
    DScheduleType::Ptr type;
    if (query.exec() && query.next()) {
        type = DScheduleType::Ptr(new DScheduleType);
        type->setTypeID(typeID);
        type->setTypeName(query.value("typeName").toString());
        type->setDisplayName(query.value("typeDisplayName").toString());
        type->setTypePath(query.value("typePath").toString());
        DTypeColor color;
        color.setColorID(query.value("typeColorID").toInt());
        color.setColorCode(query.value("ColorHex").toString());
        color.setPrivilege(static_cast<DTypeColor::Privilege>(query.value("colorPri").toInt()));
        type->setTypeColor(color);
        type->setDescription(query.value("description").toString());
        type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("typePri").toInt()));
        type->setSyncTag(query.value("syncTag").toInt());
        type->setDtCreate(query.value("dtCreate").toDateTime());
        type->setDtCreate(query.value("dtUpdate").toDateTime());
        type->setDtCreate(query.value("dtDelete").toDateTime());
        type->setDeleted(query.value("isDeleted").toInt());
    }
    return type;
}

DScheduleType::List DAccountDataBase::getScheduleTypeList(const int isDeleted)
{
    QString strSql("SELECT                              \
                   st.typeID , st.typeName ,st.typeDisplayName ,        \
                   st.typePath ,st.typeColorID ,tc.ColorHex ,           \
                   tc.privilege as colorPri,st.description,             \
                   st.privilege as typePri,st.showState ,st.syncTag ,   \
                   st.dtCreate ,st.dtUpdate ,st.dtDelete ,st.isDeleted  \
               FROM                                                     \
                   scheduleType st                                      \
               inner join typeColor tc on                               \
                   st.typeColorID = tc.ColorID                          \
               WHERE                                                    \
                   st.isDeleteD = ?");
    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.addBindValue(isDeleted);
    DScheduleType::List typeList;
    if (query.exec()) {
        while (query.next()) {
            DScheduleType::Ptr type = DScheduleType::Ptr(new DScheduleType);
            type->setTypeID(query.value("typeID").toString());
            type->setTypeName(query.value("typeName").toString());
            type->setDisplayName(query.value("typeDisplayName").toString());
            type->setTypePath(query.value("typePath").toString());
            DTypeColor color;
            color.setColorID(query.value("typeColorID").toInt());
            color.setColorCode(query.value("ColorHex").toString());
            color.setPrivilege(static_cast<DTypeColor::Privilege>(query.value("colorPri").toInt()));
            type->setTypeColor(color);
            type->setDescription(query.value("description").toString());
            type->setPrivilege(static_cast<DScheduleType::Privilege>(query.value("typePri").toInt()));
            type->setSyncTag(query.value("syncTag").toInt());
            type->setDtCreate(query.value("dtCreate").toDateTime());
            type->setDtCreate(query.value("dtUpdate").toDateTime());
            type->setDtCreate(query.value("dtDelete").toDateTime());
            type->setDeleted(query.value("isDeleted").toInt());
            typeList.append(type);
        }
    } else {
        qWarning() << "getScheduleTypeList error:" << query.lastError();
    }
    return typeList;
}

bool DAccountDataBase::scheduleTypeByUsed(const QString &typeID, const int isDeleted)
{
    QString strSql("SELECT COUNT(scheduleTypeID) FROM schedules WHERE scheduleTypeID = ? AND isDeleted = ?;");
    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.addBindValue(typeID);
    query.addBindValue(isDeleted);
    int typeCount = 0;
    if (query.exec() && query.next()) {
        typeCount = query.value(0).toInt();
    }
    return typeCount;
}

bool DAccountDataBase::deleteScheduleTypeByID(const QString &typeID, const int isDeleted)
{
    QSqlQuery query(m_database);
    QString strSql;
    if (isDeleted == 0) {
        //弱删除
        QDateTime &&dtCurrent = QDateTime::currentDateTime();
        strSql = QString("UPDATE scheduleType  SET  dtDelete=%1, isDeleted=1  WHERE typeID=?;")
                     .arg(dtToString(dtCurrent));
    } else {
        //真删除
        strSql = "DELETE FROM scheduleType WHERE typeID = ?";
    }

    query.prepare(strSql);
    query.addBindValue(typeID);
    bool res = query.exec();
    if (!res) {
        qWarning() << "DELETE scheduleType error by typeID:" << typeID << " " << query.lastError();
    }
    return res;
}

bool DAccountDataBase::updateScheduleType(const DScheduleType::Ptr &scheduleType)
{
    bool res = false;
    QSqlQuery query(m_database);
    QString strSql("UPDATE scheduleType                     \
                       SET typeName=?, typeDisplayName=?, typePath=?, typeColorID=?, description=?,     \
                       privilege=?, showState=?, dtCreate=?, dtUpdate=?, dtDelete=?, isDeleted=?        \
                       WHERE typeID = ?");
    query.prepare(strSql);
    query.addBindValue(scheduleType->typeName());
    query.addBindValue(scheduleType->displayName());
    query.addBindValue(scheduleType->typePath());
    query.addBindValue(scheduleType->typeColor().colorID());
    query.addBindValue(scheduleType->description());
    query.addBindValue(scheduleType->privilege());
    query.addBindValue(scheduleType->showState());
    query.addBindValue(dtConvert(scheduleType->dtCreate()));
    query.addBindValue(dtConvert(scheduleType->dtUpdate()));
    query.addBindValue(dtConvert(scheduleType->dtDelete()));
    query.addBindValue(scheduleType->deleted());
    query.addBindValue(scheduleType->typeID());
    if (query.exec()) {
        res = true;
    } else {
        qWarning() << "updateScheduleType error:" << query.lastError();
    }
    return res;
}

DAccount::Ptr DAccountDataBase::getAccountInfo()
{
    QString strSql("SELECT syncState, accountName, displayName, cloudPath,      \
                   accountType, syncFreq, intervalTime, syncTag          \
                   FROM account WHERE id = 1;");
    QSqlQuery query(m_database);
    query.prepare(strSql);
    if (query.exec() && query.next()) {
        m_account->setSyncState(query.value("syncState").toInt());
        m_account->setAccountName(query.value("accountName").toString());
        m_account->setDisplayName(query.value("displayName").toString());
        m_account->setCloudPath(query.value("cloudPath").toString());
        m_account->setAccountType(static_cast<DAccount::Type>(query.value("accountType").toInt()));
        m_account->setSyncFreq(query.value("syncFreq").toInt());
        m_account->setIntervalTime(query.value("intervalTime").toInt());
        m_account->setSyncTag(query.value("syncTag").toInt());
    }
    return m_account;
}

void DAccountDataBase::setAccountInfo(const DAccount::Ptr &account)
{
    QString strSql("UPDATE account                                                              \
                   SET syncState=?, accountName=?, displayName=?,                               \
                  cloudPath=?, accountType=?, syncFreq=?, intervalTime=?, syncTag=?             \
                   WHERE id=1;");
    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.addBindValue(account->syncState());
    query.addBindValue(account->accountName());
    query.addBindValue(account->displayName());
    query.addBindValue(account->cloudPath());
    query.addBindValue(account->accountType());
    query.addBindValue(account->syncFreq());
    query.addBindValue(account->intervalTime());
    query.addBindValue(account->syncTag());
    if (!query.exec()) {
        qWarning() << query.lastError();
    }
}

bool DAccountDataBase::addTypeColor(const int typeColorNo, const QString &strColorHex, const int privilege)
{
    QString strSql("INSERT INTO TypeColor                   \
                   (ColorID, ColorHex, privilege)           \
                   VALUES(:ColorID, :ColorHex, :privilege)");

    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.bindValue(":ColorID", typeColorNo);
    query.bindValue(":ColorHex", strColorHex);
    query.bindValue(":privilege", privilege);
    bool res = false;
    if (query.exec()) {
        res = true;
    } else {
        qWarning() << __FUNCTION__ << query.lastError();
    }
    return res;
}

void DAccountDataBase::createDB()
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
        //帐户信息表
        QString createAccountSql("CREATE table account(                 \
                                 id integer not null primary key,       \
                                 syncState integer not null,            \
                                 accountName text not null,             \
                                 displayName text not null,             \
                                 cloudPath text not null,               \
                                 accountType integer not null,          \
                                 syncFreq integer not null,             \
                                 intervalTime integer,                  \
                                 syncTag    integer                     \
                                )");
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
                                  allDay BOOL not null,                 \
                                  dtStart DATETIME not null,            \
                                  dtEnd DATETIME not null,              \
                                  isAlarm   INTEGER                     \
                                  titlePinyin TEXT,                     \
                                  isLunar INTEGER not null,             \
                                  ics TEXT not null,                    \
                                  fileName  TEXT,                       \
                                  dtCreate DATETIME not null,           \
                                  dtUpdate DATETIME ,                   \
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
                                syncTag INTEGER,                        \
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

void DAccountDataBase::initScheduleDB()
{
    //创建数据库时，需要初始化的日程数据
}

void DAccountDataBase::initScheduleType()
{
    //创建数据库时，需要初始化的类型数据
    //    createScheduleType()
}

void DAccountDataBase::initTypeColor()
{
    addTypeColor(1, "#ff5e97", 1);
    addTypeColor(2, "#ff9436", 1);
    addTypeColor(3, "#ffdc00", 1);
    addTypeColor(4, "#5bdd80", 1);
    addTypeColor(5, "#00b99b", 1);
    addTypeColor(6, "#4293ff", 1);
    addTypeColor(7, "#5d51ff", 1);
    addTypeColor(8, "#a950ff", 1);
    addTypeColor(9, "#717171", 1);
}

void DAccountDataBase::initAccountDB()
{
    QString strSql("INSERT                                  \
                   INTO                                     \
                   account                                  \
               (syncState,accountName,                      \
                   displayName,cloudPath,accountType,       \
                   syncFreq,intervalTime,syncTag)           \
               VALUES(0,?,?,'',0,0,0,0);");
    QSqlQuery query(m_database);
    query.prepare(strSql);
    query.addBindValue("localAccount");
    query.addBindValue("localAccount");
    if (!query.exec()) {
        qWarning() << "initAccountDB error:" << query.lastError();
    }
}
