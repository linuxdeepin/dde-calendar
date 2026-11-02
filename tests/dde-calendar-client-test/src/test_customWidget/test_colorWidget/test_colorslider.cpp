// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_colorslider.h"

#include <QPaintEvent>

TEST_F(test_colorslider, constructor)
{
    EXPECT_NE(mWidget, nullptr);
    EXPECT_EQ(mWidget->minimum(), 0);
    EXPECT_EQ(mWidget->maximum(), 359);
}

TEST_F(test_colorslider, getColor_hi0)
{
    EXPECT_EQ(mWidget->getColor(0, 0, 0).name(), "#000000");
    QColor c = mWidget->getColor(10, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorslider, getColor_hi1)
{
    QColor c = mWidget->getColor(70, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorslider, getColor_hi2)
{
    QColor c = mWidget->getColor(130, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorslider, getColor_hi3)
{
    QColor c = mWidget->getColor(190, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorslider, getColor_hi4)
{
    QColor c = mWidget->getColor(250, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorslider, getColor_hi5)
{
    QColor c = mWidget->getColor(310, 0.5, 0.5);
    EXPECT_TRUE(c.isValid());
}

TEST_F(test_colorslider, getColor_q_negative)
{
    QColor c = mWidget->getColor(55, 100, 1);
    EXPECT_FALSE(c.isValid());
}

TEST_F(test_colorslider, getColor_boundary)
{
    EXPECT_EQ(mWidget->getColor(0, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(60, 0, 0).name(), "#000000");
    EXPECT_EQ(mWidget->getColor(71, 0, 0).name(), "#000000");
}

TEST_F(test_colorslider, paintEvent)
{
    QPaintEvent e(mWidget->rect());
    mWidget->paintEvent(&e);
}
