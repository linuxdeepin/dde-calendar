// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeypressdealbase.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "keypress/ckeypressdealbase.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyPressDealBase::test_CKeyPressDealBase(QObject *parent)
    : QObject(parent)
{
}

class TestDeal : public CKeyPressDealBase
{
public:
    TestDeal(Qt::Key key, QGraphicsScene *scene = nullptr)
        : CKeyPressDealBase(key, scene)
        , focusItemDealCalled(false)
    {
    }

    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override
    {
        Q_UNUSED(item)
        Q_UNUSED(scene)
        focusItemDealCalled = true;
        return true;
    }

    bool focusItemDealCalled;
};

TEST(KeyPressDealBase_test, Constructor)
{
    CGraphicsScene *scene = new CGraphicsScene();
    TestDeal deal(Qt::Key_Left, scene);
    EXPECT_EQ(deal.getKey(), Qt::Key_Left);
    delete scene;
}

TEST(KeyPressDealBase_test, Destructor)
{
    TestDeal *deal = new TestDeal(Qt::Key_Tab);
    delete deal;
    SUCCEED();
}

TEST(KeyPressDealBase_test, GetKey)
{
    TestDeal deal(Qt::Key_Right);
    EXPECT_EQ(deal.getKey(), Qt::Key_Right);
}

TEST(KeyPressDealBase_test, DealEvent_SceneNull)
{
    TestDeal deal(Qt::Key_Left, nullptr);
    bool result = deal.dealEvent();
    EXPECT_FALSE(result);
}

TEST(KeyPressDealBase_test, DealEvent_SceneNotNull_ItemNull)
{
    KeyPressStub stub;
    SceneCurrentItem = nullptr;

    CGraphicsScene *scene = new CGraphicsScene();
    TestDeal deal(Qt::Key_Left, scene);
    bool result = deal.dealEvent();
    EXPECT_FALSE(result);
    EXPECT_FALSE(deal.focusItemDealCalled);
    delete scene;
}

TEST(KeyPressDealBase_test, DealEvent_SceneNotNull_ItemNotNull)
{
    KeyPressStub stub;

    CGraphicsScene *scene = new CGraphicsScene();
    TestDeal deal(Qt::Key_Left, scene);
    bool result = deal.dealEvent();
    EXPECT_TRUE(result);
    EXPECT_TRUE(deal.focusItemDealCalled);
    delete scene;
}
