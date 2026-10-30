// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "queryscheduletask.h"
#include "semanticanalysistask.h"
#include "queryjsondata.h"
#include "jsondata.h"
#include "timedatastruct.h"
#include "globaldef.h"
#include "reply.h"
#include "dschedule.h"
#include "queryscheduleproxy.h"
#include "stub.h"

#include <QDateTime>
#include <QDate>
#include <QTime>

// Stub for queryScheduleProxy::querySchedule — avoids DBus calls
static DSchedule::Map stub_querySchedule()
{
    return DSchedule::Map();
}

// Helper to create a SuggestDatetimeInfo
static SuggestDatetimeInfo makeSuggestion(const QDateTime &dt, bool hasTime)
{
    SuggestDatetimeInfo info;
    info.datetime = dt;
    info.hasTime = hasTime;
    return info;
}

// Helper to build a SemanticsDateTime from a list of suggestions
static SemanticsDateTime makeSemanticsDateTime(const QVector<SuggestDatetimeInfo> &suggestions)
{
    SemanticsDateTime sdt;
    sdt.suggestDatetime = suggestions;
    return sdt;
}

// ============================================================================
// Constructor
// ============================================================================

TEST(QueryScheduleTaskTest, Constructor_NoCrash)
{
    queryScheduleTask task;
    EXPECT_EQ(nullptr, task.getState());
}

TEST(QueryScheduleTaskTest, Constructor_IsScheduleBaseTask)
{
    queryScheduleTask task;
    scheduleBaseTask *base = &task;
    EXPECT_NE(nullptr, base);
}

// ============================================================================
// SchedulePress — early-return paths (no widget/DBus needed)
// ============================================================================

TEST(QueryScheduleTaskTest, SchedulePress_NullJsonData_ReturnsErrorMessage)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString(G_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(G_ERR_TTS), reply.displayMessage());
    EXPECT_EQ(0, reply.code());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(QueryScheduleTaskTest, SchedulePress_NonQueryJsonData_ReturnsErrorMessage)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    JsonData *data = new JsonData();
    semantic.setJsonData(data);
    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString(G_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(0, reply.code());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(QueryScheduleTaskTest, SchedulePress_DateTimeInvalid_ReturnsDateTimeErrTts)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    QueryJsonData *data = new QueryJsonData();
    data->m_DateTimeInvalid = true;
    semantic.setJsonData(data);
    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString(DATETIME_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(DATETIME_ERR_TTS), reply.displayMessage());
    EXPECT_EQ(0, reply.code());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(QueryScheduleTaskTest, SchedulePress_OffsetWithProNone_ReturnsCancelErrTts)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    QueryJsonData *data = new QueryJsonData();
    data->setOffset(0);
    semantic.setJsonData(data);
    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString(CANCEL_ERR_TTS), reply.ttsMessage());
    EXPECT_EQ(QString(CANCEL_ERR_TTS), reply.displayMessage());
    EXPECT_EQ(0, reply.code());
    EXPECT_TRUE(reply.getShouldEndSession());
}

// ============================================================================
// SchedulePress — full path (requires QApplication + stubbed DBus)
// ============================================================================

class QueryScheduleTaskFullTest : public ::testing::Test
{
protected:
    Stub stub;

    void SetUp() override
    {
        stub.set(ADDR(queryScheduleProxy, querySchedule), stub_querySchedule);
    }
};

