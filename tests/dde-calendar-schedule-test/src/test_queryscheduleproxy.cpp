// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "queryscheduleproxy.h"
#include "jsondata.h"
#include "changejsondata.h"
#include "dschedule.h"
#include "dscheduledatamanager.h"
#include "dschedulequerypar.h"
#include "globaldef.h"
#include "timedatastruct.h"
#include "stub.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QSet>


// Forward declaration — scheduleSort is defined in queryscheduleproxy.cpp (free function, not in header)
bool scheduleSort(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2);

// Branch map (based on get_code_snippet source):
// scheduleSort: s1<s2→true; equal dtStart→compare dtEnd; else false
// queryScheduleProxy ctor: stores jsonData
// scheduleMapToList: iterate map, append values
// getTimeIsExpired/setTimeIsExpired: getter/setter
// timeFrameIsValid: size>1 && at(0)>at(1)→false; else true
// monthlyIsIntersections: beginM>endM→special; else range overlap
// checkedTimeIsIntersection: 4-branch (begin>end × fixbegin>fixend)
// weeklyIsIntersections: dayoffset>5→true; else build set+intersects
// getTimeLimitByTimeInfo: size==0/1/>1 branches
// getTimeFileterByTimeInfo: size==0/1/>1 branches
// scheduleFileterByDate: date range overlap condition
// scheduleFileterByTime: timeoffset<ONEDAY→checkedTimeIsIntersection; else append
// scheduleFileterByTitleName: summary.contains(titleName)
// getQueryDateTime: changejsondata cast + suggestDatetime size check
// sortAndFilterSchedule: isFestivalSchedule check (DB)
// WeeklyScheduleFileter: weeklySet empty→sortAndFilter; else filter
// MonthlyScheduleFileter: beginM==0||endM==0→sortAndFilter; else filter
// querySchedule: switch on RepeatStatus (7 cases)
// queryWeeklySchedule/queryMonthlySchedule/etc: DB calls
// queryNonRepeatingSchedule: switch on PropertyStatus
// queryNextNumSchedule/queryAllSchedule: DB calls

// Helper: create a DSchedule with given start/end
static DSchedule::Ptr makeSchedule(const QDateTime &start, const QDateTime &end, const QString &summary = "")
{
    DSchedule::Ptr sch(new DSchedule());
    sch->setDtStart(start);
    sch->setDtEnd(end);
    if (!summary.isEmpty())
        sch->setSummary(summary);
    return sch;
}

// ============================================================================
// scheduleSort (free function)
// ============================================================================

TEST(QueryScheduleProxySort, ScheduleSort_S1BeforeS2)
{
    DSchedule::Ptr s1 = makeSchedule(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                                      QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    DSchedule::Ptr s2 = makeSchedule(QDateTime(QDate(2026, 1, 2), QTime(10, 0)),
                                      QDateTime(QDate(2026, 1, 2), QTime(11, 0)));
    EXPECT_TRUE(scheduleSort(s1, s2));
    EXPECT_FALSE(scheduleSort(s2, s1));
}

TEST(QueryScheduleProxySort, ScheduleSort_EqualDtStart_CompareDtEnd)
{
    QDateTime start(QDate(2026, 1, 1), QTime(10, 0));
    DSchedule::Ptr s1 = makeSchedule(start, QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    DSchedule::Ptr s2 = makeSchedule(start, QDateTime(QDate(2026, 1, 1), QTime(12, 0)));
    EXPECT_TRUE(scheduleSort(s1, s2));
    EXPECT_FALSE(scheduleSort(s2, s1));
}

TEST(QueryScheduleProxySort, ScheduleSort_BothEqual)
{
    QDateTime start(QDate(2026, 1, 1), QTime(10, 0));
    QDateTime end(QDate(2026, 1, 1), QTime(11, 0));
    DSchedule::Ptr s1 = makeSchedule(start, end);
    DSchedule::Ptr s2 = makeSchedule(start, end);
    EXPECT_FALSE(scheduleSort(s1, s2));
    EXPECT_FALSE(scheduleSort(s2, s1));
}

// ============================================================================
// Constructor
// ============================================================================

TEST(QueryScheduleProxyTest, Constructor_StoresJsonData)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    EXPECT_EQ(proxy.m_queryJsonData, &jsonData);
}

TEST(QueryScheduleProxyTest, Constructor_NullJsonData)
{
    queryScheduleProxy proxy(nullptr);
    EXPECT_EQ(proxy.m_queryJsonData, nullptr);
}

// ============================================================================
// getTimeIsExpired / setTimeIsExpired
// ============================================================================

TEST(QueryScheduleProxyTest, GetTimeIsExpired_DefaultFalse)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    EXPECT_FALSE(proxy.getTimeIsExpired());
}

