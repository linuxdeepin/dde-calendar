// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef KEYPRESSSTUB_H
#define KEYPRESSSTUB_H

#include "../third-party_stub/stub.h"
#include "view/graphicsItem/cfocusitem.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"

#include <QObject>

class QGraphicsItem;
class CSceneBackgroundItem;
class KeyPressStub
{
public:
    KeyPressStub();
    ~KeyPressStub();
    Stub &getStub();

private:
    Stub stub;
};

// 基础桩全局量（master / V-6616 / V-6617 共用）
extern bool itemFocus;
extern bool scene_activeSwitching;
extern bool WeekDayBackgroundItem_hasNextItem;
extern bool WeekDayBackgroundItem_showFocus;
extern QGraphicsItem *SceneCurrentItem;
extern CFocusItem::CItemType focusItemType;
extern QDate itemDate;

// V-6616 引入
extern CSceneBackgroundItem *SceneBackgroundItem_leftItem;
extern CSceneBackgroundItem *SceneBackgroundItem_rightItem;
extern CSceneBackgroundItem *SceneBackgroundItem_downItem;

// V-6617 引入
extern CSceneBackgroundItem *leftItem;
extern CSceneBackgroundItem *rightItem;
extern bool setCurrentFocusItemCalled;
extern bool setPrePageCalled;
extern bool setNextPageCalled;

// master 原有分支覆盖全局量
extern CSceneBackgroundItem *upItemResult;
extern CFocusItem *nextFocusItemResult;
extern QGraphicsItem *setCurrentFocusItemArg;
extern QDate setPrePageDate;
extern bool signalSwitchViewCalled;
extern QDate signalSwitchViewDate;
extern bool setActiveSwitchingCalled;
extern bool setActiveSwitchingValue;

// 既有测试依赖的虚函数重写子类（GCC/x86_64 PMF 截断问题用子类绕过）
class TestBackgroundItem : public CWeekDayBackgroundItem
{
public:
    explicit TestBackgroundItem(QGraphicsItem *parent = nullptr)
        : CWeekDayBackgroundItem(parent) {}

    void setItemFocus(bool isFocus) override
    {
        itemFocus = isFocus;
    }

    CFocusItem *setNextItemFocusAndGetNextItem() override
    {
        return nextFocusItemResult;
    }
};

#endif // KEYPRESSSTUB_H
