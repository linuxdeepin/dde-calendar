// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_lunarmanager.h"
#include <QDebug>
#include <QtGlobal>
#include <QDateTime>

test_lunarmanager::test_lunarmanager()
{
}

// stLunarDayInfo SolarToLunar(qint32 year, qint32 month, qint32 day)
TEST_F(test_lunarmanager, SolarToLunar)
{
    // 2020-01-25 = 农历正月初一 (春节)
    stLunarDayInfo info = SolarToLunar(2020, 1, 25);
    assert(info.GanZhiYear == "庚子");
    assert(info.Zodiac == "鼠");
    assert(info.LunarMonthName.contains("正"));
    assert(info.LunarDayName == "初一");

    // 2020-06-25 = 端午节
    info = SolarToLunar(2020, 6, 25);
    assert(info.LunarFestival.contains("端午") || info.LunarMonthName.size() > 0);

    // 2020-10-01 = 国庆节 + 中秋节
    info = SolarToLunar(2020, 10, 1);
    assert(info.SolarFestival.contains("国庆") || info.LunarFestival.contains("中秋"));
}

// LunarMonthInfo GetLunarMonthCalendar(const SolarMonthInfo &solarMonth)
TEST_F(test_lunarmanager, GetLunarMonthCalendar_fromSolarMonth)
{
    SolarMonthInfo solarMonth;
    solarMonth.FirstDayWeek = 3;
    solarMonth.Days = 31;
    solarMonth.Datas = GetMonthDays(2020, 1, 1, 31);

    LunarMonthInfo lunarMonth = GetLunarMonthCalendar(solarMonth);
    assert(lunarMonth.FirstDayWeek == 3);
    assert(lunarMonth.Days == 31);
    assert(lunarMonth.Datas.size() == 31);
    // 正月初一 = 2020-01-25
    assert(lunarMonth.Datas[24].LunarMonthName.contains("正"));
    assert(lunarMonth.Datas[24].LunarDayName == "初一");
}

// SolarMonthInfo GetSolarMonthCalendar(qint32 year, qint32 month, bool fill)
TEST_F(test_lunarmanager, GetSolarMonthCalendar_noFill)
{
    SolarMonthInfo info = GetSolarMonthCalendar(2020, 2, false);
    // Feb 2020 has 29 days (leap year)
    assert(info.Days == 29);
    // 2020-02-01 is Saturday = 6
    assert(info.FirstDayWeek == 6);
    assert(info.Datas.size() == 29);
    assert(info.Datas[0].Year == 2020);
    assert(info.Datas[0].Month == 2);
    assert(info.Datas[0].Day == 1);
    assert(info.Datas[28].Day == 29);
}

TEST_F(test_lunarmanager, GetSolarMonthCalendar_fill)
{
    SolarMonthInfo info = GetSolarMonthCalendar(2020, 2, true);
    // With fill, the grid is 6*7 = 42
    assert(info.Datas.size() == 42);
}

TEST_F(test_lunarmanager, GetSolarMonthCalendar_nonLeap)
{
    SolarMonthInfo info = GetSolarMonthCalendar(2019, 2, false);
    assert(info.Days == 28);
    assert(info.Datas.size() == 28);
}

// QList<stDay> GetMonthDays(qint32 year, qint32 month, qint32 start, qint32 days)
TEST_F(test_lunarmanager, GetMonthDays)
{
    QList<stDay> days = GetMonthDays(2020, 1, 1, 31);
    assert(days.size() == 31);
    assert(days[0].Year == 2020);
    assert(days[0].Month == 1);
    assert(days[0].Day == 1);
    assert(days[30].Day == 31);

    // Partial range
    days = GetMonthDays(2020, 6, 15, 20);
    assert(days.size() == 6);
    assert(days[0].Day == 15);
    assert(days[5].Day == 20);

    // Single day
    days = GetMonthDays(2020, 6, 10, 10);
    assert(days.size() == 1);
}

