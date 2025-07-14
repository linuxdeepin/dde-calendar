// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtypecolor.h"

#include "units.h"
#include "commondef.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DTypeColor::DTypeColor()
    : m_colorID("")
    , m_colorCode("")
    , m_privilege(PriUser)
{
    // qCDebug(CommonLogger) << "DTypeColor constructor called.";
}

DTypeColor::DTypeColor(const DTypeColor &typeColor)
    : m_colorID(typeColor.colorID())
    , m_colorCode(typeColor.colorCode())
    , m_privilege(typeColor.privilege())
    , m_dtCreate(typeColor.dtCreate())
{
    // qCDebug(CommonLogger) << "DTypeColor copy constructor called.";
}

QString DTypeColor::colorCode() const
{
    // qCDebug(CommonLogger) << "Getting color code:" << m_colorCode;
    return m_colorCode;
}

void DTypeColor::setColorCode(const QString &colorCode)
{
    // qCDebug(CommonLogger) << "Setting color code to:" << colorCode;
    m_colorCode = colorCode;
}

QString DTypeColor::colorID() const
{
    // qCDebug(CommonLogger) << "Getting color ID:" << m_colorID;
    return m_colorID;
}

void DTypeColor::setColorID(const QString &colorID)
{
    // qCDebug(CommonLogger) << "Setting color ID to:" << colorID;
    m_colorID = colorID;
}

DTypeColor::Privilege DTypeColor::privilege() const
{
    // qCDebug(CommonLogger) << "Getting privilege:" << m_privilege;
    return m_privilege;
}

void DTypeColor::setPrivilege(const Privilege &privilege)
{
    // qCDebug(CommonLogger) << "Setting privilege to:" << privilege;
    m_privilege = privilege;
}

bool DTypeColor::isSysColorInfo()
{
    // qCDebug(CommonLogger) << "Checking if color is system color info.";
    return this->privilege() == PriSystem;
}

bool DTypeColor::operator!=(const DTypeColor &color) const
{
    // qCDebug(CommonLogger) << "Executing inequality operator.";
    return this->colorID() != color.colorID() || this->colorCode() != this->colorCode() || this->privilege() != this->privilege();
}

DTypeColor::List DTypeColor::fromJsonString(const QString &colorJson)
{
    // qCDebug(CommonLogger) << "Parsing color list from JSON string.";
    DTypeColor::List colorList;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(colorJson.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return colorList;
    }
    QJsonArray rootArr = jsonDoc.array();
    foreach (auto json, rootArr) {
        // qCDebug(CommonLogger) << "Processing a JSON object from the array.";
        QJsonObject colorObj = json.toObject();
        DTypeColor::Ptr typeColor = DTypeColor::Ptr(new DTypeColor);
        if (colorObj.contains("colorID")) {
            // qCDebug(CommonLogger) << "Found and setting colorID.";
            typeColor->setColorID(colorObj.value("colorID").toString());
        }
        if (colorObj.contains("colorCode")) {
            // qCDebug(CommonLogger) << "Found and setting colorCode.";
            typeColor->setColorCode(colorObj.value("colorCode").toString());
        }
        if (colorObj.contains("privilege")) {
            // qCDebug(CommonLogger) << "Found and setting privilege.";
            typeColor->setPrivilege(static_cast<Privilege>(colorObj.value("privilege").toInt()));
        }
        if (colorObj.contains("dtCreate")) {
            // qCDebug(CommonLogger) << "Found and setting dtCreate.";
            typeColor->setDtCreate(dtFromString(colorObj.value("dtCreate").toString()));
        }
        colorList.append(typeColor);
    }
    return colorList;
}

QString DTypeColor::toJsonString(const DTypeColor::List &colorList)
{
    // qCDebug(CommonLogger) << "Converting color list to JSON string.";
    QJsonArray rootArr;
    foreach (auto color, colorList) {
        // qCDebug(CommonLogger) << "Processing a color object for JSON conversion.";
        QJsonObject colorObj;
        colorObj.insert("colorID", color->colorID());
        colorObj.insert("colorCode", color->colorCode());
        colorObj.insert("privilege", color->privilege());
        colorObj.insert("dtCreate", dtToString(color->dtCreate()));
        rootArr.append(colorObj);
    }
    QJsonDocument jsonDoc;
    jsonDoc.setArray(rootArr);
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

QDateTime DTypeColor::dtCreate() const
{
    // qCDebug(CommonLogger) << "Getting dtCreate:" << m_dtCreate;
    return m_dtCreate;
}

void DTypeColor::setDtCreate(const QDateTime &dtCreate)
{
    // qCDebug(CommonLogger) << "Setting dtCreate to:" << dtCreate;
    m_dtCreate = dtCreate;
}

bool operator<(const DTypeColor::Ptr &tc1, const DTypeColor::Ptr &tc2)
{
    // qCDebug(CommonLogger) << "Executing less-than operator.";
    if (tc1->privilege() != tc2->privilege()) {
        // qCDebug(CommonLogger) << "Comparing by privilege.";
        return tc1->privilege() < tc2->privilege();
    }

    if (tc1->dtCreate() != tc2->dtCreate()) {
        // qCDebug(CommonLogger) << "Comparing by dtCreate.";
        return tc1->dtCreate() < tc2->dtCreate();
    }
    // qCDebug(CommonLogger) << "Defaulting to true in less-than comparison.";
    return true;
}
