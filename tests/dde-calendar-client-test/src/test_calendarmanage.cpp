// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "../third-party_stub/stub.h"

#include "calendarmanage.h"
#include "cschedulebasewidget.h"
#include "scheduledatamanage.h"
#include "schedulemanager.h"
#include "accountmanager.h"
#include "lunarmanager.h"
#include "dbustimedate.h"
#include "dcalendargeneralsettings.h"

#include <QTest>
#include <QSignalSpy>
#include <QWidget>

// Stub functions for DBus-dependent methods
static int stub_shortTimeFormat()
{
    return 0;
}

static int stub_shortDateFormat()
{
    return 0;
}

static void stub_resetSchedule_int(int)
{
}

static void stub_resetSchedule_dt(const QDateTime &, const QDateTime &)
{
}

static DCalendarGeneralSettings::Ptr stub_getGeneralSettings()
{
    return DCalendarGeneralSettings::Ptr();
}

static void stub_ensureLunarDataLoaded(const QDate &, const QDate &)
{
}

// Concrete subclass of CScheduleBaseWidget for testing
class TestScheduleWidget : public CScheduleBaseWidget
{
    Q_OBJECT
public:
    explicit TestScheduleWidget(QWidget *parent = nullptr) : CScheduleBaseWidget(parent) {}
    void setYearData() override {}
    void updateShowDate(const bool = true) override {}
    void updateShowSchedule() override {}
    void updateShowLunar() override {}
    void setSelectSearchScheduleInfo(const DSchedule::Ptr &) override {}
};

class TestCalendarManager : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Stub DBus-dependent methods before getInstance()
        stub.set(ADDR(DaemonTimeDate, shortTimeFormat), stub_shortTimeFormat);
        stub.set(ADDR(DaemonTimeDate, shortDateFormat), stub_shortDateFormat);
        stub.set(static_cast<void(ScheduleManager::*)(int)>(&ScheduleManager::resetSchedule), stub_resetSchedule_int);
        stub.set(static_cast<void(ScheduleManager::*)(const QDateTime&, const QDateTime&)>(&ScheduleManager::resetSchedule), stub_resetSchedule_dt);
        stub.set(ADDR(AccountManager, getGeneralSettings), stub_getGeneralSettings);
        stub.set(ADDR(LunarManager, ensureLunarDataLoaded), stub_ensureLunarDataLoaded);

        manager = CalendarManager::getInstance();
    }

    void TearDown() override
    {
        CalendarManager::releaseInstance();
    }

    Stub stub;
    CalendarManager *manager = nullptr;
};

// getInstance / releaseInstance
TEST_F(TestCalendarManager, GetInstance)
{
    EXPECT_NE(manager, nullptr);
    EXPECT_EQ(manager, CalendarManager::getInstance());
}

TEST_F(TestCalendarManager, ReleaseInstance)
{
    CalendarManager::releaseInstance();
    EXPECT_EQ(CalendarManager::getInstance() != nullptr, true);
    // Restore for TearDown
    manager = CalendarManager::getInstance();
}

// setSelectDate / getSelectDate
TEST_F(TestCalendarManager, SetGetSelectDate)
{
    QDate date(2025, 6, 15);
    manager->setSelectDate(date);
    EXPECT_EQ(manager->getSelectDate(), date);
}

TEST_F(TestCalendarManager, SetSelectDateSwitchYear)
{
    QDate date(2030, 1, 1);
    manager->setSelectDate(date, true);
    EXPECT_EQ(manager->getSelectDate(), date);
}

TEST_F(TestCalendarManager, SetSelectDateOutOfRange)
{
    QDate date(2050, 12, 31);
    manager->setSelectDate(date, false);
    EXPECT_EQ(manager->getSelectDate(), date);
}

// setCurrentDateTime / getCurrentDate
TEST_F(TestCalendarManager, SetGetCurrentDateTime)
{
    QDateTime dt(QDate(2025, 3, 20), QTime(10, 30, 0));
    manager->setCurrentDateTime(dt);
    EXPECT_EQ(manager->getCurrentDate(), dt);
}

// getMonthDate
TEST_F(TestCalendarManager, GetMonthDate)
{
    QVector<QDate> dates = manager->getMonthDate(2025, 6);
    EXPECT_EQ(dates.size(), 42);
}

// getWeekDate
TEST_F(TestCalendarManager, GetWeekDate)
{
    QDate date(2025, 6, 15);
    QVector<QDate> dates = manager->getWeekDate(date);
    EXPECT_EQ(dates.size(), 7);
}

// setWeekDayFormatByID / getWeekDayFormat
TEST_F(TestCalendarManager, SetWeekDayFormatByIDZero)
{
    manager->setWeekDayFormatByID(0);
    EXPECT_EQ(manager->getWeekDayFormat(), QString("dddd"));
}

TEST_F(TestCalendarManager, SetWeekDayFormatByIDDefault)
{
    manager->setWeekDayFormatByID(1);
    EXPECT_EQ(manager->getWeekDayFormat(), QString("ddd"));
}

// getShowDateRange
TEST_F(TestCalendarManager, GetShowDateRange)
{
    ShowDateRange range = manager->getShowDateRange();
    // After setYearBeginAndEndDate in initData, showYear should be set
    EXPECT_GE(range.showYear, 0);
}

// getFirstDayOfWeek(QDate)
TEST_F(TestCalendarManager, GetFirstDayOfWeekByDate)
{
    QDate date(2025, 6, 15); // Sunday
    QDate firstDay = manager->getFirstDayOfWeek(date);
    EXPECT_TRUE(firstDay.isValid());
}