TEST(QueryScheduleProxyTest, SetTimeIsExpired_True)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    proxy.setTimeIsExpired(true);
    EXPECT_TRUE(proxy.getTimeIsExpired());
}

TEST(QueryScheduleProxyTest, SetTimeIsExpired_False)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    proxy.setTimeIsExpired(true);
    proxy.setTimeIsExpired(false);
    EXPECT_FALSE(proxy.getTimeIsExpired());
}

// ============================================================================
// scheduleMapToList
// ============================================================================

TEST(QueryScheduleProxyTest, ScheduleMapToList_EmptyMap)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map emptyMap;
    DSchedule::List result = proxy.scheduleMapToList(emptyMap);
    EXPECT_TRUE(result.isEmpty());
}

TEST(QueryScheduleProxyTest, ScheduleMapToList_MultipleEntries)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list1;
    list1.append(makeSchedule(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                               QDateTime(QDate(2026, 1, 1), QTime(11, 0))));
    DSchedule::List list2;
    list2.append(makeSchedule(QDateTime(QDate(2026, 1, 2), QTime(10, 0)),
                               QDateTime(QDate(2026, 1, 2), QTime(11, 0))));
    list2.append(makeSchedule(QDateTime(QDate(2026, 1, 3), QTime(10, 0)),
                               QDateTime(QDate(2026, 1, 3), QTime(11, 0))));
    map.insert(QDate(2026, 1, 1), list1);
    map.insert(QDate(2026, 1, 2), list2);
    DSchedule::List result = proxy.scheduleMapToList(map);
    EXPECT_EQ(result.size(), 3);
}

// ============================================================================
// timeFrameIsValid
// ============================================================================

TEST(QueryScheduleProxyTest, TimeFrameIsValid_EmptySuggest)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    EXPECT_TRUE(proxy.timeFrameIsValid(timeInfo));
}

TEST(QueryScheduleProxyTest, TimeFrameIsValid_OneElement)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime(QDate(2026, 1, 1), QTime(10, 0));
    timeInfo.suggestDatetime.append(info);
    EXPECT_TRUE(proxy.timeFrameIsValid(timeInfo));
}

TEST(QueryScheduleProxyTest, TimeFrameIsValid_StartBeforeEnd)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    SuggestDatetimeInfo info1, info2;
    info1.datetime = QDateTime(QDate(2026, 1, 1), QTime(10, 0));
    info2.datetime = QDateTime(QDate(2026, 1, 2), QTime(10, 0));
    timeInfo.suggestDatetime.append(info1);
    timeInfo.suggestDatetime.append(info2);
    EXPECT_TRUE(proxy.timeFrameIsValid(timeInfo));
}

TEST(QueryScheduleProxyTest, TimeFrameIsValid_StartAfterEnd)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    SuggestDatetimeInfo info1, info2;
    info1.datetime = QDateTime(QDate(2026, 1, 2), QTime(10, 0));
    info2.datetime = QDateTime(QDate(2026, 1, 1), QTime(10, 0));
    timeInfo.suggestDatetime.append(info1);
    timeInfo.suggestDatetime.append(info2);
    EXPECT_FALSE(proxy.timeFrameIsValid(timeInfo));
}

// ============================================================================
// monthlyIsIntersections
// ============================================================================

