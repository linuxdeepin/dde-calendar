// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEBASETASK_H
#define SCHEDULEBASETASK_H

#include "semanticanalysistask.h"
#include "../interface/reply.h"
#include "../dbus/schedulesdbus.h"
#include "../data/schedulestructs.h"
#include <QObject>

class scheduleState;
class scheduleBaseTask : public QObject
{
    Q_OBJECT
public:
    scheduleBaseTask(CSchedulesDBus *dbus, scheduleState *state = nullptr);
    virtual ~scheduleBaseTask();
    virtual Reply SchedulePress(semanticAnalysisTask &semanticTask);
    virtual Reply getFeedbackByQuerySchedule(const QVector<ScheduleDtailInfo> &info);
    virtual Reply getReplyBySelectSchedule(const ScheduleDtailInfo &info);
    virtual Reply InitState(const JsonData *jsonData, bool isUpdateState = false);
    virtual Reply repeatScheduleHandle(const ScheduleDtailInfo &info, bool isOnlyOne);
    virtual Reply confirwScheduleHandle(const ScheduleDtailInfo &info);
    virtual Reply confirmInfo(bool isOK);
    scheduleState *getState() const;
    Reply overdueScheduleProcess();
signals:
    void signaleSendMessage(Reply reply);

protected:
    void updateState();
    /**
     * @brief errorMessage   用户输入无法匹配返回错误消息提示
     * @return 关于无法处理该关键字的答复
     */
    Reply errorMessage();

protected:
    CSchedulesDBus *m_dbus {nullptr};
    scheduleState *m_State {nullptr};
};

#endif // SCHEDULEBASETASK_H
