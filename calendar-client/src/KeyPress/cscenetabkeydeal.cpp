// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscenetabkeydeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"
#include "commondef.h"

#include <QDebug>
#include <QGraphicsView>

CSceneTabKeyDeal::CSceneTabKeyDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Tab, scene)
{
    qCDebug(ClientLogger) << "CSceneTabKeyDeal constructor initialized";
}

bool CSceneTabKeyDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CSceneTabKeyDeal::focusItemDeal - Processing tab key for date:" << item->getDate();
    CSceneBackgroundItem *nextItem = qobject_cast<CSceneBackgroundItem *>(item->setNextItemFocusAndGetNextItem());
    if (nextItem == nullptr) {
        qCDebug(ClientLogger) << "No next item found, clearing focus";
        scene->setCurrentFocusItem(nullptr);
        item->setItemFocus(false);
        return false;
    } else {
        qCDebug(ClientLogger) << "Found next item for date:" << nextItem->getDate();
        CFocusItem *focusItem = nextItem->getFocusItem();
        //如果当前焦点显示不为背景则定位到当前焦点item位置
        if (focusItem->getItemType() != CFocusItem::CBACK) {
            qCDebug(ClientLogger) << "Focus item is not background, centering view on item";
            QGraphicsView *view = scene->views().at(0);
            QPointF point(scene->width() / 2, focusItem->rect().y());
            view->centerOn(point);
        }
        scene->setCurrentFocusItem(nextItem);
        return true;
    }
}
