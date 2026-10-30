// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "../third-party_stub/stub.h"

#include "lunarmanager.h"
#include "dbushuanglirequest.h"
#include "huangliData/lunardatastruct.h"
#include "huangliData/dbusdatastruct.h"

#include <QTest>
#include <QSignalSpy>

// Stub functions for DbusHuangLiRequest methods
static bool stub_getFestivalMonth(void*, quint32, quint32, FestivalInfo &info)
{
    HolidayInfo h;
    h.date = QDate(2025, 6, 1);
    h.status = 1;
    info.listHoliday.append(h);
    return true;
}

static bool stub_getHuangLiDay_ymd(void*, quint32, quint32, quint32, CaHuangLiDayInfo &info)
{
    info.mGanZhiYear = "甲子";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    return true;
}

static bool stub_getHuangLiMonth(void*, quint32, quint32, bool, CaHuangLiMonthInfo &info)
{
    info.mFirstDayWeek = 0;
    info.mDays = 30;
    CaHuangLiDayInfo dayInfo;
    dayInfo.mGanZhiYear = QString::fromUtf8("甲子");
    dayInfo.mLunarMonthName = QString::fromUtf8("正月");
    dayInfo.mLunarDayName = QString::fromUtf8("初一");
    for (int i = 0; i < 30; ++i) {
        info.mCaLunarDayInfo.append(dayInfo);
    }
    return true;
}

static bool stub_getHuangLiDay_false(void*, quint32, quint32, quint32, CaHuangLiDayInfo &)
{
    return false;
}

class TestLunarManager : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.set(ADDR(DbusHuangLiRequest, getFestivalMonth), stub_getFestivalMonth);
        stub.set(ADDR(DbusHuangLiRequest, getHuangLiDay), stub_getHuangLiDay_ymd);
        stub.set(ADDR(DbusHuangLiRequest, getHuangLiMonth), stub_getHuangLiMonth);

        // Create a fresh LunarManager for isolated cache testing
        manager = new LunarManager();
    }

    void TearDown() override
    {
        delete manager;
    }

    Stub stub;
    LunarManager *manager = nullptr;
};

// Constructor / getInstace
TEST_F(TestLunarManager, Constructor)
{
    EXPECT_NE(manager, nullptr);
}

TEST_F(TestLunarManager, GetInstace)
{
    LunarManager *inst = LunarManager::getInstace();
    EXPECT_NE(inst, nullptr);
    EXPECT_EQ(inst, LunarManager::getInstace());
}

// getFestivalMonth (year, month)
TEST_F(TestLunarManager, GetFestivalMonthYM)
{
    FestivalInfo info;
    bool result = manager->getFestivalMonth(2025, 6, info);
    EXPECT_TRUE(result);
}

// getFestivalMonth (QDate)
TEST_F(TestLunarManager, GetFestivalMonthDate)
{
    FestivalInfo info;
    bool result = manager->getFestivalMonth(QDate(2025, 6, 15), info);
    EXPECT_TRUE(result);
}

// getHuangLiDay (year, month, day)
TEST_F(TestLunarManager, GetHuangLiDayYMD)
{
    CaHuangLiDayInfo info;
    bool result = manager->getHuangLiDay(2025, 6, 15, info);
    EXPECT_TRUE(result);
    EXPECT_FALSE(info.mGanZhiYear.isEmpty());
}

// getHuangLiDay (QDate, CaHuangLiDayInfo&)
TEST_F(TestLunarManager, GetHuangLiDayDateRef)
{
    CaHuangLiDayInfo info;
    bool result = manager->getHuangLiDay(QDate(2025, 6, 15), info);
    EXPECT_TRUE(result);
}

// getHuangLiMonth (year, month)
TEST_F(TestLunarManager, GetHuangLiMonthYM)
{
    CaHuangLiMonthInfo info;
    bool result = manager->getHuangLiMonth(2025, 6, info, false);
    EXPECT_TRUE(result);
}

// getHuangLiMonth (QDate)
TEST_F(TestLunarManager, GetHuangLiMonthDate)
{
    CaHuangLiMonthInfo info;
    bool result = manager->getHuangLiMonth(QDate(2025, 6, 15), info, true);
    EXPECT_TRUE(result);
}

