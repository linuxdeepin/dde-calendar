// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dscheduletype.h"

#include "daccount.h"
#include "units.h"
#include "commondef.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DScheduleType::DScheduleType()
    : DScheduleType("")
{
    // qCDebug(CommonLogger) << "DScheduleType default constructor called.";
}

DScheduleType::DScheduleType(const QString &accountID)
    : m_accountID(accountID)
    , m_typeID("")
    , m_typeName("")
    , m_displayName("")
    , m_typePath("")
    , m_typeColor(DTypeColor())
    , m_description("")
    , m_privilege(None)
    , m_showState(Show)
    , m_deleted(0)
    , m_syncTag(0)
{
    // qCDebug(CommonLogger) << "DScheduleType constructor called with accountID:" << accountID;
}

QString DScheduleType::accountID() const
{
    // qCDebug(CommonLogger) << "Getting accountID.";
    return m_accountID;
}

void DScheduleType::setAccountID(const QString &accountID)
{
    // qCDebug(CommonLogger) << "Setting account ID to:" << accountID;
    m_accountID = accountID;
}

DScheduleType::Privileges DScheduleType::privilege() const
{
    // qCDebug(CommonLogger) << "Getting privilege.";
    return m_privilege;
}

void DScheduleType::setPrivilege(const Privileges &privilege)
{
    // qCDebug(CommonLogger) << "Setting privilege to:" << static_cast<int>(privilege);
    m_privilege = privilege;
}

DTypeColor DScheduleType::typeColor() const
{
    // qCDebug(CommonLogger) << "Getting typeColor.";
    return m_typeColor;
}

void DScheduleType::setTypeColor(const DTypeColor &typeColor)
{
    // qCDebug(CommonLogger) << "Setting type color - ID:" << typeColor.colorID() 
    //                       << "Code:" << typeColor.colorCode() 
    //                       << "Privilege:" << static_cast<int>(typeColor.privilege());
    m_typeColor = typeColor;
}

void DScheduleType::setColorID(const QString &colorID)
{
    // qCDebug(CommonLogger) << "Setting color ID to:" << colorID;
    m_typeColor.setColorID(colorID);
}

QString DScheduleType::getColorID() const
{
    // qCDebug(CommonLogger) << "Getting colorID.";
    return m_typeColor.colorID();
}

void DScheduleType::setColorCode(const QString &colorCode)
{
    // qCDebug(CommonLogger) << "Setting color code to:" << colorCode;
    m_typeColor.setColorCode(colorCode);
}

QString DScheduleType::getColorCode() const
{
    // qCDebug(CommonLogger) << "Getting color code.";
    return m_typeColor.colorCode();
}

QString DScheduleType::typeID() const
{
    // qCDebug(CommonLogger) << "Getting typeID.";
    return m_typeID;
}

void DScheduleType::setTypeID(const QString &typeID)
{
    // qCDebug(CommonLogger) << "Setting type ID to:" << typeID;
    m_typeID = typeID;
}

QString DScheduleType::displayName() const
{
    // qCDebug(CommonLogger) << "Getting display name.";
    return m_displayName;
}

void DScheduleType::setDisplayName(const QString &displayName)
{
    // qCDebug(CommonLogger) << "Setting display name to:" << displayName;
    m_displayName = displayName;
}

DScheduleType::ShowState DScheduleType::showState() const
{
    // qCDebug(CommonLogger) << "Getting show state.";
    return m_showState;
}

void DScheduleType::setShowState(const ShowState &showState)
{
    // qCDebug(CommonLogger) << "Setting show state to:" << static_cast<int>(showState);
    m_showState = showState;
}

QString DScheduleType::typeName() const
{
    // qCDebug(CommonLogger) << "Getting type name.";
    return m_typeName;
}

void DScheduleType::setTypeName(const QString &typeName)
{
    // qCDebug(CommonLogger) << "Setting type name to:" << typeName;
    m_typeName = typeName;
}

QString DScheduleType::typePath() const
{
    // qCDebug(CommonLogger) << "Getting type path.";
    return m_typePath;
}

void DScheduleType::setTypePath(const QString &typePath)
{
    // qCDebug(CommonLogger) << "Setting type path to:" << typePath;
    m_typePath = typePath;
}

QString DScheduleType::description() const
{
    // qCDebug(CommonLogger) << "Getting description.";
    return m_description;
}

void DScheduleType::setDescription(const QString &description)
{
    // qCDebug(CommonLogger) << "Setting description to:" << description;
    m_description = description;
}

QDateTime DScheduleType::dtCreate() const
{
    // qCDebug(CommonLogger) << "Getting dtCreate.";
    return m_dtCreate;
}

void DScheduleType::setDtCreate(const QDateTime &dtCreate)
{
    // qCDebug(CommonLogger) << "Setting creation date to:" << dtCreate.toString();
    m_dtCreate = dtCreate;
}

