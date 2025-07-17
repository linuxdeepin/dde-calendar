// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "tabletconfig.h"
#include "commondef.h"

//默认不为平板模式
bool TabletConfig::m_isTablet = false;
TabletConfig::TabletConfig(QObject *parent)
    : QObject(parent)
{
    // qCDebug(ClientLogger) << "TabletConfig constructor";
}

bool TabletConfig::isTablet()
{
    // qCDebug(ClientLogger) << "TabletConfig::isTablet called, current state:" << m_isTablet;
    return m_isTablet;
}

void TabletConfig::setIsTablet(bool isTablet)
{
    // qCDebug(ClientLogger) << "TabletConfig::setIsTablet called, old state:" << m_isTablet << "new state:" << isTablet;
    m_isTablet = isTablet;
}
