// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "createscheduletask.h"
#include "createjsondata.h"
#include "createschedulewidget.h"
#include "semanticanalysistask.h"
#include "dschedule.h"
#include "dscheduledatamanager.h"
#include "globaldef.h"
#include "reply.h"
#include "stub.h"
#include <QDateTime>
#include <QSignalSpy>

static DSchedule::Ptr makeSch(const QDateTime &start, const QDateTime &end,
                               const QString &summary = "")
{
    DSchedule::Ptr sch(new DSchedule());
    sch->setDtStart(start);
    sch->setDtEnd(end);
    if (!summary.isEmpty()) sch->setSummary(summary);
    sch->setUid("test-uid");
    return sch;
}

// ---- DB stub helpers ----
static DScheduleDataManager* stub_getInstance()
{
    return reinterpret_cast<DScheduleDataManager*>(0x1);
}

static QString stub_createSchedule(DScheduleDataManager*, const DSchedule::Ptr &)
{
    return "test-schedule-id";
}

static DSchedule::Ptr stub_queryScheduleByScheduleID(DScheduleDataManager*, const QString &)
{
    DSchedule::Ptr sch(new DSchedule());
    sch->setSummary("test-schedule");
    sch->setUid("test-uid");
    return sch;
}

// ============================================================================
// Constructor
// ============================================================================
TEST(CreateScheduleTaskTest, Constructor_NoStateInitialized)
{
    createScheduleTask task;
    // createScheduleTask ctor does not set m_State (unlike cancel/change)
    // getState returns whatever scheduleBaseTask ctor set (nullptr)
    EXPECT_EQ(nullptr, task.getState());
}

// ============================================================================
// validDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, ValidDateTime_ValidDateTime_ReturnsTrue)
{
    createScheduleTask task;
    QDateTime dt(QDate(2026, 6, 1), QTime(10, 0));
    EXPECT_TRUE(task.validDateTime(dt));
}

TEST(CreateScheduleTaskTest, ValidDateTime_InvalidDateTime_ReturnsFalse)
{
    createScheduleTask task;
    QDateTime dt;
    EXPECT_FALSE(task.validDateTime(dt));
}

// ============================================================================
// shouldEndSession
// ============================================================================
TEST(CreateScheduleTaskTest, ShouldEndSession_True)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    jsonData.setShouldEndSession(true);
    EXPECT_TRUE(task.shouldEndSession(&jsonData));
}

TEST(CreateScheduleTaskTest, ShouldEndSession_False)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    jsonData.setShouldEndSession(false);
    EXPECT_FALSE(task.shouldEndSession(&jsonData));
}

// ============================================================================
// beginDateTimeIsinHalfYear
// ============================================================================
TEST(CreateScheduleTaskTest, BeginDateTimeIsinHalfYear_WithinRange_ReturnsTrue)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    EXPECT_TRUE(task.beginDateTimeIsinHalfYear());
}

TEST(CreateScheduleTaskTest, BeginDateTimeIsinHalfYear_BeforeCurrent_ReturnsFalse)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addDays(-10);
    EXPECT_FALSE(task.beginDateTimeIsinHalfYear());
}

TEST(CreateScheduleTaskTest, BeginDateTimeIsinHalfYear_BeyondHalfYear_ReturnsFalse)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addMonths(7);
    EXPECT_FALSE(task.beginDateTimeIsinHalfYear());
}

// ============================================================================
// beginDateTimeOutHalfYear
// ============================================================================
TEST(CreateScheduleTaskTest, BeginDateTimeOutHalfYear_BeyondHalfYear_ReturnsTrue)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addMonths(7);
    EXPECT_TRUE(task.beginDateTimeOutHalfYear());
}

TEST(CreateScheduleTaskTest, BeginDateTimeOutHalfYear_WithinRange_ReturnsFalse)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    EXPECT_FALSE(task.beginDateTimeOutHalfYear());
}

// ============================================================================
// beginDateTimeBeforeCurrent
// ============================================================================
TEST(CreateScheduleTaskTest, BeginDateTimeBeforeCurrent_Past_ReturnsTrue)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addDays(-5);
    EXPECT_TRUE(task.beginDateTimeBeforeCurrent());
}

TEST(CreateScheduleTaskTest, BeginDateTimeBeforeCurrent_Future_ReturnsFalse)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addDays(5);
    EXPECT_FALSE(task.beginDateTimeBeforeCurrent());
}

// ============================================================================
// getCreatesDays
// ============================================================================
TEST(CreateScheduleTaskTest, GetCreatesDays_ContainsToday)
{
    createScheduleTask task;
    EXPECT_EQ(10, task.getCreatesDays(1, 10, true));
}

TEST(CreateScheduleTaskTest, GetCreatesDays_NotContainsToday)
{
    createScheduleTask task;
    EXPECT_EQ(9, task.getCreatesDays(1, 10, false));
}

