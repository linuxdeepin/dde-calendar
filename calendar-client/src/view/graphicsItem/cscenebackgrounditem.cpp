// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscenebackgrounditem.h"
#include "commondef.h"

#include <QGraphicsScene>
#include <QDebug>
#include <QMarginsF>
#include <algorithm>

CSceneBackgroundItem::CSceneBackgroundItem(ItemOnView view, QGraphicsItem *parent)
    : CFocusItem(parent)
    , m_backgroundNum(0)
    , m_leftItem(nullptr)
    , m_rightItem(nullptr)
    , m_upItem(nullptr)
    , m_downItem(nullptr)
    , m_showItemIndex(-1)
    , m_itemOfView(view)
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem constructor - view:" << view;
    //设置item类型为背景显示
    setItemType(CBACK);
}

/**
 * @brief CSceneBackgroundItem::setNextItemFocusAndGetNextItem  设置下一个item focus状态并获取下一个Item
 * @return
 */
CFocusItem *CSceneBackgroundItem::setNextItemFocusAndGetNextItem()
{
    qCDebug(ClientLogger) << "CSceneBackgroundItem::setNextItemFocusAndGetNextItem - current showItemIndex:" << m_showItemIndex
                         << "items count:" << m_item.size();
    CFocusItem *NextFocus = this;
    //若该区域没有item
    if (m_showItemIndex < 0 && m_item.size() == 0) {
        qCDebug(ClientLogger) << "No items in this area, moving focus to next background item";
        NextFocus = CFocusItem::setNextItemFocusAndGetNextItem();
    } else if (m_showItemIndex == m_item.size() - 1) {
        //若切换到最后一个item
        qCDebug(ClientLogger) << "At last item, resetting showItemIndex and moving focus to next background item";
        m_item.at(m_showItemIndex)->setItemFocus(false);
        m_showItemIndex = -1;
        NextFocus = CFocusItem::setNextItemFocusAndGetNextItem();
    } else {
        //若该背景上有显示的item
        //若显示的item未设置focus则取消背景focus效果
        if (m_showItemIndex == -1 && getItemFocus()) {
            qCDebug(ClientLogger) << "No item has focus but background does, removing background focus";
            this->setItemFocus(false);
        }
        //若显示的item有设置focus则取消该item focus效果
        if (m_showItemIndex >= 0) {
            qCDebug(ClientLogger) << "Removing focus from current item at index:" << m_showItemIndex;
            m_item.at(m_showItemIndex)->setItemFocus(false);
        }
        //当前显示的item编号+1并这是focus效果
        ++m_showItemIndex;
        qCDebug(ClientLogger) << "Setting focus to next item at index:" << m_showItemIndex;
        m_item.at(m_showItemIndex)->setItemFocus(true);
    }
    return NextFocus;
}

/**
 * @brief compareItemData       对现实的日程标签进行排序
 * @param itemfirst
 * @param itemsecond
 * @return
 */
bool compareItemData(const CFocusItem *itemfirst, const CFocusItem *itemsecond)
{
    qCDebug(ClientLogger) << "CSceneBackgroundItem::compareItemData - itemfirst:" << itemfirst->rect() 
                         << "itemsecond:" << itemsecond->rect();
    if (itemfirst->rect() == itemsecond->rect()) {
        qCDebug(ClientLogger) << "CSceneBackgroundItem::compareItemData - itemfirst and itemsecond have the same rect";
        return false;
    }
    //根据从上倒下从左至右的规则对矩阵的x,y坐标进行对比排序
    if (qAbs(itemfirst->rect().y() - itemsecond->rect().y()) < 0.01) {
        if (itemfirst->rect().x() < itemsecond->rect().x()) {
            qCDebug(ClientLogger) << "CSceneBackgroundItem::compareItemData - itemfirst is left of itemsecond";
            return true;
        } else {
            qCDebug(ClientLogger) << "CSceneBackgroundItem::compareItemData - itemfirst is right of itemsecond";
            return false;
        }
    } else if (itemfirst->rect().y() < itemsecond->rect().y()) {
        qCDebug(ClientLogger) << "CSceneBackgroundItem::compareItemData - itemfirst is above itemsecond";
        return true;
    } else {
        qCDebug(ClientLogger) << "CSceneBackgroundItem::compareItemData - itemfirst is below itemsecond";
        return false;
    }
}
/**
 * @brief CSceneBackgroundItem::updateShowItem  更新在此背景上显示的item
 */
void CSceneBackgroundItem::updateShowItem()
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::updateShowItem for background:" << m_backgroundNum;
    m_item.clear();
    //缩小背景矩阵,防止获取到其他背景上的item
    QRectF offsetRect = this->rect().marginsRemoved(QMarginsF(1, 1, 1, 1));
    QList<QGraphicsItem *> mlistitem = this->scene()->items(offsetRect);
    // qCDebug(ClientLogger) << "Found" << mlistitem.count() << "items in background area";
    for (int i = 0; i < mlistitem.count(); ++i) {
        CFocusItem *item = dynamic_cast<CFocusItem *>(mlistitem.at(i));
        if (item != nullptr && item->getItemType() != CBACK) {
            m_item.append(item);
        }
    }
    // qCDebug(ClientLogger) << "Added" << m_item.size() << "focus items to background";
    std::sort(m_item.begin(), m_item.end(), compareItemData);
    updateCurrentItemShow();
}