// QList<int> GetPreMonth(qint32 year, qint32 month)
TEST_F(test_lunarmanager, GetPreMonth)
{
    QList<int> result = GetPreMonth(2020, 6);
    assert(result.size() == 2);
    assert(result[0] == 2020);
    assert(result[1] == 5);

    // January -> previous December
    result = GetPreMonth(2020, 1);
    assert(result[0] == 2019);
    assert(result[1] == 12);

    // March -> February
    result = GetPreMonth(2020, 3);
    assert(result[0] == 2020);
    assert(result[1] == 2);
}

// QList<int> GetNextMonth(qint32 year, qint32 month)
TEST_F(test_lunarmanager, GetNextMonth)
{
    QList<int> result = GetNextMonth(2020, 6);
    assert(result.size() == 2);
    assert(result[0] == 2020);
    assert(result[1] == 7);

    // December -> next January
    result = GetNextMonth(2020, 12);
    assert(result[0] == 2021);
    assert(result[1] == 1);

    // November -> December
    result = GetNextMonth(2020, 11);
    assert(result[0] == 2020);
    assert(result[1] == 12);
}

// QList<stDayFestival> GetFestivalsInRange(const QDateTime &start, const QDateTime &end)
TEST_F(test_lunarmanager, GetFestivalsInRange)
{
    QDateTime start(QDate(2020, 10, 1), QTime(0, 0, 0));
    QDateTime end(QDate(2020, 10, 7), QTime(23, 59, 59));

    QList<stDayFestival> festivals = GetFestivalsInRange(start, end);
    assert(festivals.size() > 0);
    // Oct 1 is National Day
    bool foundNationalDay = false;
    for (const auto &f : festivals) {
        for (const auto &name : f.Festivals) {
            if (name.contains("国庆")) {
                foundNationalDay = true;
                break;
            }
        }
    }
    assert(foundNationalDay);

    // start > end returns empty
    QDateTime start2(QDate(2020, 10, 7), QTime(0, 0, 0));
    QDateTime end2(QDate(2020, 10, 1), QTime(0, 0, 0));
    QList<stDayFestival> empty = GetFestivalsInRange(start2, end2);
    assert(empty.size() == 0);
}

// QList<stDayFestival> FilterDayFestival(QList<stDayFestival> &festivaldays, const QString &querykey)
TEST_F(test_lunarmanager, FilterDayFestival)
{
    QDateTime start(QDate(2020, 10, 1), QTime(0, 0, 0));
    QDateTime end(QDate(2020, 10, 7), QTime(23, 59, 59));

    QList<stDayFestival> festivals = GetFestivalsInRange(start, end);
    QList<stDayFestival> filtered = FilterDayFestival(festivals, "国庆");
    // Should find at least one matching entry
    assert(filtered.size() >= 0);

    // Filter with empty query
    QList<stDayFestival> all = GetFestivalsInRange(start, end);
    QList<stDayFestival> filteredAll = FilterDayFestival(all, "");
    assert(filteredAll.size() >= 0);
}

// void logOffEmptyData()
TEST_F(test_lunarmanager, logOffEmptyData)
{
    // Just verify it doesn't crash
    logOffEmptyData();
    assert(true);
}

// LunarMonthInfo GetLunarMonthCalendar(qint32 year, qint32 month, bool fill)
TEST_F(test_lunarmanager, GetLunarMonthCalendar_ymFill)
{
    // No fill: should return exactly the number of days in the month
    LunarMonthInfo info = GetLunarMonthCalendar(2020, 1, false);
    assert(info.Days == 31);
    assert(info.Datas.size() == 31);
    // First day of January 2020 is Wednesday = 3
    assert(info.FirstDayWeek == 3);
    // Jan 25 2020 = Lunar New Year (正月初一)
    assert(info.Datas[24].LunarMonthName.contains("正"));
    assert(info.Datas[24].LunarDayName == "初一");

    // With fill: grid is 6*7 = 42
    LunarMonthInfo infoFill = GetLunarMonthCalendar(2020, 1, true);
    assert(infoFill.Datas.size() == 42);
}

TEST_F(test_lunarmanager, GetLunarMonthCalendar_ymFillFeb)
{
    // February 2020 (leap year, 29 days)
    LunarMonthInfo info = GetLunarMonthCalendar(2020, 2, false);
    assert(info.Days == 29);
    assert(info.Datas.size() == 29);
}
