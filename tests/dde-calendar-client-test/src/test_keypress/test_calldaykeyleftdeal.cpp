// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calldaykeyleftdeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "keypress/calldaykeyleftdeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "view/graphicsItem/cscenebackgrounditem.h"
#include "keypressstub.h"

test_CAllDayKeyLeftDeal::test_CAllDayKeyLeftDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(LeftHandle_test, test_CAllDayKeyLeftDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CAllDayKeyLeftDeal alldayLeftDeal(scene);
    alldayLeftDeal.dealEvent();
    delete scene;
}

TEST(LeftHandle_test, test_CAllDayKeyLeftDeal_WithLeftItem)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    SceneBackgroundItem_leftItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    CAllDayKeyLeftDeal alldayLeftDeal(scene);
    alldayLeftDeal.dealEvent();
    delete scene;
    delete SceneBackgroundItem_leftItem;
    SceneBackgroundItem_leftItem = nullptr;
}
