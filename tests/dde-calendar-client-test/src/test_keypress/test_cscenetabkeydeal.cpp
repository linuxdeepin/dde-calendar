// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cscenetabkeydeal.h"

#include "../third-party_stub/stub.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "view/graphicsItem/cscenebackgrounditem.h"
#include "../dialog_stub.h"

#include <QGraphicsView>

test_CSceneTabKeyDeal::test_CSceneTabKeyDeal()
{
}

void test_CSceneTabKeyDeal::SetUp()
{
    delete SceneCurrentItem;
    SceneCurrentItem = new TestBackgroundItem();
    calendarDDialogExecStub(stub.getStub());
    scene = QSharedPointer<CGraphicsScene>(new CGraphicsScene());
    view = QSharedPointer<QGraphicsView>(new QGraphicsView());
    view->setScene(scene.get());
    tabDeal = QSharedPointer<CSceneTabKeyDeal>(new CSceneTabKeyDeal(scene.get()));

    // Reset globals
    setCurrentFocusItemCalled = false;
    setCurrentFocusItemArg = nullptr;
    nextFocusItemResult = nullptr;
    focusItemType = CFocusItem::CBACK;
    itemFocus = false;
    itemDate = QDate::currentDate();
}

void test_CSceneTabKeyDeal::TearDown()
{
}

// Constructor sets the correct key
TEST_F(test_CSceneTabKeyDeal, Constructor)
{
    EXPECT_EQ(tabDeal->getKey(), Qt::Key_Tab);
}

// focusItemDeal: nextItem is null → returns false, setItemFocus(false)
TEST_F(test_CSceneTabKeyDeal, focusItemDeal_NextItemNull)
{
    nextFocusItemResult = nullptr;

    bool result = tabDeal->dealEvent();
    EXPECT_FALSE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    EXPECT_EQ(setCurrentFocusItemArg, nullptr);
    EXPECT_FALSE(itemFocus);
}

// focusItemDeal: nextItem not null, focusItem type is CBACK → returns true
TEST_F(test_CSceneTabKeyDeal, focusItemDeal_NextItemNotNull_Back)
{
    TestBackgroundItem nextItem;
    nextFocusItemResult = &nextItem;
    focusItemType = CFocusItem::CBACK;

    bool result = tabDeal->dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    EXPECT_EQ(setCurrentFocusItemArg, &nextItem);
}

// focusItemDeal: nextItem not null, focusItem type is CITEM → centerOn called, returns true
TEST_F(test_CSceneTabKeyDeal, focusItemDeal_NextItemNotNull_Item)
{
    TestBackgroundItem nextItem;
    nextFocusItemResult = &nextItem;
    focusItemType = CFocusItem::CITEM;

    bool result = tabDeal->dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    EXPECT_EQ(setCurrentFocusItemArg, &nextItem);
}

// focusItemDeal: nextItem not null, focusItem type is COTHER → centerOn called, returns true
TEST_F(test_CSceneTabKeyDeal, focusItemDeal_NextItemNotNull_Other)
{
    TestBackgroundItem nextItem;
    nextFocusItemResult = &nextItem;
    focusItemType = CFocusItem::COTHER;

    bool result = tabDeal->dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    EXPECT_EQ(setCurrentFocusItemArg, &nextItem);
}
