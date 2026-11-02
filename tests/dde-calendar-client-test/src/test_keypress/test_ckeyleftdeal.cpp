// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeyleftdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "keypress/ckeyleftdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyLeftDeal::test_CKeyLeftDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(LeftHandle_test, Constructor)
{
    CGraphicsScene *scene = new CGraphicsScene();
    CKeyLeftDeal leftDeal(scene);
    EXPECT_EQ(leftDeal.getKey(), Qt::Key_Left);
    delete scene;
}

TEST(LeftHandle_test, DealEvent_LeftItemNotNull)
{
    KeyPressStub stub;
    leftItem = dynamic_cast<CSceneBackgroundItem *>(SceneCurrentItem);
    setCurrentFocusItemCalled = false;

    CGraphicsScene *scene = new CGraphicsScene();
    CKeyLeftDeal leftDeal(scene);
    bool result = leftDeal.dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    delete scene;

    leftItem = nullptr;
}

TEST(LeftHandle_test, DealEvent_LeftItemNull)
{
    KeyPressStub stub;
    leftItem = nullptr;
    setPrePageCalled = false;

    CGraphicsScene *scene = new CGraphicsScene();
    CKeyLeftDeal leftDeal(scene);
    bool result = leftDeal.dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setPrePageCalled);
    delete scene;
}
