// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cscenebackgrounditem.h"
#include <QGraphicsRectItem>

// ---- Constructor ----
// Forward declaration: compareItemData is defined in cscenebackgrounditem.cpp but not exported in header
bool compareItemData(const CFocusItem *itemfirst, const CFocusItem *itemsecond);

TEST_F(test_cscenebackgrounditem, Constructor_01)
{
    EXPECT_NE(mItem, nullptr);
    EXPECT_EQ(mItem->getBackgroundNum(), 0);
    EXPECT_EQ(mItem->getItemOfView(), CSceneBackgroundItem::OnMonthView);
    EXPECT_EQ(mItem->getLeftItem(), nullptr);
    EXPECT_EQ(mItem->getRightItem(), nullptr);
    EXPECT_EQ(mItem->getUpItem(), nullptr);
    EXPECT_EQ(mItem->getDownItem(), nullptr);
    EXPECT_EQ(mItem->getShowItemCount(), 0);
}

TEST_F(test_cscenebackgrounditem, Constructor_02_WeekView)
{
    CSceneBackgroundItem *item = new CSceneBackgroundItem(CSceneBackgroundItem::OnWeekView);
    EXPECT_EQ(item->getItemOfView(), CSceneBackgroundItem::OnWeekView);
    delete item;
}

TEST_F(test_cscenebackgrounditem, Constructor_03_DayView)
{
    CSceneBackgroundItem *item = new CSceneBackgroundItem(CSceneBackgroundItem::OnDayView);
    EXPECT_EQ(item->getItemOfView(), CSceneBackgroundItem::OnDayView);
    delete item;
}

// ---- setBackgroundNum / getBackgroundNum ----

TEST_F(test_cscenebackgrounditem, setBackgroundNum_01)
{
    mItem->setBackgroundNum(5);
    EXPECT_EQ(mItem->getBackgroundNum(), 5);
}

TEST_F(test_cscenebackgrounditem, setBackgroundNum_02_Zero)
{
    mItem->setBackgroundNum(0);
    EXPECT_EQ(mItem->getBackgroundNum(), 0);
}

TEST_F(test_cscenebackgrounditem, setBackgroundNum_03_Negative)
{
    mItem->setBackgroundNum(-1);
    EXPECT_EQ(mItem->getBackgroundNum(), -1);
}

// ---- getShowItemCount ----

TEST_F(test_cscenebackgrounditem, getShowItemCount_01)
{
    EXPECT_EQ(mItem->getShowItemCount(), 0);
}

// ---- setItemFocus (2 branches) ----

TEST_F(test_cscenebackgrounditem, setItemFocus_01_NoChildIndex)
{
    mItem->m_showItemIndex = -1;
    mItem->setItemFocus(true);
    EXPECT_TRUE(mItem->getItemFocus());
    mItem->setItemFocus(false);
    EXPECT_FALSE(mItem->getItemFocus());
}

TEST_F(test_cscenebackgrounditem, setItemFocus_02_WithChildIndex)
{
    CFocusItem *child = new CFocusItem();
    mScene->addItem(child);
    child->setRect(QRectF(0, 0, 50, 50));
    mItem->m_item.append(child);
    mItem->m_showItemIndex = 0;
    mItem->setItemFocus(true);
    EXPECT_TRUE(child->getItemFocus());
    mItem->setItemFocus(false);
    EXPECT_FALSE(child->getItemFocus());
}

// ---- initState ----

TEST_F(test_cscenebackgrounditem, initState_01)
{
    mItem->m_showItemIndex = -1;
    mItem->setItemFocus(true);
    mItem->initState();
    EXPECT_FALSE(mItem->getItemFocus());
    EXPECT_EQ(mItem->m_showItemIndex, -1);
}

TEST_F(test_cscenebackgrounditem, initState_02_WithChild)
{
    CFocusItem *child = new CFocusItem();
    mScene->addItem(child);
    mItem->m_item.append(child);
    mItem->m_showItemIndex = 0;
    child->setItemFocus(true);
    mItem->initState();
    EXPECT_FALSE(child->getItemFocus());
    EXPECT_EQ(mItem->m_showItemIndex, -1);
}

// ---- getFocusItem (2 branches) ----

TEST_F(test_cscenebackgrounditem, getFocusItem_01_Self)
{
    mItem->m_showItemIndex = -1;
    EXPECT_EQ(mItem->getFocusItem(), mItem);
}

TEST_F(test_cscenebackgrounditem, getFocusItem_02_Child)
{
    CFocusItem *child = new CFocusItem();
    mScene->addItem(child);
    mItem->m_item.append(child);
    mItem->m_showItemIndex = 0;
    EXPECT_EQ(mItem->getFocusItem(), child);
}

// ---- updateCurrentItemShow (2 branches) ----

TEST_F(test_cscenebackgrounditem, updateCurrentItemShow_01_NoItems)
{
    mItem->m_showItemIndex = 0;
    mItem->updateCurrentItemShow();
    EXPECT_EQ(mItem->m_showItemIndex, -1);
    EXPECT_TRUE(mItem->getItemFocus());
}

TEST_F(test_cscenebackgrounditem, updateCurrentItemShow_02_WithItems)
{
    CFocusItem *child = new CFocusItem();
    mScene->addItem(child);
    mItem->m_item.append(child);
    mItem->m_showItemIndex = 0;
    mItem->updateCurrentItemShow();
    EXPECT_TRUE(child->getItemFocus());
}

