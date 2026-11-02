// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeyrightdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "keypress/ckeyrightdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyRightDeal::test_CKeyRightDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(RightHandle_test, Constructor)
{
    CGraphicsScene *scene = new CGraphicsScene();
    CKeyRightDeal rightDeal(scene);
    EXPECT_EQ(rightDeal.getKey(), Qt::Key_Right);
    delete scene;
}

TEST(RightHandle_test, DealEvent_RightItemNotNull)
{
    KeyPressStub stub;
    rightItem = dynamic_cast<CSceneBackgroundItem *>(SceneCurrentItem);
    setCurrentFocusItemCalled = false;

    CGraphicsScene *scene = new CGraphicsScene();
    CKeyRightDeal rightDeal(scene);
    bool result = rightDeal.dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setCurrentFocusItemCalled);
    delete scene;

    rightItem = nullptr;
}

TEST(RightHandle_test, DealEvent_RightItemNull)
{
    KeyPressStub stub;
    rightItem = nullptr;
    setNextPageCalled = false;

    CGraphicsScene *scene = new CGraphicsScene();
    CKeyRightDeal rightDeal(scene);
    bool result = rightDeal.dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(setNextPageCalled);
    delete scene;
}
