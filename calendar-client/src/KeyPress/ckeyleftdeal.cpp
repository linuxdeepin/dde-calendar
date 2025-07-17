// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyleftdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include "commondef.h"

#include <QDebug>

CKeyLeftDeal::CKeyLeftDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Left, scene)
{
    qCDebug(ClientLogger) << "CKeyLeftDeal constructor initialized";
}

bool CKeyLeftDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CKeyLeftDeal::focusItemDeal - Processing left key for date:" << item->getDate();
    item->initState();
    if (item->getLeftItem() != nullptr) {
        qCDebug(ClientLogger) << "Left item exists, setting focus to left item";
        scene->setCurrentFocusItem(item->getLeftItem());
        item->getLeftItem()->setItemFocus(true);
    } else {
        qCDebug(ClientLogger) << "Left item is null, switching to previous day";
        scene->setPrePage(item->getDate().addDays(-1));
    }
    return true;
}
