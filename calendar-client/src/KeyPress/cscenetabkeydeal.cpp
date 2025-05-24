// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscenetabkeydeal.h"

#include "graphicsItem/cscenebackgrounditem.h"
#include "cgraphicsscene.h"

#include <QDebug>
#include <QGraphicsView>

Q_LOGGING_CATEGORY(sceneTabKeyLog, "calendar.keypress.tabkey")

CSceneTabKeyDeal::CSceneTabKeyDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Tab, scene)
{
    qCDebug(sceneTabKeyLog) << "Created CSceneTabKeyDeal instance";
}

bool CSceneTabKeyDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(sceneTabKeyLog) << "Processing tab key event for item:" << item;
    CSceneBackgroundItem *nextItem = qobject_cast<CSceneBackgroundItem *>(item->setNextItemFocusAndGetNextItem());
    if (nextItem == nullptr) {
        qCDebug(sceneTabKeyLog) << "No next item found, clearing focus";
        scene->setCurrentFocusItem(nullptr);
        item->setItemFocus(false);
        return false;
    } else {
        CFocusItem *focusItem = nextItem->getFocusItem();
        //如果当前焦点显示不为背景则定位到当前焦点item位置
        if (focusItem->getItemType() != CFocusItem::CBACK) {
            qCDebug(sceneTabKeyLog) << "Centering view on non-background focus item";
            QGraphicsView *view = scene->views().at(0);
            QPointF point(scene->width() / 2, focusItem->rect().y());
            view->centerOn(point);
        }
        qCDebug(sceneTabKeyLog) << "Setting focus to next item";
        scene->setCurrentFocusItem(nextItem);
        return true;
    }
}