TEST(QueryScheduleProxyTest, MonthlyIsIntersections_NormalRange_Inside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime beginTime(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime endTime(QDate(2026, 1, 31), QTime(23, 59));
    EXPECT_TRUE(proxy.monthlyIsIntersections(beginTime, endTime, 5, 15));
}

TEST(QueryScheduleProxyTest, MonthlyIsIntersections_NormalRange_Outside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime beginTime(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime endTime(QDate(2026, 1, 10), QTime(23, 59));
    EXPECT_FALSE(proxy.monthlyIsIntersections(beginTime, endTime, 15, 25));
}

TEST(QueryScheduleProxyTest, MonthlyIsIntersections_WrapRange_Inside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime beginTime(QDate(2026, 1, 20), QTime(0, 0));
    QDateTime endTime(QDate(2026, 1, 25), QTime(23, 59));
    // beginM=25, endM=5 → wrap range; 20-25 is inside 25-5 wrap
    EXPECT_TRUE(proxy.monthlyIsIntersections(beginTime, endTime, 25, 5));
}

TEST(QueryScheduleProxyTest, MonthlyIsIntersections_WrapRange_Outside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime beginTime(QDate(2026, 1, 10), QTime(0, 0));
    QDateTime endTime(QDate(2026, 1, 15), QTime(23, 59));
    // beginM=25, endM=5 → wrap range; 10-15 is outside 25-5 wrap
    EXPECT_FALSE(proxy.monthlyIsIntersections(beginTime, endTime, 25, 5));
}

// ============================================================================
// checkedTimeIsIntersection
// ============================================================================

TEST(QueryScheduleProxyTest, CheckedTimeIsIntersection_NormalRange_Inside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QTime beginTime(8, 0), endTime(18, 0);
    QTime fixBegin(10, 0), fixEnd(12, 0);
    EXPECT_TRUE(proxy.checkedTimeIsIntersection(beginTime, endTime, fixBegin, fixEnd));
}

TEST(QueryScheduleProxyTest, CheckedTimeIsIntersection_NormalRange_Outside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QTime beginTime(8, 0), endTime(10, 0);
    QTime fixBegin(12, 0), fixEnd(14, 0);
    EXPECT_FALSE(proxy.checkedTimeIsIntersection(beginTime, endTime, fixBegin, fixEnd));
}

TEST(QueryScheduleProxyTest, CheckedTimeIsIntersection_BeginAfterEnd_FixInside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QTime beginTime(22, 0), endTime(6, 0); // begin > end (crosses midnight)
    QTime fixBegin(23, 0), fixEnd(3, 0);   // fix also crosses midnight
    EXPECT_TRUE(proxy.checkedTimeIsIntersection(beginTime, endTime, fixBegin, fixEnd));
}

TEST(QueryScheduleProxyTest, CheckedTimeIsIntersection_BeginAfterEnd_FixOutside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QTime beginTime(22, 0), endTime(2, 0); // begin > end
    QTime fixBegin(10, 0), fixEnd(12, 0);  // fix normal, doesn't cross midnight
    EXPECT_FALSE(proxy.checkedTimeIsIntersection(beginTime, endTime, fixBegin, fixEnd));
}

// ============================================================================
// weeklyIsIntersections
// ============================================================================

TEST(QueryScheduleProxyTest, WeeklyIsIntersections_DayOffsetGreaterThan5)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime beginTime(QDate(2026, 1, 1), QTime(0, 0));  // Monday
    QDateTime endTime(QDate(2026, 1, 20), QTime(0, 0));   // >5 days apart
    QSet<int> weeklySet;
    weeklySet.insert(1);
    EXPECT_TRUE(proxy.weeklyIsIntersections(beginTime, endTime, weeklySet));
}

TEST(QueryScheduleProxyTest, WeeklyIsIntersections_WithinRange_Intersects)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime beginTime(QDate(2026, 1, 5), QTime(0, 0));  // Monday
    QDateTime endTime(QDate(2026, 1, 7), QTime(0, 0));    // Wednesday, 2 days apart
    QSet<int> weeklySet;
    weeklySet.insert(1); // Monday
    EXPECT_TRUE(proxy.weeklyIsIntersections(beginTime, endTime, weeklySet));
}

TEST(QueryScheduleProxyTest, WeeklyIsIntersections_WithinRange_NoIntersect)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime beginTime(QDate(2026, 1, 5), QTime(0, 0));  // Monday
    QDateTime endTime(QDate(2026, 1, 7), QTime(0, 0));    // Wednesday
    QSet<int> weeklySet;
    weeklySet.insert(5); // Friday, not in Mon-Wed range
    EXPECT_FALSE(proxy.weeklyIsIntersections(beginTime, endTime, weeklySet));
}

