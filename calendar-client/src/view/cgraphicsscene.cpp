// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cgraphicsscene.h"

#include "graphicsItem/cscenebackgrounditem.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QShortcut>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(sceneLog, "calendar.view.scene")

CGraphicsScene::CGraphicsScene(QObject *parent)
    : QGraphicsScene(parent)
    , firstfocusItem(nullptr)
    , currentFocusItem(nullptr)
    , m_keyPrxy(nullptr)
    , m_activeSwitching(false)
    , m_isContextMenu(false)
    , m_isShowCurrentItem(false)
{
    qCDebug(sceneLog) << "Creating graphics scene";
}

CGraphicsScene::~CGraphicsScene()
{
    if (m_keyPrxy != nullptr) {
        qCDebug(sceneLog) << "Deleting key proxy";
        delete m_keyPrxy;
    }
}

void CGraphicsScene::setFirstFocusItem(QGraphicsItem *item)
{
    qCDebug(sceneLog) << "Setting first focus item";
    firstfocusItem = item;
}

QGraphicsItem *CGraphicsScene::getFirstFocusItem() const
{
    return firstfocusItem;
}

void CGraphicsScene::setCurrentFocusItem(QGraphicsItem *item)
{
    qCDebug(sceneLog) << "Setting current focus item";
    currentFocusItem = item;
}

QGraphicsItem *CGraphicsScene::getCurrentFocusItem() const
{
    return currentFocusItem;
}

void CGraphicsScene::setKeyPressPrxy(CKeyPressPrxy *keyPrxy)
{
    qCDebug(sceneLog) << "Setting key press proxy";
    m_keyPrxy = keyPrxy;
}

void CGraphicsScene::currentFocusItemUpdate()
{
    if (currentFocusItem != nullptr) {
        qCDebug(sceneLog) << "Updating current focus item";
        CFocusItem *item = dynamic_cast<CFocusItem *>(currentFocusItem);
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
    qCDebug(sceneLog) << "Switching to previous page, date:" << focusDate << "switch view:" << isSwitchView;
    emit signalSwitchPrePage(focusDate, isSwitchView);
}

/**
 * @brief CGraphicsScene::setPrePage        切换下一页
 * @param focusDate                         切换焦点时间
 * @param isSwitchView                      是否切换视图
 */
void CGraphicsScene::setNextPage(const QDate &focusDate, bool isSwitchView)
{
    qCDebug(sceneLog) << "Switching to next page, date:" << focusDate << "switch view:" << isSwitchView;
    emit signalSwitchNextPage(focusDate, isSwitchView);
}

bool CGraphicsScene::event(QEvent *event)
{
    bool dealResult = false;
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
        if (m_keyPrxy != nullptr && m_keyPrxy->keyPressDeal(keyEvent->key())) {
            qCDebug(sceneLog) << "Key press handled by proxy, key:" << keyEvent->key();
            dealResult = true;
        }
        //如果为左右键处理则设置为true
        if (dealResult == false && (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right)) {
            dealResult = true;
        }
        if (keyEvent->modifiers() == Qt::ALT && keyEvent->key() == Qt::Key_M) {
            CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(currentFocusItem);
            if (item != nullptr && item->getFocusItem()->getItemType() == CFocusItem::CITEM) {
                dealResult = true;
                emit signalContextMenu(item->getFocusItem());
            }
        }
    }

    if (event->type() == QEvent::FocusIn) {
        dealResult = focusInDeal(event);
    }

    if (event->type() == QEvent::FocusOut) {
        dealResult = focusOutDeal(event);
    }
    return dealResult ? true : QGraphicsScene::event(event);
}

bool CGraphicsScene::focusInDeal(QEvent *event)
{
    qCDebug(sceneLog) << "Handling focus in event";
    bool dealResult = true;
    QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
    if (firstfocusItem != nullptr && (Qt::TabFocusReason == focusEvent->reason() || Qt::BacktabFocusReason == focusEvent->reason())) {
        if (m_isShowCurrentItem || currentFocusItem == nullptr) {
            if (currentFocusItem == nullptr) {
                currentFocusItem = firstfocusItem;
            }
            CFocusItem *item = dynamic_cast<CFocusItem *>(currentFocusItem);
            item->setItemFocus(true);
        } else {
            dealResult = m_keyPrxy->keyPressDeal(Qt::Key_Tab);
            //如果切换过来需要设置下一个item焦点，但是已经没有下一个item时，发送焦点切换信号
            if (currentFocusItem == nullptr) {
                emit signalsetNextFocus();
            }
        }
    }
    if (currentFocusItem != nullptr && Qt::ActiveWindowFocusReason == focusEvent->reason()) {
        CFocusItem *item = dynamic_cast<CFocusItem *>(currentFocusItem);
        item->setItemFocus(true);
    }
    return dealResult;
}

bool CGraphicsScene::focusOutDeal(QEvent *event)
{
    qCDebug(sceneLog) << "Handling focus out event";
    QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
    if (currentFocusItem != nullptr) {
        CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(currentFocusItem);
        if (Qt::ActiveWindowFocusReason == focusEvent->reason()) {
            item->setItemFocus(false);
        } else {
            //如果为右击菜单则更新焦点显示效果
            if (m_isContextMenu) {
                item->setItemFocus(false);
            } else {
                item->initState();
                //如果为被动切换焦点则初始化当前焦点item
                if (getActiveSwitching() == false) {
                    currentFocusItem = nullptr;
                    //通知另外一个视图初始化状态,因为全天和非全天之间tab切换保存了当前item信息
                    emit signalViewFocusInit();
                } else {
                    setActiveSwitching(false);
                }
            }
        }
    }
    return true;
}

void CGraphicsScene::setIsContextMenu(bool isContextMenu)
{
    qCDebug(sceneLog) << "Setting context menu state:" << isContextMenu;
    m_isContextMenu = isContextMenu;
}

void CGraphicsScene::setIsShowCurrentItem(bool isShowCurrentItem)
{
    qCDebug(sceneLog) << "Setting show current item:" << isShowCurrentItem;
    m_isShowCurrentItem = isShowCurrentItem;
}

void CGraphicsScene::currentItemInit()
{
    qCDebug(sceneLog) << "Initializing current item";
    if (currentFocusItem != nullptr) {
        CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(currentFocusItem);
        if (item != nullptr) {
            item->initState();
        }
        currentFocusItem = nullptr;
    }
}

bool CGraphicsScene::getActiveSwitching() const
{
    return m_activeSwitching;
}

void CGraphicsScene::setActiveSwitching(bool activeSwitching)
{
    qCDebug(sceneLog) << "Setting active switching:" << activeSwitching;
    m_activeSwitching = activeSwitching;
}
