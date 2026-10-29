// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QCoreApplication>

#include "getchangedatastate.h"
#include "mock_schedulebasetask.h"
#include "jsondata.h"
#include "changejsondata.h"
#include "clocaldata.h"
#include "dschedule.h"
#include "reply.h"
#include "globaldef.h"

class GetChangeDataStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_task = new MockScheduleBaseTask();
        m_state = new getChangeDataState(m_task);
        m_task->m_State = m_state;
    }

    void TearDown() override
    {
        delete m_task;
    }

    MockScheduleBaseTask *m_task {nullptr};
    getChangeDataState *m_state {nullptr};
};

TEST_F(GetChangeDataStateTest, Constructor)
{
    ASSERT_NE(m_state, nullptr);
    EXPECT_EQ(m_task->getState(), m_state);
}

TEST_F(GetChangeDataStateTest, GetReplyByIntent)
{
    Reply result = m_state->getReplyByIntent(true);

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(GetChangeDataStateTest, GetReplyByIntent_False)
{
    Reply result = m_state->getReplyByIntent(false);

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(GetChangeDataStateTest, EventFilter_All_ReturnsInit)
{
    JsonData data;
    data.setPropertyStatus(JsonData::ALL);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(GetChangeDataStateTest, EventFilter_Next_ReturnsInit)
{
    JsonData data;
    data.setPropertyStatus(JsonData::NEXT);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(GetChangeDataStateTest, EventFilter_IsVaild_ReturnsInit)
{
    JsonData data;

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(GetChangeDataStateTest, EventFilter_HasDateTime_ReturnsInit)
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

TEST_F(GetChangeDataStateTest, EventFilter_HasTitleName_ReturnsInit)
{
    JsonData data;
    data.setTitleName("test title");

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(GetChangeDataStateTest, EventFilter_HasRepeatStatus_ReturnsInit)
{
    JsonData data;
    data.setRepeatStatus(JsonData::EVED);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Init);
}

TEST_F(GetChangeDataStateTest, EventFilter_Last_ReturnsErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(GetChangeDataStateTest, EventFilter_OffsetPositive_ReturnsErr)
{
    JsonData data;
    data.setOffset(1);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(GetChangeDataStateTest, EventFilter_ChangeDataFromDateTime_ReturnsInit)
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

TEST_F(GetChangeDataStateTest, EventFilter_ChangeDataToDateTime_ReturnsNormal)
{
    changejsondata data;
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDt.suggestDatetime.append(info);
    data.setToDateTime(toDt);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Normal);
}

TEST_F(GetChangeDataStateTest, EventFilter_ChangeDataToPlaceStr_ReturnsNormal)
{
    changejsondata data;
    data.setToPlaceStr("new place");

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Normal);
}

TEST_F(GetChangeDataStateTest, EventFilter_ChangeDataNoChangeInfo_ReturnsErr)
{
    changejsondata data;
    // Set offset to -1 and PRO_NONE so we don't hit earlier branches
    // Also set a non-empty fromDateTime to skip the "else Err" path
    // Actually, with no from/to data, isVaild() returns true → Fileter_Init
    // Need isVaild() to be false: set offset to 0 (but that hits offset > 0? No, 0 > 0 is false)
    // With offset=0, propertyStatus=PRO_NONE, isVaild() returns false (offset != -1)
    // Then we fall through to dynamic_cast, fromDateTime is empty, toDateTime is empty, toPlaceStr empty → Fileter_Err
    data.setOffset(0);

    auto result = m_state->eventFilter(&data);
    EXPECT_EQ(result, scheduleState::Fileter_Err);
}

TEST_F(GetChangeDataStateTest, ErrEvent)
{
    Reply result = m_state->ErrEvent();

    EXPECT_EQ(result.code(), 0);
    EXPECT_TRUE(result.getShouldEndSession());
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(GetChangeDataStateTest, NormalEvent_ToDateTime)
{
    CLocalData::Ptr localData(new CLocalData());
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("test");
    localData->setSelectInfo(schedule);
    m_state->setLocalData(localData);

    Reply expectedReply;
    expectedReply.code(66);
    m_task->m_getReplyBySelectScheduleReply = expectedReply;

    changejsondata data;
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDt.suggestDatetime.append(info);
    data.setToDateTime(toDt);

    Reply result = m_state->normalEvent(&data);

    EXPECT_TRUE(m_task->m_getReplyBySelectScheduleCalled);
    EXPECT_EQ(result.code(), 66);
}

TEST_F(GetChangeDataStateTest, NormalEvent_ToPlaceStr)
{
    CLocalData::Ptr localData(new CLocalData());
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("test");
    localData->setSelectInfo(schedule);
    m_state->setLocalData(localData);

    Reply expectedReply;
    expectedReply.code(77);
    m_task->m_getReplyBySelectScheduleReply = expectedReply;

    changejsondata data;
    data.setToPlaceStr("new location");

    Reply result = m_state->normalEvent(&data);

    EXPECT_TRUE(m_task->m_getReplyBySelectScheduleCalled);
    EXPECT_EQ(result.code(), 77);
}

TEST_F(GetChangeDataStateTest, NormalEvent_BothToData)
{
    CLocalData::Ptr localData(new CLocalData());
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("test");
    localData->setSelectInfo(schedule);
    m_state->setLocalData(localData);

    Reply expectedReply;
    expectedReply.code(88);
    m_task->m_getReplyBySelectScheduleReply = expectedReply;

    changejsondata data;
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    toDt.suggestDatetime.append(info);
    data.setToDateTime(toDt);
    data.setToPlaceStr("new location");

    Reply result = m_state->normalEvent(&data);

    EXPECT_TRUE(m_task->m_getReplyBySelectScheduleCalled);
    EXPECT_EQ(result.code(), 88);
}

TEST_F(GetChangeDataStateTest, Process_EventFilterErr)
{
    JsonData data;
    data.setPropertyStatus(JsonData::LAST);

    Reply result = m_state->process(&data);

    EXPECT_EQ(result.code(), 0);
    EXPECT_EQ(result.ttsMessage().toStdString(), G_ERR_TTS);
}

TEST_F(GetChangeDataStateTest, Process_EventFilterInit)
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
