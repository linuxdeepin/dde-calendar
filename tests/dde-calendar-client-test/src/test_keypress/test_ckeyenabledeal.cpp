// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ckeyenabledeal.h"

#include "../third-party_stub/stub.h"

#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "../dialog_stub.h"
#include "view/graphicsItem/scheduleitem.h"
#include "view/graphicsItem/draginfoitem.h"
#include "dschedule.h"

#include <QGraphicsView>

test_CKeyEnableDeal::test_CKeyEnableDeal()
{
}

void test_CKeyEnableDeal::SetUp()
{
    delete SceneCurrentItem;
    SceneCurrentItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    calendarDDialogExecStub(stub.getStub());
    scene = QSharedPointer<CGraphicsScene>(new CGraphicsScene());
    view = QSharedPointer<QGraphicsView>(new QGraphicsView());
    view->setScene(scene.get());
    enableDeal = QSharedPointer<CKeyEnableDeal>(new CKeyEnableDeal(scene.get()));
}

void test_CKeyEnableDeal::TearDown()
{
}

CFocusItem *focusItem = nullptr;

CFocusItem *getFocusItem_stub()
{
    return focusItem;
}

DSchedule::Ptr getData_stub(void *obj)
{
    Q_UNUSED(obj)
    return DSchedule::Ptr(new DSchedule());
}

TEST_F(test_CKeyEnableDeal, focusItemDeal_Back)
{
    focusItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    stub.getStub().set(ADDR(CSceneBackgroundItem, getFocusItem), getFocusItem_stub);
    focusItemType = CFocusItem::CBACK;
    enableDeal->dealEvent();
    delete focusItem;
    focusItem = nullptr;
}

TEST_F(test_CKeyEnableDeal, focusItemDeal_Back_addDay)
{
    itemDate = QDate::currentDate().addDays(1);
    focusItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    stub.getStub().set(ADDR(CSceneBackgroundItem, getFocusItem), getFocusItem_stub);
    focusItemType = CFocusItem::CBACK;
    enableDeal->dealEvent();
    delete focusItem;
    focusItem = nullptr;
}

TEST_F(test_CKeyEnableDeal, focusItemDeal_Item)
{
    QRectF rect(0, 0, 100, 100);
    focusItem = new CScheduleItem(rect);
    stub.getStub().set(ADDR(CSceneBackgroundItem, getFocusItem), getFocusItem_stub);
    stub.getStub().set(ADDR(DragInfoItem, getData), getData_stub);
    focusItemType = CFocusItem::CITEM;
    enableDeal->dealEvent();
    delete focusItem;
    focusItem = nullptr;
}

TEST_F(test_CKeyEnableDeal, focusItemDeal_Other)
{
    focusItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
    stub.getStub().set(ADDR(CSceneBackgroundItem, getFocusItem), getFocusItem_stub);
    focusItemType = CFocusItem::COTHER;
    enableDeal->dealEvent();
    delete focusItem;
    focusItem = nullptr;
}

TEST_F(test_CKeyEnableDeal, focusItemDeal_NullFocusItem)
{
    focusItem = nullptr;
    stub.getStub().set(ADDR(CSceneBackgroundItem, getFocusItem), getFocusItem_stub);
    enableDeal->dealEvent();
}
