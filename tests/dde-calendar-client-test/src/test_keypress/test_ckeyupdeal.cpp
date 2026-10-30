// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeyupdeal.h"

#include "../third-party_stub/stub.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "../dialog_stub.h"

test_CKeyUpDeal::test_CKeyUpDeal()
{
}

void test_CKeyUpDeal::SetUp()
{
    delete SceneCurrentItem;
    SceneCurrentItem = new TestBackgroundItem();
    calendarDDialogExecStub(stub.getStub());
    scene = QSharedPointer<CGraphicsScene>(new CGraphicsScene());
    upDeal = QSharedPointer<CKeyUpDeal>(new CKeyUpDeal(scene.get()));

    // Reset globals
    setCurrentFocusItemCalled = false;
    setCurrentFocusItemArg = nullptr;
    setPrePageCalled = false;
    setPrePageDate = QDate();
    upItemResult = nullptr;
    itemDate = QDate::currentDate();
}

void test_CKeyUpDeal::TearDown()
{
}

// Constructor sets the correct key
TEST_F(test_CKeyUpDeal, Constructor)
{
    EXPECT_EQ(upDeal->getKey(), Qt::Key_Up);
}

// focusItemDeal: getUpItem() returns non-null → setCurrentFocusItem called
TEST_F(test_CKeyUpDeal, focusItemDeal_UpItemNotNull)
{
    TestBackgroundItem upItem;
    upItemResult = &upItem;

    bool result = upDeal->dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    EXPECT_EQ(setCurrentFocusItemArg, &upItem);
    EXPECT_FALSE(setPrePageCalled);
}

// focusItemDeal: getUpItem() returns null → setPrePage called
TEST_F(test_CKeyUpDeal, focusItemDeal_UpItemNull)
{
    upItemResult = nullptr;
    itemDate = QDate(2026, 1, 15);

    bool result = upDeal->dealEvent();
    EXPECT_TRUE(result);
    EXPECT_FALSE(setCurrentFocusItemCalled);
    EXPECT_TRUE(setPrePageCalled);
    EXPECT_EQ(setPrePageDate, QDate(2026, 1, 8)); // addDays(-7)
}
