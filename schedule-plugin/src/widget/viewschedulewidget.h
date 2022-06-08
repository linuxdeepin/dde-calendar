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
#ifndef VIEWSCHEDULEWIDGET_H
#define VIEWSCHEDULEWIDGET_H

#include "icondframe.h"
#include "dschedule.h"
#include "scheduleitemwidget.h"

#include <QWidget>

class viewschedulewidget : public IconDFrame
{
    Q_OBJECT
public:
    explicit viewschedulewidget(QWidget *parent = nullptr);
    ~viewschedulewidget();
    void setScheduleDateRangeInfo(const DSchedule::Map &scheduleDateInfo);
    void viewScheduleInfoShow(const DSchedule::Map &showSchedule);
    int getScheduleNum(DSchedule::Map scheduleList);
    void setQueryBeginDateTime(QDateTime begindatetime);
    void setQueryEndDateTime(QDateTime enddatetime);
    /**
     * @brief getNextScheduleInfo 获取下一个日程
     * @return 下一个日程信息
     */
    DSchedule::Map getNextScheduleInfo();
public slots:
    void slotItemPress(const DSchedule::Ptr &info);

private:
    DSchedule::List m_scheduleInfo;
    DSchedule::Map m_scheduleDateInfo;
    DSchedule::Map m_showdate;
    QDateTime m_beginDateTime;
    QDateTime m_endDateTime;
};

#endif // QUERYSCHEDULEWIDGET_H
