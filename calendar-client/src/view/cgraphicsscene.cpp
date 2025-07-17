// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cgraphicsscene.h"
#include "commondef.h"

#include "graphicsItem/cscenebackgrounditem.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QShortcut>

CGraphicsScene::CGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
    , firstfocusItem(nullptr)
    , currentFocusItem(nullptr)
    , m_keyPrxy(nullptr)
    , m_activeSwitching(false)
    , m_isContextMenu(false)
    , m_isShowCurrentItem(false)
{
    // qCDebug(ClientLogger) << "CGraphicsScene constructor";
}

CGraphicsScene::~CGraphicsScene()
{
    // qCDebug(ClientLogger) << "CGraphicsScene destructor";
    if (m_keyPrxy != nullptr)
        delete m_keyPrxy;
}

void CGraphicsScene::setFirstFocusItem(QGraphicsItem *item)
{
    // qCDebug(ClientLogger) << "CGraphicsScene::setFirstFocusItem called, item:" << (item ? "set" : "nullptr");
    firstfocusItem = item;
}

QGraphicsItem *CGraphicsScene::getFirstFocusItem() const
{
    // qCDebug(ClientLogger) << "CGraphicsScene::getFirstFocusItem called, returning:" << (firstfocusItem ? "item" : "nullptr");
    return firstfocusItem;
}

void CGraphicsScene::setCurrentFocusItem(QGraphicsItem *item)
{
    // qCDebug(ClientLogger) << "CGraphicsScene::setCurrentFocusItem called, item:" << (item ? "set" : "nullptr");
    currentFocusItem = item;
}

QGraphicsItem *CGraphicsScene::getCurrentFocusItem() const
{
    // qCDebug(ClientLogger) << "CGraphicsScene::getCurrentFocusItem called, returning:" << (currentFocusItem ? "item" : "nullptr");
    return currentFocusItem;
}

void CGraphicsScene::setKeyPressPrxy(CKeyPressPrxy *keyPrxy)
{
    // qCDebug(ClientLogger) << "CGraphicsScene::setKeyPressPrxy called, keyPrxy:" << (keyPrxy ? "set" : "nullptr");
    m_keyPrxy = keyPrxy;
}

void CGraphicsScene::currentFocusItemUpdate()
{
    // qCDebug(ClientLogger) << "CGraphicsScene::currentFocusItemUpdate called";
    if (currentFocusItem != nullptr) {
        CFocusItem *item = dynamic_cast<CFocusItem *>(currentFocusItem);
        // qCDebug(ClientLogger) << "Updating focus state for current item";
        item->setItemFocus(true);
    }
}

/**
 * @brief CGraphicsScene::setPrePage        切换上一页
 * @param focusDate                         切换焦点时间
 * @param isSwitchView                      是否切换视图
 */
void CGraphicsScene::setPrePage(const QDate &focusDate, bool isSwitchView)
{
    qCDebug(ClientLogger) << "CGraphicsScene::setPrePage called with date:" << focusDate << "switchView:" << isSwitchView;
    emit signalSwitchPrePage(focusDate, isSwitchView);
}

/**
 * @brief CGraphicsScene::setPrePage        切换下一页
 * @param focusDate                         切换焦点时间
 * @param isSwitchView                      是否切换视图
 */
void CGraphicsScene::setNextPage(const QDate &focusDate, bool isSwitchView)
{
    qCDebug(ClientLogger) << "CGraphicsScene::setNextPage called with date:" << focusDate << "switchView:" << isSwitchView;
    emit signalSwitchNextPage(focusDate, isSwitchView);
}

bool CGraphicsScene::event(QEvent *event)
{
    // qCDebug(ClientLogger) << "CGraphicsScene::event called with event type:" << event->type();
    bool dealResult = false;
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
        qCDebug(ClientLogger) << "CGraphicsScene::event - KeyPress event with key:" << keyEvent->key();
        if (m_keyPrxy != nullptr && m_keyPrxy->keyPressDeal(keyEvent->key())) {
            qCDebug(ClientLogger) << "Key press handled by proxy";
            dealResult = true;
        }
        //如果为左右键处理则设置为true
        if (dealResult == false && (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right)) {
            qCDebug(ClientLogger) << "Left/Right key press detected, marking as handled";
            dealResult = true;
        }
        if (keyEvent->modifiers() == Qt::ALT && keyEvent->key() == Qt::Key_M) {
            qCDebug(ClientLogger) << "Alt+M key combination detected";
            CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(currentFocusItem);
            if (item != nullptr && item->getFocusItem()->getItemType() == CFocusItem::CITEM) {
                qCDebug(ClientLogger) << "Emitting context menu signal for focus item";
                dealResult = true;
                emit signalContextMenu(item->getFocusItem());
            }
        }
    }

    if (event->type() == QEvent::FocusIn) {
        // qCDebug(ClientLogger) << "CGraphicsScene::event - FocusIn event";
        dealResult = focusInDeal(event);
    }

    if (event->type() == QEvent::FocusOut) {
        // qCDebug(ClientLogger) << "CGraphicsScene::event - FocusOut event";
        dealResult = focusOutDeal(event);
    }
    return dealResult ? true : QGraphicsScene::event(event);
}