QDateTime DScheduleType::dtUpdate() const
{
    // qCDebug(CommonLogger) << "Getting dtUpdate.";
    return m_dtUpdate;
}

void DScheduleType::setDtUpdate(const QDateTime &dtUpdate)
{
    // qCDebug(CommonLogger) << "Setting update date to:" << dtUpdate.toString();
    m_dtUpdate = dtUpdate;
}

QDateTime DScheduleType::dtDelete() const
{
    // qCDebug(CommonLogger) << "Getting dtDelete.";
    return m_dtDelete;
}

void DScheduleType::setDtDelete(const QDateTime &dtDelete)
{
    // qCDebug(CommonLogger) << "Setting delete date to:" << dtDelete.toString();
    m_dtDelete = dtDelete;
}

int DScheduleType::deleted() const
{
    // qCDebug(CommonLogger) << "Getting deleted status.";
    return m_deleted;
}

void DScheduleType::setDeleted(int deleted)
{
    // qCDebug(CommonLogger) << "Setting deleted status to:" << deleted;
    m_deleted = deleted;
}

bool DScheduleType::fromJsonString(DScheduleType::Ptr &scheduleType, const QString &jsonStr)
{
    qCDebug(CommonLogger) << "Parsing schedule type from JSON string";
    if (scheduleType.isNull()) {
        qCDebug(CommonLogger) << "Creating new schedule type instance for JSON parsing";
        scheduleType = DScheduleType::Ptr(new DScheduleType);
    }
    //反序列化
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse schedule type JSON. Error:" << jsonError.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("accountID")) {
        scheduleType->setAccountID(rootObj.value("accountID").toString());
    }

    if (rootObj.contains("typeID")) {
        scheduleType->setTypeID(rootObj.value("typeID").toString());
    }

    if (rootObj.contains("typeName")) {
        scheduleType->setTypeName(rootObj.value("typeName").toString());
    }

    if (rootObj.contains("displayName")) {
        scheduleType->setDisplayName(rootObj.value("displayName").toString());
    }

    if (rootObj.contains("typePath")) {
        scheduleType->setTypePath(rootObj.value("typePath").toString());
    }

    if (rootObj.contains("TypeColor")) {
        QJsonObject colorObject = rootObj.value("TypeColor").toObject();
        DTypeColor typeColor;
        if (colorObject.contains("colorID")) {
            typeColor.setColorID(colorObject.value("colorID").toString());
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
    qCDebug(CommonLogger) << "Successfully parsed schedule type from JSON.";
    return true;
}

bool DScheduleType::toJsonString(const DScheduleType::Ptr &scheduleType, QString &jsonStr)
{
    qCDebug(CommonLogger) << "Converting schedule type to JSON string";
    if (scheduleType.isNull()) {
        qCWarning(CommonLogger) << "Cannot convert null schedule type to JSON";
        return false;
    }
    //序列化
    QJsonObject rootObject;
    rootObject.insert("accountID", scheduleType->accountID());
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
    rootObject.insert("privilege", int(scheduleType->privilege()));
    rootObject.insert("dtCreate", dtToString(scheduleType->dtCreate()));
    rootObject.insert("dtDelete", dtToString(scheduleType->dtDelete()));
    rootObject.insert("dtUpdate", dtToString(scheduleType->dtUpdate()));
    rootObject.insert("showState", scheduleType->showState());
    rootObject.insert("isDeleted", scheduleType->deleted());

    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    qCDebug(CommonLogger) << "Successfully converted schedule type to JSON:" << jsonStr;
    return true;
}

bool DScheduleType::fromJsonListString(DScheduleType::List &stList, const QString &jsonStr)
{
    // qCDebug(CommonLogger) << "Parsing schedule type list from JSON string";
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse schedule type list JSON. Error:" << jsonError.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("scheduleType")) {
        QJsonArray jsonArray = rootObj.value("scheduleType").toArray();
        for (auto ja : jsonArray) {
            QJsonObject typeObject = ja.toObject();
            DScheduleType::Ptr scheduleType = DScheduleType::Ptr(new DScheduleType);

            if (typeObject.contains("accountID")) {
                scheduleType->setAccountID(typeObject.value("accountID").toString());
            }

            if (typeObject.contains("typeID")) {
                scheduleType->setTypeID(typeObject.value("typeID").toString());
            }

            if (typeObject.contains("typeName")) {
                scheduleType->setTypeName(typeObject.value("typeName").toString());
            }

            if (typeObject.contains("displayName")) {
                scheduleType->setDisplayName(typeObject.value("displayName").toString());
            }

            if (typeObject.contains("typePath")) {
                scheduleType->setTypePath(typeObject.value("typePath").toString());
            }

            if (typeObject.contains("TypeColor")) {
                QJsonObject colorObject = typeObject.value("TypeColor").toObject();
                DTypeColor typeColor;
                if (colorObject.contains("colorID")) {
                    typeColor.setColorID(colorObject.value("colorID").toString());
                }
                if (colorObject.contains("colorCode")) {
                    typeColor.setColorCode(colorObject.value("colorCode").toString());
                }
                if (colorObject.contains("privilege")) {
                    typeColor.setPrivilege(static_cast<DTypeColor::Privilege>(colorObject.value("privilege").toInt()));
                }
                scheduleType->setTypeColor(typeColor);
            }

            if (typeObject.contains("description")) {
                scheduleType->setDescription(typeObject.value("description").toString());
            }

            if (typeObject.contains("privilege")) {
                scheduleType->setPrivilege(static_cast<Privilege>(typeObject.value("privilege").toInt()));
            }

            if (typeObject.contains("dtCreate")) {
                scheduleType->setDtCreate(QDateTime::fromString(typeObject.value("dtCreate").toString(), Qt::ISODate));
            }

            if (typeObject.contains("dtDelete")) {
                scheduleType->setDtDelete(QDateTime::fromString(typeObject.value("dtDelete").toString(), Qt::ISODate));
            }

            if (typeObject.contains("dtUpdate")) {
                scheduleType->setDtUpdate(QDateTime::fromString(typeObject.value("dtUpdate").toString(), Qt::ISODate));
            }

            if (typeObject.contains("showState")) {
                scheduleType->setShowState(static_cast<ShowState>(typeObject.value("showState").toInt()));
            }

            if (typeObject.contains("isDeleted")) {
                scheduleType->setDeleted(typeObject.value("isDeleted").toInt());
            }
            stList.append(scheduleType);
        }
    }
    // qCDebug(CommonLogger) << "Successfully parsed" << stList.count() << "schedule types from JSON list.";
    return true;
}

