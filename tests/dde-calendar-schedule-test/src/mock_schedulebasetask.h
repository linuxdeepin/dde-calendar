// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MOCK_SCHEDULEBASETASK_H
#define MOCK_SCHEDULEBASETASK_H

#include "schedulebasetask.h"
#include "schedulestate.h"
#include "reply.h"

class MockScheduleBaseTask : public scheduleBaseTask
{
public:
    explicit MockScheduleBaseTask(scheduleState *state = nullptr)
        : scheduleBaseTask(state)
    {
    }
    ~MockScheduleBaseTask() override
    {
        // Base class destructor has a bug (deletes only when null).
        // We manage m_State ourselves to avoid leaks.
        if (m_State != nullptr) {
            delete m_State;
            m_State = nullptr;
        }
    }

    Reply confirmInfo(bool isOK) override
    {
        m_confirmInfoCalled = true;
        m_confirmInfoArg = isOK;
        return m_confirmInfoReply;
    }

    Reply confirwScheduleHandle(const DSchedule::Ptr &info) override
    {
        m_confirwScheduleHandleCalled = true;
        m_confirwScheduleHandleArg = info;
        return m_confirwScheduleHandleReply;
    }

    Reply getReplyBySelectSchedule(const DSchedule::Ptr &info) override
    {
        m_getReplyBySelectScheduleCalled = true;
        m_getReplyBySelectScheduleArg = info;
        return m_getReplyBySelectScheduleReply;
    }

    Reply InitState(const JsonData *jsonData, bool isUpdateState) override
    {
        m_initStateCalled = true;
        m_initStateArg = jsonData;
        m_initStateIsUpdate = isUpdateState;
        return m_initStateReply;
    }

    Reply repeatScheduleHandle(const DSchedule::Ptr &info, bool isOnlyOne) override
    {
        m_repeatScheduleHandleCalled = true;
        m_repeatScheduleHandleArg = info;
        m_repeatScheduleHandleIsOnlyOne = isOnlyOne;
        return m_repeatScheduleHandleReply;
    }

    Reply getFeedbackByQuerySchedule(const DSchedule::List &info) override
    {
        m_getFeedbackByQueryScheduleCalled = true;
        m_getFeedbackByQueryScheduleArg = info;
        return m_getFeedbackByQueryScheduleReply;
    }

    // --- configurable return values ---
    Reply m_confirmInfoReply;
    Reply m_confirwScheduleHandleReply;
    Reply m_getReplyBySelectScheduleReply;
    Reply m_initStateReply;
    Reply m_repeatScheduleHandleReply;
    Reply m_getFeedbackByQueryScheduleReply;

    // --- call tracking ---
    bool m_confirmInfoCalled {false};
    bool m_confirmInfoArg {false};

    bool m_confirwScheduleHandleCalled {false};
    DSchedule::Ptr m_confirwScheduleHandleArg;

    bool m_getReplyBySelectScheduleCalled {false};
    DSchedule::Ptr m_getReplyBySelectScheduleArg;

    bool m_initStateCalled {false};
    const JsonData *m_initStateArg {nullptr};
    bool m_initStateIsUpdate {false};

    bool m_repeatScheduleHandleCalled {false};
    DSchedule::Ptr m_repeatScheduleHandleArg;
    bool m_repeatScheduleHandleIsOnlyOne {true};

    bool m_getFeedbackByQueryScheduleCalled {false};
    DSchedule::List m_getFeedbackByQueryScheduleArg;

    void resetTracking()
    {
        m_confirmInfoCalled = false;
        m_confirmInfoArg = false;
        m_confirwScheduleHandleCalled = false;
        m_confirwScheduleHandleArg.clear();
        m_getReplyBySelectScheduleCalled = false;
        m_getReplyBySelectScheduleArg.clear();
        m_initStateCalled = false;
        m_initStateArg = nullptr;
        m_initStateIsUpdate = false;
        m_repeatScheduleHandleCalled = false;
        m_repeatScheduleHandleArg.clear();
        m_repeatScheduleHandleIsOnlyOne = true;
        m_getFeedbackByQueryScheduleCalled = false;
        m_getFeedbackByQueryScheduleArg.clear();
    }
};

#endif // MOCK_SCHEDULEBASETASK_H
