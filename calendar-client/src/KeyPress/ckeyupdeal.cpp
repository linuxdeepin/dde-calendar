// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyupdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"

#include <QDebug>

Q_LOGGING_CATEGORY(keyUpLog, "calendar.keypress.up")

CKeyUpDeal::CKeyUpDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Up, scene)
{
    qCDebug(keyUpLog) << "Created CKeyUpDeal instance";
}

bool CKeyUpDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(keyUpLog) << "Processing up key event for item:" << item;
    item->initState();
    if (item->getUpItem() != nullptr) {
        qCDebug(keyUpLog) << "Moving focus to up item";
        scene->setCurrentFocusItem(item->getUpItem());
        item->getUpItem()->setItemFocus(true);
    } else {
        qCDebug(keyUpLog) << "No up item found, switching to previous page with date:" << item->getDate().addDays(-7);
        scene->setPrePage(item->getDate().addDays(-7));
    }
    return true;
}