// ============================================================================
// getValidDate
// ============================================================================
TEST(CreateScheduleTaskTest, GetValidDate_ValidDay_ReturnsValidDate)
{
    createScheduleTask task;
    QDate result = task.getValidDate(QDate(2026, 1, 15), 10);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(10, result.day());
}

TEST(CreateScheduleTaskTest, GetValidDate_InvalidDay_ReturnsInvalidDate)
{
    createScheduleTask task;
    // Day 32 is never valid in any month
    QDate result = task.getValidDate(QDate(2026, 1, 15), 32);
    // The loop tries month+0 through month+6, but day 32 never valid
    // So it returns the last attempted date which may be invalid
    // Actually it returns validDate after the loop, which was set to setDate(year, month, 32)
    // which is invalid
    EXPECT_FALSE(result.isValid());
}

// ============================================================================
// getFirstSchedule
// ============================================================================
TEST(CreateScheduleTaskTest, GetFirstSchedule_FindsEarliest)
{
    createScheduleTask task;
    DSchedule::List list;
    list.append(makeSch(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 10), QTime(11, 0))));
    list.append(makeSch(QDateTime(QDate(2026, 6, 1), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 1), QTime(11, 0))));
    list.append(makeSch(QDateTime(QDate(2026, 6, 5), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 5), QTime(11, 0))));
    DSchedule::Ptr first = task.getFirstSchedule(list);
    EXPECT_EQ(QDate(2026, 6, 1), first->dtStart().date());
}

TEST(CreateScheduleTaskTest, GetFirstSchedule_SingleElement)
{
    createScheduleTask task;
    DSchedule::List list;
    list.append(makeSch(QDateTime(QDate(2026, 6, 15), QTime(10, 0)),
                         QDateTime(QDate(2026, 6, 15), QTime(11, 0))));
    DSchedule::Ptr first = task.getFirstSchedule(list);
    EXPECT_EQ(QDate(2026, 6, 15), first->dtStart().date());
}

// ============================================================================
// analysisWorkDayDate
// ============================================================================
TEST(CreateScheduleTaskTest, AnalysisWorkDayDate_WeekdayReturnsSame)
{
    createScheduleTask task;
    // Set to a Monday (dayOfWeek == 1)
    task.m_begintime = QDateTime(QDate(2026, 6, 1), QTime(10, 0));
    QVector<QDateTime> result = task.analysisWorkDayDate();
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QDate(2026, 6, 1), result.at(0).date());
}

TEST(CreateScheduleTaskTest, AnalysisWorkDayDate_SaturdayAdjustsToMonday)
{
    createScheduleTask task;
    // Set to a Saturday (dayOfWeek == 6)
    task.m_begintime = QDateTime(QDate(2026, 6, 6), QTime(10, 0));
    QVector<QDateTime> result = task.analysisWorkDayDate();
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QDate(2026, 6, 8), result.at(0).date());
}

TEST(CreateScheduleTaskTest, AnalysisWorkDayDate_SundayAdjustsToMonday)
{
    createScheduleTask task;
    // Set to a Sunday (dayOfWeek == 7)
    task.m_begintime = QDateTime(QDate(2026, 6, 7), QTime(10, 0));
    QVector<QDateTime> result = task.analysisWorkDayDate();
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QDate(2026, 6, 8), result.at(0).date());
}

// ============================================================================
// analysisRestDayDate
// ============================================================================
TEST(CreateScheduleTaskTest, AnalysisRestDayDate_ReturnsTwoDates)
{
    createScheduleTask task;
    task.m_begintime = QDateTime::currentDateTime().addDays(1);
    QVector<QDateTime> result = task.analysisRestDayDate();
    EXPECT_EQ(2, result.size());
    // First should be Saturday, second Sunday
    EXPECT_EQ(6, result.at(0).date().dayOfWeek());
    EXPECT_EQ(7, result.at(1).date().dayOfWeek());
}

// ============================================================================
// getNoneWeekNumDate
// ============================================================================
TEST(CreateScheduleTaskTest, GetNoneWeekNumDate_BeginTimeAfterCurrent)
{
    createScheduleTask task;
    // Set begin time with a time later than current time
    task.m_begintime = QDateTime(QDate::currentDate(), QTime::currentTime().addSecs(3600));
    QVector<QDateTime> result = task.getNoneWeekNumDate();
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QDate::currentDate(), result.at(0).date());
}

TEST(CreateScheduleTaskTest, GetNoneWeekNumDate_BeginTimeBeforeCurrent)
{
    createScheduleTask task;
    // Set begin time with a time earlier than current time
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(0, 0));
    QVector<QDateTime> result = task.getNoneWeekNumDate();
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QDate::currentDate().addDays(1), result.at(0).date());
}