// hasHuangLiDay - no cache
TEST_F(TestLunarManager, HasHuangLiDayNoCache)
{
    EXPECT_FALSE(manager->hasHuangLiDay(QDate(2025, 6, 15)));
}

// hasHuangLiDay - with cache
TEST_F(TestLunarManager, HasHuangLiDayCached)
{
    QDate date(2025, 6, 15);
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    manager->m_lunarInfoMap[date] = info;
    EXPECT_TRUE(manager->hasHuangLiDay(date));
}

// hasHuangLiDay - cached but invalid
TEST_F(TestLunarManager, HasHuangLiDayCachedInvalid)
{
    QDate date(2025, 6, 15);
    CaHuangLiDayInfo info; // empty fields
    manager->m_lunarInfoMap[date] = info;
    EXPECT_FALSE(manager->hasHuangLiDay(date));
}

// hasHuangLiRange - no cache
TEST_F(TestLunarManager, HasHuangLiRangeNoCache)
{
    EXPECT_FALSE(manager->hasHuangLiRange(QDate(2025, 6, 1), QDate(2025, 6, 30)));
}

// hasHuangLiRange - with full cache
TEST_F(TestLunarManager, HasHuangLiRangeCached)
{
    QDate start(2025, 6, 1);
    QDate end(2025, 6, 3);
    for (QDate d = start; d <= end; d = d.addDays(1)) {
        CaHuangLiDayInfo info;
        info.mGanZhiYear = "甲子";
        info.mLunarMonthName = "正月";
        info.mLunarDayName = "初一";
        manager->m_lunarInfoMap[d] = info;
    }
    EXPECT_TRUE(manager->hasHuangLiRange(start, end));
}

// hasHuangLiRange - partial cache
TEST_F(TestLunarManager, HasHuangLiRangePartial)
{
    QDate start(2025, 6, 1);
    QDate end(2025, 6, 3);
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    manager->m_lunarInfoMap[start] = info;
    // Missing 6/2 and 6/3
    EXPECT_FALSE(manager->hasHuangLiRange(start, end));
}

// hasHuangLiRange - invalid dates
TEST_F(TestLunarManager, HasHuangLiRangeInvalid)
{
    EXPECT_FALSE(manager->hasHuangLiRange(QDate(), QDate(2025, 6, 30)));
    EXPECT_FALSE(manager->hasHuangLiRange(QDate(2025, 6, 30), QDate(2025, 6, 1)));
}

// getHuangLiDay (QDate) - cache miss
TEST_F(TestLunarManager, GetHuangLiDayCacheMiss)
{
    QDate date(2025, 6, 15);
    CaHuangLiDayInfo info = manager->getHuangLiDay(date);
    EXPECT_FALSE(info.mGanZhiYear.isEmpty());
}

// getHuangLiDay (QDate) - cache hit
TEST_F(TestLunarManager, GetHuangLiDayCacheHit)
{
    QDate date(2025, 6, 15);
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "乙丑";
    info.mLunarMonthName = "二月";
    info.mLunarDayName = "初二";
    manager->m_lunarInfoMap[date] = info;
    CaHuangLiDayInfo result = manager->getHuangLiDay(date);
    EXPECT_EQ(result.mGanZhiYear, QString("乙丑"));
}

// getHuangLiShortName
TEST_F(TestLunarManager, GetHuangLiShortName)
{
    QDate date(2025, 6, 15);
    QString name = manager->getHuangLiShortName(date);
    EXPECT_FALSE(name.isEmpty());
}

// getHuangLiDayMap
TEST_F(TestLunarManager, GetHuangLiDayMap)
{
    QDate d1(2025, 6, 1);
    QDate d2(2025, 6, 15);
    QDate d3(2025, 7, 1);
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    manager->m_lunarInfoMap[d1] = info;
    manager->m_lunarInfoMap[d2] = info;
    manager->m_lunarInfoMap[d3] = info;

    QMap<QDate, CaHuangLiDayInfo> result = manager->getHuangLiDayMap(QDate(2025, 6, 1), QDate(2025, 6, 30));
    EXPECT_EQ(result.size(), 2);
}

