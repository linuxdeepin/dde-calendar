// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeypressprxy.h"
#include "commondef.h"

#include <QDebug>

CKeyPressPrxy::CKeyPressPrxy()
{
    qCDebug(ClientLogger) << "CKeyPressPrxy constructor initialized";
}

CKeyPressPrxy::~CKeyPressPrxy()
{
    qCDebug(ClientLogger) << "CKeyPressPrxy destructor called, cleaning up" << m_keyEventMap.size() << "key handlers";
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
    qCDebug(ClientLogger) << "CKeyPressPrxy::keyPressDeal - Processing key:" << key;
    bool result = m_keyEventMap.contains(key);
    if (result) {
        //如果有注册对应的key事件 开始处理
        qCDebug(ClientLogger) << "Found registered handler for key, delegating to handler";
        result = m_keyEventMap[key]->dealEvent();
    } else {
        qCDebug(ClientLogger) << "No handler registered for key:" << key;
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
        qCDebug(ClientLogger) << "CKeyPressPrxy::addkeyPressDeal - Adding handler for key:" << deal->getKey();
        m_keyEventMap[deal->getKey()] = deal;
    } else {
        qCDebug(ClientLogger) << "CKeyPressPrxy::addkeyPressDeal - Attempted to add null handler";
    }
}

/**
 * @brief CKeyPressPrxy::removeDeal     移除添加的键盘事件
 * @param deal
 */
void CKeyPressPrxy::removeDeal(CKeyPressDealBase *deal)
{
    if (deal != nullptr) {
        qCDebug(ClientLogger) << "CKeyPressPrxy::removeDeal - Removing handler for key:" << deal->getKey();
        if (m_keyEventMap.contains(deal->getKey())) {
            m_keyEventMap.remove(deal->getKey());
            delete deal;
            qCDebug(ClientLogger) << "Handler removed and deleted";
        } else {
            qCDebug(ClientLogger) << "No handler found for key:" << deal->getKey();
        }
    } else {
        qCDebug(ClientLogger) << "CKeyPressPrxy::removeDeal - Attempted to remove null handler";
    }
}
