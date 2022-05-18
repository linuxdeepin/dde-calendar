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
#include "dscheduletype.h"

#include "daccount.h"
#include "units.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DScheduleType::DScheduleType()
{
}

DScheduleType::DScheduleType(DAccount *parent)
{
}

DAccount *DScheduleType::account() const
{
    return m_account;
}

DScheduleType::Privilege DScheduleType::privilege() const
{
    return m_privilege;
}

void DScheduleType::setPrivilege(const Privilege &privilege)
{
    m_privilege = privilege;
}

DTypeColor DScheduleType::typeColor() const
{
    return m_typeColor;
}

void DScheduleType::setTypeColor(const DTypeColor &typeColor)
{
    m_typeColor = typeColor;
}

QString DScheduleType::typeID() const
{
    return m_typeID;
}

void DScheduleType::setTypeID(const QString &typeID)
{
    m_typeID = typeID;
}

QString DScheduleType::displayName() const
{
    return m_displayName;
}

void DScheduleType::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

DScheduleType::ShowState DScheduleType::showState() const
{
    return m_showState;
}

void DScheduleType::setShowState(const ShowState &showState)
{
    m_showState = showState;
}

QString DScheduleType::typeName() const
{
    return m_typeName;
}

void DScheduleType::setTypeName(const QString &typeName)
{
    m_typeName = typeName;
}

QString DScheduleType::typePath() const
{
    return m_typePath;
}

void DScheduleType::setTypePath(const QString &typePath)
{
    m_typePath = typePath;
}

QString DScheduleType::description() const
{
    return m_description;
}

void DScheduleType::setDescription(const QString &description)
{
    m_description = description;
}

QDateTime DScheduleType::dtCreate() const
{
    return m_dtCreate;
}

void DScheduleType::setDtCreate(const QDateTime &dtCreate)
{
    m_dtCreate = dtCreate;
}

QDateTime DScheduleType::dtUpdate() const
{
    return m_dtUpdate;
}

void DScheduleType::setDtUpdate(const QDateTime &dtUpdate)
{
    m_dtUpdate = dtUpdate;
}

QDateTime DScheduleType::dtDelete() const
{
    return m_dtDelete;
}

void DScheduleType::setDtDelete(const QDateTime &dtDelete)
{
    m_dtDelete = dtDelete;
}

int DScheduleType::deleted() const
{
    return m_deleted;
}

void DScheduleType::setDeleted(int deleted)
{
    m_deleted = deleted;
}

bool DScheduleType::fromJsonString(DScheduleType::Ptr &scheduleType, const QString &jsonStr)
{
    //TODO:反序列化
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "error:" << jsonError.errorString();
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("typeID")) {
        scheduleType->setTypeID(rootObj.value("typeID").toString());
    }

    if (rootObj.contains("typeName")) {
        scheduleType->setTypeID(rootObj.value("typeName").toString());
    }

    if (rootObj.contains("displayName")) {
        scheduleType->setTypeID(rootObj.value("displayName").toString());
    }

    if (rootObj.contains("typePath")) {
        scheduleType->setTypeID(rootObj.value("typePath").toString());
    }

    if (rootObj.contains("TypeColor")) {
        QJsonObject colorObject = rootObj.value("TypeColor").toObject();
        DTypeColor typeColor;
        if (colorObject.contains("colorID")) {
            typeColor.setColorID(colorObject.value("colorID").toInt());
        }
        if (colorObject.contains("colorCode")) {
            typeColor.setColorCode(colorObject.value("colorCode").toString());
        }
        if (colorObject.contains("privilege")) {
            typeColor.setPrivilege(static_cast<DTypeColor::Privilege>(colorObject.value("privilege").toInt()));
        }
        scheduleType->setTypeColor(typeColor);
    }

    if (rootObj.contains("description")) {
        scheduleType->setDescription(rootObj.value("description").toString());
    }

    if (rootObj.contains("privilege")) {
        scheduleType->setPrivilege(static_cast<Privilege>(rootObj.value("privilege").toInt()));
    }

    if (rootObj.contains("dtCreate")) {
        scheduleType->setDtCreate(QDateTime::fromString(rootObj.value("dtCreate").toString(), Qt::ISODate));
    }

    if (rootObj.contains("dtDelete")) {
        scheduleType->setDtDelete(QDateTime::fromString(rootObj.value("dtDelete").toString(), Qt::ISODate));
    }

    if (rootObj.contains("dtUpdate")) {
        scheduleType->setDtUpdate(QDateTime::fromString(rootObj.value("dtUpdate").toString(), Qt::ISODate));
    }

    if (rootObj.contains("showState")) {
        scheduleType->setShowState(static_cast<ShowState>(rootObj.value("showState").toInt()));
    }

    if (rootObj.contains("isDeleted")) {
        scheduleType->setDeleted(rootObj.value("isDeleted").toInt());
    }
    return true;
}

bool DScheduleType::toJsonString(const DScheduleType::Ptr &scheduleType, QString &jsonStr)
{
    //TODO:序列化
    QJsonObject rootObject;
    rootObject.insert("typeID", scheduleType->typeID());
    rootObject.insert("typeName", scheduleType->typeName());
    rootObject.insert("displayName", scheduleType->displayName());
    rootObject.insert("typePath", scheduleType->typePath());
    //类型颜色信息
    QJsonObject colorObject;
    colorObject.insert("colorID", scheduleType->typeColor().colorID());
    colorObject.insert("colorCode", scheduleType->typeColor().colorCode());
    colorObject.insert("privilege", scheduleType->typeColor().privilege());
    rootObject.insert("TypeColor", colorObject);

    rootObject.insert("description", scheduleType->description());
    rootObject.insert("privilege", scheduleType->privilege());
    rootObject.insert("dtCreate", dtToString(scheduleType->dtCreate()));
    rootObject.insert("dtDelete", dtToString(scheduleType->dtDelete()));
    rootObject.insert("dtUpdate", dtToString(scheduleType->dtUpdate()));
    rootObject.insert("showState", scheduleType->showState());
    rootObject.insert("isDeleted", scheduleType->deleted());

    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}