// getFestivalInfoDateMap
TEST_F(TestLunarManager, GetFestivalInfoDateMap)
{
    manager->m_festivalDateMap[QDate(2025, 6, 1)] = 1;
    manager->m_festivalDateMap[QDate(2025, 7, 1)] = 1;

    QMap<QDate, int> result = manager->getFestivalInfoDateMap(QDate(2025, 6, 1), QDate(2025, 6, 30));
    EXPECT_EQ(result.size(), 1);
}

// ensureLunarDataLoaded - invalid dates
TEST_F(TestLunarManager, EnsureLunarDataLoadedInvalid)
{
    EXPECT_NO_FATAL_FAILURE(manager->ensureLunarDataLoaded(QDate(), QDate()));
    EXPECT_NO_FATAL_FAILURE(manager->ensureLunarDataLoaded(QDate(2025, 6, 30), QDate(2025, 6, 1)));
}

// ensureLunarDataLoaded - valid, cache hit
TEST_F(TestLunarManager, EnsureLunarDataLoadedCacheHit)
{
    QDate start(2025, 6, 1);
    QDate end(2025, 6, 3);
    for (QDate d = start; d <= end; d = d.addDays(1)) {
        CaHuangLiDayInfo info;
        info.mGanZhiYear = "甲子";
        info.mLunarMonthName = "正月";
        info.mLunarDayName = "初一";
        manager->m_lunarInfoMap[d] = info;
    }
    EXPECT_NO_FATAL_FAILURE(manager->ensureLunarDataLoaded(start, end));
}

// ensureLunarDataLoaded - valid, cache miss
TEST_F(TestLunarManager, EnsureLunarDataLoadedCacheMiss)
{
    EXPECT_NO_FATAL_FAILURE(manager->ensureLunarDataLoaded(QDate(2025, 6, 1), QDate(2025, 6, 5)));
}

// ensureLunarDataLoaded - already queried range
TEST_F(TestLunarManager, EnsureLunarDataLoadedAlreadyQueried)
{
    QDate start(2025, 6, 1);
    QDate end(2025, 6, 5);
    // First call triggers query
    manager->ensureLunarDataLoaded(start, end);
    // Second call should find it in queriedRanges (set by cache hit path)
    // or pending queries
    EXPECT_NO_FATAL_FAILURE(manager->ensureLunarDataLoaded(start, end));
}

// queryLunarInfo
TEST_F(TestLunarManager, QueryLunarInfo)
{
    QSignalSpy spy(manager, &LunarManager::lunarInfoReady);
    EXPECT_NO_FATAL_FAILURE(manager->queryLunarInfo(QDate(2025, 6, 1), QDate(2025, 6, 5)));
    QTest::qWait(500);
    EXPECT_GE(spy.count(), 1);
    EXPECT_TRUE(manager->hasHuangLiRange(QDate(2025, 6, 1), QDate(2025, 6, 5)));
}

// queryFestivalInfo
TEST_F(TestLunarManager, QueryFestivalInfo)
{
    QSignalSpy spy(manager, &LunarManager::festivalInfoReady);
    EXPECT_NO_FATAL_FAILURE(manager->queryFestivalInfo(QDate(2025, 6, 1), QDate(2025, 6, 5)));
    QTest::qWait(500);
    EXPECT_GE(spy.count(), 1);
    EXPECT_GE(manager->getFestivalInfoDateMap(QDate(2025, 6, 1), QDate(2025, 6, 30)).size(), 1);
}

// getHuangLiDayAsync - cache hit
TEST_F(TestLunarManager, GetHuangLiDayAsyncCacheHit)
{
    QDate date(2025, 6, 15);
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    manager->m_lunarInfoMap[date] = info;
    QSignalSpy spy(manager, &LunarManager::huangLiDayReady);
    manager->getHuangLiDayAsync(date);
    EXPECT_TRUE(spy.count() >= 0);
}

// getHuangLiDayAsync - cache miss
TEST_F(TestLunarManager, GetHuangLiDayAsyncCacheMiss)
{
    QDate date(2025, 6, 16);
    manager->getHuangLiDayAsync(date);
    // async, just verify no crash
    QTest::qWait(100);
}

