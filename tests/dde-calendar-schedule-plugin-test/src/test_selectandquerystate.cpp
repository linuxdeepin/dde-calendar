// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "test_helpers.h"

// ============================================================================
// Constructor
// ============================================================================

TEST(SelectAndQueryStateTest, Constructor_StoresTaskPointer)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    EXPECT_EQ(&task, state.m_Task);
}

// ============================================================================
// getReplyByIntent()
// ============================================================================

TEST(SelectAndQueryStateTest, GetReplyByIntent_ReturnsGErrTts)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    Reply reply = state.getReplyByIntent(true);
    EXPECT_EQ(0, reply.code());
    EXPECT_EQ(QString(G_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(G_ERR_TTS), reply.displayMessage());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(SelectAndQueryStateTest, GetReplyByIntent_IgnoresIsOK)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    Reply replyTrue = state.getReplyByIntent(true);
    Reply replyFalse = state.getReplyByIntent(false);
    EXPECT_EQ(replyTrue.code(), replyFalse.code());
    EXPECT_EQ(replyTrue.ttsMessage(), replyFalse.ttsMessage());
}

// ============================================================================
// eventFilter()
// ============================================================================

TEST(SelectAndQueryStateEventFilterTest, PropertyAll_ReturnsInit)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setPropertyStatus(JsonData::ALL);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectAndQueryStateEventFilterTest, PropertyNext_ReturnsInit)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setPropertyStatus(JsonData::NEXT);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectAndQueryStateEventFilterTest, IsVaild_ReturnsInit)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    // Default changejsondata has isVaild()=true → Init
    changejsondata jsonData;
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectAndQueryStateEventFilterTest, HasDateTime_ReturnsInit)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    // Set offset to non-default to make isVaild()=false
    jsonData.setOffset(0);
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    dt.suggestDatetime.append(info);
    jsonData.setDateTime(dt);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectAndQueryStateEventFilterTest, HasTitleName_ReturnsInit)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(0); // make isVaild()=false
    jsonData.setTitleName("meeting");
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectAndQueryStateEventFilterTest, HasRepeatStatus_ReturnsInit)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(0); // make isVaild()=false
    jsonData.setRepeatStatus(JsonData::EVED);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectAndQueryStateEventFilterTest, PropertyLast_ReturnsNormal)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setPropertyStatus(JsonData::LAST);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(SelectAndQueryStateEventFilterTest, HasFromDateTime_ReturnsInit)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(0); // make isVaild()=false
    SemanticsDateTime fromDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    fromDt.suggestDatetime.append(info);
    jsonData.setFromDateTime(fromDt);
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Init, result);
}

TEST(SelectAndQueryStateEventFilterTest, BothOffsetsNegative_ReturnsErr)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    // getOffet() default is -1
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(-2); // isVaild()=false, offset < 0
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Err, result);
}

TEST(SelectAndQueryStateEventFilterTest, OffsetExceedsShowcount_ReturnsErr)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(3));
    // showcount = min(10, 3) = 3
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(5); // 5 > 3 → Err
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Err, result);
}

TEST(SelectAndQueryStateEventFilterTest, OffsetWithinRange_ReturnsNormal)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    // showcount = min(10, 5) = 5
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(3); // 3 <= 5 → Normal
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

TEST(SelectAndQueryStateEventFilterTest, OffsetEqualsShowcount_ReturnsNormal)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    // showcount = 5
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(5); // 5 <= 5 → Normal (not strictly greater)
    auto result = state.eventFilter(&jsonData);
    EXPECT_EQ(scheduleState::Fileter_Normal, result);
}

// ============================================================================
// ErrEvent()
// ============================================================================

TEST(SelectAndQueryStateTest, ErrEvent_ReturnsQueryErrTts)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    Reply reply = state.ErrEvent();
    EXPECT_EQ(0, reply.code());
    EXPECT_EQ(QString(QUERY_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(QUERY_ERR_TTS), reply.displayMessage());
    EXPECT_TRUE(reply.getShouldEndSession());
}

// ============================================================================
// normalEvent()
// ============================================================================

TEST(SelectAndQueryStateNormalEventTest, LastProperty_OffsetPositive_CallsSelectSchedule)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
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
    EXPECT_EQ(3, localData->getOffet());
    EXPECT_FALSE(localData->SelectInfo().isNull());
}

TEST(SelectAndQueryStateNormalEventTest, NormalOffset_CallsSelectSchedule)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    state.setLocalData(localData);

    task.m_selectReply = Reply(20, "selected");

    changejsondata jsonData;
    jsonData.setOffset(2);
    Reply reply = state.normalEvent(&jsonData);
    EXPECT_TRUE(task.m_selectCalled);
    EXPECT_EQ(20, reply.code());
    EXPECT_EQ(2, localData->getOffet());
}

TEST(SelectAndQueryStateNormalEventTest, OffsetZero_ReturnsGErrTts)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(3));
    state.setLocalData(localData);

    changejsondata jsonData;
    jsonData.setOffset(0);
    Reply reply = state.normalEvent(&jsonData);
    EXPECT_FALSE(task.m_selectCalled);
    EXPECT_EQ(0, reply.code());
    EXPECT_EQ(QString(G_ERR_TTS), reply.ttsMessage());
    EXPECT_FALSE(reply.getShouldEndSession());
}

TEST(SelectAndQueryStateNormalEventTest, WithToDateTime_SetsToTime)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    state.setLocalData(localData);

    task.m_selectReply = Reply(30, "selected");

    changejsondata jsonData;
    jsonData.setOffset(1);
    SemanticsDateTime toDt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime(QDate(2026, 1, 1), QTime(10, 0));
    toDt.suggestDatetime.append(info);
    jsonData.setToDateTime(toDt);

    state.normalEvent(&jsonData);
    EXPECT_TRUE(task.m_selectCalled);
    EXPECT_EQ(1, localData->getToTime().suggestDatetime.size());
}

TEST(SelectAndQueryStateNormalEventTest, WithToPlaceStr_SetsToTitleName)
{
    MockScheduleBaseTask task;
    SelectAndQueryState state(&task);
    CLocalData::Ptr localData(new CLocalData());
    localData->setScheduleInfoVector(makeScheduleList(5));
    state.setLocalData(localData);

    task.m_selectReply = Reply(40, "selected");

    changejsondata jsonData;
    jsonData.setOffset(1);
    jsonData.setToPlaceStr("new_location");

    state.normalEvent(&jsonData);
    EXPECT_TRUE(task.m_selectCalled);
    EXPECT_EQ(QString("new_location"), localData->getToTitleName());
}
