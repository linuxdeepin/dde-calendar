// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeypressdealbase.h"

#include "cgraphicsscene.h"
#include "graphicsItem/cscenebackgrounditem.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(keyPressBaseLog, "calendar.keypress.base")

CKeyPressDealBase::CKeyPressDealBase(Qt::Key key, QGraphicsScene *scene)
    : m_key(key)
    , m_scene(scene)
{
    qCDebug(keyPressBaseLog) << "Created CKeyPressDealBase instance for key:" << key;
}

CKeyPressDealBase::~CKeyPressDealBase()
{
    qCDebug(keyPressBaseLog) << "Destroying CKeyPressDealBase instance for key:" << m_key;
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
    qCDebug(keyPressBaseLog) << "Processing key event for key:" << m_key;
    CGraphicsScene *scene = qobject_cast<CGraphicsScene *>(m_scene);
    if (scene != nullptr) {
        CSceneBackgroundItem *item = dynamic_cast<CSceneBackgroundItem *>(scene->getCurrentFocusItem());
        if (item != nullptr) {
            qCDebug(keyPressBaseLog) << "Found focus item, delegating to specific handler";
            return focusItemDeal(item, scene);
        } else {
            qCDebug(keyPressBaseLog) << "No focus item found";
            return false;
        }
    }
    qCDebug(keyPressBaseLog) << "Invalid scene";
    return false;
}