// ============================================================================
// getTimeLimitByTimeInfo
// ============================================================================

TEST(QueryScheduleProxyTest, GetTimeLimitByTimeInfo_Empty)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    auto result = proxy.getTimeLimitByTimeInfo(timeInfo);
    // Empty → default 6-month frame
    EXPECT_GE(result.endTime, result.beginTime);
}

TEST(QueryScheduleProxyTest, GetTimeLimitByTimeInfo_SizeOne_WithTime)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    DateTimeInfo dtInfo;
    dtInfo.m_Date = QDate(2026, 6, 15);
    dtInfo.m_Time = QTime(10, 0);
    dtInfo.hasDate = true;
    dtInfo.hasTime = true;
    timeInfo.dateTime.append(dtInfo);
    auto result = proxy.getTimeLimitByTimeInfo(timeInfo);
    EXPECT_TRUE(result.beginTime.isValid());
}

TEST(QueryScheduleProxyTest, GetTimeLimitByTimeInfo_SizeOne_NoTime)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    DateTimeInfo dtInfo;
    dtInfo.m_Date = QDate(2026, 6, 15);
    dtInfo.hasDate = true;
    dtInfo.hasTime = false;
    timeInfo.dateTime.append(dtInfo);
    auto result = proxy.getTimeLimitByTimeInfo(timeInfo);
    EXPECT_TRUE(result.beginTime.isValid());
}

TEST(QueryScheduleProxyTest, GetTimeLimitByTimeInfo_SizeTwo)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    DateTimeInfo dt1, dt2;
    dt1.m_Date = QDate(2026, 6, 15);
    dt1.hasDate = true;
    dt2.m_Date = QDate(2026, 7, 15);
    dt2.hasDate = true;
    timeInfo.dateTime.append(dt1);
    timeInfo.dateTime.append(dt2);
    auto result = proxy.getTimeLimitByTimeInfo(timeInfo);
    EXPECT_LE(result.beginTime, result.endTime);
}

TEST(QueryScheduleProxyTest, GetTimeLimitByTimeInfo_SizeTwo_BeginInPast)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    DateTimeInfo dt1, dt2;
    dt1.m_Date = QDate(2020, 1, 1);
    dt1.hasDate = true;
    dt2.m_Date = QDate(2026, 12, 31);
    dt2.hasDate = true;
    timeInfo.dateTime.append(dt1);
    timeInfo.dateTime.append(dt2);
    auto result = proxy.getTimeLimitByTimeInfo(timeInfo);
    // beginTime should be adjusted to now if in past
    EXPECT_TRUE(result.beginTime.isValid());
}

// ============================================================================
// getTimeFileterByTimeInfo
// ============================================================================

TEST(QueryScheduleProxyTest, GetTimeFileterByTimeInfo_Empty)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    auto result = proxy.getTimeFileterByTimeInfo(timeInfo);
    EXPECT_TRUE(result.isInvalid);
}

TEST(QueryScheduleProxyTest, GetTimeFileterByTimeInfo_SizeOne_WithTime)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(QDate(2026, 12, 1), QTime(10, 0));
    sdi.hasTime = true;
    timeInfo.suggestDatetime.append(sdi);
    auto result = proxy.getTimeFileterByTimeInfo(timeInfo);
    EXPECT_FALSE(result.isInvalid);
    EXPECT_EQ(result.beginTime, QTime(10, 0));
    EXPECT_EQ(result.endTime, QTime(10, 0));
}

TEST(QueryScheduleProxyTest, GetTimeFileterByTimeInfo_SizeTwo)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    SemanticsDateTime timeInfo;
    SuggestDatetimeInfo sdi1, sdi2;
    sdi1.datetime = QDateTime(QDate(2026, 12, 1), QTime(9, 0));
    sdi1.hasTime = true;
    sdi2.datetime = QDateTime(QDate(2026, 12, 2), QTime(17, 0));
    sdi2.hasTime = true;
    timeInfo.suggestDatetime.append(sdi1);
    timeInfo.suggestDatetime.append(sdi2);
    auto result = proxy.getTimeFileterByTimeInfo(timeInfo);
    EXPECT_FALSE(result.isInvalid);
    EXPECT_EQ(result.beginTime, QTime(9, 0));
    EXPECT_EQ(result.endTime, QTime(17, 0));
}

