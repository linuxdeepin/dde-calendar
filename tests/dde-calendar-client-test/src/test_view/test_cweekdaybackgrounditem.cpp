// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cweekdaybackgrounditem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>

test_CWeekDayBackgroundItem::test_CWeekDayBackgroundItem()
{
}

void test_CWeekDayBackgroundItem::SetUp()
{
    m_scene = new QGraphicsScene();
    m_weekItem = new CWeekDayBackgroundItem();
    m_scene->addItem(m_weekItem);
    m_weekItem->setRect(QRectF(0, 0, 100, 100));
}

void test_CWeekDayBackgroundItem::TearDown()
{
    delete m_scene;
    m_scene = nullptr;
    m_weekItem = nullptr;
}

// Constructor
TEST_F(test_CWeekDayBackgroundItem, Constructor)
{
    EXPECT_FALSE(m_weekItem->drawDividingLine());
    EXPECT_FALSE(m_weekItem->showFocus());
    EXPECT_EQ(m_weekItem->getItemOfView(), CSceneBackgroundItem::OnWeekView);
}

// setTheMe
TEST_F(test_CWeekDayBackgroundItem, setTheMe_Light)
{
    m_weekItem->setTheMe(0);
    SUCCEED();
}

TEST_F(test_CWeekDayBackgroundItem, setTheMe_Light1)
{
    m_weekItem->setTheMe(1);
    SUCCEED();
}

TEST_F(test_CWeekDayBackgroundItem, setTheMe_Dark)
{
    m_weekItem->setTheMe(2);
    SUCCEED();
}

// showFocus / setShowFocus
TEST_F(test_CWeekDayBackgroundItem, ShowFocus)
{
    m_weekItem->setShowFocus(true);
    EXPECT_TRUE(m_weekItem->showFocus());
    m_weekItem->setShowFocus(false);
    EXPECT_FALSE(m_weekItem->showFocus());
}

// drawDividingLine / setDrawDividingLine
TEST_F(test_CWeekDayBackgroundItem, DrawDividingLine)
{
    m_weekItem->setDrawDividingLine(false);
    EXPECT_FALSE(m_weekItem->drawDividingLine());
    m_weekItem->setDrawDividingLine(true);
    EXPECT_TRUE(m_weekItem->drawDividingLine());
}

// hasNextSubItem
TEST_F(test_CWeekDayBackgroundItem, hasNextSubItem_NoItems)
{
    EXPECT_FALSE(m_weekItem->hasNextSubItem());
}

// setItemFocus — delegates to CSceneBackgroundItem::setItemFocus → CFocusItem::setItemFocus
TEST_F(test_CWeekDayBackgroundItem, setItemFocus_BackgroundFocus)
{
    m_weekItem->setShowFocus(true);
    m_weekItem->setItemFocus(true);
    EXPECT_TRUE(m_weekItem->getItemFocus());
    m_weekItem->setItemFocus(false);
    EXPECT_FALSE(m_weekItem->getItemFocus());
}

// setItemFocus — first-time, background doesn't accept focus, no next sub item
TEST_F(test_CWeekDayBackgroundItem, setItemFocus_NoSubItem)
{
    m_weekItem->setShowFocus(false);
    m_weekItem->setItemFocus(true);
    SUCCEED();
}

// updateCurrentItemShow — showItemIndex < 0, does nothing
TEST_F(test_CWeekDayBackgroundItem, updateCurrentItemShow_NoIndex)
{
    m_weekItem->updateCurrentItemShow();
    SUCCEED();
}

// updateCurrentItemShow — showItemIndex >= 0 but no items → sets background focus
TEST_F(test_CWeekDayBackgroundItem, updateCurrentItemShow_NoItemsButIndex)
{
    m_weekItem->m_showItemIndex = 0;
    m_weekItem->updateCurrentItemShow();
    EXPECT_EQ(m_weekItem->m_showItemIndex, -1);
}

// paint — drawDividingLine=false, showFocus=false
TEST_F(test_CWeekDayBackgroundItem, paint_NoDividingLine)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    m_weekItem->paint(&painter, &option);
    SUCCEED();
}

// paint — drawDividingLine=true, backgroundNum != 6, weekday
TEST_F(test_CWeekDayBackgroundItem, paint_DividingLine_Weekday)
{
    m_weekItem->setDrawDividingLine(true);
    m_weekItem->setBackgroundNum(1);
    m_weekItem->setDate(QDate(2026, 10, 28)); // Wednesday
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    m_weekItem->paint(&painter, &option);
    SUCCEED();
}

// paint — drawDividingLine=true, weekend (dayOfWeek > 5)
TEST_F(test_CWeekDayBackgroundItem, paint_DividingLine_Weekend)
{
    m_weekItem->setDrawDividingLine(true);
    m_weekItem->setBackgroundNum(1);
    m_weekItem->setDate(QDate(2026, 10, 31)); // Saturday
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    m_weekItem->paint(&painter, &option);
    SUCCEED();
}

// paint — drawDividingLine=true, backgroundNum == 6 (skip dividing line)
TEST_F(test_CWeekDayBackgroundItem, paint_DividingLine_BackgroundNum6)
{
    m_weekItem->setDrawDividingLine(true);
    m_weekItem->setBackgroundNum(6);
    m_weekItem->setDate(QDate(2026, 10, 28));
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    m_weekItem->paint(&painter, &option);
    SUCCEED();
}

// paint — showFocus=true, has focus
TEST_F(test_CWeekDayBackgroundItem, paint_ShowFocus)
{
    m_weekItem->setShowFocus(true);
    m_weekItem->setItemFocus(true);
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    m_weekItem->paint(&painter, &option);
    SUCCEED();
}
