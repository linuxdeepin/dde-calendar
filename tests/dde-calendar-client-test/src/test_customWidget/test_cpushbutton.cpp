// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cpushbutton.h"
#include <QMouseEvent>
#include <QPaintEvent>
#include <QSignalSpy>

test_cpushbutton::test_cpushbutton(QObject *parent) : QObject(parent)
{

}

// CPushButton::CPushButton(QWidget *parent)
TEST_F(test_cpushbutton, Constructor)
{
    EXPECT_NE(mWidget->m_textLabel, nullptr);
    EXPECT_NE(mWidget->m_iconButton, nullptr);
    EXPECT_FALSE(mWidget->m_Highlighted);
    EXPECT_FALSE(mWidget->m_pressed);
}

// void CPushButton::setHighlight(bool status)
TEST_F(test_cpushbutton, setHighlight)
{
    mWidget->setHighlight(true);
    EXPECT_TRUE(mWidget->m_Highlighted);

    mWidget->setHighlight(false);
    EXPECT_FALSE(mWidget->m_Highlighted);
}

// void CPushButton::setHighlight(bool status) — same value, early return
TEST_F(test_cpushbutton, setHighlightSameValue)
{
    mWidget->m_Highlighted = true;
    mWidget->setHighlight(true);
    EXPECT_TRUE(mWidget->m_Highlighted);
}

// bool CPushButton::isHighlight()
TEST_F(test_cpushbutton, isHighlight)
{
    mWidget->setHighlight(true);
    EXPECT_TRUE(mWidget->isHighlight());

    mWidget->setHighlight(false);
    EXPECT_FALSE(mWidget->isHighlight());
}

// void CPushButton::mousePressEvent(QMouseEvent *event)
TEST_F(test_cpushbutton, mousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mousePressEvent(&event);
    EXPECT_TRUE(mWidget->m_pressed);
}

// void CPushButton::mouseReleaseEvent(QMouseEvent *event) — inside rect, emit clicked
TEST_F(test_cpushbutton, mouseReleaseEventInside)
{
    QSignalSpy spy(mWidget, &CPushButton::clicked);
    mWidget->m_pressed = true;
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseReleaseEvent(&event);
    EXPECT_FALSE(mWidget->m_pressed);
    EXPECT_EQ(spy.count(), 1);
}

// void CPushButton::mouseReleaseEvent(QMouseEvent *event) — outside rect, no emit
TEST_F(test_cpushbutton, mouseReleaseEventOutside)
{
    QSignalSpy spy(mWidget, &CPushButton::clicked);
    mWidget->m_pressed = true;
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(500, 500), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseReleaseEvent(&event);
    EXPECT_FALSE(mWidget->m_pressed);
    EXPECT_EQ(spy.count(), 0);
}

// void CPushButton::mouseReleaseEvent(QMouseEvent *event) — m_pressed false
TEST_F(test_cpushbutton, mouseReleaseEventNotPressed)
{
    QSignalSpy spy(mWidget, &CPushButton::clicked);
    mWidget->m_pressed = false;
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mWidget->mouseReleaseEvent(&event);
    EXPECT_FALSE(mWidget->m_pressed);
    EXPECT_EQ(spy.count(), 0);
}

// void CPushButton::paintEvent(QPaintEvent *event) — highlighted
TEST_F(test_cpushbutton, paintEventHighlighted)
{
    mWidget->setHighlight(true);
    mWidget->show();
    QPaintEvent event(QRect(0, 0, 200, 34));
    mWidget->paintEvent(&event);
    EXPECT_TRUE(mWidget->m_Highlighted);
}

// void CPushButton::paintEvent(QPaintEvent *event) — not highlighted
TEST_F(test_cpushbutton, paintEventNotHighlighted)
{
    mWidget->setHighlight(false);
    mWidget->show();
    QPaintEvent event(QRect(0, 0, 200, 34));
    mWidget->paintEvent(&event);
    EXPECT_FALSE(mWidget->m_Highlighted);
}