TEST_F(QueryScheduleTaskFullTest, SchedulePress_ShouldEndSession_OverdueDate_ReturnsOverdueTts)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    QueryJsonData *data = new QueryJsonData();

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime(QDate(2020, 1, 1), QTime(10, 0, 0)), true)});
    data->setDateTime(sdt);
    data->setShouldEndSession(true);
    semantic.setJsonData(data);

    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString(VIEW_DATE_IS_OVERDUE_TTS), reply.ttsMessage());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST_F(QueryScheduleTaskFullTest, SchedulePress_ShouldEndSession_BeyondHalfYear_ReturnsOutTts)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    QueryJsonData *data = new QueryJsonData();

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime::currentDateTime().addMonths(12), true)});
    data->setDateTime(sdt);
    data->setShouldEndSession(true);
    semantic.setJsonData(data);

    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString(VIEW_DATETIME_OUT_TTS), reply.ttsMessage());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST_F(QueryScheduleTaskFullTest, SchedulePress_ShouldEndSession_NoSchedules_ReturnsNoScheduleTts)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    QueryJsonData *data = new QueryJsonData();

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime::currentDateTime().addDays(7), true)});
    data->setDateTime(sdt);
    data->setShouldEndSession(true);
    semantic.setJsonData(data);

    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString(NO_SCHEDULE_VIEWED_TTS), reply.ttsMessage());
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST_F(QueryScheduleTaskFullTest, SchedulePress_MultiRound_ReturnsSuggestMsg)
{
    queryScheduleTask task;
    semanticAnalysisTask semantic;
    QueryJsonData *data = new QueryJsonData();

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime::currentDateTime().addDays(7), true)});
    data->setDateTime(sdt);
    data->setShouldEndSession(false);
    data->setSuggestMsg("test_suggest_msg");
    semantic.setJsonData(data);

    Reply reply = task.SchedulePress(semantic);
    EXPECT_EQ(QString("test_suggest_msg"), reply.ttsMessage());
    EXPECT_FALSE(reply.getShouldEndSession());
}

// ============================================================================
// setDateTime — private, accessible via -fno-access-control
// ============================================================================

TEST(QueryScheduleTaskTest, SetDateTime_Case1_SingleDateWithTime)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime(QDate(2026, 6, 15), QTime(10, 30, 0)), true)});
    data.setDateTime(sdt);

    task.setDateTime(&data);
    EXPECT_EQ(QDateTime(QDate(2026, 6, 15), QTime(10, 30, 0)), task.m_BeginDateTime);
    EXPECT_EQ(QDateTime(QDate(2026, 6, 15), QTime(10, 30, 0)), task.m_EndDateTime);
}

TEST(QueryScheduleTaskTest, SetDateTime_Case1_SingleDateNoTime_OtherDay)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime(QDate(2026, 6, 15), QTime(0, 0, 0)), false)});
    data.setDateTime(sdt);

    task.setDateTime(&data);
    EXPECT_EQ(QTime(0, 0, 0), task.m_BeginDateTime.time());
    EXPECT_EQ(QTime(23, 59, 59), task.m_EndDateTime.time());
    EXPECT_EQ(QDate(2026, 6, 15), task.m_BeginDateTime.date());
}

TEST(QueryScheduleTaskTest, SetDateTime_Case1_SingleDateNoTime_Today)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime(QDate::currentDate(), QTime(0, 0, 0)), false)});
    data.setDateTime(sdt);

    task.setDateTime(&data);
    EXPECT_EQ(QDate::currentDate(), task.m_BeginDateTime.date());
    EXPECT_EQ(QTime(23, 59, 59), task.m_EndDateTime.time());
}

TEST(QueryScheduleTaskTest, SetDateTime_Case2_DateRangeNormal)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime({
        makeSuggestion(QDateTime::currentDateTime().addDays(1), false),
        makeSuggestion(QDateTime::currentDateTime().addDays(7), false),
    });
    data.setDateTime(sdt);

    task.setDateTime(&data);
    EXPECT_EQ(QTime(0, 0, 0), task.m_BeginDateTime.time());
    EXPECT_EQ(QTime(23, 59, 59), task.m_EndDateTime.time());
}

