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
#ifndef DSCHEDULEDATAMANAGER_H
#define DSCHEDULEDATAMANAGER_H

#include "dschedule.h"
#include "dschedulequerypar.h"

#include <QMap>

//日程数据管理模块
class DScheduleDataManager
{
public:
    static DScheduleDataManager *getInstance();

    QString createSchedule(const DSchedule::Ptr &schedule);

    DSchedule::Ptr queryScheduleByScheduleID(const QString &scheduleID);

    bool deleteScheduleByScheduleID(const QString &scheduleID);

    bool updateSchedule(const DSchedule::Ptr &schedule);

    //根据类型ID判断是否为节假日日程
    bool isFestivalSchedule(const QString &scheduleTypeID);

    DSchedule::Map querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params);
    DSchedule::Map queryScheduleByRRule(const QDateTime &dtStart, const QDateTime &dtEnd, const DScheduleQueryPar::RRuleType &rrultTyep);
    DSchedule::Map queryScheduleByLimit(const QDateTime &dtStart, const QDateTime &dtEnd, int topNum);
    DSchedule::Map queryScheduleBySummary(const QDateTime &dtStart, const QDateTime &dtEnd, const QString &summary);

private:
    DScheduleDataManager();
};

#endif // DSCHEDULEDATAMANAGER_H
