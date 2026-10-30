// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QPaintEvent>
#include "test_cweekwidget.h"

test_cweekwidget::test_cweekwidget()
{
    mWidget = new CWeekWidget();
}

test_cweekwidget::~test_cweekwidget()
{
    delete mWidget;
    mWidget = nullptr;
}

// CWeekWidget(QWidget *parent) — constructor
TEST_F(test_cweekwidget, constructor)
{
    EXPECT_EQ(mWidget->focusPolicy(), Qt::NoFocus);
    EXPECT_GE(mWidget->minimumHeight(), 10);
}

// void CWeekWidget::setFirstDay(Qt::DayOfWeek first)
TEST_F(test_cweekwidget, setFirstDay)
{
    mWidget->setFirstDay(Qt::Wednesday);
    EXPECT_EQ(mWidget->m_firstDay, Qt::Wednesday);
    EXPECT_TRUE(mWidget->m_autoFirstDay);
}

// void CWeekWidget::setAutoFirstDay(bool is)
TEST_F(test_cweekwidget, setAutoFirstDay)
{
    mWidget->setAutoFirstDay(false);
    EXPECT_FALSE(mWidget->m_autoFirstDay);
    mWidget->setAutoFirstDay(true);
    EXPECT_TRUE(mWidget->m_autoFirstDay);
}

// void CWeekWidget::setAutoFontSizeByWindow(bool is)
TEST_F(test_cweekwidget, setAutoFontSizeByWindow)
{
    mWidget->setAutoFontSizeByWindow(true);
    EXPECT_TRUE(mWidget->m_autoFontSizeByWindow);
    mWidget->setAutoFontSizeByWindow(false);
    EXPECT_FALSE(mWidget->m_autoFontSizeByWindow);
}

// void CWeekWidget::setTextColor(const QColor &color)
TEST_F(test_cweekwidget, setTextColor)
{
    QColor color(255, 0, 0);
    mWidget->setTextColor(color);
    EXPECT_EQ(mWidget->m_textColor, color);
}

// void CWeekWidget::paintEvent(QPaintEvent *event) — autoFontSizeByWindow true
TEST_F(test_cweekwidget, paintEvent_autoFont)
{
    mWidget->resize(200, 40);
    mWidget->setAutoFontSizeByWindow(true);
    QPaintEvent e(QRect(0, 0, 200, 40));
    mWidget->paintEvent(&e);
}

// void CWeekWidget::paintEvent(QPaintEvent *event) — autoFontSizeByWindow false
TEST_F(test_cweekwidget, paintEvent_fixedFont)
{
    mWidget->resize(200, 40);
    mWidget->setAutoFontSizeByWindow(false);
    QPaintEvent e(QRect(0, 0, 200, 40));
    mWidget->paintEvent(&e);
}

// void CWeekWidget::paintEvent(QPaintEvent *event) — autoFirstDay true
TEST_F(test_cweekwidget, paintEvent_autoFirstDay)
{
    mWidget->resize(200, 40);
    mWidget->setAutoFirstDay(true);
    QPaintEvent e(QRect(0, 0, 200, 40));
    mWidget->paintEvent(&e);
}
