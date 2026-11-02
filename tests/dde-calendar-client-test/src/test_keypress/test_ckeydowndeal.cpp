// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeydowndeal.h"

#include "../third-party_stub/stub.h"
#include "gtest/gtest.h"
#include "view/cgraphicsscene.h"
#include "keypress/ckeydowndeal.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "view/graphicsItem/cscenebackgrounditem.h"
#include "keypressstub.h"

test_CKeyDownDeal::test_CKeyDownDeal(QObject *parent)
    : QObject(parent)
{
}

TEST(DownHandle_test, test_CKeyDownDeal)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    CKeyDownDeal downDeal(scene);
    downDeal.dealEvent();
    delete scene;
}

TEST(DownHandle_test, test_CKeyDownDeal_WithDownItem)
{
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    SceneBackgroundItem_downItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    CKeyDownDeal downDeal(scene);
    downDeal.dealEvent();
    delete scene;
    delete SceneBackgroundItem_downItem;
    SceneBackgroundItem_downItem = nullptr;
}
