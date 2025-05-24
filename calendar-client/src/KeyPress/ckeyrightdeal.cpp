// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyrightdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(keyRightLog, "calendar.keypress.right")

CKeyRightDeal::CKeyRightDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Right, scene)
{
    qCDebug(keyRightLog) << "Created CKeyRightDeal instance";
}

bool CKeyRightDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(keyRightLog) << "Processing right key press";
    item->initState();
    if (item->getRightItem() != nullptr) {
        qCDebug(keyRightLog) << "Moving focus to right item";
        scene->setCurrentFocusItem(item->getRightItem());
        item->getRightItem()->setItemFocus(true);
    } else {
        qCDebug(keyRightLog) << "No right item found, switching to next page" << item->getDate().addDays(1);
        scene->setNextPage(item->getDate().addDays(1));
    }
    return true;
}
