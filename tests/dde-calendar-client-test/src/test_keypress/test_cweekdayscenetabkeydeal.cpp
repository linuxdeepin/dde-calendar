// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cweekdayscenetabkeydeal.h"

#include "../third-party_stub/stub.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "view/graphicsItem/cscenebackgrounditem.h"
#include "../dialog_stub.h"

#include <QGraphicsView>

test_CWeekDaySceneTabKeyDeal::test_CWeekDaySceneTabKeyDeal()
{
}

void test_CWeekDaySceneTabKeyDeal::SetUp()
{
    delete SceneCurrentItem;
    SceneCurrentItem = new TestBackgroundItem();
    calendarDDialogExecStub(stub.getStub());
    scene = QSharedPointer<CGraphicsScene>(new CGraphicsScene());
    view = QSharedPointer<QGraphicsView>(new QGraphicsView());
    view->setScene(scene.get());
    weekTabDeal = QSharedPointer<CWeekDaySceneTabKeyDeal>(new CWeekDaySceneTabKeyDeal(scene.get()));

    // Reset globals
    setCurrentFocusItemCalled = false;
    setCurrentFocusItemArg = nullptr;
    nextFocusItemResult = nullptr;
    focusItemType = CFocusItem::CBACK;
    itemFocus = false;
    WeekDayBackgroundItem_hasNextItem = false;
    WeekDayBackgroundItem_showFocus = false;
    setActiveSwitchingCalled = false;
    setActiveSwitchingValue = false;
    signalSwitchViewCalled = false;
    signalSwitchViewDate = QDate();
    itemDate = QDate(2026, 6, 15);
}

void test_CWeekDaySceneTabKeyDeal::TearDown()
{
}

// Constructor sets the correct key (inherits from CSceneTabKeyDeal)
TEST_F(test_CWeekDaySceneTabKeyDeal, Constructor)
{
    EXPECT_EQ(weekTabDeal->getKey(), Qt::Key_Tab);
}

// focusItemDeal: dynamic_cast fails (item is CSceneBackgroundItem, not CWeekDayBackgroundItem) → returns false
TEST_F(test_CWeekDaySceneTabKeyDeal, focusItemDeal_CastFail)
{
    delete SceneCurrentItem;
    SceneCurrentItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);

    bool result = weekTabDeal->dealEvent();
    EXPECT_FALSE(result);
}

// focusItemDeal: getItemFocus() is true → switch view, returns true
TEST_F(test_CWeekDaySceneTabKeyDeal, focusItemDeal_HasFocus)
{
    itemFocus = true;

    bool result = weekTabDeal->dealEvent();
    EXPECT_TRUE(result);
    EXPECT_FALSE(itemFocus); // setItemFocus(false) called
    EXPECT_TRUE(setActiveSwitchingCalled);
    EXPECT_TRUE(setActiveSwitchingValue);
    EXPECT_TRUE(signalSwitchViewCalled);
    EXPECT_EQ(signalSwitchViewDate, QDate(2026, 6, 15));
}

// focusItemDeal: getItemFocus() is false, hasNextSubItem is true → delegates to parent
TEST_F(test_CWeekDaySceneTabKeyDeal, focusItemDeal_NoFocus_HasNextSubItem)
{
    itemFocus = false;
    WeekDayBackgroundItem_hasNextItem = true;
    nextFocusItemResult = nullptr; // parent will get null nextItem

    bool result = weekTabDeal->dealEvent();
    // Parent (CSceneTabKeyDeal::focusItemDeal) returns false when nextItem is null
    EXPECT_FALSE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    EXPECT_EQ(setCurrentFocusItemArg, nullptr);
}

// focusItemDeal: getItemFocus() is false, showFocus is true → delegates to parent
TEST_F(test_CWeekDaySceneTabKeyDeal, focusItemDeal_NoFocus_ShowFocus)
{
    itemFocus = false;
    WeekDayBackgroundItem_showFocus = true;
    nextFocusItemResult = nullptr; // parent will get null nextItem

    bool result = weekTabDeal->dealEvent();
    // Parent (CSceneTabKeyDeal::focusItemDeal) returns false when nextItem is null
    EXPECT_FALSE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    EXPECT_EQ(setCurrentFocusItemArg, nullptr);
}

// focusItemDeal: getItemFocus() is false, no sub-items, no showFocus → initState, switch view, returns true
TEST_F(test_CWeekDaySceneTabKeyDeal, focusItemDeal_NoFocus_NoSubItem_NoShowFocus)
{
    itemFocus = false;
    WeekDayBackgroundItem_hasNextItem = false;
    WeekDayBackgroundItem_showFocus = false;

    bool result = weekTabDeal->dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setActiveSwitchingCalled);
    EXPECT_TRUE(setActiveSwitchingValue);
    EXPECT_TRUE(signalSwitchViewCalled);
    EXPECT_EQ(signalSwitchViewDate, QDate(2026, 6, 15));
}