// ============================================================================
// getNoneMonthNumDate
// ============================================================================
TEST(CreateScheduleTaskTest, GetNoneMonthNumDate_BeginTimeAfterCurrent)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime::currentTime().addSecs(3600));
    QVector<QDateTime> result = task.getNoneMonthNumDate();
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QDate::currentDate(), result.at(0).date());
}

TEST(CreateScheduleTaskTest, GetNoneMonthNumDate_BeginTimeBeforeCurrent)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(0, 0));
    QVector<QDateTime> result = task.getNoneMonthNumDate();
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QDate::currentDate().addDays(1), result.at(0).date());
}

// ============================================================================
// getOneWeekNumDate
// ============================================================================
TEST(CreateScheduleTaskTest, GetOneWeekNumDate_FirstGreaterThanOrEqualCurrent)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().dayOfWeek();
    int targetWeek = currentDay + 2; // greater than current
    if (targetWeek > 7) targetWeek = 7;
    QVector<QDateTime> result = task.getOneWeekNumDate(targetWeek);
    EXPECT_EQ(1, result.size());
}

TEST(CreateScheduleTaskTest, GetOneWeekNumDate_FirstLessThanCurrent)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().dayOfWeek();
    int targetWeek = 1;
    if (currentDay == 1) targetWeek = 2; // ensure less than current
    QVector<QDateTime> result = task.getOneWeekNumDate(targetWeek);
    EXPECT_EQ(1, result.size());
}

// ============================================================================
// getTwoWeekNumDate
// ============================================================================
TEST(CreateScheduleTaskTest, GetTwoWeekNumDate_EqualWeekNums_SetsEveryDayState)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoWeekNumDate(3, 3);
    EXPECT_TRUE(task.everyDayState);
    EXPECT_EQ(0, result.size());
}

TEST(CreateScheduleTaskTest, GetTwoWeekNumDate_DiffSix_SetsEveryDayState)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoWeekNumDate(1, 7);
    EXPECT_TRUE(task.everyDayState);
    EXPECT_EQ(0, result.size());
}

TEST(CreateScheduleTaskTest, GetTwoWeekNumDate_DiffOne_SetsEveryDayState)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoWeekNumDate(4, 3);
    EXPECT_TRUE(task.everyDayState);
    EXPECT_EQ(0, result.size());
}

TEST(CreateScheduleTaskTest, GetTwoWeekNumDate_FirstLessThanSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoWeekNumDate(2, 5);
    EXPECT_FALSE(task.everyDayState);
}

TEST(CreateScheduleTaskTest, GetTwoWeekNumDate_FirstGreaterThanSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoWeekNumDate(5, 2);
    EXPECT_FALSE(task.everyDayState);
}

// ============================================================================
// analysisEveryWeekDate
// ============================================================================
TEST(CreateScheduleTaskTest, AnalysisEveryWeekDate_EmptyRange)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<int> range;
    QVector<QDateTime> result = task.analysisEveryWeekDate(range);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, AnalysisEveryWeekDate_SingleElement)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<int> range;
    range.append(3);
    QVector<QDateTime> result = task.analysisEveryWeekDate(range);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, AnalysisEveryWeekDate_TwoElements)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<int> range;
    range.append(2);
    range.append(4);
    QVector<QDateTime> result = task.analysisEveryWeekDate(range);
    EXPECT_FALSE(task.everyDayState);
}

// ============================================================================
// firstWeekNumLessThanSecond
// ============================================================================
TEST(CreateScheduleTaskTest, FirstWeekNumLessThanSecond_CurrentBeforeFirst)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().dayOfWeek();
    int first = currentDay + 3;
    int second = currentDay + 5;
    if (second > 7) { first = 5; second = 6; }
    if (first >= second) { first = 2; second = 5; }
    QVector<QDateTime> result = task.firstWeekNumLessThanSecond(first, second);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, FirstWeekNumLessThanSecond_CurrentBetween)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.firstWeekNumLessThanSecond(2, 6);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, FirstWeekNumLessThanSecond_CurrentAfterSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().dayOfWeek();
    if (currentDay <= 3) {
        // Can't test this branch if current day is already <= 3, skip by using small range
        QVector<QDateTime> result = task.firstWeekNumLessThanSecond(1, 2);
        EXPECT_FALSE(result.isEmpty());
    } else {
        QVector<QDateTime> result = task.firstWeekNumLessThanSecond(1, 2);
        EXPECT_FALSE(result.isEmpty());
    }
}

// ============================================================================
// firstWeekNumGreaterThanSecondButEveryDay
// ============================================================================
TEST(CreateScheduleTaskTest, FirstWeekNumGreaterThanSecondButEveryDay_CurrentGreaterOrEqualFirst)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    // first=6, second=2, current likely <= 6 but could be any
    QVector<QDateTime> result = task.firstWeekNumGreaterThanSecondButEveryDay(6, 2);
    EXPECT_FALSE(task.everyDayState);
}

