// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>

#include "repeatfeedbackstate.h"
#include "mock_schedulebasetask.h"
#include "jsondata.h"
#include "changejsondata.h"
#include "clocaldata.h"
#include "dschedule.h"
#include "reply.h"
#include "globaldef.h"

class RepeatFeedbackStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_task = new MockScheduleBaseTask();
        m_state = new repeatfeedbackstate(m_task);
        m_task->m_State = m_state;
    }

    void TearDown() override
    {
        delete m_task;
    }

    MockScheduleBaseTask *m_task {nullptr};
    repeatfeedbackstate *m_state {nullptr};
};

TEST_F(RepeatFeedbackStateTest, Constructor)
{
    ASSERT_NE(m_state, nullptr);
    EXPECT_EQ(m_task->getState(), m_state);
}

TEST_F(RepeatFeedbackStateTest, GetReplyByIntent_True)
{
    Reply result = m_state->getReplyByIntent(true);

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
    EXPECT_EQ(result.displayMessage().toStdString(), G_ERR_TTS);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_TTS, 0);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_DISPLAY, 0);
}

TEST_F(RepeatFeedbackStateTest, GetReplyByIntent_False)
{
    Reply expectedReply;
    expectedReply.code(42);
    m_task->m_initStateReply = expectedReply;

    Reply result = m_state->getReplyByIntent(false);

    EXPECT_TRUE(m_task->m_initStateCalled);
    EXPECT_EQ(m_task->m_initStateArg, nullptr);
    EXPECT_EQ(result.code(), 42);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_Next_ReturnsInit)
{
    JsonData data;
    data.setPropertyStatus(JsonData::NEXT);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_HasDateTime_ReturnsInit)
{
    JsonData data;
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    dt.suggestDatetime.append(info);
    data.setDateTime(dt);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_HasTitleName_ReturnsInit)
{
    JsonData data;
    data.setTitleName("test title");

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_HasRepeatStatus_ReturnsInit)
{
    JsonData data;
    data.setRepeatStatus(JsonData::EVED);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_All_ReturnsNormal)
{
    JsonData data;
    data.setPropertyStatus(JsonData::ALL);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Normal);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_ProThis_ReturnsNormal)
{
    JsonData data;
    data.setPropertyStatus(JsonData::PRO_THIS);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Normal);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_Last_ReturnsErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_OffsetPositive_ReturnsErr)
{
    JsonData data;
    data.setOffset(1);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_DefaultChangeData_ReturnsInit)
{
    changejsondata data;
    // offset=-1, PRO_NONE, no from/to data → changeDateErrJudge returns defaultflag (Fileter_Init)

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(RepeatFeedbackStateTest, EventFilter_ChangeDataWithToButNoFrom_ReturnsErr)
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

TEST_F(RepeatFeedbackStateTest, ErrEvent)
{
    Reply result = m_state->ErrEvent();

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_TTS, 0);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_DISPLAY, 0);
}

TEST_F(RepeatFeedbackStateTest, NormalEvent_All)
{
    CLocalData::Ptr localData(new CLocalData());
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("test");
    localData->setSelectInfo(schedule);
    m_state->setLocalData(localData);

    Reply expectedReply;
    expectedReply.code(66);
    m_task->m_repeatScheduleHandleReply = expectedReply;

    JsonData data;
    data.setPropertyStatus(JsonData::ALL);

    Reply result = m_state->normalEvent(&data);

    EXPECT_TRUE(m_task->m_repeatScheduleHandleCalled);
    EXPECT_FALSE(m_task->m_repeatScheduleHandleIsOnlyOne);
    EXPECT_EQ(m_task->m_repeatScheduleHandleArg, schedule);
    EXPECT_EQ(result.code(), 66);
}

TEST_F(RepeatFeedbackStateTest, NormalEvent_NotAll)
{
    CLocalData::Ptr localData(new CLocalData());
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("test");
    localData->setSelectInfo(schedule);
    m_state->setLocalData(localData);

    Reply expectedReply;
    expectedReply.code(77);
    m_task->m_repeatScheduleHandleReply = expectedReply;

    JsonData data;
    data.setPropertyStatus(JsonData::PRO_NONE);

    Reply result = m_state->normalEvent(&data);

    EXPECT_TRUE(m_task->m_repeatScheduleHandleCalled);
    EXPECT_TRUE(m_task->m_repeatScheduleHandleIsOnlyOne);
    EXPECT_EQ(m_task->m_repeatScheduleHandleArg, schedule);
    EXPECT_EQ(result.code(), 77);
}

TEST_F(RepeatFeedbackStateTest, Process_EventFilterErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    Reply result = m_state->process(&data);

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(RepeatFeedbackStateTest, Process_EventFilterNormal)
{
    CLocalData::Ptr localData(new CLocalData());
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("test");
    localData->setSelectInfo(schedule);
    m_state->setLocalData(localData);

    Reply expectedReply;
    expectedReply.code(88);
    m_task->m_repeatScheduleHandleReply = expectedReply;

    JsonData data;
    data.setPropertyStatus(JsonData::ALL);

    Reply result = m_state->process(&data);

    EXPECT_TRUE(m_task->m_repeatScheduleHandleCalled);
    EXPECT_EQ(result.code(), 88);
}

TEST_F(RepeatFeedbackStateTest, Process_EventFilterInit)
{
    JsonData data;
    data.setPropertyStatus(JsonData::NEXT);

    Reply expectedReply;
    expectedReply.code(33);
    m_task->m_initStateReply = expectedReply;

    Reply result = m_state->process(&data);

    EXPECT_TRUE(m_task->m_initStateCalled);
    EXPECT_EQ(result.code(), 33);
}
