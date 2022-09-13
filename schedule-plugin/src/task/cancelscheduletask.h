// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CANCELSCHEDULETASK_H
#define CANCELSCHEDULETASK_H

#include "schedulebasetask.h"

class scheduleState;

class cancelScheduleTask : public scheduleBaseTask
{
    Q_OBJECT
public:
    explicit cancelScheduleTask(CSchedulesDBus *dbus);
    Reply getFeedbackByQuerySchedule(const QVector<ScheduleDtailInfo> &infoVector) override;
    Reply getReplyBySelectSchedule(const ScheduleDtailInfo &info) override;
    Reply InitState(const JsonData *jsonData, bool isUpdateState = false) override;
    Reply repeatScheduleHandle(const ScheduleDtailInfo &info, bool isOnlyOne) override;
    Reply confirwScheduleHandle(const ScheduleDtailInfo &info) override;
    Reply confirmInfo(bool isOK) override;
public slots:
    void slotSelectScheduleIndex(int index);
    void slotButtonCheckNum(int index, const QString &text, const int buttonCount);

private:
    scheduleState *getCurrentState();
    QWidget *createRepeatWidget(const ScheduleDtailInfo &info);
    QWidget *createConfirmWidget(const ScheduleDtailInfo &info);

    Reply getListScheduleReply(const QVector<ScheduleDtailInfo> &infoVector);

    Reply getConfirwScheduleReply(const ScheduleDtailInfo &info);
    Reply getRepeatReply(const ScheduleDtailInfo &info);

    void deleteRepeatSchedule(const ScheduleDtailInfo &info, bool isOnlyOne = true);
    void deleteOrdinarySchedule(const ScheduleDtailInfo &info);

private:
};

#endif // CANCELSCHEDULETASK_H