bool DScheduleType::toJsonListString(const DScheduleType::List &stList, QString &jsonStr)
{
    qCDebug(CommonLogger) << "Converting schedule type list to JSON string," << stList.count() << "items.";
    //序列化
    QJsonObject rootObject;
    QJsonArray jsonArray;

    for (auto &scheduleType : stList) {
        QJsonObject typeObject;
        typeObject.insert("accountID", scheduleType->accountID());
        typeObject.insert("typeID", scheduleType->typeID());
        typeObject.insert("typeName", scheduleType->typeName());
        typeObject.insert("displayName", scheduleType->displayName());
        typeObject.insert("typePath", scheduleType->typePath());
        //类型颜色信息
        QJsonObject colorObject;
        colorObject.insert("colorID", scheduleType->typeColor().colorID());
        colorObject.insert("colorCode", scheduleType->typeColor().colorCode());
        colorObject.insert("privilege", scheduleType->typeColor().privilege());
        typeObject.insert("TypeColor", colorObject);

        typeObject.insert("description", scheduleType->description());
        typeObject.insert("privilege", int(scheduleType->privilege()));
        typeObject.insert("dtCreate", dtToString(scheduleType->dtCreate()));
        typeObject.insert("dtDelete", dtToString(scheduleType->dtDelete()));
        typeObject.insert("dtUpdate", dtToString(scheduleType->dtUpdate()));
        typeObject.insert("showState", scheduleType->showState());
        typeObject.insert("isDeleted", scheduleType->deleted());
        jsonArray.append(typeObject);
    }
    rootObject.insert("scheduleType", jsonArray);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    qCDebug(CommonLogger) << "Successfully converted schedule type list to JSON.";
    return true;
}

int DScheduleType::syncTag() const
{
    // qCDebug(CommonLogger) << "Getting sync tag.";
    return m_syncTag;
}

void DScheduleType::setSyncTag(int syncTag)
{
    // qCDebug(CommonLogger) << "Setting sync tag to:" << syncTag;
    m_syncTag = syncTag;
}

bool operator<(const DScheduleType::Ptr &st1, const DScheduleType::Ptr &st2)
{
    // qCDebug(CommonLogger) << "Comparing schedule types.";
    //权限不一致权限小的排在前面
    if (st1->privilege() != st2->privilege()) {
        qCDebug(CommonLogger) << "Comparing privileges - st1:" << static_cast<int>(st1->privilege()) 
                              << "st2:" << static_cast<int>(st2->privilege());
        return st1->privilege() < st2->privilege();
    }
    //权限一一致的创建时间早的排在前面
    if (st1->dtCreate() != st2->dtCreate()) {
        qCDebug(CommonLogger) << "Comparing creation dates - st1:" << st1->dtCreate().toString() 
                              << "st2:" << st2->dtCreate().toString();
        return st1->dtCreate() < st2->dtCreate();
    }
    // qCDebug(CommonLogger) << "Comparing schedule types - st1:" << st1->typeID() << "st2:" << st2->typeID();
    return true;
}