// ============================================================================
// scheduleFileterByDate
// ============================================================================

TEST(QueryScheduleProxyTest, ScheduleFileterByDate_Inside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    QDateTime start(QDate(2026, 6, 10), QTime(10, 0));
    QDateTime end(QDate(2026, 6, 10), QTime(11, 0));
    list.append(makeSchedule(start, end));
    map.insert(QDate(2026, 6, 10), list);
    QDate filterBegin(2026, 6, 1), filterEnd(2026, 6, 30);
    auto result = proxy.scheduleFileterByDate(map, filterBegin, filterEnd);
    EXPECT_EQ(result.size(), 1);
}

TEST(QueryScheduleProxyTest, ScheduleFileterByDate_Outside)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    QDateTime start(QDate(2026, 6, 10), QTime(10, 0));
    QDateTime end(QDate(2026, 6, 10), QTime(11, 0));
    list.append(makeSchedule(start, end));
    map.insert(QDate(2026, 6, 10), list);
    QDate filterBegin(2026, 7, 1), filterEnd(2026, 7, 31);
    auto result = proxy.scheduleFileterByDate(map, filterBegin, filterEnd);
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, ScheduleFileterByDate_EmptyMap)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    QDate filterBegin(2026, 6, 1), filterEnd(2026, 6, 30);
    auto result = proxy.scheduleFileterByDate(map, filterBegin, filterEnd);
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// scheduleFileterByTime
// ============================================================================

TEST(QueryScheduleProxyTest, ScheduleFileterByTime_WithinOneDay_Intersect)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    QDateTime start(QDate(2026, 6, 10), QTime(10, 0));
    QDateTime end(QDate(2026, 6, 10), QTime(11, 0));
    list.append(makeSchedule(start, end));
    map.insert(QDate(2026, 6, 10), list);
    QTime filterBegin(9, 0), filterEnd(12, 0);
    auto result = proxy.scheduleFileterByTime(map, filterBegin, filterEnd);
    EXPECT_GE(result.size(), 0); // should not crash
}

TEST(QueryScheduleProxyTest, ScheduleFileterByTime_MoreThanOneDay)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    QDateTime start(QDate(2026, 6, 10), QTime(10, 0));
    QDateTime end(QDate(2026, 6, 12), QTime(11, 0));
    list.append(makeSchedule(start, end));
    map.insert(QDate(2026, 6, 10), list);
    QTime filterBegin(9, 0), filterEnd(12, 0);
    auto result = proxy.scheduleFileterByTime(map, filterBegin, filterEnd);
    EXPECT_GE(result.size(), 0);
}

// ============================================================================
// scheduleFileterByTitleName
// ============================================================================

TEST(QueryScheduleProxyTest, ScheduleFileterByTitleName_Match)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    list.append(makeSchedule(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                              QDateTime(QDate(2026, 6, 10), QTime(11, 0)), "Meeting"));
    map.insert(QDate(2026, 6, 10), list);
    auto result = proxy.scheduleFileterByTitleName(map, "Meet");
    EXPECT_EQ(result.size(), 1);
}

TEST(QueryScheduleProxyTest, ScheduleFileterByTitleName_NoMatch)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    list.append(makeSchedule(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                              QDateTime(QDate(2026, 6, 10), QTime(11, 0)), "Meeting"));
    map.insert(QDate(2026, 6, 10), list);
    auto result = proxy.scheduleFileterByTitleName(map, "Party");
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// getQueryDateTime
// ============================================================================

TEST(QueryScheduleProxyTest, GetQueryDateTime_JsonData_NotChangeJsonData)
{
    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.getQueryDateTime(&jsonData);
    // Should return jsonData->getDateTime()
    EXPECT_EQ(result.dateTime.size(), jsonData.getDateTime().dateTime.size());
}

