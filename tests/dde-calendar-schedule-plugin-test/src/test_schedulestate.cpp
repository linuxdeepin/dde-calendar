// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "queryschedulestate.h"
#include <QJsonObject>
#include <QJsonArray>

// ============================================================================
// Constructor / Destructor
// ============================================================================

TEST(ScheduleStateTest, Constructor_StoresTaskPointer)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    EXPECT_EQ(&task, state.m_Task);
}

TEST(ScheduleStateTest, Destructor_NoCrash)
{
    MockScheduleBaseTask task;
    {
        TestableScheduleState state(&task);
    }
    SUCCEED();
}

TEST(ScheduleStateTest, Constructor_WithNullTask)
{
    TestableScheduleState state(nullptr);
    EXPECT_EQ(nullptr, state.m_Task);
}

// ============================================================================
// setNextState / getNextState
// ============================================================================

TEST(ScheduleStateTest, SetNextState_StoresAndReturns)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    TestableScheduleState next(&task);
    state.setNextState(&next);
    EXPECT_EQ(&next, state.getNextState());
}

TEST(ScheduleStateTest, GetNextState_DefaultNull)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    EXPECT_EQ(nullptr, state.getNextState());
}

// ============================================================================
// setLocalData / getLocalData
// ============================================================================

TEST(ScheduleStateTest, SetLocalData_StoresAndReturns)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);
    EXPECT_EQ(localData, state.getLocalData());
}

TEST(ScheduleStateTest, GetLocalData_DefaultNull)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);
    EXPECT_EQ(nullptr, state.getLocalData());
}

// ============================================================================
// process()
// ============================================================================

TEST(ScheduleStateTest, Process_DateTimeInvalid_ReturnsDateTimeErr)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    // getDateTimeInvalid() returns false by default; we cannot set it directly
    // because the setter is not public. But we can test the non-invalid path.
    // This test verifies the default (non-invalid) path does not take the early return.
    state.m_filterResult = scheduleState::Fileter_Normal;
    Reply reply = state.process(&jsonData);
    EXPECT_TRUE(state.m_normalCalled);
}

TEST(ScheduleStateTest, Process_FilterErr_CallsErrEvent)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    state.m_filterResult = scheduleState::Fileter_Err;
    state.m_errReply = Reply(42, "err");
    Reply reply = state.process(&jsonData);
    EXPECT_TRUE(state.m_errCalled);
    EXPECT_FALSE(state.m_normalCalled);
    EXPECT_EQ(42, reply.code());
}

TEST(ScheduleStateTest, Process_FilterNormal_CallsNormalEvent)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    state.m_filterResult = scheduleState::Fileter_Normal;
    state.m_normalReply = Reply(99, "normal");
    Reply reply = state.process(&jsonData);
    EXPECT_TRUE(state.m_normalCalled);
    EXPECT_FALSE(state.m_errCalled);
    EXPECT_EQ(99, reply.code());
}

TEST(ScheduleStateTest, Process_FilterInit_CallsInitEvent)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    state.m_filterResult = scheduleState::Fileter_Init;
    task.m_initReply = Reply(77, "init");
    Reply reply = state.process(&jsonData);
    EXPECT_TRUE(task.m_initCalled);
    EXPECT_EQ(77, reply.code());
}

// ============================================================================
// initEvent()
// ============================================================================

TEST(ScheduleStateTest, InitEvent_DelegatesToTask)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    task.m_initReply = Reply(55, "init_reply");
    Reply reply = state.initEvent(&jsonData);
    EXPECT_TRUE(task.m_initCalled);
    EXPECT_EQ(55, reply.code());
}

// ============================================================================
// process() — getDateTimeInvalid() == true path
// ============================================================================

TEST(ScheduleStateTest, Process_DateTimeInvalid_EarlyReturn)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    QJsonObject root;
    QJsonArray slotsArr;
    QJsonObject invalidSlot;
    invalidSlot["name"] = "datetimeInvalid";
    slotsArr.append(invalidSlot);
    root["slots"] = slotsArr;
    jsonData.JosnResolve(root);
    ASSERT_TRUE(jsonData.getDateTimeInvalid());

    Reply reply = state.process(&jsonData);
    EXPECT_FALSE(state.m_normalCalled);
    EXPECT_FALSE(state.m_errCalled);
    EXPECT_EQ(0, reply.code());
}

// ============================================================================
// Destructor — deleting destructor (D0) via base pointer
// ============================================================================

TEST(ScheduleStateTest, Destructor_DeletingViaBasePointer)
{
    MockScheduleBaseTask task;
    scheduleState *state = new TestableScheduleState(&task);
    delete state;
    SUCCEED();
}

// ============================================================================
// changeDateErrJudge()
// ============================================================================

TEST(ScheduleStateTest, ChangeDateErrJudge_NonChangeJsonData_ReturnsDefault)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    auto result = state.changeDateErrJudge(&jsonData, scheduleState::Fileter_Normal);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(ScheduleStateTest, ChangeDateErrJudge_NonChangeJsonData_ReturnsDefaultErr)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    JsonData jsonData;
    auto result = state.changeDateErrJudge(&jsonData, scheduleState::Fileter_Err);
    EXPECT_EQ(scheduleState::Fileter_Err, result);
}

TEST(ScheduleStateTest, ChangeDateErrJudge_HasToPlaceButNoFrom_ReturnsErr)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    changejsondata changeData;
    changeData.setToPlaceStr("test_place");
    // fromDateTime is empty, TitleName is empty
    auto result = state.changeDateErrJudge(&changeData, scheduleState::Fileter_Normal);
    EXPECT_EQ(scheduleState::Fileter_Err, result);
}

TEST(ScheduleStateTest, ChangeDateErrJudge_HasToDateTimeButNoFrom_ReturnsErr)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    changejsondata changeData;
    SemanticsDateTime toDateTime;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDateTime.suggestDatetime.append(info);
    changeData.setToDateTime(toDateTime);
    // fromDateTime is empty, TitleName is empty
    auto result = state.changeDateErrJudge(&changeData, scheduleState::Fileter_Normal);
    EXPECT_EQ(scheduleState::Fileter_Err, result);
}

TEST(ScheduleStateTest, ChangeDateErrJudge_HasFromDateTime_ReturnsDefault)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    changejsondata changeData;
    changeData.setToPlaceStr("test_place");
    SemanticsDateTime fromDateTime;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    fromDateTime.suggestDatetime.append(info);
    changeData.setFromDateTime(fromDateTime);
    // hasChangeToData = true, but noChangeDate = false (fromDateTime has data)
    auto result = state.changeDateErrJudge(&changeData, scheduleState::Fileter_Normal);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(ScheduleStateTest, ChangeDateErrJudge_HasTitleName_ReturnsDefault)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    changejsondata changeData;
    changeData.setToPlaceStr("test_place");
    changeData.setTitleName("some_title");
    // hasChangeToData = true, noChangeDate = false (TitleName is not empty)
    auto result = state.changeDateErrJudge(&changeData, scheduleState::Fileter_Normal);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(ScheduleStateTest, ChangeDateErrJudge_NoChangeToData_ReturnsDefault)
{
    MockScheduleBaseTask task;
    TestableScheduleState state(&task);

    changejsondata changeData;
    // No toPlaceStr, no toDateTime → hasChangeToData = false
    auto result = state.changeDateErrJudge(&changeData, scheduleState::Fileter_Normal);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(ScheduleStateTest, Destructor_MinimalDerivedDeletingViaBasePointer)
{
    MockScheduleBaseTask task;
    scheduleState *state = new MinimalDerivedState(&task);
    delete state;
    SUCCEED();
}
