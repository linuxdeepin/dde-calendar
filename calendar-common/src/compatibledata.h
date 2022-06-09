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
#ifndef COMPATIBLEDATA_H
#define COMPATIBLEDATA_H

#include "dschedule.h"

#include <QString>

namespace DDE_Calendar {
//通过旧的日程id获取新的日程id
QString getNewTypeIDByOldTypeID(int oldTypeID);
int getOldTypeIDByNewTypeID(const QString &newTypeID);

DSchedule::Ptr getScheduleByExported(const QString &scheduleStr);

//
void setAlarmByOldRemind(const DSchedule::Ptr &schedule, const QString &remind);
void setRRuleByOldRRule(const DSchedule::Ptr &schedule, const QString &rrule);
void setExDate(const DSchedule::Ptr &schedule, const QJsonArray &ignore);
QString getOldRemindByAlarm(const DSchedule::AlarmType &alarmType);

//将新的日程数据转换为旧的查询数据({"Date":"",Jobs:["",""]})
QString getExternalSchedule(const DSchedule::Map &scheduleMap);
} // namespace DDE_Calendar

#endif // COMPATIBLEDATA_H
