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
#ifndef SCHEDULEBASETASK_H
#define SCHEDULEBASETASK_H

#include "semanticanalysistask.h"
#include "../interface/reply.h"
#include "dschedule.h"

#include <QObject>

class scheduleState;
class scheduleBaseTask : public QObject
{
    Q_OBJECT
public:
    scheduleBaseTask(scheduleState *state = nullptr);
    virtual ~scheduleBaseTask();
    virtual Reply SchedulePress(semanticAnalysisTask &semanticTask);
    virtual Reply getFeedbackByQuerySchedule(const DSchedule::List &info);
    virtual Reply getReplyBySelectSchedule(const DSchedule::Ptr &info);
    virtual Reply InitState(const JsonData *jsonData, bool isUpdateState = false);
    virtual Reply repeatScheduleHandle(const DSchedule::Ptr &info, bool isOnlyOne);
    virtual Reply confirwScheduleHandle(const DSchedule::Ptr &info);
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
    scheduleState *m_State {nullptr};
};

#endif // SCHEDULEBASETASK_H
