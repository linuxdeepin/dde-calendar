// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeypressprxy.h"

#include <QDebug>

Q_LOGGING_CATEGORY(keyPressProxyLog, "calendar.keypress.proxy")

CKeyPressPrxy::CKeyPressPrxy()
{
    qCDebug(keyPressProxyLog) << "Created CKeyPressPrxy instance";
}

CKeyPressPrxy::~CKeyPressPrxy()
{
    qCDebug(keyPressProxyLog) << "Destroying CKeyPressPrxy instance, cleaning up" << m_keyEventMap.size() << "key events";
    QMap<int, CKeyPressDealBase *>::iterator iter = m_keyEventMap.begin();
    for (; iter != m_keyEventMap.end(); ++iter) {
        delete iter.value();
    }
    m_keyEventMap.clear();
}

/**
 * @brief CKeyPressPrxy::keyPressDeal   键盘事件处理
 * @param event
 * @return
 */
bool CKeyPressPrxy::keyPressDeal(int key)
{
    qCDebug(keyPressProxyLog) << "Processing key press event for key:" << key;
    bool result = m_keyEventMap.contains(key);
    if (result) {
        //如果有注册对应的key事件 开始处理
        qCDebug(keyPressProxyLog) << "Found registered handler for key:" << key << ", executing handler";
        result = m_keyEventMap[key]->dealEvent();
        qCDebug(keyPressProxyLog) << "Key event handler execution result:" << result;
    } else {
        qCDebug(keyPressProxyLog) << "No handler registered for key:" << key;
    }
    return result;
}

/**
 * @brief CKeyPressPrxy::addkeyPressDeal    添加需要处理的键盘事件
 * @param deal
 */
void CKeyPressPrxy::addkeyPressDeal(CKeyPressDealBase *deal)
{
    if (deal != nullptr) {
        qCDebug(keyPressProxyLog) << "Adding key press handler for key:" << deal->getKey();
        m_keyEventMap[deal->getKey()] = deal;
    } else {
        qCWarning(keyPressProxyLog) << "Attempted to add null key press handler";
    }
}

/**
 * @brief CKeyPressPrxy::removeDeal     移除添加的键盘事件
 * @param deal
 */
void CKeyPressPrxy::removeDeal(CKeyPressDealBase *deal)
{
    if (deal == nullptr) {
        qCWarning(keyPressProxyLog) << "Attempted to remove null key press handler";
        return;
    }
    
    qCDebug(keyPressProxyLog) << "Removing key press handler for key:" << deal->getKey();
    if (m_keyEventMap.contains(deal->getKey())) {
        m_keyEventMap.remove(deal->getKey());
        delete deal;
        qCDebug(keyPressProxyLog) << "Successfully removed and deleted key press handler";
    } else {
        qCDebug(keyPressProxyLog) << "No handler found for key:" << deal->getKey();
    }
}
