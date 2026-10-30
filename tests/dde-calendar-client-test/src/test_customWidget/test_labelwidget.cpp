// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QPaintEvent>
#include "test_labelwidget.h"

test_labelwidget::test_labelwidget()
{
    mWidget = new LabelWidget();
}

test_labelwidget::~test_labelwidget()
{
    delete mWidget;
    mWidget = nullptr;
}

// LabelWidget(QWidget *parent) — constructor sets TabFocus
TEST_F(test_labelwidget, constructor)
{
    EXPECT_EQ(mWidget->focusPolicy(), Qt::FocusPolicy::TabFocus);
}

// void LabelWidget::paintEvent(QPaintEvent *ev) — no focus branch
TEST_F(test_labelwidget, paintEvent_nofocus)
{
    mWidget->resize(100, 30);
    QPaintEvent e(QRect(0, 0, 100, 30));
    mWidget->paintEvent(&e);
}

// void LabelWidget::paintEvent(QPaintEvent *ev) — hasFocus branch
TEST_F(test_labelwidget, paintEvent_hasfocus)
{
    mWidget->resize(100, 30);
    mWidget->setFocusPolicy(Qt::StrongFocus);
    mWidget->setFocus();
    QPaintEvent e(QRect(0, 0, 100, 30));
    mWidget->paintEvent(&e);
}
