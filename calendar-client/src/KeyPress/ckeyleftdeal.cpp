// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyleftdeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include <QLoggingCategory>
#include <QDebug>
Q_LOGGING_CATEGORY(keyLeftLog, "calendar.keypress.left")

CKeyLeftDeal::CKeyLeftDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Left, scene)
{
    qCDebug(keyLeftLog) << "Created CKeyLeftDeal instance";
}

bool CKeyLeftDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(keyLeftLog) << "Processing left key press";
    item->initState();
    if (item->getLeftItem() != nullptr) {
        qCDebug(keyLeftLog) << "Moving focus to left item";
        scene->setCurrentFocusItem(item->getLeftItem());
        item->getLeftItem()->setItemFocus(true);
    } else {
        qCDebug(keyLeftLog) << "No left item found, switching to previous page" << item->getDate().addDays(-1);
        scene->setPrePage(item->getDate().addDays(-1));
    }
    return true;
}
