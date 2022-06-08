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
#ifndef CANCELSCHEDULETASK_H
#define CANCELSCHEDULETASK_H

#include "schedulebasetask.h"

class scheduleState;

class cancelScheduleTask : public scheduleBaseTask
{
    Q_OBJECT
public:
    cancelScheduleTask();
    Reply getFeedbackByQuerySchedule(const DSchedule::List &infoVector) override;
    Reply getReplyBySelectSchedule(const DSchedule::Ptr &info) override;
    Reply InitState(const JsonData *jsonData, bool isUpdateState = false) override;
    Reply repeatScheduleHandle(const DSchedule::Ptr &info, bool isOnlyOne) override;
    Reply confirwScheduleHandle(const DSchedule::Ptr &info) override;
    Reply confirmInfo(bool isOK) override;
public slots:
    void slotSelectScheduleIndex(int index);
    void slotButtonCheckNum(int index, const QString &text, const int buttonCount);

private:
    scheduleState *getCurrentState();
    QWidget *createRepeatWidget(const DSchedule::Ptr &info);
    QWidget *createConfirmWidget(const DSchedule::Ptr &info);

    Reply getListScheduleReply(const DSchedule::List &infoVector);

    Reply getConfirwScheduleReply(const DSchedule::Ptr &info);
    Reply getRepeatReply(const DSchedule::Ptr &info);

    void deleteRepeatSchedule(const DSchedule::Ptr &info, bool isOnlyOne = true);
    void deleteOrdinarySchedule(const DSchedule::Ptr &info);

private:
};

#endif // CANCELSCHEDULETASK_H
