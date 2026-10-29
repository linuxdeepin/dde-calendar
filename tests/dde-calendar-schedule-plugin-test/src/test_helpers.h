// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <gtest/gtest.h>
#include "schedulestate.h"
#include "selectandquerystate.h"
#include "selectinquirystate.h"
#include "schedulebasetask.h"
#include "jsondata.h"
#include "changejsondata.h"
#include "clocaldata.h"
#include "dschedule.h"
#include "reply.h"
#include "globaldef.h"

// Mock scheduleBaseTask that allows controlling virtual method returns
class MockScheduleBaseTask : public scheduleBaseTask
{
public:
    explicit MockScheduleBaseTask(scheduleState *state = nullptr)
        : scheduleBaseTask(state)
    {
    }

    Reply m_initReply;
    Reply m_selectReply;
    Reply m_feedbackReply;
    bool m_initCalled = false;
    bool m_selectCalled = false;
    bool m_feedbackCalled = false;
    DSchedule::Ptr m_lastSelectInfo;

    Reply InitState(const JsonData *jsonData, bool isUpdateState = false) override
    {
        Q_UNUSED(jsonData);
        Q_UNUSED(isUpdateState);
        m_initCalled = true;
        return m_initReply;
    }

    Reply getReplyBySelectSchedule(const DSchedule::Ptr &info) override
    {
        m_selectCalled = true;
        m_lastSelectInfo = info;
        return m_selectReply;
    }

    Reply getFeedbackByQuerySchedule(const DSchedule::List &info) override
    {
        Q_UNUSED(info);
        m_feedbackCalled = true;
        return m_feedbackReply;
    }
};

// Testable scheduleState that implements pure virtuals with configurable returns
class TestableScheduleState : public scheduleState
{
public:
    explicit TestableScheduleState(scheduleBaseTask *task)
        : scheduleState(task)
    {
    }

    Filter_Flag m_filterResult = Fileter_Normal;
    Reply m_errReply;
    Reply m_normalReply;
    Reply m_intentReply;
    bool m_errCalled = false;
    bool m_normalCalled = false;

    Reply getReplyByIntent(bool isOK) override
    {
        Q_UNUSED(isOK);
        return m_intentReply;
    }

    Filter_Flag eventFilter(const JsonData *jsonData) override
    {
        Q_UNUSED(jsonData);
        return m_filterResult;
    }

    Reply ErrEvent() override
    {
        m_errCalled = true;
        return m_errReply;
    }

    Reply normalEvent(const JsonData *jsonData) override
    {
        Q_UNUSED(jsonData);
        m_normalCalled = true;
        return m_normalReply;
    }
};

// Helper to create a DSchedule::List with n items
inline DSchedule::List makeScheduleList(int count)
{
    DSchedule::List list;
    for (int i = 0; i < count; ++i) {
        list.append(DSchedule::Ptr(new DSchedule()));
    }
    return list;
}

#endif // TEST_HELPERS_H

// Minimal derived class with no additional members — to trigger base D0
class MinimalDerivedState : public scheduleState
{
public:
    explicit MinimalDerivedState(scheduleBaseTask *task) : scheduleState(task) {}
    Reply getReplyByIntent(bool) override { return Reply(); }
    Filter_Flag eventFilter(const JsonData *) override { return Fileter_Normal; }
    Reply ErrEvent() override { return Reply(); }
    Reply normalEvent(const JsonData *) override { return Reply(); }
};
