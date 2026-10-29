// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>

#include "queryschedulestate.h"
#include "mock_schedulebasetask.h"
#include "jsondata.h"
#include "changejsondata.h"
#include "clocaldata.h"
#include "dschedule.h"
#include "reply.h"
#include "globaldef.h"

class QueryScheduleStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_task = new MockScheduleBaseTask();
        m_state = new queryScheduleState(m_task);
        m_task->m_State = m_state;
    }

    void TearDown() override
    {
        delete m_task;
    }

    MockScheduleBaseTask *m_task {nullptr};
    queryScheduleState *m_state {nullptr};
};

TEST_F(QueryScheduleStateTest, Constructor)
{
    ASSERT_NE(m_state, nullptr);
    EXPECT_EQ(m_task->getState(), m_state);
}

TEST_F(QueryScheduleStateTest, GetReplyByIntent_True)
{
    Reply result = m_state->getReplyByIntent(true);

    EXPECT_EQ(result.code(), 0);
    EXPECT_FALSE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(QueryScheduleStateTest, GetReplyByIntent_False)
{
    Reply result = m_state->getReplyByIntent(false);

    EXPECT_EQ(result.code(), 0);
    EXPECT_FALSE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(QueryScheduleStateTest, ErrEvent)
{
    Reply result = m_state->ErrEvent();

    EXPECT_EQ(result.code(), 0);
    EXPECT_FALSE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
    EXPECT_EQ(result.displayMessage().toStdString(), G_ERR_TTS);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_TTS, 0);
    EXPECT_NE(result.getReplyType() & Reply::RT_STRING_DISPLAY, 0);
}

TEST_F(QueryScheduleStateTest, EventFilter_Last_ReturnsErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(QueryScheduleStateTest, EventFilter_ProThis_ReturnsErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::PRO_THIS);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(QueryScheduleStateTest, EventFilter_OffsetWithProNone_ReturnsErr)
{
    JsonData data;
    data.setOffset(0);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(QueryScheduleStateTest, EventFilter_DefaultChangeDataWithFrom_ReturnsNormal)
{
    changejsondata data;
    // offset defaults to -1, propertyStatus defaults to PRO_NONE
    // offset > -1 is false (−1 > −1), so skip that branch
    // changeDateErrJudge: not a nullptr changejsondata, but no to-data → returns defaultflag (Fileter_Normal)
    SemanticsDateTime fromDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    fromDt.suggestDatetime.append(info);
    data.setFromDateTime(fromDt);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Normal);
}

TEST_F(QueryScheduleStateTest, EventFilter_DefaultJsonData_ReturnsNormal)
{
    JsonData data;
    // offset=-1, PRO_NONE → offset > -1 is false, skip
    // changeDateErrJudge: dynamic_cast to changejsondata returns nullptr → returns defaultflag (Fileter_Normal)
    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Normal);
}

TEST_F(QueryScheduleStateTest, EventFilter_ChangeDataWithToButNoFrom_ReturnsErr)
{
    changejsondata data;
    // Set offset to -1, PRO_NONE → skip first two branches
    // changeDateErrJudge: hasChangeToData=true, noChangeDate=true → returns Fileter_Err
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDt.suggestDatetime.append(info);
    data.setToDateTime(toDt);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(QueryScheduleStateTest, Process_EventFilterErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    Reply result = m_state->process(&data);

    EXPECT_EQ(result.code(), 0);
    EXPECT_FALSE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(QueryScheduleStateTest, Process_EventFilterInit)
{
    // With default JsonData (not changejsondata), eventFilter returns Fileter_Normal
    // For Fileter_Init, we'd need to get past the filter branches to changeDateErrJudge with a non-Err result
    // But queryScheduleState::eventFilter only returns Err or Normal/default — never Init directly
    // So test Fileter_Normal path instead via process
    JsonData data;

    Reply expectedReply;
    expectedReply.code(44);
    m_task->m_getFeedbackByQueryScheduleReply = expectedReply;

    // This will call normalEvent which creates queryScheduleProxy (DBus)
    // Use a timeout-friendly approach
    Reply result = m_state->process(&data);

    // If DBus is unavailable, normalEvent should still return (empty schedule list)
    // getFeedbackByQuerySchedule is mocked
    EXPECT_TRUE(m_task->m_getFeedbackByQueryScheduleCalled);
}

TEST_F(QueryScheduleStateTest, NormalEvent_WithChangeJsonData)
{
    // Test the changejsondata path of normalEvent
    // This exercises the m_localData creation and toDateTime/toPlaceStr branches
    changejsondata data;
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDt.suggestDatetime.append(info);
    data.setToDateTime(toDt);
    data.setToPlaceStr("new location");

    Reply expectedReply;
    expectedReply.code(55);
    m_task->m_getFeedbackByQueryScheduleReply = expectedReply;

    // normalEvent creates queryScheduleProxy which hits DBus — may be slow
    Reply result = m_state->normalEvent(&data);

    EXPECT_TRUE(m_task->m_getFeedbackByQueryScheduleCalled);
    EXPECT_FALSE(m_state->getLocalData().isNull());
}