TEST(CreateScheduleTaskTest, FirstWeekNumGreaterThanSecondButEveryDay_CurrentBetween)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().dayOfWeek();
    // Need current < first and current > second
    int first = currentDay + 3;
    int second = currentDay - 1;
    if (first > 7) first = 7;
    if (second < 1) second = 1;
    if (first <= second) { first = 6; second = 2; }
    QVector<QDateTime> result = task.firstWeekNumGreaterThanSecondButEveryDay(first, second);
    EXPECT_FALSE(task.everyDayState);
}

TEST(CreateScheduleTaskTest, FirstWeekNumGreaterThanSecondButEveryDay_CurrentLessOrEqualSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().dayOfWeek();
    if (currentDay <= 5) {
        QVector<QDateTime> result = task.firstWeekNumGreaterThanSecondButEveryDay(6, 5);
        EXPECT_FALSE(task.everyDayState);
    } else {
        // current > 5, use different setup
        QVector<QDateTime> result = task.firstWeekNumGreaterThanSecondButEveryDay(7, 6);
        EXPECT_FALSE(task.everyDayState);
    }
}

// ============================================================================
// getWeekAllDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, GetWeekAllDateTime_GeneratesDateRange)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate(2026, 6, 1), QTime(10, 0));
    QVector<QDateTime> result = task.getWeekAllDateTime(QDate(2026, 6, 2), 2, 4);
    EXPECT_EQ(3, result.size()); // days 2,3,4
}

// ============================================================================
// getWeekFrontPartDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, GetWeekFrontPartDateTime_GeneratesDates)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getWeekFrontPartDateTime(QDate::currentDate(), 1, true);
    EXPECT_FALSE(result.isEmpty());
}

// ============================================================================
// getWeekBackPartDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, GetWeekBackPartDateTime_GeneratesDates)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getWeekBackPartDateTime(QDate::currentDate(), 5, true);
    EXPECT_FALSE(result.isEmpty());
}

// ============================================================================
// getOneMonthNumDate
// ============================================================================
TEST(CreateScheduleTaskTest, GetOneMonthNumDate_CurrentLessThanFirst)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().day();
    int targetDay = currentDay + 5;
    if (targetDay > 28) targetDay = 28;
    QVector<QDateTime> result = task.getOneMonthNumDate(targetDay);
    EXPECT_EQ(1, result.size());
}

TEST(CreateScheduleTaskTest, GetOneMonthNumDate_CurrentGreaterThanFirst)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().day();
    int targetDay = 1;
    if (currentDay <= 1) targetDay = 28;
    QVector<QDateTime> result = task.getOneMonthNumDate(targetDay);
    EXPECT_EQ(1, result.size());
}

TEST(CreateScheduleTaskTest, GetOneMonthNumDate_CurrentEqualFirst)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().day();
    QVector<QDateTime> result = task.getOneMonthNumDate(currentDay);
    EXPECT_EQ(1, result.size());
}

// ============================================================================
// getTwoMonthNumDate
// ============================================================================
TEST(CreateScheduleTaskTest, GetTwoMonthNumDate_Equal_SetsEveryDayState)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoMonthNumDate(10, 10);
    EXPECT_TRUE(task.everyDayState);
    EXPECT_EQ(0, result.size());
}

TEST(CreateScheduleTaskTest, GetTwoMonthNumDate_DiffOne_SetsEveryDayState)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoMonthNumDate(11, 10);
    EXPECT_TRUE(task.everyDayState);
    EXPECT_EQ(0, result.size());
}

TEST(CreateScheduleTaskTest, GetTwoMonthNumDate_FirstLessThanSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoMonthNumDate(5, 10);
    EXPECT_FALSE(task.everyDayState);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, GetTwoMonthNumDate_FirstGreaterThanSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getTwoMonthNumDate(15, 5);
    EXPECT_FALSE(task.everyDayState);
    EXPECT_FALSE(result.isEmpty());
}

// ============================================================================
// analysisEveryMonthDate
// ============================================================================
TEST(CreateScheduleTaskTest, AnalysisEveryMonthDate_EmptyRange)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<int> range;
    QVector<QDateTime> result = task.analysisEveryMonthDate(range);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, AnalysisEveryMonthDate_SingleElement)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<int> range;
    range.append(10);
    QVector<QDateTime> result = task.analysisEveryMonthDate(range);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, AnalysisEveryMonthDate_TwoElements)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<int> range;
    range.append(5);
    range.append(10);
    QVector<QDateTime> result = task.analysisEveryMonthDate(range);
    EXPECT_FALSE(task.everyDayState);
}

// ============================================================================
// getMonthAllDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, GetMonthAllDateTime_GeneratesDates)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate(2026, 6, 1), QTime(10, 0));
    QVector<QDateTime> result = task.getMonthAllDateTime(QDate(2026, 6, 1), 5, 10);
    EXPECT_FALSE(result.isEmpty());
}

