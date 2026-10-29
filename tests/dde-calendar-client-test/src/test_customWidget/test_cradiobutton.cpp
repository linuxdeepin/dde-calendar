// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cradiobutton.h"

#include <QPaintEvent>

test_cradiobutton::test_cradiobutton()
{

}

// Constructor: verify object creation without crash
TEST_F(test_cradiobutton, constructor)
{
    CRadioButton *btn = new CRadioButton();
    EXPECT_NE(btn, nullptr);
    delete btn;
}

// Constructor with parent
TEST_F(test_cradiobutton, constructor_with_parent)
{
    QWidget parent;
    CRadioButton *btn = new CRadioButton(&parent);
    EXPECT_EQ(btn->parent(), &parent);
    delete btn;
}

// setColor: set a color and verify getColor returns it
TEST_F(test_cradiobutton, setColor_001)
{
    QColor color("#000000");
    mWidget->setColor(color);
    EXPECT_EQ(mWidget->getColor(), color);
}

// setColor: set a different color
TEST_F(test_cradiobutton, setColor_002)
{
    QColor color(Qt::red);
    mWidget->setColor(color);
    EXPECT_EQ(mWidget->getColor(), color);
}

// getColor: default color should be invalid (default-constructed QColor)
TEST_F(test_cradiobutton, getColor_default)
{
    QColor defaultColor = mWidget->getColor();
    EXPECT_FALSE(defaultColor.isValid());
}

// paintEvent: unchecked state
TEST_F(test_cradiobutton, paintEvent_unchecked)
{
    QColor color("#000000");
    mWidget->setColor(color);
    mWidget->setChecked(false);
    QPaintEvent *e = new QPaintEvent(QRect(0, 0, 20, 20));
    mWidget->paintEvent(e);
    delete e;
}

// paintEvent: checked state
TEST_F(test_cradiobutton, paintEvent_checked)
{
    QColor color("#FF0000");
    mWidget->setColor(color);
    mWidget->setChecked(true);
    QPaintEvent *e = new QPaintEvent(QRect(0, 0, 20, 20));
    mWidget->paintEvent(e);
    delete e;
}
