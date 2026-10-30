// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthWidget/monthwindow.h"
#include "scheduledatamanage.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QDate>
#include <QDateTime>
#include <QResizeEvent>

// ===== CMonthWindow =====

TEST(MonthWindowTest, Constructor)
{
    CMonthWindow *window = new CMonthWindow();
    ASSERT_NE(nullptr, window);
    EXPECT_NE(nullptr, window->m_monthView);
    EXPECT_NE(nullptr, window->m_monthDayView);
    EXPECT_NE(nullptr, window->m_today);
    EXPECT_NE(nullptr, window->m_YearLabel);
    EXPECT_NE(nullptr, window->m_YearLunarLabel);
    EXPECT_NE(nullptr, window->m_gridWidget);
    EXPECT_NE(nullptr, window->m_tMainLayout);
    delete window;
}

TEST(MonthWindowTest, Destructor)
{
    CMonthWindow *window = new CMonthWindow();
    delete window;
    SUCCEED();
}

TEST(MonthWindowTest, SetLunarVisibleTrue)
{
    CMonthWindow window;
    window.setLunarVisible(true);
    EXPECT_FALSE(window.m_YearLunarLabel->isHidden());
}

TEST(MonthWindowTest, SetLunarVisibleFalse)
{
    CMonthWindow window;
    window.setLunarVisible(false);
    EXPECT_TRUE(window.m_YearLunarLabel->isHidden());
}

TEST(MonthWindowTest, SetTheMeLight0)
{
    CMonthWindow window;
    window.setTheMe(0);
    SUCCEED();
}

TEST(MonthWindowTest, SetTheMeLight1)
{
    CMonthWindow window;
    window.setTheMe(1);
    SUCCEED();
}

TEST(MonthWindowTest, SetTheMeDark)
{
    CMonthWindow window;
    window.setTheMe(2);
    SUCCEED();
}

TEST(MonthWindowTest, SetYearData)
{
    CMonthWindow window;
    window.setYearData();
    EXPECT_FALSE(window.m_YearLabel->text().isEmpty());
}

TEST(MonthWindowTest, UpdateShowDateWithBar)
{
    CMonthWindow window;
    window.updateShowDate(true);
    SUCCEED();
}

TEST(MonthWindowTest, UpdateShowDateWithoutBar)
{
    CMonthWindow window;
    window.updateShowDate(false);
    SUCCEED();
}

TEST(MonthWindowTest, SetCurrentDateTime)
{
    CMonthWindow window;
    QDateTime dt(QDate(2026, 6, 15), QTime(10, 30, 0));
    window.setCurrentDateTime(dt);
    EXPECT_EQ(dt, window.getCurrendDateTime());
}

TEST(MonthWindowTest, UpdateShowSchedule)
{
    CMonthWindow window;
    window.updateShowDate(true);
    window.updateShowSchedule();
    SUCCEED();
}

TEST(MonthWindowTest, UpdateShowLunar)
{
    CMonthWindow window;
    window.updateShowDate(true);
    window.updateShowLunar();
    SUCCEED();
}

TEST(MonthWindowTest, UpdateSearchScheduleInfo)
{
    CMonthWindow window;
    window.updateSearchScheduleInfo();
    SUCCEED();
}

TEST(MonthWindowTest, SetSelectSearchScheduleInfo)
{
    CMonthWindow window;
    DSchedule::Ptr info(new DSchedule());
    info->setSummary("Test Schedule");
    window.setSelectSearchScheduleInfo(info);
    SUCCEED();
}

TEST(MonthWindowTest, SetSearchWFlagTrue)
{
    CMonthWindow window;
    window.setSearchWFlag(true);
    EXPECT_TRUE(window.m_searchFlag);
}

TEST(MonthWindowTest, SetSearchWFlagFalse)
{
    CMonthWindow window;
    window.setSearchWFlag(false);
    EXPECT_FALSE(window.m_searchFlag);
}

TEST(MonthWindowTest, DeleteselectSchedule)
{
    CMonthWindow window;
    window.deleteselectSchedule();
    SUCCEED();
}

TEST(MonthWindowTest, PreviousMonth)
{
    CMonthWindow window;
    window.previousMonth();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, NextMonth)
{
    CMonthWindow window;
    window.nextMonth();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlideMonthNext)
{
    CMonthWindow window;
    window.slideMonth(true);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlideMonthPrev)
{
    CMonthWindow window;
    window.slideMonth(false);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotScheduleHide)
{
    CMonthWindow window;
    window.slotScheduleHide();
    SUCCEED();
}

TEST(MonthWindowTest, SlotAngleDeltaPositive)
{
    CMonthWindow window;
    window.slotAngleDelta(120);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotAngleDeltaNegative)
{
    CMonthWindow window;
    window.slotAngleDelta(-120);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotViewSelectDate)
{
    CMonthWindow window;
    QDate date(2026, 6, 15);
    window.slotViewSelectDate(date);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotSwitchPrePage)
{
    CMonthWindow window;
    window.slotSwitchPrePage();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotSwitchNextPage)
{
    CMonthWindow window;
    window.slotSwitchNextPage();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, ResizeEvent)
{
    CMonthWindow window;
    window.resize(800, 600);
    QResizeEvent event(QSize(800, 600), QSize(400, 300));
    QCoreApplication::sendEvent(&window, &event);
    SUCCEED();
}

TEST(MonthWindowTest, SlotToday)
{
    CMonthWindow window;
    window.slottoday();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotSetSelectDate)
{
    CMonthWindow window;
    QDate date(2026, 7, 20);
    window.slotSetSelectDate(date);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}
