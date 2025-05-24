// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "configsettings.h"

#include <DStandardPaths>
#include <QDebug>
#include <QLoggingCategory>

DCORE_USE_NAMESPACE;

Q_LOGGING_CATEGORY(configSettings, "calendar.settings")

CConfigSettings::CConfigSettings()
{
    qCDebug(configSettings) << "Initializing CConfigSettings";
    init();
}

CConfigSettings::~CConfigSettings()
{
    qCDebug(configSettings) << "Destroying CConfigSettings";
    releaseInstance();
}

void CConfigSettings::init()
{
    //如果为空则创建
    if (m_settings == nullptr) {
        auto configFilepath = DStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).value(0) + "/config.ini";
        qCDebug(configSettings) << "Creating settings with config file:" << configFilepath;
        m_settings = new QSettings(configFilepath, QSettings::IniFormat);
    }
    initSetting();
}

void CConfigSettings::initSetting()
{
    qCDebug(configSettings) << "Initializing settings values";
    m_userSidebarStatus = value("userSidebarStatus", true).toBool();
    qCDebug(configSettings) << "User sidebar status set to:" << m_userSidebarStatus;
}

/**
 * @brief CConfigSettings::releaseInstance  释放配置指针
 */
void CConfigSettings::releaseInstance()
{
    if (m_settings) {
        qCDebug(configSettings) << "Releasing settings instance";
        delete m_settings;
        m_settings = nullptr;
    }
}

CConfigSettings *CConfigSettings::getInstance()
{
    static CConfigSettings configSettings;
    qCDebug(configSettings) << "Getting CConfigSettings instance";
    return &configSettings;
}

void CConfigSettings::sync()
{
    qCDebug(configSettings) << "Syncing settings to disk";
    m_settings->sync();
}

QVariant CConfigSettings::value(const QString &key, const QVariant &defaultValue)
{
    QVariant val = m_settings->value(key, defaultValue);
    qCDebug(configSettings) << "Retrieved value for key:" << key << "value:" << val;
    return val;
}

//设置对应key的值
void CConfigSettings::setOption(const QString &key, const QVariant &value)
{
    qCDebug(configSettings) << "Setting option key:" << key << "to value:" << value;
    m_settings->setValue(key, value);
}

//移除对应的配置信息
void CConfigSettings::remove(const QString &key)
{
    qCDebug(configSettings) << "Removing key:" << key;
    m_settings->remove(key);
}

//判断是否包含对应的key
bool CConfigSettings::contains(const QString &key) const
{
    bool hasKey = m_settings->contains(key);
    qCDebug(configSettings) << "Checking if contains key:" << key << "result:" << hasKey;
    return hasKey;
}

CConfigSettings *CConfigSettings::operator->() const
{
    return getInstance();
}

bool CConfigSettings::getUserSidebarStatus()
{
    qCDebug(configSettings) << "Getting user sidebar status:" << m_userSidebarStatus;
    return m_userSidebarStatus;
}

void CConfigSettings::setUserSidebarStatus(bool status)
{
    qCDebug(configSettings) << "Setting user sidebar status from" << m_userSidebarStatus << "to" << status;
    m_userSidebarStatus = status;
    setOption("userSidebarStatus", m_userSidebarStatus);
}
