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
#ifndef DCALENDARGENERALSETTINGS_H
#define DCALENDARGENERALSETTINGS_H

#include <Qt>
#include <QString>
#include <QSharedPointer>

/**
 * @brief The DCalendarGeneralSettings class
 * 日历通用设置
 */
class DCalendarGeneralSettings
{
public:
    enum TimeShowType {
        Twelve, //12
        TwentyFour //24
    };

    typedef QSharedPointer<DCalendarGeneralSettings> Ptr;

    DCalendarGeneralSettings();
    Qt::DayOfWeek firstDayOfWeek() const;
    void setFirstDayOfWeek(const Qt::DayOfWeek &firstDayOfWeek);

    TimeShowType timeShowType() const;
    void setTimeShowType(const TimeShowType &timeShowType);

    static void toJsonString(const DCalendarGeneralSettings &cgSet, QString &jsonStr);
    static bool fromJsonString(DCalendarGeneralSettings &cgSet, const QString &jsonStr);

private:
    Qt::DayOfWeek m_firstDayOfWeek; //一周首日
    TimeShowType m_timeShowType;
};

#endif // DCALENDARGENERALSETTINGS_H
