// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyrightdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include "commondef.h"

#include <QDebug>

CKeyRightDeal::CKeyRightDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Right, scene)
{
    qCDebug(ClientLogger) << "CKeyRightDeal constructor initialized";
}

bool CKeyRightDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CKeyRightDeal::focusItemDeal - Processing right key for date:" << item->getDate();
    item->initState();
    if (item->getRightItem() != nullptr) {
        qCDebug(ClientLogger) << "Right item exists, setting focus to right item";
        scene->setCurrentFocusItem(item->getRightItem());
        item->getRightItem()->setItemFocus(true);
    } else {
        qCDebug(ClientLogger) << "Right item is null, switching to next day";
        scene->setNextPage(item->getDate().addDays(1));
    }
    return true;
}