TEST(QueryScheduleTaskTest, SetDateTime_Case2_DateRangePastEnd_InvalidTime)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime({
        makeSuggestion(QDateTime(QDate(2020, 1, 1), QTime(0, 0, 0)), true),
        makeSuggestion(QDateTime(QDate(2020, 1, 7), QTime(0, 0, 0)), true),
    });
    data.setDateTime(sdt);

    task.setDateTime(&data);
    EXPECT_EQ(QDate(0, 0, 0), task.m_BeginDateTime.date());
    EXPECT_EQ(QTime(0, 0, 0), task.m_BeginDateTime.time());
    EXPECT_EQ(QDate(0, 0, 0), task.m_EndDateTime.date());
    EXPECT_EQ(QTime(0, 0, 0), task.m_EndDateTime.time());
}

TEST(QueryScheduleTaskTest, SetDateTime_Case2_DateRangeBeyondHalfYear_InvalidTime)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime({
        makeSuggestion(QDateTime::currentDateTime().addMonths(12), true),
        makeSuggestion(QDateTime::currentDateTime().addMonths(18), true),
    });
    data.setDateTime(sdt);

    task.setDateTime(&data);
    EXPECT_EQ(QDate(0, 0, 0), task.m_BeginDateTime.date());
    EXPECT_EQ(QTime(0, 0, 0), task.m_BeginDateTime.time());
}

TEST(QueryScheduleTaskTest, SetDateTime_Default_NoDates_InvalidTime)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt;
    data.setDateTime(sdt);

    task.setDateTime(&data);
    EXPECT_EQ(QDate(0, 0, 0), task.m_BeginDateTime.date());
    EXPECT_EQ(QTime(0, 0, 0), task.m_BeginDateTime.time());
    EXPECT_EQ(QDate(0, 0, 0), task.m_EndDateTime.date());
    EXPECT_EQ(QTime(0, 0, 0), task.m_EndDateTime.time());
}

// ============================================================================
// queryOverDueDate — private, accessible via -fno-access-control
// ============================================================================

TEST(QueryScheduleTaskTest, QueryOverDueDate_EmptySuggestions_ReturnsFalse)
{
    queryScheduleTask task;
    QueryJsonData data;
    EXPECT_FALSE(task.queryOverDueDate(&data));
}

TEST(QueryScheduleTaskTest, QueryOverDueDate_Size1_PastDate_ReturnsTrue)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime(QDate(2020, 1, 1), QTime(10, 0, 0)), true)});
    data.setDateTime(sdt);

    EXPECT_TRUE(task.queryOverDueDate(&data));
}

TEST(QueryScheduleTaskTest, QueryOverDueDate_Size1_TodayPastTime_ReturnsTrue)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime(QDate::currentDate(), QTime(0, 0, 0)), true)});
    data.setDateTime(sdt);

    EXPECT_TRUE(task.queryOverDueDate(&data));
}

TEST(QueryScheduleTaskTest, QueryOverDueDate_Size1_FutureDate_ReturnsFalse)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime(
        {makeSuggestion(QDateTime::currentDateTime().addDays(7), true)});
    data.setDateTime(sdt);

    EXPECT_FALSE(task.queryOverDueDate(&data));
}

TEST(QueryScheduleTaskTest, QueryOverDueDate_Size2_PastEndDate_ReturnsTrue)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime({
        makeSuggestion(QDateTime::currentDateTime().addDays(1), true),
        makeSuggestion(QDateTime(QDate(2020, 1, 1), QTime(10, 0, 0)), true),
    });
    data.setDateTime(sdt);

    EXPECT_TRUE(task.queryOverDueDate(&data));
}

TEST(QueryScheduleTaskTest, QueryOverDueDate_Size2_FutureEndDate_ReturnsFalse)
{
    queryScheduleTask task;
    QueryJsonData data;

    SemanticsDateTime sdt = makeSemanticsDateTime({
        makeSuggestion(QDateTime::currentDateTime().addDays(1), true),
        makeSuggestion(QDateTime::currentDateTime().addDays(7), true),
    });
    data.setDateTime(sdt);

    EXPECT_FALSE(task.queryOverDueDate(&data));
}
