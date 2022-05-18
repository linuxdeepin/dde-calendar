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
#ifndef DSCHEDULE_H
#define DSCHEDULE_H

#include "event.h"

#include <QString>
#include <QDebug>
#include <QSharedPointer>

//日程信息
class DSchedule : public KCalendarCore::Event
{
public:
    typedef QSharedPointer<DSchedule> Ptr;

    DSchedule();
    QString scheduleType() const;
    void setScheduleType(const QString &scheduleType);

    bool isLunar() const;
    void setIsLunar(bool isLunar);

    bool isValid() const;

    bool operator=(const DSchedule &schedule) const;

    bool operator==(const DSchedule &schedule) const;

    bool operator<(const DSchedule &schedule) const;

    bool operator>(const DSchedule &schedule) const;
    //TODO:需要做json序列化和反序列化，ics的数据的转换
    static bool fromJsonString(const DSchedule::Ptr &schedule, const QString &string);
    static bool toJsonString(const DSchedule::Ptr &schedule, const QString &string);
    static bool fromIcsString(const DSchedule::Ptr &schedule, const QString &string);
    static bool toIcsString(const DSchedule::Ptr &schedule, const QString &string);

    static void registerMetaType();

public:
    //日程信息调试打印
    friend QDebug operator<<(QDebug debug, const DSchedule &scheduleJsonData);

private:
    bool m_isLunar;
    //日程类型
    QString m_scheduleType;
};

#endif // DSCHEDULE_H
