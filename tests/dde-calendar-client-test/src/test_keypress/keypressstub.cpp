// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "keypressstub.h"

#include "view/graphicsItem/cweekdaybackgrounditem.h"
#include "view/graphicsItem/cscenebackgrounditem.h"
#include "view/cgraphicsscene.h"
#include "keypress/cscenetabkeydeal.h"

#include <QDebug>

bool itemFocus = false;
bool scene_activeSwitching = false;
bool WeekDayBackgroundItem_hasNextItem = false;
bool WeekDayBackgroundItem_showFocus = false;
QGraphicsItem *SceneCurrentItem = nullptr;
CFocusItem::CItemType focusItemType = CFocusItem::CBACK;
QDate itemDate = QDate::currentDate();

CSceneBackgroundItem *SceneBackgroundItem_leftItem = nullptr;
CSceneBackgroundItem *SceneBackgroundItem_rightItem = nullptr;
CSceneBackgroundItem *SceneBackgroundItem_downItem = nullptr;

CSceneBackgroundItem *leftItem = nullptr;
CSceneBackgroundItem *rightItem = nullptr;
bool setCurrentFocusItemCalled = false;
bool setPrePageCalled = false;
bool setNextPageCalled = false;

CSceneBackgroundItem *upItemResult = nullptr;
CFocusItem *nextFocusItemResult = nullptr;
QGraphicsItem *setCurrentFocusItemArg = nullptr;
QDate setPrePageDate;
bool signalSwitchViewCalled = false;
QDate signalSwitchViewDate;
bool setActiveSwitchingCalled = false;
bool setActiveSwitchingValue = false;

// CFocusItem stub
bool getItemFocus_stub()
{
    return itemFocus;
}

CFocusItem::CItemType getItemType_stub()
{
    return focusItemType;
}

// CGraphicsScene stub
void setActiveSwitching_stub(void *obj, bool activeSwitching)
{
    Q_UNUSED(obj)
    scene_activeSwitching = activeSwitching;
    setActiveSwitchingCalled = true;
    setActiveSwitchingValue = activeSwitching;
}

QGraphicsItem *getCurrentFocusItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return SceneCurrentItem;
}

void signalSwitchView_stub(void *obj, const QDate &focusDate, bool setItemFocus)
{
    Q_UNUSED(obj)
    Q_UNUSED(focusDate);
    Q_UNUSED(setItemFocus);
    signalSwitchViewCalled = true;
    signalSwitchViewDate = focusDate;
}

void setCurrentFocusItem_stub(void *obj, QGraphicsItem *item)
{
    Q_UNUSED(obj)
    setCurrentFocusItemCalled = true;
    setCurrentFocusItemArg = item;
}

void setPrePage_stub(void *obj, const QDate &focusDate, bool isSwitchView)
{
    Q_UNUSED(obj)
    Q_UNUSED(isSwitchView)
    setPrePageCalled = true;
    setPrePageDate = focusDate;
}

void setNextPage_stub(void *obj, const QDate &focusDate, bool isSwitchView)
{
    Q_UNUSED(obj)
    Q_UNUSED(focusDate)
    Q_UNUSED(isSwitchView)
    setNextPageCalled = true;
}

// CWeekDayBackgroundItem stub
void setItemFocus(void *obj, bool isFocus)
{
    Q_UNUSED(obj)
    itemFocus = isFocus;
}

bool hasNextSubItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return WeekDayBackgroundItem_hasNextItem;
}

bool showFocus_stub(void *obj)
{
    Q_UNUSED(obj)
    return WeekDayBackgroundItem_showFocus;
}

void initState_stub(void *obj) {
    Q_UNUSED(obj)}

QDate getDate_stub(void *obj)
{
    Q_UNUSED(obj)
    return itemDate;
}

// CSceneBackgroundItem stub（V-6616/V-6617 两套全局量带回退合并）
CSceneBackgroundItem *getLeftItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return SceneBackgroundItem_leftItem ? SceneBackgroundItem_leftItem : leftItem;
}

CSceneBackgroundItem *getRightItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return SceneBackgroundItem_rightItem ? SceneBackgroundItem_rightItem : rightItem;
}

CSceneBackgroundItem *getDownItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return SceneBackgroundItem_downItem;
}

CSceneBackgroundItem *getUpItem_stub(void *obj)
{
    Q_UNUSED(obj)
    return upItemResult;
}

KeyPressStub::KeyPressStub()
{
    SceneCurrentItem = new TestBackgroundItem();
    typedef void (*fptr)(CWeekDayBackgroundItem *, bool);
    fptr A_foo = (fptr)(&CWeekDayBackgroundItem::setItemFocus); //obtaining an address
    stub.set(ADDR(CFocusItem, getItemFocus), getItemFocus_stub);
    stub.set(ADDR(CFocusItem, getItemType), getItemType_stub);
    stub.set(A_foo, setItemFocus);
    stub.set(ADDR(CGraphicsScene, setActiveSwitching), setActiveSwitching_stub);
    stub.set(ADDR(CGraphicsScene, signalSwitchView), signalSwitchView_stub);
    stub.set(ADDR(CGraphicsScene, getCurrentFocusItem), getCurrentFocusItem_stub);
    stub.set(ADDR(CGraphicsScene, setCurrentFocusItem), setCurrentFocusItem_stub);
    stub.set(ADDR(CGraphicsScene, setPrePage), setPrePage_stub);
    stub.set(ADDR(CGraphicsScene, setNextPage), setNextPage_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, hasNextSubItem), hasNextSubItem_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, showFocus), showFocus_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, initState), initState_stub);
    stub.set(ADDR(CWeekDayBackgroundItem, getDate), getDate_stub);
    stub.set(ADDR(CSceneBackgroundItem, getLeftItem), getLeftItem_stub);
    stub.set(ADDR(CSceneBackgroundItem, getRightItem), getRightItem_stub);
    stub.set(ADDR(CSceneBackgroundItem, getDownItem), getDownItem_stub);
    stub.set(ADDR(CSceneBackgroundItem, getUpItem), getUpItem_stub);
}

KeyPressStub::~KeyPressStub()
{
    delete SceneCurrentItem;
    SceneCurrentItem = nullptr;
}

Stub &KeyPressStub::getStub()
{
    return stub;
}
