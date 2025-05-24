// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cweekdayscenetabkeydeal.h"

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cgraphicsscene.h"

Q_LOGGING_CATEGORY(weekDaySceneTabKeyLog, "calendar.keypress.weekdaytabkey")

CWeekDaySceneTabKeyDeal::CWeekDaySceneTabKeyDeal(QGraphicsScene *scene)
    : CSceneTabKeyDeal(scene)
{
    qCDebug(weekDaySceneTabKeyLog) << "Created CWeekDaySceneTabKeyDeal instance";
}

bool CWeekDaySceneTabKeyDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(weekDaySceneTabKeyLog) << "Processing weekday tab key event for item:" << item;
    CWeekDayBackgroundItem *focusItem = dynamic_cast<CWeekDayBackgroundItem *>(item);
    if (focusItem != nullptr) {
        //如果当前背景是焦点显示则切换到另一个视图
        if (focusItem->getItemFocus()) {
            qCDebug(weekDaySceneTabKeyLog) << "Switching view for focused item with date:" << focusItem->getDate();
            focusItem->setItemFocus(false);
            scene->setActiveSwitching(true);
            scene->signalSwitchView(focusItem->getDate());
            return true;
        } else {
            //如果该背景上还有切换显示的日程标签
            if (focusItem->hasNextSubItem() || focusItem->showFocus()) {
                qCDebug(weekDaySceneTabKeyLog) << "Processing tab key for item with sub-items";
                return CSceneTabKeyDeal::focusItemDeal(item, scene);
            } else {
                qCDebug(weekDaySceneTabKeyLog) << "Switching view for non-focused item with date:" << focusItem->getDate();
                focusItem->initState();
                scene->setActiveSwitching(true);
                scene->signalSwitchView(focusItem->getDate());
                return true;
            }
        }
    }
    qCDebug(weekDaySceneTabKeyLog) << "Invalid item type, focus handling failed";
    return false;
}
