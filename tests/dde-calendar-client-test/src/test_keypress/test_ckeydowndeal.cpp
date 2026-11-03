// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeydowndeal.h"

// 无头（offscreen/minimal）平台下弹窗/焦点/动画类用例会崩溃或卡死，跳过；
// 真实图形会话（如 CI 桌面环境）下正常执行。
static bool headlessQpaPlatform()
{
    const QByteArray platform = qEnvironmentVariable("QT_QPA_PLATFORM").toUtf8();
    return platform == "offscreen" || platform == "minimal";
}

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
    if (headlessQpaPlatform()) {
        GTEST_SKIP() << "requires a graphical session (popup/focus/animation)";
    }
    KeyPressStub stub;
    CGraphicsScene *scene = new CGraphicsScene();
    SceneBackgroundItem_downItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    CKeyDownDeal downDeal(scene);
    downDeal.dealEvent();
    delete scene;
    delete SceneBackgroundItem_downItem;
    SceneBackgroundItem_downItem = nullptr;
}
