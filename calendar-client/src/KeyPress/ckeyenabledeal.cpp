// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyenabledeal.h"
#include "cgraphicsscene.h"
#include "scheduledlg.h"
#include "graphicsItem/cscenebackgrounditem.h"
#include "graphicsItem/draginfoitem.h"
#include "myscheduleview.h"
#include "commondef.h"

#include <QGraphicsView>

CKeyEnableDeal::CKeyEnableDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Return, scene)
{
    qCDebug(ClientLogger) << "CKeyEnableDeal constructor initialized";
}

/**
 * @brief CKeyEnableDeal::focusItemDeal     焦点项处理
 * @param item
 * @param scene
 * @return
 */
bool CKeyEnableDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    qCDebug(ClientLogger) << "CKeyEnableDeal::focusItemDeal - Processing return key for date:" << item->getDate();
    bool result = false;
    CFocusItem *focusItem = item->getFocusItem();
    if (focusItem != nullptr) {
        qCDebug(ClientLogger) << "Focus item exists, type:" << focusItem->getItemType();
        result = true;
        QWidget *parentWidget = scene->views().at(0);
        switch (focusItem->getItemType()) {
        case CFocusItem::CBACK: {
            qCDebug(ClientLogger) << "Processing background item type";
            CSceneBackgroundItem *backgroundItem = dynamic_cast<CSceneBackgroundItem *>(focusItem);
            if (backgroundItem != nullptr) {
                QDateTime createDateTime;
                //设置创建时间
                createDateTime.setDate(backgroundItem->getDate());
                createDateTime.setTime(QTime(0, 0, 0));
                //如果为月视图背景则根据是否为当前时间设置不一样的创建时间
                if (backgroundItem->getItemOfView() == CSceneBackgroundItem::OnMonthView) {
                    qCDebug(ClientLogger) << "Month view background item detected";
                    QDateTime currentDateTime = QDateTime::currentDateTime();
                    //如果为当前时间则设置创建开始时间为当前时间
                    if (backgroundItem->getDate() == currentDateTime.date()) {
                        qCDebug(ClientLogger) << "Setting create time to current time";
                        createDateTime.setTime(currentDateTime.time());
                    } else {
                        qCDebug(ClientLogger) << "Setting create time to 8:00";
                        createDateTime.setTime(QTime(8, 0, 0));
                    }
                }
                createSchedule(createDateTime, parentWidget);
            }
        } break;
        case CFocusItem::CITEM: {
            qCDebug(ClientLogger) << "Processing schedule item type";
            DragInfoItem *scheduleItem = dynamic_cast<DragInfoItem *>(focusItem);
            CMyScheduleView dlg(scheduleItem->getData(), parentWidget);
            dlg.exec();
        } break;
        default: {
            qCDebug(ClientLogger) << "Processing default item type, going to day view for date:" << focusItem->getDate();
            scene->signalGotoDayView(focusItem->getDate());
        } break;
        }
    } else {
        qCDebug(ClientLogger) << "No focus item found";
    }
    return result;
}

/**
 * @brief CKeyEnableDeal::createSchedule        创建日程
 * @param createDate
 * @param parent
 */
void CKeyEnableDeal::createSchedule(const QDateTime &createDate, QWidget *parent)
{
    qCDebug(ClientLogger) << "CKeyEnableDeal::createSchedule - Creating schedule for datetime:" << createDate;
    CScheduleDlg dlg(1, parent);
    dlg.setDate(createDate);
    dlg.setAllDay(true);
    dlg.exec();
}
