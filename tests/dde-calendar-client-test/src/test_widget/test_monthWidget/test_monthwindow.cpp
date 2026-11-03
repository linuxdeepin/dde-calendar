// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthWidget/monthwindow.h"
#include "scheduledatamanage.h"

// 无头（offscreen/minimal）平台下弹窗/焦点/动画类用例会崩溃或卡死，跳过；
// 真实图形会话（如 CI 桌面环境）下正常执行。
static bool headlessQpaPlatform()
{
    const QByteArray platform = qEnvironmentVariable("QT_QPA_PLATFORM").toUtf8();
    return platform == "offscreen" || platform == "minimal";
}

#include <gtest/gtest.h>
#include <QApplication>
#include <QDate>
#include <QDateTime>
#include <QResizeEvent>

// ===== CMonthWindow =====

TEST(MonthWindowTest, Constructor)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
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
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow *window = new CMonthWindow();
    delete window;
    SUCCEED();
}

TEST(MonthWindowTest, SetLunarVisibleTrue)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setLunarVisible(true);
    EXPECT_FALSE(window.m_YearLunarLabel->isHidden());
}

TEST(MonthWindowTest, SetLunarVisibleFalse)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setLunarVisible(false);
    EXPECT_TRUE(window.m_YearLunarLabel->isHidden());
}

TEST(MonthWindowTest, SetTheMeLight0)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setTheMe(0);
    SUCCEED();
}

TEST(MonthWindowTest, SetTheMeLight1)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setTheMe(1);
    SUCCEED();
}

TEST(MonthWindowTest, SetTheMeDark)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setTheMe(2);
    SUCCEED();
}

TEST(MonthWindowTest, SetYearData)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setYearData();
    EXPECT_FALSE(window.m_YearLabel->text().isEmpty());
}

TEST(MonthWindowTest, UpdateShowDateWithBar)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.updateShowDate(true);
    SUCCEED();
}

TEST(MonthWindowTest, UpdateShowDateWithoutBar)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.updateShowDate(false);
    SUCCEED();
}

TEST(MonthWindowTest, SetCurrentDateTime)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    QDateTime dt(QDate(2026, 6, 15), QTime(10, 30, 0));
    window.setCurrentDateTime(dt);
    EXPECT_EQ(dt, window.getCurrendDateTime());
}

TEST(MonthWindowTest, UpdateShowSchedule)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.updateShowDate(true);
    window.updateShowSchedule();
    SUCCEED();
}

TEST(MonthWindowTest, UpdateShowLunar)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.updateShowDate(true);
    window.updateShowLunar();
    SUCCEED();
}

TEST(MonthWindowTest, UpdateSearchScheduleInfo)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.updateSearchScheduleInfo();
    SUCCEED();
}

TEST(MonthWindowTest, SetSelectSearchScheduleInfo)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    DSchedule::Ptr info(new DSchedule());
    info->setSummary("Test Schedule");
    window.setSelectSearchScheduleInfo(info);
    SUCCEED();
}

TEST(MonthWindowTest, SetSearchWFlagTrue)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setSearchWFlag(true);
    EXPECT_TRUE(window.m_searchFlag);
}

TEST(MonthWindowTest, SetSearchWFlagFalse)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.setSearchWFlag(false);
    EXPECT_FALSE(window.m_searchFlag);
}

TEST(MonthWindowTest, DeleteselectSchedule)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (popup/focus/animation)";
    }
    CMonthWindow window;
    window.deleteselectSchedule();
    SUCCEED();
}

TEST(MonthWindowTest, PreviousMonth)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (paint/interaction)";
    }
    CMonthWindow window;
    window.previousMonth();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, NextMonth)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (paint/interaction)";
    }
    CMonthWindow window;
    window.nextMonth();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlideMonthNext)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (paint/interaction)";
    }
    CMonthWindow window;
    window.slideMonth(true);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlideMonthPrev)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (paint/interaction)";
    }
    CMonthWindow window;
    window.slideMonth(false);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotScheduleHide)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.slotScheduleHide();
    SUCCEED();
}

TEST(MonthWindowTest, SlotAngleDeltaPositive)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (paint/interaction)";
    }
    CMonthWindow window;
    window.slotAngleDelta(120);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotAngleDeltaNegative)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (paint/interaction)";
    }
    CMonthWindow window;
    window.slotAngleDelta(-120);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotViewSelectDate)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    QDate date(2026, 6, 15);
    window.slotViewSelectDate(date);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotSwitchPrePage)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.slotSwitchPrePage();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotSwitchNextPage)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.slotSwitchNextPage();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, ResizeEvent)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.resize(800, 600);
    QResizeEvent event(QSize(800, 600), QSize(400, 300));
    QCoreApplication::sendEvent(&window, &event);
    SUCCEED();
}

TEST(MonthWindowTest, SlotToday)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    window.slottoday();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}

TEST(MonthWindowTest, SlotSetSelectDate)
{
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (window/paint/interaction)";
    }
    CMonthWindow window;
    QDate date(2026, 7, 20);
    window.slotSetSelectDate(date);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    SUCCEED();
}
