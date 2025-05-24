// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "tabletconfig.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(tabletConfig, "calendar.tablet")

//默认不为平板模式
bool TabletConfig::m_isTablet = false;
TabletConfig::TabletConfig(QObject *parent)
    : QObject(parent)
{
    qCDebug(tabletConfig) << "Initializing TabletConfig";
}

bool TabletConfig::isTablet()
{
    qCDebug(tabletConfig) << "Getting tablet mode status:" << m_isTablet;
    return m_isTablet;
}

void TabletConfig::setIsTablet(bool isTablet)
{
    qCDebug(tabletConfig) << "Setting tablet mode from" << m_isTablet << "to" << isTablet;
    m_isTablet = isTablet;
}