// getFirstDayOfWeek() / setFirstDayOfWeek
TEST_F(TestCalendarManager, SetGetFirstDayOfWeek)
{
    manager->setFirstDayOfWeek(1, false); // Monday
    EXPECT_EQ(manager->getFirstDayOfWeek(), Qt::Monday);
}

TEST_F(TestCalendarManager, SetFirstDayOfWeekWithUpdate)
{
    manager->setFirstDayOfWeek(3, true);
    EXPECT_EQ(manager->getFirstDayOfWeek(), Qt::Wednesday);
}

// getWeekNumOfYear (static)
TEST_F(TestCalendarManager, GetWeekNumOfYear)
{
    QDate date(2025, 6, 15);
    int weekNum = CalendarManager::getWeekNumOfYear(date);
    EXPECT_GE(weekNum, 1);
    EXPECT_LE(weekNum, 53);
}

// setTimeFormatChanged / getTimeFormat
TEST_F(TestCalendarManager, SetTimeFormatChangedZero)
{
    manager->setTimeFormatChanged(0);
    EXPECT_EQ(manager->getTimeFormat(), QString("h:mm"));
}

TEST_F(TestCalendarManager, SetTimeFormatChangedOne)
{
    manager->setTimeFormatChanged(1);
    EXPECT_EQ(manager->getTimeFormat(), QString("hh:mm"));
}

// setDateFormatChanged / getDateFormat
TEST_F(TestCalendarManager, SetDateFormatChanged)
{
    struct TestCase { int value; QString expected; };
    QVector<TestCase> cases = {
        {0, "yyyy/M/d"}, {1, "yyyy-M-d"}, {2, "yyyy.M.d"},
        {3, "yyyy/MM/dd"}, {4, "yyyy-MM-dd"}, {5, "yyyy.MM.dd"},
        {6, "yy/M/d"}, {7, "yy-M-d"}, {8, "yy.M.d"},
        {99, "yyyy-MM-dd"} // default
    };
    for (const auto &tc : cases) {
        manager->setDateFormatChanged(tc.value);
        EXPECT_EQ(manager->getDateFormat(), tc.expected)
            << "Failed for value " << tc.value;
    }
}

// setTimeShowType / getTimeShowType
TEST_F(TestCalendarManager, SetGetTimeShowType)
{
    manager->setTimeShowType(1, false);
    EXPECT_EQ(manager->getTimeShowType(), 1);
}

TEST_F(TestCalendarManager, SetTimeShowTypeWithUpdate)
{
    manager->setTimeShowType(0, true);
    EXPECT_EQ(manager->getTimeShowType(), 0);
}

// addShowWidget / removeShowWidget / getShowWidget / getShowWidgetSize
TEST_F(TestCalendarManager, AddRemoveShowWidget)
{
    TestScheduleWidget widget;
    int initialSize = manager->getShowWidgetSize();
    manager->addShowWidget(&widget);
    EXPECT_EQ(manager->getShowWidgetSize(), initialSize + 1);
    EXPECT_EQ(manager->getShowWidget(0), &widget);
    manager->removeShowWidget(&widget);
    EXPECT_EQ(manager->getShowWidgetSize(), initialSize);
}

TEST_F(TestCalendarManager, GetShowWidgetInvalidIndex)
{
    EXPECT_EQ(manager->getShowWidget(-1), nullptr);
    EXPECT_EQ(manager->getShowWidget(999), nullptr);
}

// getShowLunar
TEST_F(TestCalendarManager, GetShowLunar)
{
    // Just verify it doesn't crash; value depends on system locale
    manager->getShowLunar();
}

// updateData
TEST_F(TestCalendarManager, UpdateData)
{
    QSignalSpy spy(manager, &CalendarManager::sigNotifySidebarFirstDayChanged);
    manager->updateData();
    EXPECT_EQ(spy.count(), 1);
}

// weekBeginsChanged
TEST_F(TestCalendarManager, WeekBeginsChanged)
{
    manager->weekBeginsChanged(2);
    EXPECT_EQ(manager->getFirstDayOfWeek(), Qt::Tuesday);
}

// slotGetLunarSuccess
TEST_F(TestCalendarManager, SlotGetLunarSuccess)
{
    TestScheduleWidget widget;
    manager->addShowWidget(&widget);
    EXPECT_NO_FATAL_FAILURE(manager->slotGetLunarSuccess());
    manager->removeShowWidget(&widget);
}

// slotGeneralSettingsUpdate (null settings - stubbed to return null)
TEST_F(TestCalendarManager, SlotGeneralSettingsUpdateNull)
{
    EXPECT_NO_FATAL_FAILURE(manager->slotGeneralSettingsUpdate());
}

// slotDateFormatChanged
TEST_F(TestCalendarManager, SlotDateFormatChanged)
{
    manager->slotDateFormatChanged(1);
    EXPECT_EQ(manager->getDateFormat(), QString("yyyy-M-d"));
}

// slotTimeFormatChanged
TEST_F(TestCalendarManager, SlotTimeFormatChanged)
{
    manager->slotTimeFormatChanged(1);
    EXPECT_EQ(manager->getTimeFormat(), QString("hh:mm"));
}

// setYearBeginAndEndDate (tested indirectly via setSelectDate with year switch)
TEST_F(TestCalendarManager, SetYearBeginAndEndDate)
{
    manager->setSelectDate(QDate(2025, 1, 1), true);
    ShowDateRange range = manager->getShowDateRange();
    EXPECT_EQ(range.showYear, 2025);
}

#include "test_calendarmanage.moc"
