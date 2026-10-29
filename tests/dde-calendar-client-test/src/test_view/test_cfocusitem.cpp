// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cfocusitem.h"

#include <QGraphicsScene>
#include <QGraphicsRectItem>

// Constructor initializes member defaults
TEST_F(test_cfocusitem, Constructor)
{
    CFocusItem *item = new CFocusItem();
    ASSERT_NE(item, nullptr);
    EXPECT_FALSE(item->getItemFocus());
    EXPECT_EQ(item->getItemType(), CFocusItem::CITEM);
    delete item;
}

// setNextFocusItem / setNextItemFocusAndGetNextItem with null next item
TEST_F(test_cfocusitem, SetNextFocusItem_NullNext)
{
    m_item->setNextFocusItem(nullptr);
    m_item->setItemFocus(true);
    CFocusItem *result = m_item->setNextItemFocusAndGetNextItem();
    EXPECT_EQ(result, nullptr);
    EXPECT_TRUE(m_item->getItemFocus());
}

// setNextFocusItem / setNextItemFocusAndGetNextItem with a valid next item
TEST_F(test_cfocusitem, SetNextFocusItem_ValidNext)
{
    CFocusItem *nextItem = new CFocusItem();
    m_scene->addItem(nextItem);

    m_item->setNextFocusItem(nextItem);
    m_item->setItemFocus(true);
    EXPECT_TRUE(m_item->getItemFocus());

    CFocusItem *result = m_item->setNextItemFocusAndGetNextItem();
    EXPECT_EQ(result, nextItem);
    EXPECT_FALSE(m_item->getItemFocus());
    EXPECT_TRUE(nextItem->getItemFocus());
}

// setItemFocus true and getItemFocus
TEST_F(test_cfocusitem, SetItemFocus_True)
{
    m_item->setItemFocus(true);
    EXPECT_TRUE(m_item->getItemFocus());
}

// setItemFocus false and getItemFocus
TEST_F(test_cfocusitem, SetItemFocus_False)
{
    m_item->setItemFocus(true);
    m_item->setItemFocus(false);
    EXPECT_FALSE(m_item->getItemFocus());
}

// setItemType / getItemType for all enum values
TEST_F(test_cfocusitem, SetItemType_AllValues)
{
    m_item->setItemType(CFocusItem::CBACK);
    EXPECT_EQ(m_item->getItemType(), CFocusItem::CBACK);

    m_item->setItemType(CFocusItem::CITEM);
    EXPECT_EQ(m_item->getItemType(), CFocusItem::CITEM);

    m_item->setItemType(CFocusItem::COTHER);
    EXPECT_EQ(m_item->getItemType(), CFocusItem::COTHER);
}

// getItemFocus default is false
TEST_F(test_cfocusitem, GetItemFocus_Default)
{
    EXPECT_FALSE(m_item->getItemFocus());
}

// getItemType default is CITEM
TEST_F(test_cfocusitem, GetItemType_Default)
{
    EXPECT_EQ(m_item->getItemType(), CFocusItem::CITEM);
}

// getSystemActiveColor returns a valid color
TEST_F(test_cfocusitem, GetSystemActiveColor)
{
    QColor color = m_item->getSystemActiveColor();
    EXPECT_TRUE(color.isValid());
}

// setDate / getDate
TEST_F(test_cfocusitem, SetGetDate)
{
    QDate testDate(2025, 6, 15);
    m_item->setDate(testDate);
    EXPECT_EQ(m_item->getDate(), testDate);
}

// setDate with default date
TEST_F(test_cfocusitem, GetDate_Default)
{
    EXPECT_EQ(m_item->getDate(), QDate());
}
