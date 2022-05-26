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
#include "dcalendargeneralsettings.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

DCalendarGeneralSettings::DCalendarGeneralSettings()
    : m_firstDayOfWeek(Qt::Sunday)
    , m_timeShowType(TwentyFour)
{
}

DCalendarGeneralSettings::DCalendarGeneralSettings(const DCalendarGeneralSettings &setting)
    : m_firstDayOfWeek(setting.firstDayOfWeek())
    , m_timeShowType(setting.timeShowType())
{
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
    m_firstDayOfWeek = firstDayOfWeek;
}

DCalendarGeneralSettings::TimeShowType DCalendarGeneralSettings::timeShowType() const
{
    return m_timeShowType;
}

void DCalendarGeneralSettings::setTimeShowType(const TimeShowType &timeShowType)
{
    m_timeShowType = timeShowType;
}

void DCalendarGeneralSettings::toJsonString(const Ptr &cgSet, QString &jsonStr)
{
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
        qWarning() << "error:" << jsonError.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("firstDayOfWeek")) {
        cgSet->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(rootObj.value("firstDayOfWeek").toInt()));
    }

    if (rootObj.contains("TimeShowType")) {
        cgSet->setTimeShowType(static_cast<TimeShowType>(rootObj.value("TimeShowType").toInt()));
    }
    return true;
}
