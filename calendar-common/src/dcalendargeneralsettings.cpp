// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcalendargeneralsettings.h"
#include "commondef.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

DCalendarGeneralSettings::DCalendarGeneralSettings()
    : m_firstDayOfWeek(Qt::Sunday)
    , m_timeShowType(TwentyFour)
{
    qCDebug(CommonLogger) << "DCalendarGeneralSettings::DCalendarGeneralSettings (default)";
}

DCalendarGeneralSettings::DCalendarGeneralSettings(const DCalendarGeneralSettings &setting)
    : m_firstDayOfWeek(setting.firstDayOfWeek())
    , m_timeShowType(setting.timeShowType())
{
    qCDebug(CommonLogger) << "DCalendarGeneralSettings::DCalendarGeneralSettings (copy)";
}

DCalendarGeneralSettings *DCalendarGeneralSettings::clone() const
{
    // qCDebug(CommonLogger) << "DCalendarGeneralSettings::clone";
    return new DCalendarGeneralSettings(*this);
}

Qt::DayOfWeek DCalendarGeneralSettings::firstDayOfWeek() const
{
    // qCDebug(CommonLogger) << "DCalendarGeneralSettings::firstDayOfWeek";
    return m_firstDayOfWeek;
}

void DCalendarGeneralSettings::setFirstDayOfWeek(const Qt::DayOfWeek &firstDayOfWeek)
{
    // qCDebug(CommonLogger) << "DCalendarGeneralSettings::setFirstDayOfWeek, day:" << firstDayOfWeek;
    m_firstDayOfWeek = firstDayOfWeek;
}

DCalendarGeneralSettings::TimeShowType DCalendarGeneralSettings::timeShowType() const
{
    // qCDebug(CommonLogger) << "DCalendarGeneralSettings::timeShowType";
    return m_timeShowType;
}

void DCalendarGeneralSettings::setTimeShowType(const TimeShowType &timeShowType)
{
    // qCDebug(CommonLogger) << "DCalendarGeneralSettings::setTimeShowType, type:" << timeShowType;
    m_timeShowType = timeShowType;
}

void DCalendarGeneralSettings::toJsonString(const Ptr &cgSet, QString &jsonStr)
{
    qCDebug(CommonLogger) << "DCalendarGeneralSettings::toJsonString";
    QJsonObject rootObject;
    rootObject.insert("firstDayOfWeek", cgSet->firstDayOfWeek());
    rootObject.insert("TimeShowType", cgSet->timeShowType());
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

bool DCalendarGeneralSettings::fromJsonString(Ptr &cgSet, const QString &jsonStr)
{
    qCDebug(CommonLogger) << "DCalendarGeneralSettings::fromJsonString";
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("firstDayOfWeek")) {
        qCDebug(CommonLogger) << "Setting first day of week to" << rootObj.value("firstDayOfWeek").toInt();
        cgSet->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(rootObj.value("firstDayOfWeek").toInt()));
    }

    if (rootObj.contains("TimeShowType")) {
        qCDebug(CommonLogger) << "Setting time show type to" << rootObj.value("TimeShowType").toInt();
        cgSet->setTimeShowType(static_cast<TimeShowType>(rootObj.value("TimeShowType").toInt()));
    }
    return true;
}
