// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calldaykeyrightdeal.h"

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cgraphicsscene.h"
#include "commondef.h"

CAllDayKeyRightDeal::CAllDayKeyRightDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Right, scene)
{
    qCDebug(ClientLogger) << "CAllDayKeyRightDeal constructor initialized";
}

bool CAllDayKeyRightDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CAllDayKeyRightDeal::focusItemDeal - Processing right key for date:" << item->getDate();
    CWeekDayBackgroundItem *backgroundItem = dynamic_cast<CWeekDayBackgroundItem *>(item);
    backgroundItem->initState();
    //如果没有下一个则切换时间
    scene->setActiveSwitching(true);
    if (backgroundItem->getRightItem() == nullptr) {
        qCDebug(ClientLogger) << "Right item is null, switching to next page";
        scene->setNextPage(item->getDate().addDays(1), true);
    } else {
        qCDebug(ClientLogger) << "Switching view to next day";
        scene->signalSwitchView(backgroundItem->getDate().addDays(1), true);
    }
    return true;
}
