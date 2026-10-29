// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cmonthscheduleitem.h"
#include <QPainter>
#include <QPixmap>

// ---- Constructor ----

TEST_F(test_cmonthscheduleitem, Constructor_01)
{
    EXPECT_NE(mItem, nullptr);
    EXPECT_EQ(mItem->m_pos, QPoint(13, 5));
}

TEST_F(test_cmonthscheduleitem, Constructor_02_WithEditType)
{
    CMonthScheduleItem *item = new CMonthScheduleItem(QRect(0, 0, 100, 50), nullptr, 1);
    EXPECT_NE(item, nullptr);
    EXPECT_EQ(item->m_pos, QPoint(13, 5));
    delete item;
}

// ---- Destructor ----

TEST_F(test_cmonthscheduleitem, Destructor_01)
{
    CMonthScheduleItem *item = new CMonthScheduleItem(QRect(0, 0, 100, 50));
    delete item;
    SUCCEED();
}

// ---- getPixmap ----

TEST_F(test_cmonthscheduleitem, getPixmap_01)
{
    QPixmap pixmap = mItem->getPixmap();
    EXPECT_FALSE(pixmap.isNull());
}

// ---- paintBackground (multiple branches) ----

TEST_F(test_cmonthscheduleitem, paintBackground_01_Default)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mItem->m_vHoverflag = false;
    mItem->m_vSelectflag = false;
    mItem->m_vHighflag = false;
    mItem->paintBackground(&painter, rect, false);
}

TEST_F(test_cmonthscheduleitem, paintBackground_02_AsPixMap)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mItem->paintBackground(&painter, rect, true);
}

TEST_F(test_cmonthscheduleitem, paintBackground_03_SelectFlag)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mItem->m_vSelectflag = true;
    mItem->m_vHoverflag = false;
    mItem->paintBackground(&painter, rect, false);
    mItem->m_vSelectflag = false;
}

TEST_F(test_cmonthscheduleitem, paintBackground_04_HoverFlag)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mItem->m_vHoverflag = true;
    mItem->m_vSelectflag = false;
    mItem->paintBackground(&painter, rect, false);
    mItem->m_vHoverflag = false;
}

TEST_F(test_cmonthscheduleitem, paintBackground_05_HighFlag)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mItem->m_vHighflag = true;
    mItem->m_vSelectflag = false;
    mItem->m_vHoverflag = false;
    mItem->paintBackground(&painter, rect, false);
    mItem->m_vHighflag = false;
}

TEST_F(test_cmonthscheduleitem, paintBackground_06_PressedSameSchedule)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    CMonthScheduleItem::setPressSchedule(mSchedule);
    mItem->paintBackground(&painter, rect, false);
    CMonthScheduleItem::setPressSchedule(DSchedule::Ptr());
}

TEST_F(test_cmonthscheduleitem, paintBackground_07_Focus)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mItem->m_isFocus = true;
    mItem->paintBackground(&painter, rect, false);
    mItem->m_isFocus = false;
}

TEST_F(test_cmonthscheduleitem, paintBackground_08_LongTitle)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mSchedule->setSummary("This is a very long schedule title that should be truncated when displayed");
    mItem->paintBackground(&painter, rect, false);
}

TEST_F(test_cmonthscheduleitem, paintBackground_09_HoverAndSelect)
{
    QPixmap pixmap(200, 30);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QRectF rect(0, 0, 200, 30);
    mItem->m_vHoverflag = true;
    mItem->m_vSelectflag = true;
    mItem->paintBackground(&painter, rect, false);
    mItem->m_vHoverflag = false;
    mItem->m_vSelectflag = false;
}
