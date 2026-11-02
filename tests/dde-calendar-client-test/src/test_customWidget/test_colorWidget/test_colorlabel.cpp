// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_colorlabel.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QSignalSpy>

TEST_F(test_colorlabel, constructor)
{
    EXPECT_NE(mWidget, nullptr);
    EXPECT_EQ(mWidget->m_hue, 0);
    EXPECT_FALSE(mWidget->m_pressed);
    EXPECT_FALSE(mWidget->m_entered);
}

TEST_F(test_colorlabel, getColor_hi0)
{
    EXPECT_EQ(mWidget->getColor(0, 0, 0).name(), "#000000");
    QColor c = mWidget->getColor(10, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorlabel, getColor_hi1)
{
    QColor c = mWidget->getColor(70, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorlabel, getColor_hi2)
{
    QColor c = mWidget->getColor(130, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorlabel, getColor_hi3)
{
    QColor c = mWidget->getColor(190, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorlabel, getColor_hi4)
{
    QColor c = mWidget->getColor(250, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorlabel, getColor_hi5)
{
    QColor c = mWidget->getColor(310, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorlabel, getColor_boundary)
{
    EXPECT_EQ(mWidget->getColor(0, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(60, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(120, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(180, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(241, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(305, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(360, 0, 0).name(), "#000000");
}

TEST_F(test_colorlabel, setHue)
{
    mWidget->setHue(1);
    EXPECT_EQ(mWidget->m_hue, 1);
    mWidget->setHue(5);
    EXPECT_EQ(mWidget->m_hue, 5);
    mWidget->setHue(100);
    EXPECT_EQ(mWidget->m_hue, 100);
}

TEST_F(test_colorlabel, pickColor_valid_pos_not_picked)
{
    QSignalSpy spy(mWidget, &ColorLabel::signalPreViewColor);
    mWidget->pickColor(QPoint(10, 10), false);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(test_colorlabel, pickColor_valid_pos_picked)
{
    QSignalSpy spy(mWidget, &ColorLabel::signalpickedColor);
    mWidget->pickColor(QPoint(10, 10), true);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(test_colorlabel, pickColor_invalid_pos)
{
    QSignalSpy spyPreview(mWidget, &ColorLabel::signalPreViewColor);
    QSignalSpy spyPicked(mWidget, &ColorLabel::signalpickedColor);
    mWidget->pickColor(QPoint(200, 200), false);
    EXPECT_EQ(spyPreview.count(), 0);
    EXPECT_EQ(spyPicked.count(), 0);
}

TEST_F(test_colorlabel, paintEvent_entered)
{
    mWidget->m_entered = true;
    QPaintEvent e(mWidget->rect());
    mWidget->paintEvent(&e);
}

TEST_F(test_colorlabel, paintEvent_not_entered)
{
    mWidget->m_entered = false;
    QPaintEvent e(mWidget->rect());
    mWidget->paintEvent(&e);
}

TEST_F(test_colorlabel, mousePressEvent)
{
    QMouseEvent e(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mousePressEvent(&e);
    EXPECT_TRUE(mWidget->m_pressed);
}

TEST_F(test_colorlabel, mouseMoveEvent_inside)
{
    QMouseEvent e(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10),
                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseMoveEvent(&e);
    EXPECT_TRUE(mWidget->m_entered);
}

TEST_F(test_colorlabel, mouseMoveEvent_outside)
{
    QMouseEvent e(QEvent::MouseMove, QPointF(200, 200), QPointF(200, 200),
                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseMoveEvent(&e);
    EXPECT_FALSE(mWidget->m_entered);
}

TEST_F(test_colorlabel, mouseReleaseEvent_pressed_inside)
{
    mWidget->m_pressed = true;
    QSignalSpy spy(mWidget, &ColorLabel::clicked);
    QMouseEvent e(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseReleaseEvent(&e);
    EXPECT_FALSE(mWidget->m_pressed);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(test_colorlabel, mouseReleaseEvent_pressed_outside)
{
    mWidget->m_pressed = true;
    QSignalSpy spy(mWidget, &ColorLabel::clicked);
    QMouseEvent e(QEvent::MouseButtonRelease, QPointF(200, 200), QPointF(200, 200),
                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseReleaseEvent(&e);
    EXPECT_FALSE(mWidget->m_pressed);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(test_colorlabel, mouseReleaseEvent_not_pressed)
{
    mWidget->m_pressed = false;
    QSignalSpy spy(mWidget, &ColorLabel::clicked);
    QMouseEvent e(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseReleaseEvent(&e);
    EXPECT_FALSE(mWidget->m_pressed);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(test_colorlabel, pickColorCursor)
{
    QCursor cursor = mWidget->pickColorCursor();
    EXPECT_FALSE(cursor.pixmap().isNull());
}
