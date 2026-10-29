// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cmonthschedulenumitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>

TEST_F(test_cmonthschedulenumItem, Constructor_01)
{
    EXPECT_NE(mItem, nullptr);
    EXPECT_EQ(mItem->m_num, 0);
    EXPECT_EQ(mItem->m_SizeType, DFontSizeManager::T8);
}

TEST_F(test_cmonthschedulenumItem, setColor_01)
{
    QColor color1("#000000");
    QColor color2("#111111");
    mItem->setColor(color1, color2);
    EXPECT_EQ(mItem->m_color1, color1);
    EXPECT_EQ(mItem->m_color2, color2);
}

TEST_F(test_cmonthschedulenumItem, setText_01)
{
    QColor color("#000000");
    QFont font;
    mItem->setText(color, font);
    EXPECT_EQ(mItem->m_textcolor, color);
    EXPECT_EQ(mItem->m_font, font);
}

TEST_F(test_cmonthschedulenumItem, setSizeType_01)
{
    mItem->setSizeType(DFontSizeManager::T5);
    EXPECT_EQ(mItem->m_SizeType, DFontSizeManager::T5);
}

TEST_F(test_cmonthschedulenumItem, setData_01)
{
    mItem->setData(4);
    EXPECT_EQ(mItem->m_num, 4);
}

TEST_F(test_cmonthschedulenumItem, setData_02)
{
    mItem->setData(0);
    EXPECT_EQ(mItem->m_num, 0);
}

TEST_F(test_cmonthschedulenumItem, paint_01)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

TEST_F(test_cmonthschedulenumItem, paint_02_Focus)
{
    mItem->m_isFocus = true;
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

TEST_F(test_cmonthschedulenumItem, paint_03_WithNum)
{
    mItem->setData(10);
    mItem->setRect(QRectF(0, 0, 200, 30));
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}