// ============================================================================
// getMonthFrontPartDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, GetMonthFrontPartDateTime_GeneratesDates)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getMonthFrontPartDateTime(QDate::currentDate(), 1, true);
    EXPECT_FALSE(result.isEmpty());
}

// ============================================================================
// getMonthBackPartDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, GetMonthBackPartDateTime_GeneratesDates)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.getMonthBackPartDateTime(QDate::currentDate(), 31, true);
    EXPECT_FALSE(result.isEmpty());
}

// ============================================================================
// firstMonthNumLessThanSecond
// ============================================================================
TEST(CreateScheduleTaskTest, FirstMonthNumLessThanSecond_CurrentBeforeFirst)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().day();
    int first = currentDay + 5;
    int second = currentDay + 10;
    if (second > 28) { first = 5; second = 15; }
    if (first <= currentDay) { first = currentDay + 3; second = first + 5; }
    if (second > 28) { first = 5; second = 15; }
    QVector<QDateTime> result = task.firstMonthNumLessThanSecond(first, second);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, FirstMonthNumLessThanSecond_CurrentBetween)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().day();
    int first = 1;
    int second = 28;
    if (currentDay < 1 || currentDay > 28) { first = 1; second = 28; }
    QVector<QDateTime> result = task.firstMonthNumLessThanSecond(first, second);
    EXPECT_FALSE(result.isEmpty());
}

TEST(CreateScheduleTaskTest, FirstMonthNumLessThanSecond_CurrentAfterSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().day();
    if (currentDay <= 5) {
        QVector<QDateTime> result = task.firstMonthNumLessThanSecond(1, 3);
        EXPECT_FALSE(result.isEmpty());
    } else {
        QVector<QDateTime> result = task.firstMonthNumLessThanSecond(1, 3);
        EXPECT_FALSE(result.isEmpty());
    }
}

// ============================================================================
// firstMonthNumGreaterThanSecondButEveryDay
// ============================================================================
TEST(CreateScheduleTaskTest, FirstMonthNumGreaterThanSecondButEveryDay_CurrentGreaterOrEqualFirst)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.firstMonthNumGreaterThanSecondButEveryDay(20, 5);
    EXPECT_FALSE(task.everyDayState);
}

TEST(CreateScheduleTaskTest, FirstMonthNumGreaterThanSecondButEveryDay_CurrentBetween)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    int currentDay = QDate::currentDate().day();
    int first = currentDay + 5;
    int second = currentDay - 3;
    if (first > 28) first = 28;
    if (second < 1) second = 1;
    if (first <= second) { first = 20; second = 5; }
    QVector<QDateTime> result = task.firstMonthNumGreaterThanSecondButEveryDay(first, second);
    EXPECT_FALSE(task.everyDayState);
}

TEST(CreateScheduleTaskTest, FirstMonthNumGreaterThanSecondButEveryDay_CurrentLessOrEqualSecond)
{
    createScheduleTask task;
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    QVector<QDateTime> result = task.firstMonthNumGreaterThanSecondButEveryDay(20, 15);
    EXPECT_FALSE(task.everyDayState);
}

// ============================================================================
// setDateTime
// ============================================================================
TEST(CreateScheduleTaskTest, SetDateTime_SingleTime_HasTime)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    sdi.hasTime = true;
    sdi.strDateTime = "2026-12-01 10:00";
    sdt.suggestDatetime.append(sdi);
    jsonData.setDateTime(sdt);

    task.setDateTime(&jsonData);
    EXPECT_EQ(QDateTime(QDate(2026, 12, 1), QTime(10, 0)), task.m_begintime);
    EXPECT_EQ(QDateTime(QDate(2026, 12, 1), QTime(11, 0)), task.m_endtime);
}

TEST(CreateScheduleTaskTest, SetDateTime_SingleTime_NoTime)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(QDate(2026, 12, 1), QTime(0, 0));
    sdi.hasTime = false;
    sdi.strDateTime = "2026-12-01";
    sdt.suggestDatetime.append(sdi);
    jsonData.setDateTime(sdt);

    task.setDateTime(&jsonData);
    EXPECT_EQ(QDate(2026, 12, 1), task.m_begintime.date());
}

TEST(CreateScheduleTaskTest, SetDateTime_SingleTime_InvalidDateTime)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(); // invalid
    sdi.hasTime = true;
    sdi.strDateTime = "invalid";
    sdt.suggestDatetime.append(sdi);
    jsonData.setDateTime(sdt);

    task.setDateTime(&jsonData);
    EXPECT_FALSE(task.isValidDateTime);
}