int CSceneBackgroundItem::getShowItemCount()
{
    qCDebug(ClientLogger) << "CSceneBackgroundItem::getShowItemCount returning:" << m_item.size();
    return m_item.size();
}

/**
 * @brief CSceneBackgroundItem::setBackgroundNum        设置背景编号
 * @param num
 */
void CSceneBackgroundItem::setBackgroundNum(int num)
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::setBackgroundNum - num:" << num;
    m_backgroundNum = num;
}

/**
 * @brief CSceneBackgroundItem::getBackgroundNum        获取背景编号
 * @return
 */
int CSceneBackgroundItem::getBackgroundNum() const
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::getBackgroundNum - returning:" << m_backgroundNum;
    return m_backgroundNum;
}

/**
 * @brief CSceneBackgroundItem::setItemFocus 设置item是否获取focus
 * @param isFocus
 */
void CSceneBackgroundItem::setItemFocus(bool isFocus)
{
    qCDebug(ClientLogger) << "CSceneBackgroundItem::setItemFocus - isFocus:" << isFocus 
                         << "showItemIndex:" << m_showItemIndex;
    if (m_showItemIndex < 0) {
        qCDebug(ClientLogger) << "Setting focus on background item itself";
        CFocusItem::setItemFocus(isFocus);
    } else {
        if (m_showItemIndex < m_item.size()) {
            qCDebug(ClientLogger) << "Setting focus on child item at index:" << m_showItemIndex;
            m_item.at(m_showItemIndex)->setItemFocus(isFocus);
        }
    }
}

/**
 * @brief CSceneBackgroundItem::initState   恢复初始状态
 */
void CSceneBackgroundItem::initState()
{
    qCDebug(ClientLogger) << "CSceneBackgroundItem::initState called";
    if (getItemFocus()) {
        qCDebug(ClientLogger) << "Removing focus from background item";
        setItemFocus(false);
    }
    if (m_showItemIndex > -1 && m_showItemIndex < m_item.size()) {
        qCDebug(ClientLogger) << "Removing focus from child item at index:" << m_showItemIndex;
        m_item.at(m_showItemIndex)->setItemFocus(false);
    }
    m_showItemIndex = -1;
}

/**
 * @brief CSceneBackgroundItem::getFocusItem    获取当前焦点的item
 * @return
 */
CFocusItem *CSceneBackgroundItem::getFocusItem()
{
    qCDebug(ClientLogger) << "CSceneBackgroundItem::getFocusItem - showItemIndex:" << m_showItemIndex;
    if (m_showItemIndex < 0) {
        qCDebug(ClientLogger) << "CSceneBackgroundItem::getFocusItem - returning background item";
        return this;
    } else {
        qCDebug(ClientLogger) << "CSceneBackgroundItem::getFocusItem - returning child item at index:" << m_showItemIndex;
        return m_item.at(m_showItemIndex);
    }
}

CSceneBackgroundItem *CSceneBackgroundItem::getLeftItem() const
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::getLeftItem - returning:"
    //                      << (m_leftItem ? "item" : "nullptr");
    return m_leftItem;
}

void CSceneBackgroundItem::setLeftItem(CSceneBackgroundItem *leftItem)
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::setLeftItem - item:"
    //                     << (leftItem ? "set" : "nullptr");
    m_leftItem = leftItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getRightItem() const
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::getRightItem - returning:"
    //                      << (m_rightItem ? "item" : "nullptr");
    return m_rightItem;
}

void CSceneBackgroundItem::setRightItem(CSceneBackgroundItem *rightItem)
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::setRightItem - item:"
    //                      << (rightItem ? "set" : "nullptr");
    m_rightItem = rightItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getUpItem() const
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::getUpItem - returning:"
    //                      << (m_upItem ? "item" : "nullptr");
    return m_upItem;
}

void CSceneBackgroundItem::setUpItem(CSceneBackgroundItem *upItem)
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::setUpItem - item:"
    //                      << (upItem ? "set" : "nullptr");
    m_upItem = upItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getDownItem() const
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::getDownItem - returning:"
    //                      << (m_downItem ? "item" : "nullptr");
    return m_downItem;
}

void CSceneBackgroundItem::setDownItem(CSceneBackgroundItem *downItem)
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::setDownItem - item:"
    //                      << (downItem ? "set" : "nullptr");
    m_downItem = downItem;
}

void CSceneBackgroundItem::updateCurrentItemShow()
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::updateCurrentItemShow - showItemIndex:" << m_showItemIndex
    //                      << "items count:" << m_item.size();
    if (m_showItemIndex >= 0) {
        if (m_item.size() > 0) {
            qCDebug(ClientLogger) << "Setting focus to item at index:" << m_showItemIndex;
            m_showItemIndex = m_showItemIndex < m_item.size() ? m_showItemIndex : 0;
            m_item.at(m_showItemIndex)->setItemFocus(true);
        } else {
            qCDebug(ClientLogger) << "No items available, setting focus to background";
            m_showItemIndex = -1;
            setItemFocus(true);
        }
    }
}

CSceneBackgroundItem::ItemOnView CSceneBackgroundItem::getItemOfView() const
{
    // qCDebug(ClientLogger) << "CSceneBackgroundItem::getItemOfView - returning:" << m_itemOfView;
    return m_itemOfView;
}
