// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyupdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include "commondef.h"

#include <QDebug>

CKeyUpDeal::CKeyUpDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Up, scene)
{
    qCDebug(ClientLogger) << "CKeyUpDeal constructor initialized";
}

bool CKeyUpDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CKeyUpDeal::focusItemDeal - Processing up key for date:" << item->getDate();
    item->initState();
    if (item->getUpItem() != nullptr) {
        qCDebug(ClientLogger) << "Up item exists, setting focus to up item";
        scene->setCurrentFocusItem(item->getUpItem());
        item->getUpItem()->setItemFocus(true);
    } else {
        qCDebug(ClientLogger) << "Up item is null, switching to previous week";
        scene->setPrePage(item->getDate().addDays(-7));
    }
    return true;
}
