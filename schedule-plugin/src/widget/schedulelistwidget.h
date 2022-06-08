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
#ifndef SCHEDULELISTWIDGET_H
#define SCHEDULELISTWIDGET_H

#include <DFrame>

#include "dschedule.h"
#include "icondframe.h"

DWIDGET_USE_NAMESPACE
class scheduleListWidget : public IconDFrame
{
    Q_OBJECT
public:
    explicit scheduleListWidget(QWidget *parent = nullptr);

    void setScheduleInfoVector(const DSchedule::List &ScheduleInfoVector);

    void updateUI();
signals:
    void signalSelectScheduleIndex(int index);

private:
    DSchedule::List m_ScheduleInfoVector;
};

#endif // SCHEDULELISTWIDGET_H
