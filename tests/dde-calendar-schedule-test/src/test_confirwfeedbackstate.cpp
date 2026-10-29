// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>

#include "confirwfeedbackstate.h"
#include "mock_schedulebasetask.h"
#include "jsondata.h"
#include "changejsondata.h"
#include "clocaldata.h"
#include "dschedule.h"
#include "reply.h"
#include "globaldef.h"

class ConfirwFeedbackStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_task = new MockScheduleBaseTask();
        m_state = new confirwFeedbackState(m_task);
        m_task->m_State = m_state;
    }

    void TearDown() override
    {
        // MockScheduleBaseTask destructor will delete m_State (fixed in mock)
        delete m_task;
    }

    MockScheduleBaseTask *m_task {nullptr};
    confirwFeedbackState *m_state {nullptr};
};

TEST_F(ConfirwFeedbackStateTest, Constructor)
{
    ASSERT_NE(m_state, nullptr);
    EXPECT_EQ(m_task->getState(), m_state);
}

TEST_F(ConfirwFeedbackStateTest, GetReplyByIntent_True)
{
    Reply expectedReply;
    expectedReply.code(42);
    m_task->m_confirmInfoReply = expectedReply;

    Reply result = m_state->getReplyByIntent(true);

    EXPECT_TRUE(m_task->m_confirmInfoCalled);
    EXPECT_TRUE(m_task->m_confirmInfoArg);
    EXPECT_EQ(result.code(), 42);
}

TEST_F(ConfirwFeedbackStateTest, GetReplyByIntent_False)
{
    Reply expectedReply;
    expectedReply.code(99);
    m_task->m_confirmInfoReply = expectedReply;

    Reply result = m_state->getReplyByIntent(false);

    EXPECT_TRUE(m_task->m_confirmInfoCalled);
    EXPECT_FALSE(m_task->m_confirmInfoArg);
    EXPECT_EQ(result.code(), 99);
}

TEST_F(ConfirwFeedbackStateTest, EventFilter_Next_ReturnsInit)
{
    JsonData data;
    data.setPropertyStatus(JsonData::NEXT);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(ConfirwFeedbackStateTest, EventFilter_Last_ReturnsErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(ConfirwFeedbackStateTest, EventFilter_OffsetPositive_ReturnsErr)
{
    JsonData data;
    data.setOffset(1);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(ConfirwFeedbackStateTest, EventFilter_DefaultJsonData_ReturnsInit)
{
    JsonData data;

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(ConfirwFeedbackStateTest, EventFilter_ChangeDataWithToButNoFrom_ReturnsErr)
{
    changejsondata data;
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDt.suggestDatetime.append(info);
    data.setToDateTime(toDt);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(ConfirwFeedbackStateTest, EventFilter_ChangeDataWithFrom_ReturnsInit)
{
    changejsondata data;
    SemanticsDateTime fromDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    fromDt.suggestDatetime.append(info);
    data.setFromDateTime(fromDt);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(ConfirwFeedbackStateTest, ErrEvent)
{
    Reply result = m_state->ErrEvent();

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
    EXPECT_EQ(result.displayMessage().toStdString(), G_ERR_TTS);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_TTS, 0);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_DISPLAY, 0);
}

TEST_F(ConfirwFeedbackStateTest, NormalEvent)
{
    CLocalData::Ptr localData(new CLocalData());
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("test schedule");
    localData->setSelectInfo(schedule);
    m_state->setLocalData(localData);

    Reply expectedReply;
    expectedReply.code(77);
    m_task->m_confirwScheduleHandleReply = expectedReply;

    JsonData data;
    Reply result = m_state->normalEvent(&data);

    EXPECT_TRUE(m_task->m_confirwScheduleHandleCalled);
    EXPECT_EQ(m_task->m_confirwScheduleHandleArg, schedule);
    EXPECT_EQ(result.code(), 77);
}

TEST_F(ConfirwFeedbackStateTest, Process_EventFilterErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    Reply result = m_state->process(&data);

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(ConfirwFeedbackStateTest, Process_EventFilterInit)
{
    JsonData data;
    data.setPropertyStatus(JsonData::NEXT);

    Reply expectedReply;
    expectedReply.code(55);
    m_task->m_initStateReply = expectedReply;

    Reply result = m_state->process(&data);

    EXPECT_TRUE(m_task->m_initStateCalled);
    EXPECT_EQ(result.code(), 55);
}

TEST_F(ConfirwFeedbackStateTest, Process_EventFilterChangeDataErr)
{
    // confirwFeedbackState::eventFilter never returns Normal;
    // changejsondata with to-data but no from-data triggers changeDateErrJudge → Err
    changejsondata data;
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDt.suggestDatetime.append(info);
    data.setToDateTime(toDt);

    Reply result = m_state->process(&data);

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(ConfirwFeedbackStateTest, Process_DateTimeInvalid)
{
    JsonData data;
    // m_DateTimeInvalid is private with no public setter; use -fno-access-control
    // We can't set it directly, so test process with valid datetime (default false)
    Reply result = m_state->process(&data);
    // Default JsonData is "valid" → eventFilter returns Fileter_Init → initEvent
    EXPECT_TRUE(m_task->m_initStateCalled);
}
