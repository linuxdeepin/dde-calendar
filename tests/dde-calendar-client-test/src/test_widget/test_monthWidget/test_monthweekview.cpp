// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthWidget/monthweekview.h"
#include "scheduledatamanage.h"
#include "constants.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QDate>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>

// ===== CMonthWeekView =====

TEST(MonthWeekViewTest, Constructor)
{
    CMonthWeekView view;
    EXPECT_EQ(7, view.m_weekRect.size());
    for (int i = 0; i < 7; ++i) {
        EXPECT_NE(nullptr, view.m_weekRect.at(i));
    }
}

TEST(MonthWeekViewTest, Destructor)
{
    CMonthWeekView *view = new CMonthWeekView();
    delete view;
    SUCCEED();
}

TEST(MonthWeekViewTest, SetFirstDay)
{
    CMonthWeekView view;
    view.setFirstDay(Qt::Monday);
    EXPECT_EQ(Qt::Monday, view.m_firstWeek);

    view.setFirstDay(Qt::Wednesday);
    EXPECT_EQ(Qt::Wednesday, view.m_firstWeek);
}

TEST(MonthWeekViewTest, SetFirstDaySunday)
{
    CMonthWeekView view;
    view.setFirstDay(Qt::Sunday);
    EXPECT_EQ(Qt::Sunday, view.m_firstWeek);
}

TEST(MonthWeekViewTest, SetTheMeLight)
{
    CMonthWeekView view;
    view.setTheMe(0);
    EXPECT_EQ(QColor("#E6EEF2"), view.m_backgroundColor);

    view.setTheMe(1);
    EXPECT_EQ(QColor("#E6EEF2"), view.m_backgroundColor);
}

TEST(MonthWeekViewTest, SetTheMeDark)
{
    CMonthWeekView view;
    view.setTheMe(2);
    QColor expected("#82AEC1");
    expected.setAlphaF(0.10);
    EXPECT_EQ(expected, view.m_backgroundColor);
}

TEST(MonthWeekViewTest, UpdateWeek)
{
    CMonthWeekView view;
    view.m_firstWeek = Qt::Monday;
    view.m_currentWeek = Qt::Wednesday;
    view.updateWeek();

    bool foundCurrent = false;
    for (int i = 0; i < 7; ++i) {
        int weekNum = (Qt::Monday + i) % 7;
        Qt::DayOfWeek expected = static_cast<Qt::DayOfWeek>(weekNum == 0 ? 7 : weekNum);
        EXPECT_EQ(expected, view.m_weekRect.at(i)->m_showWeek);
        if (expected == Qt::Wednesday) {
            foundCurrent = true;
            EXPECT_TRUE(view.m_weekRect.at(i)->m_showLine);
        } else {
            EXPECT_FALSE(view.m_weekRect.at(i)->m_showLine);
        }
    }
    EXPECT_TRUE(foundCurrent);
}

TEST(MonthWeekViewTest, SetCurrentDate)
{
    CMonthWeekView view;
    QDate date(2026, 3, 15);
    view.setCurrentDate(date);
    EXPECT_EQ(static_cast<Qt::DayOfWeek>(date.dayOfWeek()), view.m_currentWeek);
}

TEST(MonthWeekViewTest, ResizeEvent)
{
    CMonthWeekView view;
    view.resize(700, 50);
    QResizeEvent event(QSize(700, 50), QSize(0, 0));
    QCoreApplication::sendEvent(&view, &event);

    qreal weekRectWidth = 700.0 / 7;
    for (int i = 0; i < 7; ++i) {
        QRectF expected(i * weekRectWidth, 0, weekRectWidth, 50);
        EXPECT_EQ(expected, view.m_weekRect.at(i)->m_rectF);
    }
}

TEST(MonthWeekViewTest, PaintEvent)
{
    CMonthWeekView view;
    view.resize(700, 50);
    view.show();
    QCoreApplication::processEvents();
    QPaintEvent event(QRect(0, 0, 700, 50));
    QCoreApplication::sendEvent(&view, &event);
    SUCCEED();
}

// ===== WeekRect =====

TEST(WeekRectTest, Constructor)
{
    WeekRect rect;
    EXPECT_FALSE(rect.m_showLine);
    EXPECT_EQ(QFont::Medium, rect.m_font.weight());
    EXPECT_EQ(DDECalendar::FontSizeSixteen, rect.m_font.pixelSize());
}

TEST(WeekRectTest, SetWeek)
{
    WeekRect rect;
    rect.setWeek(Qt::Monday, true);
    EXPECT_EQ(Qt::Monday, rect.m_showWeek);
    EXPECT_TRUE(rect.m_showLine);

    QLocale locale;
    EXPECT_EQ(locale.dayName(Qt::Monday, QLocale::ShortFormat), rect.m_weekStr);
}

TEST(WeekRectTest, SetWeekNoLine)
{
    WeekRect rect;
    rect.setWeek(Qt::Friday, false);
    EXPECT_EQ(Qt::Friday, rect.m_showWeek);
    EXPECT_FALSE(rect.m_showLine);
}

TEST(WeekRectTest, SetRect)
{
    WeekRect rect;
    QRectF testRect(10.0, 20.0, 100.0, 50.0);
    rect.setRect(testRect);
    EXPECT_EQ(testRect, rect.m_rectF);
}

TEST(WeekRectTest, PaintRectWeekday)
{
    WeekRect rect;
    rect.setWeek(Qt::Monday, false);
    rect.setRect(QRectF(0, 0, 100, 50));
    rect.setTheMe(0);

    QPixmap pixmap(100, 50);
    QPainter painter(&pixmap);
    rect.paintRect(painter);
    painter.end();
    SUCCEED();
}

TEST(WeekRectTest, PaintRectWeekend)
{
    WeekRect rect;
    rect.setWeek(Qt::Sunday, false);
    rect.setRect(QRectF(0, 0, 100, 50));
    rect.setTheMe(0);

    QPixmap pixmap(100, 50);
    QPainter painter(&pixmap);
    rect.paintRect(painter);
    painter.end();
    SUCCEED();
}

TEST(WeekRectTest, PaintRectWithLine)
{
    WeekRect rect;
    rect.setWeek(Qt::Wednesday, true);
    rect.setRect(QRectF(0, 0, 100, 50));
    rect.setTheMe(1);

    QPixmap pixmap(100, 50);
    QPainter painter(&pixmap);
    rect.paintRect(painter);
    painter.end();
    SUCCEED();
}

TEST(WeekRectTest, SetTheMeLight)
{
    WeekRect rect;
    rect.setTheMe(0);
    EXPECT_EQ(QColor(0, 0, 0, 128), rect.m_testColor);
}

TEST(WeekRectTest, SetTheMeLight1)
{
    WeekRect rect;
    rect.setTheMe(1);
    EXPECT_EQ(QColor(0, 0, 0, 128), rect.m_testColor);
}

TEST(WeekRectTest, SetTheMeDark)
{
    WeekRect rect;
    rect.setTheMe(2);
    EXPECT_EQ(QColor(255, 255, 255, 128), rect.m_testColor);
}
