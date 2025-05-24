// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeydowndeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include <QLoggingCategory>
#include <QDebug>
Q_LOGGING_CATEGORY(keyDownLog, "calendar.keypress.down")

CKeyDownDeal::CKeyDownDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Down, scene)
{
    qCDebug(keyDownLog) << "Created CKeyDownDeal instance";
}

bool CKeyDownDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(keyDownLog) << "Processing down key press";
    item->initState();
    if (item->getDownItem() != nullptr) {
        qCDebug(keyDownLog) << "Moving focus to down item";
        scene->setCurrentFocusItem(item->getDownItem());
        item->getDownItem()->setItemFocus(true);
    } else {
        qCDebug(keyDownLog) << "No down item found, switching to next page" << item->getDate().addDays(7);
        scene->setNextPage(item->getDate().addDays(7));
    }
    return true;
}
