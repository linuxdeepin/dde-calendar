// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calldaykeyleftdeal.h"

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cgraphicsscene.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(allDayKeyLeftLog, "calendar.keypress.alldayleft")

CAllDayKeyLeftDeal::CAllDayKeyLeftDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Left, scene)
{
    qCDebug(allDayKeyLeftLog) << "Created CAllDayKeyLeftDeal instance";
}

bool CAllDayKeyLeftDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(allDayKeyLeftLog) << "Processing left key press for all day item";
    CWeekDayBackgroundItem *backgroundItem = dynamic_cast<CWeekDayBackgroundItem *>(item);
    backgroundItem->initState();
    //如果是第一个则切换时间
    scene->setActiveSwitching(true);
    if (backgroundItem->getLeftItem() == nullptr) {
        qCDebug(allDayKeyLeftLog) << "No left item found, switching to previous page" << item->getDate().addDays(-1);
        scene->setPrePage(item->getDate().addDays(-1), true);
    } else {
        qCDebug(allDayKeyLeftLog) << "Switching view to" << backgroundItem->getDate().addDays(-1);
        scene->signalSwitchView(backgroundItem->getDate().addDays(-1), true);
    }
    return true;
}
