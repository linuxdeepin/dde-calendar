// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cweekdayscenetabkeydeal.h"

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cgraphicsscene.h"
#include "commondef.h"

CWeekDaySceneTabKeyDeal::CWeekDaySceneTabKeyDeal(QGraphicsScene *scene)
    : CSceneTabKeyDeal(scene)
{
    qCDebug(ClientLogger) << "CWeekDaySceneTabKeyDeal constructor initialized";
}

bool CWeekDaySceneTabKeyDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CWeekDaySceneTabKeyDeal::focusItemDeal - Processing tab key for date:" << item->getDate();
    CWeekDayBackgroundItem *focusItem = dynamic_cast<CWeekDayBackgroundItem *>(item);
    if (focusItem != nullptr) {
        //如果当前背景是焦点显示则切换到另一个视图
        if (focusItem->getItemFocus()) {
            qCDebug(ClientLogger) << "Background item has focus, switching to another view";
            focusItem->setItemFocus(false);
            scene->setActiveSwitching(true);
            scene->signalSwitchView(focusItem->getDate());
            return true;
        } else {
            //如果该背景上还有切换显示的日程标签
            if (focusItem->hasNextSubItem() || focusItem->showFocus()) {
                qCDebug(ClientLogger) << "Background item has sub-items, delegating to parent handler";
                return CSceneTabKeyDeal::focusItemDeal(item, scene);
            } else {
                qCDebug(ClientLogger) << "No sub-items, initializing state and switching view";
                focusItem->initState();
                scene->setActiveSwitching(true);
                scene->signalSwitchView(focusItem->getDate());
                return true;
            }
        }
    } else {
        qCDebug(ClientLogger) << "Failed to cast to CWeekDayBackgroundItem";
    }
    return false;
}
