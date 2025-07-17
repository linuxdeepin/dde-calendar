// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeydowndeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include "commondef.h"

#include <QDebug>

CKeyDownDeal::CKeyDownDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Down, scene)
{
    qCDebug(ClientLogger) << "CKeyDownDeal constructor initialized";
}

bool CKeyDownDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CKeyDownDeal::focusItemDeal - Processing down key for date:" << item->getDate();
    item->initState();
    if (item->getDownItem() != nullptr) {
        qCDebug(ClientLogger) << "Down item exists, setting focus to down item";
        scene->setCurrentFocusItem(item->getDownItem());
        item->getDownItem()->setItemFocus(true);
    } else {
        qCDebug(ClientLogger) << "Down item is null, switching to next week";
        scene->setNextPage(item->getDate().addDays(7));
    }
    return true;
}