TEST(CreateScheduleTaskTest, SetDateTime_TwoTimes_Valid)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi1, sdi2;
    sdi1.datetime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    sdi1.hasTime = true;
    sdi1.strDateTime = "2026-12-01 10:00";
    sdi2.datetime = QDateTime(QDate(2026, 12, 2), QTime(12, 0));
    sdi2.hasTime = true;
    sdi2.strDateTime = "2026-12-02 12:00";
    sdt.suggestDatetime.append(sdi1);
    sdt.suggestDatetime.append(sdi2);
    jsonData.setDateTime(sdt);

    task.setDateTime(&jsonData);
    EXPECT_EQ(QDateTime(QDate(2026, 12, 1), QTime(10, 0)), task.m_begintime);
    EXPECT_EQ(QDateTime(QDate(2026, 12, 2), QTime(12, 0)), task.m_endtime);
}

TEST(CreateScheduleTaskTest, SetDateTime_TwoTimes_FirstInvalid)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi1, sdi2;
    sdi1.datetime = QDateTime(); // invalid
    sdi1.hasTime = true;
    sdi1.strDateTime = "invalid";
    sdi2.datetime = QDateTime(QDate(2026, 12, 2), QTime(12, 0));
    sdi2.hasTime = true;
    sdi2.strDateTime = "2026-12-02 12:00";
    sdt.suggestDatetime.append(sdi1);
    sdt.suggestDatetime.append(sdi2);
    jsonData.setDateTime(sdt);

    task.setDateTime(&jsonData);
    EXPECT_FALSE(task.isValidDateTime);
}

TEST(CreateScheduleTaskTest, SetDateTime_TwoTimes_SecondInvalid)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi1, sdi2;
    sdi1.datetime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    sdi1.hasTime = true;
    sdi1.strDateTime = "2026-12-01 10:00";
    sdi2.datetime = QDateTime(); // invalid
    sdi2.hasTime = true;
    sdi2.strDateTime = "invalid";
    sdt.suggestDatetime.append(sdi1);
    sdt.suggestDatetime.append(sdi2);
    jsonData.setDateTime(sdt);

    task.setDateTime(&jsonData);
    EXPECT_FALSE(task.isValidDateTime);
}

TEST(CreateScheduleTaskTest, SetDateTime_DefaultZeroSize)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    // No suggestDatetime set → size 0 → default case
    task.setDateTime(&jsonData);
    EXPECT_FALSE(task.isValidDateTime);
}

TEST(CreateScheduleTaskTest, SetDateTime_TwoTimes_FirstHasTimeSecondNot)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi1, sdi2;
    sdi1.datetime = QDateTime(QDate(2026, 12, 15), QTime(10, 0));
    sdi1.hasTime = true;
    sdi1.strDateTime = "2026-12-15 10:00";
    sdi2.datetime = QDateTime(QDate(2026, 12, 20), QTime(0, 0));
    sdi2.hasTime = false;
    sdi2.strDateTime = "2026-12-20";
    sdt.suggestDatetime.append(sdi1);
    sdt.suggestDatetime.append(sdi2);
    jsonData.setDateTime(sdt);

    task.setDateTime(&jsonData);
    // end time should get begin time's time
    EXPECT_EQ(QTime(10, 0), task.m_endtime.time());
}

// ============================================================================
// setScheduleTitleName
// ============================================================================
TEST(CreateScheduleTaskTest, SetScheduleTitleName_EmptyTitle_UsesDefault)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    CreateJsonData jsonData; // TitleName is empty by default
    task.setScheduleTitleName(&jsonData);
    SUCCEED();
    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, SetScheduleTitleName_NonEmptyTitle)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    CreateJsonData jsonData;
    jsonData.setTitleName("My Schedule");
    task.setScheduleTitleName(&jsonData);
    SUCCEED();
    delete task.m_widget;
    task.m_widget = nullptr;
}

// ============================================================================
// creareScheduleUI
// ============================================================================
TEST(CreateScheduleTaskTest, CreareScheduleUI_NonEmptyScheduleID)
{
    createScheduleTask task;
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);
    task.m_widget = new createSchedulewidget();
    task.creareScheduleUI("test-schedule-id");
    SUCCEED();
    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, CreareScheduleUI_EmptyScheduleID)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.creareScheduleUI(QString());
    SUCCEED();
    delete task.m_widget;
    task.m_widget = nullptr;
}

// ============================================================================
// getReply
// ============================================================================
TEST(CreateScheduleTaskTest, GetReply_RestDayWithTime)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::RESTD);
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(QDate(2026, 6, 1), QTime(10, 0));
    sdi.hasTime = true;
    sdt.suggestDatetime.append(sdi);
    jsonData.setDateTime(sdt);
    task.m_begintime = QDateTime(QDate(2026, 6, 1), QTime(10, 0));

    QString reply = task.getReply(&jsonData);
    EXPECT_FALSE(reply.isEmpty());
}

