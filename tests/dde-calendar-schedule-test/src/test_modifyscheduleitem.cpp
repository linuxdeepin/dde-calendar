// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>

#include "modifyscheduleitem.h"
#include "globaldef.h"
#include "dschedule.h"

class TestModifyScheduleItem : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_item = new modifyScheduleItem();
        m_item->resize(400, 48);

        m_schedule = DSchedule::Ptr(new DSchedule());
        m_schedule->setAllDay(false);
        m_schedule->setDtStart(QDateTime(QDate(2026, 1, 1), QTime(9, 0, 0)));
        m_schedule->setDtEnd(QDateTime(QDate(2026, 1, 1), QTime(10, 30, 0)));
        m_item->setScheduleInfo(m_schedule);
        m_item->setShowDate(QDate(2026, 1, 1));
        m_item->setScheduleBeginTime(QDateTime(QDate(2026, 1, 1), QTime(9, 0, 0)));
        m_item->setScheduleEndTime(QDateTime(QDate(2026, 1, 1), QTime(10, 30, 0)));
        m_item->setTitleContent("Test Schedule Title");
    }

    void TearDown() override
    {
        delete m_item;
    }

    modifyScheduleItem *m_item = nullptr;
    DSchedule::Ptr m_schedule;
};

TEST_F(TestModifyScheduleItem, Constructor)
{
    modifyScheduleItem *item = new modifyScheduleItem();
    EXPECT_EQ(item->height(), MODIFY_ITEM_HEIGHT);
    EXPECT_EQ(item->getIndex(), 1);
    delete item;
}

TEST_F(TestModifyScheduleItem, GetSetIndex)
{
    m_item->setIndex(42);
    EXPECT_EQ(m_item->getIndex(), 42);

    m_item->setIndex(0);
    EXPECT_EQ(m_item->getIndex(), 0);

    m_item->setIndex(-1);
    EXPECT_EQ(m_item->getIndex(), -1);
}

TEST_F(TestModifyScheduleItem, GetSetLineColor)
{
    QColor testColor(255, 0, 0);
    m_item->setLineColor(testColor);
    EXPECT_EQ(m_item->LineColor(), testColor);

    QColor defaultColor("#000000");
    m_item->setLineColor(defaultColor);
    EXPECT_EQ(m_item->LineColor(), defaultColor);
}

TEST_F(TestModifyScheduleItem, DrawItem)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_item->DrawItem(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, DrawIndex)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_item->setIndex(5);
    m_item->drawIndex(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, DrawLine)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_item->drawLine(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, DrawTitleContent)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_item->drawTitleContent(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, DrawScheduleColorLine)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_item->drawScheduleColorLine(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, DrawTimeNotAllDay)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_schedule->setAllDay(false);
    m_item->drawTime(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, DrawTimeAllDay)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_schedule->setAllDay(true);
    m_item->drawTime(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, DrawDate)
{
    QImage image(400, 48, QImage::Format_ARGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    m_item->drawDate(painter);
    painter.end();
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, MousePressEventLeftButton)
{
    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(10, 10),
                      QPointF(10, 10),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
    m_item->mousePressEvent(&event);
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, MousePressEventRightButton)
{
    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(10, 10),
                      QPointF(10, 10),
                      Qt::RightButton,
                      Qt::RightButton,
                      Qt::NoModifier);
    m_item->mousePressEvent(&event);
    SUCCEED();
}

TEST_F(TestModifyScheduleItem, SetTheMeType0)
{
    m_item->setTheMe(0);
    QColor expected("#000000");
    expected.setAlphaF(0.1);
    EXPECT_EQ(m_item->LineColor(), expected);
}

TEST_F(TestModifyScheduleItem, SetTheMeType1)
{
    m_item->setTheMe(1);
    QColor expected("#000000");
    expected.setAlphaF(0.1);
    EXPECT_EQ(m_item->LineColor(), expected);
}

TEST_F(TestModifyScheduleItem, SetTheMeType2)
{
    m_item->setTheMe(2);
    QColor expected("#FFFFFF");
    expected.setAlphaF(0.1);
    EXPECT_EQ(m_item->LineColor(), expected);
}