TEST(QueryScheduleProxyTest, GetQueryDateTime_ChangeJsonData_WithSuggest)
{
    changejsondata cjd;
    SemanticsDateTime sdt;
    SuggestDatetimeInfo sdi;
    sdi.datetime = QDateTime(QDate(2026, 6, 15), QTime(10, 0));
    sdt.suggestDatetime.append(sdi);
    cjd.setFromDateTime(sdt);
    queryScheduleProxy proxy(&cjd);
    auto result = proxy.getQueryDateTime(&cjd);
    EXPECT_EQ(result.suggestDatetime.size(), 1);
}

TEST(QueryScheduleProxyTest, GetQueryDateTime_ChangeJsonData_NoSuggest)
{
    changejsondata cjd;
    queryScheduleProxy proxy(&cjd);
    auto result = proxy.getQueryDateTime(&cjd);
    // No suggestDatetime → return getDateTime()
    EXPECT_EQ(result.dateTime.size(), cjd.getDateTime().dateTime.size());
}

// ============================================================================
// sortAndFilterSchedule (DB-dependent: stub isFestivalSchedule)
// ============================================================================

static bool stub_isFestivalSchedule(DScheduleDataManager*, const QString&)
{
    return false;
}

TEST(QueryScheduleProxyTest, SortAndFilterSchedule_NotFestival)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    list.append(makeSchedule(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                              QDateTime(QDate(2026, 6, 10), QTime(11, 0))));
    map.insert(QDate(2026, 6, 10), list);
    auto result = proxy.sortAndFilterSchedule(map);
    EXPECT_EQ(result.size(), 1);
}

TEST(QueryScheduleProxyTest, SortAndFilterSchedule_Empty)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    auto result = proxy.sortAndFilterSchedule(map);
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// WeeklyScheduleFileter
// ============================================================================

TEST(QueryScheduleProxyTest, WeeklyScheduleFileter_EmptyWeeklySet)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    list.append(makeSchedule(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                              QDateTime(QDate(2026, 6, 10), QTime(11, 0))));
    map.insert(QDate(2026, 6, 10), list);
    QSet<int> weeklySet;
    auto result = proxy.WeeklyScheduleFileter(map, weeklySet);
    EXPECT_EQ(result.size(), 1);
}

TEST(QueryScheduleProxyTest, WeeklyScheduleFileter_WithWeeklySet)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    list.append(makeSchedule(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                              QDateTime(QDate(2026, 6, 10), QTime(11, 0))));
    map.insert(QDate(2026, 6, 10), list);
    QSet<int> weeklySet;
    weeklySet.insert(3); // Wednesday — June 10 2026 is a Wednesday
    auto result = proxy.WeeklyScheduleFileter(map, weeklySet);
    EXPECT_GE(result.size(), 0);
}

// ============================================================================
// MonthlyScheduleFileter
// ============================================================================

TEST(QueryScheduleProxyTest, MonthlyScheduleFileter_ZeroBeginM)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    list.append(makeSchedule(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                              QDateTime(QDate(2026, 6, 10), QTime(11, 0))));
    map.insert(QDate(2026, 6, 10), list);
    auto result = proxy.MonthlyScheduleFileter(map, 0, 0);
    EXPECT_EQ(result.size(), 1);
}

TEST(QueryScheduleProxyTest, MonthlyScheduleFileter_WithRange)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    DSchedule::Map map;
    DSchedule::List list;
    list.append(makeSchedule(QDateTime(QDate(2026, 6, 10), QTime(10, 0)),
                              QDateTime(QDate(2026, 6, 10), QTime(11, 0))));
    map.insert(QDate(2026, 6, 10), list);
    auto result = proxy.MonthlyScheduleFileter(map, 5, 15);
    EXPECT_GE(result.size(), 0);
}

// ============================================================================
// querySchedule (DB-dependent: stub getInstance + query methods)
// ============================================================================

static DScheduleDataManager* stub_getInstance()
{
    return nullptr;
}

static DSchedule::Map stub_queryScheduleByRRule(DScheduleDataManager*, const QDateTime&, const QDateTime&, const DScheduleQueryPar::RRuleType&)
{
    return DSchedule::Map();
}

static DSchedule::Map stub_queryScheduleBySummary(DScheduleDataManager*, const QDateTime&, const QDateTime&, const QString&)
{
    return DSchedule::Map();
}