// ---- setNextItemFocusAndGetNextItem (3 branches) ----

TEST_F(test_cscenebackgrounditem, setNextItemFocusAndGetNextItem_01_NoItems)
{
    mItem->m_showItemIndex = -1;
    mItem->setItemFocus(true);
    CFocusItem *result = mItem->setNextItemFocusAndGetNextItem();
    EXPECT_EQ(result, nullptr);
}

TEST_F(test_cscenebackgrounditem, setNextItemFocusAndGetNextItem_02_NormalAdvance)
{
    CFocusItem *child1 = new CFocusItem();
    CFocusItem *child2 = new CFocusItem();
    mScene->addItem(child1);
    mScene->addItem(child2);
    mItem->m_item.append(child1);
    mItem->m_item.append(child2);
    mItem->m_showItemIndex = -1;
    mItem->setItemFocus(true);
    mItem->setNextItemFocusAndGetNextItem();
    EXPECT_EQ(mItem->m_showItemIndex, 0);
    EXPECT_TRUE(child1->getItemFocus());
    EXPECT_FALSE(mItem->getItemFocus());
}

TEST_F(test_cscenebackgrounditem, setNextItemFocusAndGetNextItem_03_AtLastItem)
{
    CFocusItem *child1 = new CFocusItem();
    CFocusItem *child2 = new CFocusItem();
    mScene->addItem(child1);
    mScene->addItem(child2);
    mItem->m_item.append(child1);
    mItem->m_item.append(child2);
    mItem->m_showItemIndex = 1;
    child2->setItemFocus(true);
    mItem->setNextItemFocusAndGetNextItem();
    EXPECT_EQ(mItem->m_showItemIndex, -1);
    EXPECT_FALSE(child2->getItemFocus());
}

// ---- compareItemData (free function) ----

TEST_F(test_cscenebackgrounditem, compareItemData_01_SameRect)
{
    CFocusItem *item1 = new CFocusItem();
    CFocusItem *item2 = new CFocusItem();
    item1->setRect(QRectF(0, 0, 50, 50));
    item2->setRect(QRectF(0, 0, 50, 50));
    EXPECT_FALSE(compareItemData(item1, item2));
    delete item1;
    delete item2;
}

TEST_F(test_cscenebackgrounditem, compareItemData_02_FirstAbove)
{
    CFocusItem *item1 = new CFocusItem();
    CFocusItem *item2 = new CFocusItem();
    item1->setRect(QRectF(0, 0, 50, 50));
    item2->setRect(QRectF(0, 10, 50, 50));
    EXPECT_TRUE(compareItemData(item1, item2));
    delete item1;
    delete item2;
}

TEST_F(test_cscenebackgrounditem, compareItemData_03_FirstBelow)
{
    CFocusItem *item1 = new CFocusItem();
    CFocusItem *item2 = new CFocusItem();
    item1->setRect(QRectF(0, 10, 50, 50));
    item2->setRect(QRectF(0, 0, 50, 50));
    EXPECT_FALSE(compareItemData(item1, item2));
    delete item1;
    delete item2;
}

TEST_F(test_cscenebackgrounditem, compareItemData_04_SameY_FirstLeft)
{
    CFocusItem *item1 = new CFocusItem();
    CFocusItem *item2 = new CFocusItem();
    item1->setRect(QRectF(0, 0, 50, 50));
    item2->setRect(QRectF(10, 0, 50, 50));
    EXPECT_TRUE(compareItemData(item1, item2));
    delete item1;
    delete item2;
}

TEST_F(test_cscenebackgrounditem, compareItemData_05_SameY_FirstRight)
{
    CFocusItem *item1 = new CFocusItem();
    CFocusItem *item2 = new CFocusItem();
    item1->setRect(QRectF(10, 0, 50, 50));
    item2->setRect(QRectF(0, 0, 50, 50));
    EXPECT_FALSE(compareItemData(item1, item2));
    delete item1;
    delete item2;
}

// ---- updateShowItem (needs scene) ----

TEST_F(test_cscenebackgrounditem, updateShowItem_01)
{
    CFocusItem *child1 = new CFocusItem();
    mScene->addItem(child1);
    child1->setRect(QRectF(10, 10, 30, 30));
    mItem->updateShowItem();
    EXPECT_GE(mItem->getShowItemCount(), 0);
}

// ---- Neighbor get/set ----

TEST_F(test_cscenebackgrounditem, setLeftItem_01)
{
    CSceneBackgroundItem *left = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    mItem->setLeftItem(left);
    EXPECT_EQ(mItem->getLeftItem(), left);
    delete left;
}

TEST_F(test_cscenebackgrounditem, setRightItem_01)
{
    CSceneBackgroundItem *right = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    mItem->setRightItem(right);
    EXPECT_EQ(mItem->getRightItem(), right);
    delete right;
}

TEST_F(test_cscenebackgrounditem, setUpItem_01)
{
    CSceneBackgroundItem *up = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    mItem->setUpItem(up);
    EXPECT_EQ(mItem->getUpItem(), up);
    delete up;
}

TEST_F(test_cscenebackgrounditem, setDownItem_01)
{
    CSceneBackgroundItem *down = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    mItem->setDownItem(down);
    EXPECT_EQ(mItem->getDownItem(), down);
    delete down;
}

TEST_F(test_cscenebackgrounditem, setLeftItem_02_Null)
{
    mItem->setLeftItem(nullptr);
    EXPECT_EQ(mItem->getLeftItem(), nullptr);
}
