// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulebasetask.h"
#include "../state/schedulestate.h"
#include "../globaldef.h"

scheduleBaseTask::scheduleBaseTask(CSchedulesDBus *dbus, scheduleState *state)
    : QObject()
    , m_dbus(dbus)
    , m_State(state)
{
}

scheduleBaseTask::~scheduleBaseTask()
{
    if (m_State == nullptr) {
        delete m_State;
        m_State = nullptr;
    }
}

Reply scheduleBaseTask::SchedulePress(semanticAnalysisTask &semanticTask)
{
    Reply reply;
    if (m_State != nullptr) {
        if (semanticTask.Intent() == JSON_NO_INTENT) {
            reply = m_State->getReplyByIntent(false);
        } else if (semanticTask.Intent() == JSON_YES_INTENT) {
            reply = m_State->getReplyByIntent(true);
        } else {
            reply = m_State->process(semanticTask.getJsonData());
        }
        updateState();
    }
    return reply;
}

Reply scheduleBaseTask::getFeedbackByQuerySchedule(const QVector<ScheduleDtailInfo> &info)
{
    Q_UNUSED(info);
    return Reply();
}

Reply scheduleBaseTask::getReplyBySelectSchedule(const ScheduleDtailInfo &info)
{
    Q_UNUSED(info);
    return Reply();
}

Reply scheduleBaseTask::InitState(const JsonData *jsonData, bool isUpdateState)
{
    Q_UNUSED(jsonData);
    Q_UNUSED(isUpdateState);
    return Reply();
}

Reply scheduleBaseTask::repeatScheduleHandle(const ScheduleDtailInfo &info, bool isOnlyOne)
{
    Q_UNUSED(info);
    Q_UNUSED(isOnlyOne);
    return Reply();
}

Reply scheduleBaseTask::confirwScheduleHandle(const ScheduleDtailInfo &info)
{
    Q_UNUSED(info);
    return Reply();
}

Reply scheduleBaseTask::confirmInfo(bool isOK)
{
    Q_UNUSED(isOK);
    return Reply();
}

scheduleState *scheduleBaseTask::getState() const
{
    return m_State;
}

Reply scheduleBaseTask::overdueScheduleProcess()
{
    Reply reply;
    REPLY_ONLY_TTS(reply, OVERTIME_TTS, OVERTIME_TTS, true);
    return reply;
}

void scheduleBaseTask::updateState()
{
    do {
        scheduleState *nextState = m_State->getNextState();
        if (nextState != nullptr) {
            delete m_State;
            m_State = nextState;
        } else {
            break;
        }
    } while (1);
}
/**
 * @brief scheduleBaseTask::errorMessage   用户输入无法匹配返回错误消息提示
 * @return 关于无法处理该关键字的答复
 */
Reply scheduleBaseTask::errorMessage()
{
    //答复
    Reply reply;
    REPLY_ONLY_TTS(reply, G_ERR_TTS, G_ERR_TTS, true)
    return reply;
}
