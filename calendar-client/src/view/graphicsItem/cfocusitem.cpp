// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cfocusitem.h"
#include "commondef.h"

#include "scheduledatamanage.h"

#include <QGraphicsScene>
#include <QPainter>

CFocusItem::CFocusItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , m_NextFocusItem(nullptr)
    , m_itemType(CITEM)
    , m_isFocus(false)
{
    // qCDebug(ClientLogger) << "CFocusItem constructor";
}

/**
 * @brief CFocusItem::setNextFocusItem  设置下一个FocusItem
 * @param nextFocusItem
 */
void CFocusItem::setNextFocusItem(CFocusItem *nextFocusItem)
{
    // qCDebug(ClientLogger) << "CFocusItem::setNextFocusItem - next item:" << (nextFocusItem ? "set" : "null");
    m_NextFocusItem = nextFocusItem;
}

/**
 * @brief CFocusItem::setItemFocus  设置item是否获取focus
 * @param isFocus
 */
void CFocusItem::setItemFocus(bool isFocus)
{
    // qCDebug(ClientLogger) << "CFocusItem::setItemFocus - isFocus:" << isFocus;
    m_isFocus = isFocus;
    this->scene()->update();
}

/**
 * @brief CFocusItem::getItemFocus      获取该item是否focus
 * @return
 */
bool CFocusItem::getItemFocus() const
{
    // qCDebug(ClientLogger) << "CFocusItem::getItemFocus - returning:" << m_isFocus;
    return m_isFocus;
}

/**
 * @brief CFocusItem::setItemType 设置item类型
 * @param itemType
 */
void CFocusItem::setItemType(CFocusItem::CItemType itemType)
{
    // qCDebug(ClientLogger) << "CFocusItem::setItemType - itemType:" << itemType;
    m_itemType = itemType;
}

/**
 * @brief CFocusItem::getItemType 获取item类型
 * @return
 */
CFocusItem::CItemType CFocusItem::getItemType() const
{
    // qCDebug(ClientLogger) << "CFocusItem::getItemType - returning:" << m_itemType;
    return m_itemType;
}

/**
 * @brief CFocusItem::setNextItemFocusAndGetNextItem    设置下一个item focus状态并获取下一个Item
 * @return
 */
CFocusItem *CFocusItem::setNextItemFocusAndGetNextItem()
{
    qCDebug(ClientLogger) << "CFocusItem::setNextItemFocusAndGetNextItem called";
    if (m_NextFocusItem != nullptr) {
        qCDebug(ClientLogger) << "Setting focus to next item";
        m_isFocus = false;
        m_NextFocusItem->setItemFocus(true);
    } else {
        qCDebug(ClientLogger) << "No next focus item available";
    }
    return m_NextFocusItem;
}

/**
 * @brief CFocusItem::getSystemActiveColor      获取系统活动色
 * @return
 */
QColor CFocusItem::getSystemActiveColor()
{
    QColor color = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    // qCDebug(ClientLogger) << "CFocusItem::getSystemActiveColor - returning:" << color;
    return color;
}

void CFocusItem::setDate(const QDate &date)
{
    // qCDebug(ClientLogger) << "CFocusItem::setDate - date:" << date;
    m_Date = date;
}
