// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "schedulestate.h"
#include "globaldef.h"
#include "createjsondata.h"

// ============================================================================
// Constructor
// ============================================================================

TEST(ScheduleBaseTaskTest, Constructor_WithState_StoresState)
{
    TestableScheduleState *state = nullptr;
    {
        MockScheduleBaseTask task;
        state = new TestableScheduleState(&task);
        task.m_State = state;
        EXPECT_EQ(state, task.getState());
    }
}

TEST(ScheduleBaseTaskTest, Constructor_WithNullState)
{
    MockScheduleBaseTask task(nullptr);
    EXPECT_EQ(nullptr, task.getState());
}

// ============================================================================
// Destructor — suspected defect: condition uses == instead of !=
// The destructor body is: if (m_State == nullptr) { delete m_State; m_State = nullptr; }
// This means m_State is only "deleted" when it's already null (a no-op).
// When m_State is non-null, it is NOT deleted (memory leak).
// We verify the destructor does not crash in either case.
// ============================================================================

TEST(ScheduleBaseTaskTest, Destructor_WithNullState_NoCrash)
{
    MockScheduleBaseTask *task = new MockScheduleBaseTask(nullptr);
    delete task;
    SUCCEED();
}

TEST(ScheduleBaseTaskTest, Destructor_WithNonNullState_NoCrash)
{
    // Due to the suspected defect (== instead of !=), the state will NOT be
    // deleted by the base destructor. We still verify no crash.
    MockScheduleBaseTask *task = new MockScheduleBaseTask();
    TestableScheduleState *state = new TestableScheduleState(task);
    task->m_State = state;
    delete task;
    // Clean up the leaked state to avoid memory leak in test
    delete state;
    SUCCEED();
}

// ============================================================================
// SchedulePress
// ============================================================================

TEST(ScheduleBaseTaskTest, SchedulePress_NullState_ReturnsDefaultReply)
{
    MockScheduleBaseTask task(nullptr);
    semanticAnalysisTask semantic;
    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(ERR_FAIL, reply.code());
}

TEST(ScheduleBaseTaskTest, SchedulePress_NoIntent_CallsGetReplyByIntentFalse)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    task.m_State = &state;
    state.m_intentReply = Reply(10, "no_intent_reply");

    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_NO_INTENT);

    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(10, reply.code());
}

TEST(ScheduleBaseTaskTest, SchedulePress_YesIntent_CallsGetReplyByIntentTrue)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    task.m_State = &state;
    state.m_intentReply = Reply(20, "yes_intent_reply");

    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_YES_INTENT);

    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(20, reply.code());
}

TEST(ScheduleBaseTaskTest, SchedulePress_OtherIntent_CallsProcess)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    task.m_State = &state;
    state.m_filterResult = scheduleState::Fileter_Normal;
    state.m_normalReply = Reply(30, "normal_reply");

    semanticAnalysisTask semantic;
    semantic.setIntent(JSON_CREATE);
    semantic.setJsonData(new CreateJsonData());

    Reply reply = task.SchedulePress(semantic);
    EXPECT_TRUE(state.m_normalCalled);
    EXPECT_EQ(30, reply.code());
}

// ============================================================================
// updateState
// ============================================================================

TEST(ScheduleBaseTaskTest, UpdateState_NoNextState_KeepsCurrentState)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    task.m_State = &state;
    // No next state set — should break immediately
    task.updateState();  // protected, accessible via -fno-access-control
    EXPECT_EQ(&state, task.getState());
}

TEST(ScheduleBaseTaskTest, UpdateState_WithNextStateChain_DeletesOldStates)
{
    MockScheduleBaseTask task;
    TestableScheduleState *state1 = new TestableScheduleState(&task);
    TestableScheduleState *state2 = new TestableScheduleState(&task);
    state1->setNextState(state2);
    task.m_State = state1;

    task.updateState();
    // After updateState, m_State should be state2 (the last in chain with no next)
    EXPECT_EQ(state2, task.getState());
}

// ============================================================================
// errorMessage
// ============================================================================

TEST(ScheduleBaseTaskTest, ErrorMessage_ReturnsCorrectTTS)
{
    MockScheduleBaseTask task;
    Reply reply = task.errorMessage();  // protected, accessible via -fno-access-control
    EXPECT_EQ(QString(G_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(G_ERR_TTS), reply.displayMessage());
    EXPECT_EQ(0, reply.code());
    EXPECT_TRUE(reply.getShouldEndSession());
}

// ============================================================================
// overdueScheduleProcess
// ============================================================================

TEST(ScheduleBaseTaskTest, OverdueScheduleProcess_ReturnsCorrectTTS)
{
    MockScheduleBaseTask task;
    Reply reply = task.overdueScheduleProcess();
    EXPECT_EQ(QString(OVERTIME_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(OVERTIME_TTS), reply.displayMessage());
    EXPECT_EQ(0, reply.code());
    EXPECT_TRUE(reply.getShouldEndSession());
}

// ============================================================================
// getState
// ============================================================================

TEST(ScheduleBaseTaskTest, GetState_ReturnsState)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    task.m_State = &state;
    EXPECT_EQ(&state, task.getState());
}

TEST(ScheduleBaseTaskTest, GetState_DefaultNull)
{
    MockScheduleBaseTask task(nullptr);
    EXPECT_EQ(nullptr, task.getState());
}

// ============================================================================
// Virtual default implementations
// ============================================================================

TEST(ScheduleBaseTaskTest, GetFeedbackByQuerySchedule_ReturnsDefaultReply)
{
    MockScheduleBaseTask task;
    DSchedule::List list = makeScheduleList(2);
    Reply reply = task.getFeedbackByQuerySchedule(list);
    EXPECT_EQ(ERR_FAIL, reply.code());
}

TEST(ScheduleBaseTaskTest, GetReplyBySelectSchedule_ReturnsDefaultReply)
{
    MockScheduleBaseTask task;
    DSchedule::Ptr info(new DSchedule());
    Reply reply = task.getReplyBySelectSchedule(info);
    EXPECT_EQ(ERR_FAIL, reply.code());
}

TEST(ScheduleBaseTaskTest, InitState_ReturnsDefaultReply)
{
    MockScheduleBaseTask task;
    JsonData jsonData;
    Reply reply = task.InitState(&jsonData, false);
    EXPECT_EQ(ERR_FAIL, reply.code());
}

TEST(ScheduleBaseTaskTest, RepeatScheduleHandle_ReturnsDefaultReply)
{
    MockScheduleBaseTask task;
    DSchedule::Ptr info(new DSchedule());
    Reply reply = task.repeatScheduleHandle(info, true);
    EXPECT_EQ(ERR_FAIL, reply.code());
}

TEST(ScheduleBaseTaskTest, ConfirwScheduleHandle_ReturnsDefaultReply)
{
    MockScheduleBaseTask task;
    DSchedule::Ptr info(new DSchedule());
    Reply reply = task.confirwScheduleHandle(info);
    EXPECT_EQ(ERR_FAIL, reply.code());
}

TEST(ScheduleBaseTaskTest, ConfirmInfo_ReturnsDefaultReply)
{
    MockScheduleBaseTask task;
    Reply reply = task.confirmInfo(true);
    EXPECT_EQ(ERR_FAIL, reply.code());
}
