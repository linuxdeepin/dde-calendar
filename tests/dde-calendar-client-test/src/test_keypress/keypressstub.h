// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef KEYPRESSSTUB_H
#define KEYPRESSSTUB_H

#include "../third-party_stub/stub.h"
#include "view/graphicsItem/cfocusitem.h"
#include "view/graphicsItem/cscenebackgrounditem.h"
#include "view/graphicsItem/cweekdaybackgrounditem.h"

#include <QObject>
#include <QDate>

class QGraphicsItem;
class KeyPressStub
{
public:
    KeyPressStub();
    ~KeyPressStub();
    Stub &getStub();

private:
    Stub stub;
};

extern bool itemFocus;
extern bool scene_activeSwitching;
extern bool WeekDayBackgroundItem_hasNextItem;
extern bool WeekDayBackgroundItem_showFocus;
extern QGraphicsItem *SceneCurrentItem;
extern CFocusItem::CItemType focusItemType;
extern QDate itemDate;

// Additional globals for branch coverage
extern CSceneBackgroundItem *upItemResult;
extern CFocusItem *nextFocusItemResult;
extern bool setCurrentFocusItemCalled;
extern QGraphicsItem *setCurrentFocusItemArg;
extern bool setPrePageCalled;
extern QDate setPrePageDate;
extern bool signalSwitchViewCalled;
extern QDate signalSwitchViewDate;
extern bool setActiveSwitchingCalled;
extern bool setActiveSwitchingValue;

// Test subclass that overrides virtual methods to avoid stub library limitations
// with virtual method patching on GCC/x86_64 (PMF is 16 bytes, addrof() truncates to 8)
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
