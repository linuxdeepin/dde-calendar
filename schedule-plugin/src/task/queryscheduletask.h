/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#ifndef QUERYSCHEDULETASK_H
#define QUERYSCHEDULETASK_H

#include "schedulebasetask.h"
#include "../data/queryjsondata.h"
#include "../widget/viewschedulewidget.h"

class queryScheduleTask : public scheduleBaseTask
{
    Q_OBJECT
public:
    queryScheduleTask();
    Reply SchedulePress(semanticAnalysisTask &semanticTask) override;

private:
    /**
     * @brief setDateTime 设置查询的开始结束时间
     * @param queryJsonData jsondata句柄
     */
    void setDateTime(QueryJsonData *queryJsonData);

    /**
     * @brief queryOverDueDate 是否查询的是过去的日程
     * @param queryJsonData jsondata句柄
     * @return bool
     */
    bool queryOverDueDate(QueryJsonData *queryJsonData);

private:
    QDateTime       m_BeginDateTime;
    QDateTime       m_EndDateTime;
    viewschedulewidget      *viewWidget = nullptr;

protected:
};

#endif // QUERYSCHEDULETASK_H
