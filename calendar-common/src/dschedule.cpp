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
#include "dschedule.h"

#include "icalformat.h"
#include "memorycalendar.h"

#include <QtDBus/QtDBus>

DSchedule::DSchedule()
    : KCalendarCore::Event()
{
}

QString DSchedule::scheduleTypeID() const
{
    return m_scheduleTypeID;
}

void DSchedule::setScheduleTypeID(const QString &typeID)
{
    m_scheduleTypeID = typeID;
}

bool DSchedule::isValid() const
{
}

DSchedule &DSchedule::operator=(const DSchedule &schedule)
{
    //TODO:待修改
    return *this;
}

bool DSchedule::operator==(const DSchedule &schedule) const
{
}

bool DSchedule::operator<(const DSchedule &schedule) const
{
}

bool DSchedule::operator>(const DSchedule &schedule) const
{
}

bool DSchedule::fromJsonString(DSchedule::Ptr &schedule, const QString &json)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "error:" << jsonError.errorString();
        return false;
    }
    bool resBool = false;
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("schedule")) {
        QString str = rootObj.value("schedule").toString();
        if (fromIcsString(schedule, str) && rootObj.contains("type")) {
            schedule->setScheduleTypeID(rootObj.value("type").toString());
            resBool = true;
        }
    }
    return resBool;
}

bool DSchedule::toJsonString(const DSchedule::Ptr &schedule, QString &json)
{
    QJsonObject rootObject;
    rootObject.insert("type", schedule->scheduleTypeID());
    rootObject.insert("schedule", toIcsString(schedule));
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    json = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

bool DSchedule::fromIcsString(Ptr &schedule, const QString &string)
{
    bool resBool = false;
    KCalendarCore::ICalFormat icalformat;
    QTimeZone timezone = QDateTime::currentDateTime().timeZone();
    KCalendarCore::MemoryCalendar::Ptr _cal(new KCalendarCore::MemoryCalendar(timezone));
    if (icalformat.fromString(_cal, string)) {
        KCalendarCore::Event::List eventList = _cal->events();
        if (eventList.size() > 0) {
            schedule = eventList.at(0).staticCast<DSchedule>();
            resBool = true;
        }
    }
    return resBool;
}

QString DSchedule::toIcsString(const DSchedule::Ptr &schedule)
{
    KCalendarCore::ICalFormat icalformat;
    KCalendarCore::MemoryCalendar::Ptr _cal(new KCalendarCore::MemoryCalendar(nullptr));
    _cal->addEvent(schedule);
    return icalformat.toString(_cal.staticCast<KCalendarCore::Calendar>());
}

QString DSchedule::fileName() const
{
    return m_fileName;
}

void DSchedule::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

QDebug operator<<(QDebug debug, const DSchedule &scheduleJsonData)
{
    return debug;
}
