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

#include <QtDBus/QtDBus>

DSchedule::DSchedule()
    : KCalendarCore::Event()
{
}

QString DSchedule::scheduleType() const
{
    return m_scheduleType;
}

void DSchedule::setScheduleType(const QString &scheduleType)
{
    m_scheduleType = scheduleType;
}

bool DSchedule::isLunar() const
{
    return m_isLunar;
}

void DSchedule::setIsLunar(bool isLunar)
{
    if (m_isLunar == isLunar)
        return;
    m_isLunar = isLunar;
}

bool DSchedule::isValid() const
{
}

bool DSchedule::operator=(const DSchedule &schedule) const
{
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

void DSchedule::registerMetaType()
{
    //TODO:
    //    qRegisterMetaType<DSchedule>();
    //    qDBusRegisterMetaType<DSchedule>();
}

QDebug operator<<(QDebug debug, const DSchedule &scheduleJsonData)
{
    return debug;
}
