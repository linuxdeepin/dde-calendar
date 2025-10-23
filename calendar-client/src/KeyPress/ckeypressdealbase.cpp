// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeypressdealbase.h"

#include "cgraphicsscene.h"
#include "graphicsItem/cscenebackgrounditem.h"
#include "commondef.h"

CKeyPressDealBase::CKeyPressDealBase(Qt::Key key, QGraphicsScene *scene)
    : m_key(key)
    , m_scene(scene)
{
    // qCDebug(ClientLogger) << "CKeyPressDealBase constructor initialized with key:" << key;
}

CKeyPressDealBase::~CKeyPressDealBase()
{
    // qCDebug(ClientLogger) << "CKeyPressDealBase destructor called for key:" << m_key;
}

/**
 * @brief CKeyPressDealBase::getKey 获取注册的key
 * @return
 */
Qt::Key CKeyPressDealBase::getKey() const
{
    return m_key;
}

bool CKeyPressDealBase::dealEvent()
{
    // qCDebug(ClientLogger) << "CKeyPressDealBase::dealEvent - Processing key event for key:" << m_key;
    CGraphicsScene *scene = qobject_cast<CGraphicsScene *>(m_scene);
    if (scene != nullptr) {
        CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(scene->getCurrentFocusItem());
        if (item != nullptr) {
            // qCDebug(ClientLogger) << "Found focus item, delegating to focusItemDeal for date:" << item->getDate();
            return focusItemDeal(item, scene);
        } else {
            // qCDebug(ClientLogger) << "No focus item found";
            return false;
        }
    }
    // qCDebug(ClientLogger) << "Scene is null";
    return false;
}
