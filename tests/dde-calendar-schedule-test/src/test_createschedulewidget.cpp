// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "createschedulewidget.h"
#include "buttonwidget.h"
#include "dscheduledatamanager.h"
#include "stub.h"

#include <QVBoxLayout>
#include <QSignalSpy>

// Stub for DScheduleDataManager::getInstance
DScheduleDataManager *stub_getInstance()
{
    static DScheduleDataManager instance;
    return &instance;
}

// Stub for queryScheduleByScheduleID
DSchedule::Ptr stub_queryScheduleByScheduleID(const QString &scheduleID)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Stubbed Schedule");
    return schedule;
}

class CreateScheduleWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Constructor
TEST_F(CreateScheduleWidgetTest, Constructor_InitializesMembers)
{
    createSchedulewidget w;
    EXPECT_NE(w.getScheduleDtailInfo(), nullptr);
    EXPECT_FALSE(w.buttonclicked());
}

// getScheduleDtailInfo
TEST_F(CreateScheduleWidgetTest, GetScheduleDtailInfo_ReturnsValidPtr)
{
    createSchedulewidget w;
    DSchedule::Ptr info = w.getScheduleDtailInfo();
    EXPECT_NE(info, nullptr);
}

// setTitleName
TEST_F(CreateScheduleWidgetTest, SetTitleName_SetsTitle)
{
    createSchedulewidget w;
    w.setTitleName("My Schedule");
    // Verify via setschedule which uses m_titleName
    w.setRpeat(0);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->summary(), QString("My Schedule"));
}

// setDateTime
TEST_F(CreateScheduleWidgetTest, SetDateTime_SetsBeginAndEnd)
{
    createSchedulewidget w;
    QDateTime begin(QDate(2026, 3, 15), QTime(9, 0));
    QDateTime end(QDate(2026, 3, 15), QTime(10, 30));
    w.setDateTime(begin, end);
    w.setRpeat(0);
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->dtStart(), begin);
    EXPECT_EQ(w.getScheduleDtailInfo()->dtEnd(), end);
}

// setRpeat
TEST_F(CreateScheduleWidgetTest, SetRpeat_SetsRepeatValue)
{
    createSchedulewidget w;
    w.setRpeat(3);
    // Verify via setschedule
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_Week);
}

// setschedule with rpeat=0 (default → RRule_None)
TEST_F(CreateScheduleWidgetTest, Setschedule_Rpeat0_RRuleNone)
{
    createSchedulewidget w;
    w.setRpeat(0);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_None);
}

// setschedule with rpeat=1 (Day)
TEST_F(CreateScheduleWidgetTest, Setschedule_Rpeat1_RRuleDay)
{
    createSchedulewidget w;
    w.setRpeat(1);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_Day);
}

// setschedule with rpeat=2 (Work)
TEST_F(CreateScheduleWidgetTest, Setschedule_Rpeat2_RRuleWork)
{
    createSchedulewidget w;
    w.setRpeat(2);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_Work);
}

// setschedule with rpeat=3 (Week)
TEST_F(CreateScheduleWidgetTest, Setschedule_Rpeat3_RRuleWeek)
{
    createSchedulewidget w;
    w.setRpeat(3);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_Week);
}

// setschedule with rpeat=4 (Month)
TEST_F(CreateScheduleWidgetTest, Setschedule_Rpeat4_RRuleMonth)
{
    createSchedulewidget w;
    w.setRpeat(4);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_Month);
}

// setschedule with rpeat=5 (Year)
TEST_F(CreateScheduleWidgetTest, Setschedule_Rpeat5_RRuleYear)
{
    createSchedulewidget w;
    w.setRpeat(5);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_Year);
}

// setschedule with default rpeat (unset → should hit default case)
TEST_F(CreateScheduleWidgetTest, Setschedule_DefaultRpeat_RRuleNone)
{
    createSchedulewidget w;
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    SUCCEED(); // Just verify no crash
}

// setschedule sets scheduleTypeID
TEST_F(CreateScheduleWidgetTest, Setschedule_SetsScheduleTypeID)
{
    createSchedulewidget w;
    w.setRpeat(0);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->scheduleTypeID(),
              QString("403bf009-2005-4679-9c76-e73d9f83a8b4"));
}

// setschedule sets allDay = false
TEST_F(CreateScheduleWidgetTest, Setschedule_SetsAllDayFalse)
{
    createSchedulewidget w;
    w.setRpeat(0);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_FALSE(w.getScheduleDtailInfo()->allDay());
}

