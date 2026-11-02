// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cschedulebasewidget.h"
#include "../third-party_stub/stub.h"

#include <QTest>
#include <QDate>
#include <QDateTime>

test_cschedulebasewidget::test_cschedulebasewidget()
{
}

test_cschedulebasewidget::~test_cschedulebasewidget()
{
}

void test_cschedulebasewidget::SetUp()
{
    mWidget = new TestScheduleBaseWidget();
}

void test_cschedulebasewidget::TearDown()
{
    delete mWidget;
    mWidget = nullptr;
}

// Constructor / Destructor — exercised via SetUp/TearDown
TEST_F(test_cschedulebasewidget, ConstructorDestructor)
{
    ASSERT_NE(mWidget, nullptr);
}

// initConnect — private, accessible via -fno-access-control
TEST_F(test_cschedulebasewidget, initConnect)
{
    mWidget->initConnect();
    SUCCEED();
}

// setSelectDate (static) — valid and invalid dates
TEST_F(test_cschedulebasewidget, setSelectDateStatic)
{
    QDate validDate(2024, 6, 15);
    bool result = TestScheduleBaseWidget::setSelectDate(validDate, false);
    // invalid date (year out of range)
    QDate invalidDate(1800, 1, 1);
    TestScheduleBaseWidget::setSelectDate(invalidDate, false);
    QDate invalidDate2(2200, 1, 1);
    TestScheduleBaseWidget::setSelectDate(invalidDate2, false);
    SUCCEED();
}

// setSelectDate (non-static)
TEST_F(test_cschedulebasewidget, setSelectDateNonStatic)
{
    QDate date(2024, 6, 15);
    mWidget->setSelectDate(date, mWidget);
    SUCCEED();
}

// getSelectDate
TEST_F(test_cschedulebasewidget, getSelectDate)
{
    QDate date(2024, 6, 15);
    TestScheduleBaseWidget::setSelectDate(date, false);
    QDate result = TestScheduleBaseWidget::getSelectDate();
    EXPECT_EQ(result, date);
}

// setCurrentDateTime
TEST_F(test_cschedulebasewidget, setCurrentDateTime)
{
    QDateTime dt(QDate(2024, 6, 15), QTime(10, 30, 0));
    mWidget->setCurrentDateTime(dt);
    SUCCEED();
}

// getCurrendDateTime
TEST_F(test_cschedulebasewidget, getCurrendDateTime)
{
    QDateTime dt(QDate(2024, 6, 15), QTime(10, 30, 0));
    mWidget->setCurrentDateTime(dt);
    QDateTime result = mWidget->getCurrendDateTime();
    EXPECT_EQ(result.date(), dt.date());
}

// getShowLunar
TEST_F(test_cschedulebasewidget, getShowLunar)
{
    bool result = TestScheduleBaseWidget::getShowLunar();
    // just verify it returns a bool without crashing
    Q_UNUSED(result)
    SUCCEED();
}

// updateData
TEST_F(test_cschedulebasewidget, updateData)
{
    mWidget->updateData();
    SUCCEED();
}

// ensureLunarDataLoaded
TEST_F(test_cschedulebasewidget, ensureLunarDataLoaded)
{
    QDate startDate(2024, 1, 1);
    QDate endDate(2024, 12, 31);
    mWidget->ensureLunarDataLoaded(startDate, endDate);
    SUCCEED();
}

// updateDBusData (static)
TEST_F(test_cschedulebasewidget, updateDBusData)
{
    TestScheduleBaseWidget::updateDBusData();
    SUCCEED();
}

// updateSearchScheduleInfo (virtual, base impl is empty)
TEST_F(test_cschedulebasewidget, updateSearchScheduleInfo)
{
    mWidget->updateSearchScheduleInfo();
    SUCCEED();
}

// deleteselectSchedule (virtual, base impl is empty)
TEST_F(test_cschedulebasewidget, deleteselectSchedule)
{
    mWidget->deleteselectSchedule();
    SUCCEED();
}

// getLunarInfo (protected, accessible via -fno-access-control)
TEST_F(test_cschedulebasewidget, getLunarInfo)
{
    QDate date(2024, 6, 15);
    TestScheduleBaseWidget::setSelectDate(date, false);
    auto info = mWidget->getLunarInfo();
    Q_UNUSED(info)
    SUCCEED();
}

// slotScheduleUpdate
TEST_F(test_cschedulebasewidget, slotScheduleUpdate)
{
    mWidget->slotScheduleUpdate();
    SUCCEED();
}

// slotSearchedScheduleUpdate
TEST_F(test_cschedulebasewidget, slotSearchedScheduleUpdate)
{
    mWidget->slotSearchedScheduleUpdate();
    SUCCEED();
}

// slotLunarInfoReady
TEST_F(test_cschedulebasewidget, slotLunarInfoReady)
{
    QDate startDate(2024, 1, 1);
    QDate endDate(2024, 12, 31);
    mWidget->slotLunarInfoReady(startDate, endDate);
    SUCCEED();
}

// slotFestivalInfoReady
TEST_F(test_cschedulebasewidget, slotFestivalInfoReady)
{
    mWidget->slotFestivalInfoReady();
    SUCCEED();
}
