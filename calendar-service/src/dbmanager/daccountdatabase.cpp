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

#include "dscheduletype.h"
#include "units.h"

#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QSqlError>

DAccountDataBase::DAccountDataBase(QObject *parent)
    : DDataBase(parent)
{
}

void DAccountDataBase::dbOpen(const QString &connectionName)
{
    QStringList cntNames = QSqlDatabase::connectionNames();
    if (cntNames.contains(connectionName)) {
        m_database = QSqlDatabase::database(connectionName);
    } else {
        m_database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }
    m_database.setDatabaseName(getDBPath());
}

void DAccountDataBase::initDBData()
{
    initTypeColor();
    initScheduleDB();
    initScheduleType();
}

QString DAccountDataBase::createScheduleType(const QString &typeInfo)
{
    DScheduleType::Ptr scheduleType(new DScheduleType);
    QString res("");
    if (DScheduleType::fromJsonString(scheduleType, typeInfo)) {
        DScheduleType::Ptr scheduleType(new DScheduleType);
        if (DScheduleType::fromJsonString(scheduleType, typeInfo)) {
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
            QDateTime dt = scheduleType->dtCreate();
            dt.setOffsetFromUtc(dt.offsetFromUtc());
            query.bindValue(":dtCreate", dt);
            dt = scheduleType->dtDelete();
            dt.setOffsetFromUtc(dt.offsetFromUtc());
            query.bindValue(":dtUpdate", dt);
            dt = scheduleType->dtUpdate();
            dt.setOffsetFromUtc(dt.offsetFromUtc());
            query.bindValue(":dtDelete", dt);
            query.bindValue("isDeleted", scheduleType->deleted());

            if (!query.exec()) {
                qWarning() << __FUNCTION__ << query.lastError();
                res = "";
            }
        }
    } else {
        qWarning() << " Data parsing failed：" << typeInfo;
    }
    return res;
}

QString DAccountDataBase::getScheduleTypeByID(const QString &typeID)
{
}

QString DAccountDataBase::getScheduleTypeList()
{
}

bool DAccountDataBase::scheduleTypeByUsed(const QString &typeID)
{
}

bool DAccountDataBase::deleteScheduleTypeByID(const QString &typeID)
{
    QSqlQuery query(m_database);
    QString strSql("DELETE FROM scheduleType WHERE typeID = ?");
    query.prepare(strSql);
    query.addBindValue(typeID);
    bool res = query.exec();
    if (!res) {
        qWarning() << "DELETE scheduleType error by typeID:" << typeID << " " << query.lastError();
    }
    return res;
}

bool DAccountDataBase::updateScheduleType(const QString &typeInfo)
{
    bool res = false;
    DScheduleType::Ptr scheduleType(new DScheduleType);
    if (DScheduleType::fromJsonString(scheduleType, typeInfo)) {
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
    }
    return res;
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

void DAccountDataBase::initScheduleDB()
{
}

void DAccountDataBase::initScheduleType()
{
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
