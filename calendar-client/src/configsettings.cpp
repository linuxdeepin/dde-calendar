/*
 * Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
 *
 * Author:     Iceyer <me@iceyer.net>
 *
 * Maintainer: Iceyer <me@iceyer.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "configsettings.h"

#include <DStandardPaths>

DCORE_USE_NAMESPACE;

CConfigSettings::CConfigSettings()
{
    init();
}

CConfigSettings::~CConfigSettings()
{
    releaseInstance();
}

void CConfigSettings::init()
{
    //如果为空则创建
    if (m_settings == nullptr) {
        auto configFilepath = DStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).value(0) + "/config.ini";
        m_settings = new QSettings(configFilepath, QSettings::IniFormat);
    }
    initSetting();
}

void CConfigSettings::initSetting()
{
    m_firstDayOfWeek = value("firstDayOfWeek", 1).toInt();
    m_userSidebarStatus = value("userSidebarStatus", true).toBool();
}

/**
 * @brief CConfigSettings::releaseInstance  释放配置指针
 */
void CConfigSettings::releaseInstance()
{
    delete m_settings;
    m_settings = nullptr;
}

CConfigSettings *CConfigSettings::getInstance()
{
    static CConfigSettings configSettings;
    return &configSettings;
}

void CConfigSettings::sync()
{
    m_settings->sync();
}

QVariant CConfigSettings::value(const QString &key, const QVariant &defaultValue)
{
    return m_settings->value(key, defaultValue);
}

//设置对应key的值
void CConfigSettings::setOption(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}

//移除对应的配置信息
void CConfigSettings::remove(const QString &key)
{
    m_settings->remove(key);
}

//判断是否包含对应的key
bool CConfigSettings::contains(const QString &key) const
{
    return m_settings->contains(key);
}

CConfigSettings *CConfigSettings::operator->() const
{
    return getInstance();
}

Qt::DayOfWeek CConfigSettings::getFirstDayOfWeek()
{
    return Qt::DayOfWeek(m_firstDayOfWeek);
}

void CConfigSettings::setFirstDayOfWeek(int day)
{
    m_firstDayOfWeek = day;
    setOption("firstDayOfWeek", m_firstDayOfWeek);
    emit signalFirstDayOfWeekChange();
}

bool CConfigSettings::getUserSidebarStatus()
{
    return m_userSidebarStatus;
}

void CConfigSettings::setUserSidebarStatus(bool status)
{
    m_userSidebarStatus = status;
    setOption("userSidebarStatus", m_userSidebarStatus);
}