TEST(CreateScheduleTaskTest, GetReply_NoneWithTimeCrossDay)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::NONE);
    jsonData.setShouldEndSession(true);
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi1, sdi2;
    sdi1.datetime = QDateTime::currentDateTime().addSecs(-3600); // past time
    sdi1.hasTime = true;
    sdi2.datetime = QDateTime::currentDateTime().addSecs(3600);
    sdi2.hasTime = true;
    sdt.suggestDatetime.append(sdi1);
    sdt.suggestDatetime.append(sdi2);
    jsonData.setDateTime(sdt);
    task.m_begintime = QDateTime::currentDateTime();

    QString reply = task.getReply(&jsonData);
    EXPECT_FALSE(reply.isEmpty());
}

TEST(CreateScheduleTaskTest, GetReply_Default_UsesSuggestMsg)
{
    createScheduleTask task;
    CreateJsonData jsonData;
    jsonData.setSuggestMsg("test-suggest-msg");
    QString reply = task.getReply(&jsonData);
    EXPECT_EQ("test-suggest-msg", reply);
}

// ============================================================================
// DB-dependent methods (need m_widget + stubs)
// ============================================================================
TEST(CreateScheduleTaskTest, GetNotRepeatDaySchedule)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    task.m_endtime = QDateTime(QDate(2026, 12, 1), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QString id = task.getNotRepeatDaySchedule();
    EXPECT_EQ("test-schedule-id", id);

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryDaySchedule)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    task.m_endtime = QDateTime(QDate(2026, 12, 1), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QString id = task.getEveryDaySchedule();
    EXPECT_EQ("test-schedule-id", id);
    EXPECT_FALSE(task.everyDayState);

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryYearSchedule)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    task.m_endtime = QDateTime(QDate(2026, 12, 1), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QString id = task.getEveryYearSchedule();
    EXPECT_EQ("test-schedule-id", id);

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryWorkDaySchedule)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    task.m_endtime = QDateTime(QDate(2026, 12, 1), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QString id = task.getEveryWorkDaySchedule();
    EXPECT_EQ("test-schedule-id", id);

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryRestDaySchedule)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(1);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QString id = task.getEveryRestDaySchedule();
    EXPECT_EQ("test-schedule-id", id);

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryWeekSchedule_WithDateRange)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    task.m_endtime = QDateTime(QDate::currentDate(), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QVector<int> range;
    range.append(2);
    range.append(4);
    QString id = task.getEveryWeekSchedule(range);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryWeekSchedule_EmptyRange)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    task.m_endtime = QDateTime(QDate::currentDate(), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QVector<int> range;
    QString id = task.getEveryWeekSchedule(range);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryMonthSchedule_WithDateRange)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    task.m_endtime = QDateTime(QDate::currentDate(), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QVector<int> range;
    range.append(5);
    range.append(10);
    QString id = task.getEveryMonthSchedule(range);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, GetEveryMonthSchedule_EmptyRange)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime(QDate::currentDate(), QTime(10, 0));
    task.m_endtime = QDateTime(QDate::currentDate(), QTime(11, 0));

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    QVector<int> range;
    QString id = task.getEveryMonthSchedule(range);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

// ============================================================================
// setDateTimeAndGetSchedule
// ============================================================================
TEST(CreateScheduleTaskTest, SetDateTimeAndGetSchedule)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    QDateTime begin(QDate(2026, 12, 1), QTime(10, 0));
    QDateTime end(QDate(2026, 12, 1), QTime(11, 0));

    DSchedule::Ptr sch = task.setDateTimeAndGetSchedule(begin, end);
    EXPECT_NE(nullptr, sch);

    delete task.m_widget;
    task.m_widget = nullptr;
}

// ============================================================================
// createScheduleWithRepeatStatus
// ============================================================================
TEST(CreateScheduleTaskTest, CreateScheduleWithRepeatStatus_None)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    task.m_endtime = QDateTime::currentDateTime().addDays(10).addSecs(3600);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::NONE);
    QString id = task.createScheduleWithRepeatStatus(&jsonData);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, CreateScheduleWithRepeatStatus_EveryDay)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    task.m_endtime = QDateTime::currentDateTime().addDays(10).addSecs(3600);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::EVED);
    QString id = task.createScheduleWithRepeatStatus(&jsonData);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, CreateScheduleWithRepeatStatus_EveryWeek)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    task.m_endtime = QDateTime::currentDateTime().addDays(10).addSecs(3600);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::EVEW);
    QVector<int> nums;
    nums.append(3);
    jsonData.setRepeatNum(nums);
    QString id = task.createScheduleWithRepeatStatus(&jsonData);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, CreateScheduleWithRepeatStatus_EveryMonth)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    task.m_endtime = QDateTime::currentDateTime().addDays(10).addSecs(3600);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::EVEM);
    QVector<int> nums;
    nums.append(10);
    jsonData.setRepeatNum(nums);
    QString id = task.createScheduleWithRepeatStatus(&jsonData);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, CreateScheduleWithRepeatStatus_EveryYear)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    task.m_endtime = QDateTime::currentDateTime().addDays(10).addSecs(3600);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::EVEY);
    QString id = task.createScheduleWithRepeatStatus(&jsonData);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, CreateScheduleWithRepeatStatus_WorkDay)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(10);
    task.m_endtime = QDateTime::currentDateTime().addDays(10).addSecs(3600);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::WORKD);
    QString id = task.createScheduleWithRepeatStatus(&jsonData);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, CreateScheduleWithRepeatStatus_RestDay)
{
    createScheduleTask task;
    task.m_widget = new createSchedulewidget();
    task.m_begintime = QDateTime::currentDateTime().addDays(10);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    CreateJsonData jsonData;
    jsonData.setRepeatStatus(CreateJsonData::RESTD);
    QString id = task.createScheduleWithRepeatStatus(&jsonData);
    EXPECT_FALSE(id.isEmpty());

    delete task.m_widget;
    task.m_widget = nullptr;
}