bool CGraphicsScene::focusInDeal(QEvent *event)
{
    qCDebug(ClientLogger) << "CGraphicsScene::focusInDeal called";
    bool dealResult = true;
    QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
    if (firstfocusItem != nullptr && (Qt::TabFocusReason == focusEvent->reason() || Qt::BacktabFocusReason == focusEvent->reason())) {
        qCDebug(ClientLogger) << "Focus in with Tab/Backtab reason, showCurrentItem:" << m_isShowCurrentItem;
        if (m_isShowCurrentItem || currentFocusItem == nullptr) {
            if (currentFocusItem == nullptr) {
                qCDebug(ClientLogger) << "No current focus item, using first focus item";
                currentFocusItem = firstfocusItem;
            }
            CFocusItem *item = dynamic_cast<CFocusItem *>(currentFocusItem);
            item->setItemFocus(true);
        } else {
            qCDebug(ClientLogger) << "Handling Tab key press via proxy";
            dealResult = m_keyPrxy->keyPressDeal(Qt::Key_Tab);
            //如果切换过来需要设置下一个item焦点，但是已经没有下一个item时，发送焦点切换信号
            if (currentFocusItem == nullptr) {
                qCDebug(ClientLogger) << "No more items, emitting next focus signal";
                emit signalsetNextFocus();
            }
        }
    }
    if (currentFocusItem != nullptr && Qt::ActiveWindowFocusReason == focusEvent->reason()) {
        qCDebug(ClientLogger) << "Focus in with ActiveWindow reason";
        CFocusItem *item = dynamic_cast<CFocusItem *>(currentFocusItem);
        item->setItemFocus(true);
    }
    return dealResult;
}

bool CGraphicsScene::focusOutDeal(QEvent *event)
{
    qCDebug(ClientLogger) << "CGraphicsScene::focusOutDeal called";
    QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
    if (currentFocusItem != nullptr) {
        CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(currentFocusItem);
        if (Qt::ActiveWindowFocusReason == focusEvent->reason()) {
            qCDebug(ClientLogger) << "Focus out with ActiveWindow reason, setting item focus to false";
            item->setItemFocus(false);
        } else {
            //如果为右击菜单则更新焦点显示效果
            if (m_isContextMenu) {
                qCDebug(ClientLogger) << "Focus out with context menu active, setting item focus to false";
                item->setItemFocus(false);
            } else {
                qCDebug(ClientLogger) << "Focus out, initializing item state";
                item->initState();
                //如果为被动切换焦点则初始化当前焦点item
                if (getActiveSwitching() == false) {
                    qCDebug(ClientLogger) << "Passive focus switching, clearing current focus item";
                    currentFocusItem = nullptr;
                    //通知另外一个视图初始化状态,因为全天和非全天之间tab切换保存了当前item信息
                    emit signalViewFocusInit();
                } else {
                    qCDebug(ClientLogger) << "Active focus switching, resetting active switching flag";
                    setActiveSwitching(false);
                }
            }
        }
    }
    return true;
}

void CGraphicsScene::setIsContextMenu(bool isContextMenu)
{
    // qCDebug(ClientLogger) << "CGraphicsScene::setIsContextMenu called with value:" << isContextMenu;
    m_isContextMenu = isContextMenu;
}

void CGraphicsScene::setIsShowCurrentItem(bool isShowCurrentItem)
{
    // qCDebug(ClientLogger) << "CGraphicsScene::setIsShowCurrentItem called with value:" << isShowCurrentItem;
    m_isShowCurrentItem = isShowCurrentItem;
}

void CGraphicsScene::currentItemInit()
{
    // qCDebug(ClientLogger) << "CGraphicsScene::currentItemInit called";
    if (currentFocusItem != nullptr) {
        CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(currentFocusItem);
        if (item != nullptr) {
            // qCDebug(ClientLogger) << "Initializing current item state";
            item->initState();
        }
        currentFocusItem = nullptr;
    }
}

bool CGraphicsScene::getActiveSwitching() const
{
    // qCDebug(ClientLogger) << "CGraphicsScene::getActiveSwitching called, returning:" << m_activeSwitching;
    return m_activeSwitching;
}

void CGraphicsScene::setActiveSwitching(bool activeSwitching)
{
    // qCDebug(ClientLogger) << "CGraphicsScene::setActiveSwitching called with value:" << activeSwitching;
    m_activeSwitching = activeSwitching;
}
