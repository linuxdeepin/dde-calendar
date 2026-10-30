// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QColor>
#include <QFont>

#include "opencalendarwidget.h"
#include "globaldef.h"

class TestOpenCalendarWidget : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_widget = new OpenCalendarWidget();
        m_widget->resize(300, OPENCALENDAR_WDIGET_HEIGHT);
    }

    void TearDown() override
    {
        delete m_widget;
    }

    OpenCalendarWidget *m_widget = nullptr;
};

TEST_F(TestOpenCalendarWidget, Constructor)
{
    OpenCalendarWidget *w = new OpenCalendarWidget();
    EXPECT_EQ(w->height(), OPENCALENDAR_WDIGET_HEIGHT);
    delete w;
}

TEST_F(TestOpenCalendarWidget, GetSetTitleColor)
{
    QColor testColor(100, 150, 200);
    m_widget->setTitleColor(testColor);
    EXPECT_EQ(m_widget->TitleColor(), testColor);

    QColor redColor(Qt::red);
    m_widget->setTitleColor(redColor);
    EXPECT_EQ(m_widget->TitleColor(), redColor);
}

TEST_F(TestOpenCalendarWidget, TitleFont)
{
    QFont font = m_widget->TitleFont();
    EXPECT_EQ(font.pixelSize(), 12);
}

TEST_F(TestOpenCalendarWidget, SetTheMeType0)
{
    m_widget->setTheMe(0);
    QColor expected(0, 0, 0);
    expected.setAlphaF(0.4);
    EXPECT_EQ(m_widget->TitleColor(), expected);
}

TEST_F(TestOpenCalendarWidget, SetTheMeType1)
{
    m_widget->setTheMe(1);
    QColor expected(0, 0, 0);
    expected.setAlphaF(0.4);
    EXPECT_EQ(m_widget->TitleColor(), expected);
}

TEST_F(TestOpenCalendarWidget, SetTheMeType2)
{
    m_widget->setTheMe(2);
    QColor expected(255, 255, 255);
    expected.setAlphaF(0.4);
    EXPECT_EQ(m_widget->TitleColor(), expected);
}

TEST_F(TestOpenCalendarWidget, SetScheduleCount)
{
    m_widget->setScheduleCount(5);
    m_widget->setScheduleCount(0);
    m_widget->setScheduleCount(100);
    SUCCEED();
}

TEST_F(TestOpenCalendarWidget, PaintEvent)
{
    m_widget->setScheduleCount(3);
    m_widget->show();
    QTest::qWait(50);
    m_widget->repaint();
    QTest::qWait(50);
    m_widget->hide();
    SUCCEED();
}

TEST_F(TestOpenCalendarWidget, MousePressEventLeftButton)
{
    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(10, 5),
                      QPointF(10, 5),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
    m_widget->mousePressEvent(&event);
    SUCCEED();
}

TEST_F(TestOpenCalendarWidget, MousePressEventRightButton)
{
    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(10, 5),
                      QPointF(10, 5),
                      Qt::RightButton,
                      Qt::RightButton,
                      Qt::NoModifier);
    m_widget->mousePressEvent(&event);
    SUCCEED();
}