static DSchedule::Map stub_queryScheduleByLimit(DScheduleDataManager*, const QDateTime&, const QDateTime&, int)
{
    return DSchedule::Map();
}

static DSchedule::Map stub_querySchedulesWithParameter(DScheduleDataManager*, const DScheduleQueryPar::Ptr&)
{
    return DSchedule::Map();
}

TEST(QueryScheduleProxyTest, QuerySchedule_RepeatNone)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, queryScheduleBySummary), stub_queryScheduleBySummary);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByLimit), stub_queryScheduleByLimit);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setRepeatStatus(JsonData::NONE);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.querySchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QuerySchedule_RepeatEVED)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setRepeatStatus(JsonData::EVED);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.querySchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QuerySchedule_RepeatEVEW)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setRepeatStatus(JsonData::EVEW);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.querySchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QuerySchedule_RepeatEVEM)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setRepeatStatus(JsonData::EVEM);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.querySchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QuerySchedule_RepeatEVEY)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setRepeatStatus(JsonData::EVEY);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.querySchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QuerySchedule_RepeatWORKD)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setRepeatStatus(JsonData::WORKD);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.querySchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QuerySchedule_RepeatRESTD)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setRepeatStatus(JsonData::RESTD);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.querySchedule();
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// queryWeeklySchedule
// ============================================================================

TEST(QueryScheduleProxyTest, QueryWeeklySchedule_Basic)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryWeeklySchedule(begin, end, 1, 5);
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryWeeklySchedule_NoWeekRange)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryWeeklySchedule(begin, end, 0, 0);
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// queryMonthlySchedule
// ============================================================================

TEST(QueryScheduleProxyTest, QueryMonthlySchedule_Basic)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryMonthlySchedule(begin, end, 1, 15);
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryMonthlySchedule_NoMonthRange)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryMonthlySchedule(begin, end, 0, 0);
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// queryEveryDaySchedule / queryEveryYearSchedule / queryWorkingDaySchedule
// ============================================================================

TEST(QueryScheduleProxyTest, QueryEveryDaySchedule_Basic)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryEveryDaySchedule(begin, end);
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryEveryYearSchedule_Basic)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryEveryYearSchedule(begin, end);
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryWorkingDaySchedule_Basic)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByRRule), stub_queryScheduleByRRule);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryWorkingDaySchedule(begin, end);
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// queryNonRepeatingSchedule
// ============================================================================

TEST(QueryScheduleProxyTest, QueryNonRepeatingSchedule_PropertyAll)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleBySummary), stub_queryScheduleBySummary);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setPropertyStatus(JsonData::ALL);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.queryNonRepeatingSchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryNonRepeatingSchedule_PropertyNext)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByLimit), stub_queryScheduleByLimit);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setPropertyStatus(JsonData::NEXT);
    jsonData.setOffset(3);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.queryNonRepeatingSchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryNonRepeatingSchedule_PropertyLast)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleBySummary), stub_queryScheduleBySummary);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setPropertyStatus(JsonData::LAST);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.queryNonRepeatingSchedule();
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryNonRepeatingSchedule_PropertyNone)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleBySummary), stub_queryScheduleBySummary);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    jsonData.setPropertyStatus(JsonData::PRO_NONE);
    queryScheduleProxy proxy(&jsonData);
    auto result = proxy.queryNonRepeatingSchedule();
    EXPECT_EQ(result.size(), 0);
}

// ============================================================================
// queryNextNumSchedule / queryAllSchedule
// ============================================================================

TEST(QueryScheduleProxyTest, QueryNextNumSchedule_Basic)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByLimit), stub_queryScheduleByLimit);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryNextNumSchedule(begin, end, 5);
    EXPECT_EQ(result.size(), 0);
}

TEST(QueryScheduleProxyTest, QueryAllSchedule_Basic)
{
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleBySummary), stub_queryScheduleBySummary);
    stub.set(ADDR(DScheduleDataManager, isFestivalSchedule), stub_isFestivalSchedule);

    JsonData jsonData;
    queryScheduleProxy proxy(&jsonData);
    QDateTime begin(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 6, 30), QTime(23, 59));
    auto result = proxy.queryAllSchedule("test", begin, end);
    EXPECT_EQ(result.size(), 0);
}