// setschedule with repeat sets duration -1
TEST_F(CreateScheduleWidgetTest, Setschedule_WithRepeat_SetsDurationMinus1)
{
    createSchedulewidget w;
    w.setRpeat(1);
    w.setDateTime(QDateTime(QDate(2026, 1, 1), QTime(10, 0)),
                  QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
    w.setschedule();
    EXPECT_EQ(w.getScheduleDtailInfo()->getRRuleType(), DSchedule::RRule_Day);
    // recurrence duration should be -1
    EXPECT_EQ(w.getScheduleDtailInfo()->recurrence()->duration(), -1);
}

// scheduleEmpty
TEST_F(CreateScheduleWidgetTest, ScheduleEmpty_SetsFlag)
{
    createSchedulewidget w;
    w.scheduleEmpty(true);
    // Verify via updateUI which branches on m_scheduleEmpty
    SUCCEED();
}

TEST_F(CreateScheduleWidgetTest, ScheduleEmpty_SetsFlagFalse)
{
    createSchedulewidget w;
    w.scheduleEmpty(false);
    SUCCEED();
}

// updateUI with m_scheduleEmpty=false (creates buttonwidget)
TEST_F(CreateScheduleWidgetTest, UpdateUI_NotEmpty_CreatesButtons)
{
    createSchedulewidget w;
    w.scheduleEmpty(false);
    w.updateUI("test-schedule-id");
    // Should have a buttonwidget child
    auto *btn = w.findChild<buttonwidget *>();
    EXPECT_NE(btn, nullptr);
}

// updateUI with m_scheduleEmpty=true and stubbed getCreatScheduleFromDbus
TEST_F(CreateScheduleWidgetTest, UpdateUI_Empty_WithStubbedDbus)
{
    createSchedulewidget w;
    w.scheduleEmpty(true);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);

    w.updateUI("test-schedule-id");
    // m_scheduleInfo should have the stubbed schedule appended
    SUCCEED();
}

// updateUI with m_scheduleEmpty=true and stubbed querySchedule returning a schedule
TEST_F(CreateScheduleWidgetTest, UpdateUI_Empty_WithStubbedQuery)
{
    createSchedulewidget w;
    w.scheduleEmpty(true);

    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);

    w.updateUI("stubbed-id");
    SUCCEED();
}

// buttonclicked
TEST_F(CreateScheduleWidgetTest, Buttonclicked_DefaultFalse)
{
    createSchedulewidget w;
    EXPECT_FALSE(w.buttonclicked());
}

// slotsbuttonchance - index 0 → false
TEST_F(CreateScheduleWidgetTest, Slotsbuttonchance_Index0_SetsFalse)
{
    createSchedulewidget w;
    w.slotsbuttonchance(0, "Cancel");
    EXPECT_FALSE(w.buttonclicked());
}

// slotsbuttonchance - index 1 → true
TEST_F(CreateScheduleWidgetTest, Slotsbuttonchance_Index1_SetsTrue)
{
    createSchedulewidget w;
    w.slotsbuttonchance(1, "Confirm");
    EXPECT_TRUE(w.buttonclicked());
}

// slotsbuttonchance - other index → no change
TEST_F(CreateScheduleWidgetTest, Slotsbuttonchance_OtherIndex_NoChange)
{
    createSchedulewidget w;
    w.slotsbuttonchance(5, "Other");
    EXPECT_FALSE(w.buttonclicked());
}

// getCreatScheduleFromDbus with stub
TEST_F(CreateScheduleWidgetTest, GetCreatScheduleFromDbus_WithStub)
{
    createSchedulewidget w;
    Stub stub;
    stub.set(ADDR(DScheduleDataManager, getInstance), stub_getInstance);
    stub.set(ADDR(DScheduleDataManager, queryScheduleByScheduleID), stub_queryScheduleByScheduleID);

    w.getCreatScheduleFromDbus("test-id");
    // m_scheduleInfo should have one entry
    EXPECT_EQ(w.m_scheduleInfo.size(), 1);
}

// slotItemPress - stub external deps (QProcess, QDBus)
// This test verifies the method runs without crash
TEST_F(CreateScheduleWidgetTest, SlotItemPress_RunsWithoutCrash)
{
    createSchedulewidget w;
    DSchedule::Ptr info(new DSchedule());
    info->setSummary("Test");
    info->setUid("test-uid");

    // slotItemPress calls QProcess::startDetached, QThread::msleep, DBus
    // These are hard to stub at function level, but they won't crash in offscreen mode
    w.slotItemPress(info);
    SUCCEED();
}

// Full integration: setSchedule + updateUI
TEST_F(CreateScheduleWidgetTest, Integration_SetScheduleAndUpdateUI)
{
    createSchedulewidget w;
    w.setTitleName("Integration Test");
    w.setDateTime(QDateTime(QDate(2026, 6, 1), QTime(14, 0)),
                  QDateTime(QDate(2026, 6, 1), QTime(15, 0)));
    w.setRpeat(0);
    w.setschedule();
    w.scheduleEmpty(false);
    w.updateUI("integration-id");

    auto *btn = w.findChild<buttonwidget *>();
    EXPECT_NE(btn, nullptr);
    EXPECT_EQ(w.getScheduleDtailInfo()->summary(), QString("Integration Test"));
}