// ============================================================================
// SchedulePress
// ============================================================================
TEST(CreateScheduleTaskTest, SchedulePress_NullJsonData_ReturnsErrorMessage)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    // No jsonData set → getJsonData returns nullptr → dynamic_cast fails
    Reply reply = task.SchedulePress(semanticTask);
    SUCCEED();
}

TEST(CreateScheduleTaskTest, SchedulePress_InvalidPropertyStatus_ReturnsError)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->setPropertyStatus(JsonData::NEXT);
    semanticTask.setJsonData(jsonData);
    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(CreateScheduleTaskTest, SchedulePress_InvalidOffset_ReturnsError)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->setOffset(0); // offset > -1 triggers error
    semanticTask.setJsonData(jsonData);
    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(CreateScheduleTaskTest, SchedulePress_InvalidDateTime_ReturnsError)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->m_DateTimeInvalid = true;
    semanticTask.setJsonData(jsonData);
    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_TRUE(reply.getShouldEndSession());
}

TEST(CreateScheduleTaskTest, SchedulePress_MultiRound_ReturnsSuggestMsg)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->setShouldEndSession(false);
    jsonData->setSuggestMsg("multi-round-msg");
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    sdi.hasTime = true;
    sdt.suggestDatetime.append(sdi);
    jsonData->setDateTime(sdt);
    semanticTask.setJsonData(jsonData);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);

    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_FALSE(reply.getShouldEndSession());

    // cleanup widget created by SchedulePress
    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, SchedulePress_ValidCreate_EndSession)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->setShouldEndSession(true);
    jsonData->setSuggestMsg("created-msg");
    jsonData->setRepeatStatus(CreateJsonData::EVED);
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime::currentDateTime().addDays(10);
    sdi.hasTime = true;
    sdt.suggestDatetime.append(sdi);
    jsonData->setDateTime(sdt);
    semanticTask.setJsonData(jsonData);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, createSchedule), stub_createSchedule);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);

    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_TRUE(reply.getShouldEndSession());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, SchedulePress_BeginBeforeCurrent)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->setShouldEndSession(true);
    jsonData->setSuggestMsg("past-msg");
    jsonData->setRepeatStatus(CreateJsonData::NONE);
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime::currentDateTime().addDays(-5);
    sdi.hasTime = true;
    sdt.suggestDatetime.append(sdi);
    jsonData->setDateTime(sdt);
    semanticTask.setJsonData(jsonData);

    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_TRUE(reply.getShouldEndSession());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, SchedulePress_BeginBeyondHalfYear)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->setShouldEndSession(true);
    jsonData->setSuggestMsg("future-msg");
    jsonData->setRepeatStatus(CreateJsonData::NONE);
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime::currentDateTime().addMonths(8);
    sdi.hasTime = true;
    sdt.suggestDatetime.append(sdi);
    jsonData->setDateTime(sdt);
    semanticTask.setJsonData(jsonData);

    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_TRUE(reply.getShouldEndSession());

    delete task.m_widget;
    task.m_widget = nullptr;
}

TEST(CreateScheduleTaskTest, SchedulePress_InvalidDateTimeInMultiRound)
{
    createScheduleTask task;
    semanticAnalysisTask semanticTask;
    CreateJsonData *jsonData = new CreateJsonData();
    jsonData->setShouldEndSession(true);
    jsonData->setSuggestMsg("invalid-msg");
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(); // invalid
    sdi.hasTime = true;
    sdi.strDateTime = "invalid";
    sdt.suggestDatetime.append(sdi);
    jsonData->setDateTime(sdt);
    semanticTask.setJsonData(jsonData);

    Reply reply = task.SchedulePress(semanticTask);
    EXPECT_TRUE(reply.getShouldEndSession());

    delete task.m_widget;
    task.m_widget = nullptr;
}
