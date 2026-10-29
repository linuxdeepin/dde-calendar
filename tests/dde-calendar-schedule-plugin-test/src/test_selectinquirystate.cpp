// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "test_helpers.h"

// ============================================================================
// Constructor
// ============================================================================

TEST(SelectInquiryStateTest, Constructor_StoresTaskPointer)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    EXPECT_EQ(&task, state.m_Task);
}

// ============================================================================
// getReplyByIntent()
// ============================================================================

TEST(SelectInquiryStateTest, GetReplyByIntent_ReturnsGErrTts)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    Reply reply = state.getReplyByIntent(true);
    EXPECT_EQ(0, reply.code());
    EXPECT_EQ(QString(G_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(G_ERR_TTS), reply.displayMessage());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(SelectInquiryStateTest, GetReplyByIntent_IgnoresIsOK)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    Reply replyTrue = state.getReplyByIntent(true);
    Reply replyFalse = state.getReplyByIntent(false);
    EXPECT_EQ(replyTrue.code(), replyFalse.code());
    EXPECT_EQ(replyTrue.ttsMessage(), replyFalse.ttsMessage());
}

// ============================================================================
// eventFilter()
// ============================================================================

TEST(SelectInquiryStateEventFilterTest, PropertyAll_ReturnsInit)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setPropertyStatus(JsonData::ALL);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectInquiryStateEventFilterTest, PropertyNext_ReturnsInit)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setPropertyStatus(JsonData::NEXT);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectInquiryStateEventFilterTest, IsVaild_ReturnsInit)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    // Default changejsondata has isVaild()=true → Init
    changejsondata jsonData;
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectInquiryStateEventFilterTest, HasRepeatStatus_ReturnsInit)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(0); // make isVaild()=false
    jsonData.setRepeatStatus(JsonData::EVEW);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectInquiryStateEventFilterTest, PropertyLast_ReturnsNormal)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setPropertyStatus(JsonData::LAST);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(SelectInquiryStateEventFilterTest, HasDateTime_ReturnsInit)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(0); // make isVaild()=false
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    dt.suggestDatetime.append(info);
    jsonData.setDateTime(dt);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectInquiryStateEventFilterTest, HasTitleName_ReturnsInit)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(0); // make isVaild()=false
    jsonData.setTitleName("reminder");
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectInquiryStateEventFilterTest, OffsetExceedsShowcount_ReturnsErr)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(3));
    // showcount = min(10, 3) = 3
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(5); // 5 > 3 → Err, also makes isVaild()=false
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Err, result);
}

TEST(SelectInquiryStateEventFilterTest, OffsetWithinRange_ReturnsNormal)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    // showcount = min(10, 5) = 5
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(3); // 3 <= 5 → Normal
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(SelectInquiryStateEventFilterTest, OffsetEqualsShowcount_ReturnsNormal)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    // showcount = 5
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(5); // 5 <= 5 → Normal
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

// ============================================================================
// ErrEvent()
// ============================================================================

TEST(SelectInquiryStateTest, ErrEvent_ReturnsQueryErrTts)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    Reply reply = state.ErrEvent();
    EXPECT_EQ(0, reply.code());
    EXPECT_EQ(QString(QUERY_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(QUERY_ERR_TTS), reply.displayMessage());
    EXPECT_TRUE(reply.getShouldEndSession());
}

// ============================================================================
// normalEvent()
// ============================================================================

TEST(SelectInquiryStateNormalEventTest, LastProperty_CallsSelectSchedule)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(3));
    state.setLocalData(localData);

    task.m_selectReply = Reply(10, "selected");

    changejsondata jsonData;
    jsonData.setPropertyStatus(JsonData::LAST);
    // offset = showcount = 3
    Reply reply = state.normalEvent(&jsonData);
    EXPECT_TRUE(task.m_selectCalled);
    EXPECT_EQ(10, reply.code());
    EXPECT_FALSE(task.m_lastSelectInfo.isNull());
}

TEST(SelectInquiryStateNormalEventTest, NormalOffset_CallsSelectSchedule)
{
    MockScheduleBaseTask task;
    selectInquiryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    state.setLocalData(localData);

    task.m_selectReply = Reply(20, "selected");

    changejsondata jsonData;
    jsonData.setOffset(2);
    Reply reply = state.normalEvent(&jsonData);
    EXPECT_TRUE(task.m_selectCalled);
    EXPECT_EQ(20, reply.code());
}
