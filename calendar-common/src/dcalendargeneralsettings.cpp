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
    qCDebug(CommonLogger) << "Created new DCalendarGeneralSettings with default values";
}

DCalendarGeneralSettings::DCalendarGeneralSettings(const DCalendarGeneralSettings &setting)
    : m_firstDayOfWeek(setting.firstDayOfWeek())
    , m_timeShowType(setting.timeShowType())
{
    qCDebug(CommonLogger) << "Copied DCalendarGeneralSettings with firstDayOfWeek:" << m_firstDayOfWeek 
                         << "timeShowType:" << m_timeShowType;
}

DCalendarGeneralSettings *DCalendarGeneralSettings::clone() const
{
    return new DCalendarGeneralSettings(*this);
}

Qt::DayOfWeek DCalendarGeneralSettings::firstDayOfWeek() const
{
    return m_firstDayOfWeek;
}

void DCalendarGeneralSettings::setFirstDayOfWeek(const Qt::DayOfWeek &firstDayOfWeek)
{
    qCDebug(CommonLogger) << "Setting first day of week to:" << firstDayOfWeek;
    m_firstDayOfWeek = firstDayOfWeek;
}

DCalendarGeneralSettings::TimeShowType DCalendarGeneralSettings::timeShowType() const
{
    return m_timeShowType;
}

void DCalendarGeneralSettings::setTimeShowType(const TimeShowType &timeShowType)
{
    qCDebug(CommonLogger) << "Setting time show type to:" << timeShowType;
    m_timeShowType = timeShowType;
}

void DCalendarGeneralSettings::toJsonString(const Ptr &cgSet, QString &jsonStr)
{
    qCDebug(CommonLogger) << "Converting DCalendarGeneralSettings to JSON string";
    QJsonObject rootObject;
    rootObject.insert("firstDayOfWeek", cgSet->firstDayOfWeek());
    rootObject.insert("TimeShowType", cgSet->timeShowType());
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
}

bool DCalendarGeneralSettings::fromJsonString(Ptr &cgSet, const QString &jsonStr)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse JSON string:" << jsonError.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("firstDayOfWeek")) {
        cgSet->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(rootObj.value("firstDayOfWeek").toInt()));
    }

    if (rootObj.contains("TimeShowType")) {
        cgSet->setTimeShowType(static_cast<TimeShowType>(rootObj.value("TimeShowType").toInt()));
    }
    qCDebug(CommonLogger) << "Successfully parsed DCalendarGeneralSettings from JSON string";
    return true;
}
