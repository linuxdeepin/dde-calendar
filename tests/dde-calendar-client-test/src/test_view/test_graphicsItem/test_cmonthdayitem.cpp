// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cmonthdayitem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>

// Constructor creates a valid item
TEST_F(test_cmonthdayitem, Constructor)
{
    CMonthDayItem *item = new CMonthDayItem();
    ASSERT_NE(item, nullptr);
    delete item;
}

// Destructor does not crash
TEST_F(test_cmonthdayitem, Destructor)
{
    CMonthDayItem *item = new CMonthDayItem();
    delete item;
    SUCCEED();
}

// setLunar sets the lunar info
TEST_F(test_cmonthdayitem, SetLunar)
{
    mItem->setLunar("初一");
    EXPECT_EQ(mItem->m_DayLunar, QString("初一"));
}

// setStatus sets the holiday status
TEST_F(test_cmonthdayitem, SetStatus_None)
{
    mItem->setStatus(CMonthDayItem::H_NONE);
    EXPECT_EQ(mItem->m_DayStatus, CMonthDayItem::H_NONE);
}

TEST_F(test_cmonthdayitem, SetStatus_Work)
{
    mItem->setStatus(CMonthDayItem::H_WORK);
    EXPECT_EQ(mItem->m_DayStatus, CMonthDayItem::H_WORK);
}

TEST_F(test_cmonthdayitem, SetStatus_Rest)
{
    mItem->setStatus(CMonthDayItem::H_REST);
    EXPECT_EQ(mItem->m_DayStatus, CMonthDayItem::H_REST);
}

// setCurrentMonth sets the flag
TEST_F(test_cmonthdayitem, SetCurrentMonth_True)
{
    mItem->setCurrentMonth(true);
    EXPECT_TRUE(mItem->m_IsCurrentMonth);
}

TEST_F(test_cmonthdayitem, SetCurrentMonth_False)
{
    mItem->setCurrentMonth(true);
    mItem->setCurrentMonth(false);
    EXPECT_FALSE(mItem->m_IsCurrentMonth);
}

// setTheMe with light theme (type 0)
TEST_F(test_cmonthdayitem, SetTheMe_Light0)
{
    mItem->setTheMe(0);
    EXPECT_EQ(mItem->m_themetype, 0);
    EXPECT_EQ(mItem->m_dayNumColor, QColor(0, 0, 0, 204));
    EXPECT_EQ(mItem->m_fillColor, QColor(Qt::white));
}

// setTheMe with light theme (type 1)
TEST_F(test_cmonthdayitem, SetTheMe_Light1)
{
    mItem->setTheMe(1);
    EXPECT_EQ(mItem->m_themetype, 1);
    EXPECT_EQ(mItem->m_dayNumColor, QColor(0, 0, 0, 204));
}

// setTheMe with dark theme (type 2)
TEST_F(test_cmonthdayitem, SetTheMe_Dark)
{
    mItem->setTheMe(2);
    EXPECT_EQ(mItem->m_themetype, 2);
    EXPECT_EQ(mItem->m_dayNumColor, QColor(255, 255, 255, 204));
}

// paint with lunar not visible, normal day
TEST_F(test_cmonthdayitem, Paint_LunarNotVisible)
{
    CMonthDayItem::m_LunarVisible = false;
    mItem->setCurrentMonth(true);

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint with lunar visible, rest day
TEST_F(test_cmonthdayitem, Paint_LunarVisible_RestDay)
{
    CMonthDayItem::m_LunarVisible = true;
    mItem->setCurrentMonth(true);
    mItem->setStatus(CMonthDayItem::H_REST);
    mItem->setLunar("休息");

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint with lunar visible, work day
TEST_F(test_cmonthdayitem, Paint_LunarVisible_WorkDay)
{
    CMonthDayItem::m_LunarVisible = true;
    mItem->setCurrentMonth(true);
    mItem->setStatus(CMonthDayItem::H_WORK);
    mItem->setLunar("补班");

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint with lunar visible, none day
TEST_F(test_cmonthdayitem, Paint_LunarVisible_NoneDay)
{
    CMonthDayItem::m_LunarVisible = true;
    mItem->setCurrentMonth(true);
    mItem->setStatus(CMonthDayItem::H_NONE);
    mItem->setLunar("初一");

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint not current month
TEST_F(test_cmonthdayitem, Paint_NotCurrentMonth)
{
    CMonthDayItem::m_LunarVisible = false;
    mItem->setCurrentMonth(false);

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint with focus and background num 35 (bottom-left corner)
TEST_F(test_cmonthdayitem, Paint_Focus_BottomLeft)
{
    CMonthDayItem::m_LunarVisible = false;
    mItem->setCurrentMonth(true);
    mItem->setBackgroundNum(35);
    mItem->setItemFocus(true);

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint with focus and background num 41 (bottom-right corner)
TEST_F(test_cmonthdayitem, Paint_Focus_BottomRight)
{
    CMonthDayItem::m_LunarVisible = false;
    mItem->setCurrentMonth(true);
    mItem->setBackgroundNum(41);
    mItem->setItemFocus(true);

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint with focus and normal background num
TEST_F(test_cmonthdayitem, Paint_Focus_Normal)
{
    CMonthDayItem::m_LunarVisible = false;
    mItem->setCurrentMonth(true);
    mItem->setBackgroundNum(20);
    mItem->setItemFocus(true);

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}

// paint current date highlight
TEST_F(test_cmonthdayitem, Paint_CurrentDate)
{
    CMonthDayItem::m_LunarVisible = false;
    mItem->setCurrentMonth(true);
    mItem->setDate(QDate::currentDate());

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    mItem->paint(&painter, &option);
}
