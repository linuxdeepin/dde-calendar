// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeypressprxy.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "keypress/ckeypressprxy.h"
#include "keypress/ckeypressdealbase.h"
#include "keypress/ckeyleftdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "keypressstub.h"

test_CKeyPressPrxy::test_CKeyPressPrxy(QObject *parent)
    : QObject(parent)
{
}

class MockDeal : public CKeyPressDealBase
{
public:
    MockDeal(Qt::Key key, QGraphicsScene *scene = nullptr)
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

TEST(KeyPressPrxy_test, Constructor)
{
    CKeyPressPrxy prxy;
    SUCCEED();
}

TEST(KeyPressPrxy_test, Destructor)
{
    CKeyPressPrxy *prxy = new CKeyPressPrxy;
    delete prxy;
    SUCCEED();
}

TEST(KeyPressPrxy_test, KeyPressDeal_KeyNotFound)
{
    CKeyPressPrxy prxy;
    bool result = prxy.keyPressDeal(Qt::Key_Left);
    EXPECT_FALSE(result);
}

TEST(KeyPressPrxy_test, KeyPressDeal_KeyFound)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    MockDeal *deal = new MockDeal(Qt::Key_Left, scene);

    CKeyPressPrxy prxy;
    prxy.addkeyPressDeal(deal);

    bool result = prxy.keyPressDeal(Qt::Key_Left);
    EXPECT_TRUE(result);
    EXPECT_TRUE(deal->focusItemDealCalled);

    delete scene;
}

TEST(KeyPressPrxy_test, AddkeyPressDeal_NullDeal)
{
    CKeyPressPrxy prxy;
    prxy.addkeyPressDeal(nullptr);
    bool result = prxy.keyPressDeal(Qt::Key_Left);
    EXPECT_FALSE(result);
}

TEST(KeyPressPrxy_test, AddkeyPressDeal_NotNullDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    MockDeal *deal = new MockDeal(Qt::Key_Right, scene);

    CKeyPressPrxy prxy;
    prxy.addkeyPressDeal(deal);
    bool result = prxy.keyPressDeal(Qt::Key_Right);
    EXPECT_TRUE(result);

    delete scene;
}

TEST(KeyPressPrxy_test, RemoveDeal_NullDeal)
{
    CKeyPressPrxy prxy;
    prxy.removeDeal(nullptr);
    SUCCEED();
}

TEST(KeyPressPrxy_test, RemoveDeal_KeyNotInMap)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    MockDeal *deal = new MockDeal(Qt::Key_Up, scene);

    CKeyPressPrxy prxy;
    prxy.removeDeal(deal);

    EXPECT_EQ(deal->getKey(), Qt::Key_Up);

    delete deal;
    delete scene;
}

TEST(KeyPressPrxy_test, RemoveDeal_KeyInMap)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    MockDeal *deal = new MockDeal(Qt::Key_Down, scene);

    CKeyPressPrxy prxy;
    prxy.addkeyPressDeal(deal);
    prxy.removeDeal(deal);

    bool result = prxy.keyPressDeal(Qt::Key_Down);
    EXPECT_FALSE(result);

    delete scene;
}
