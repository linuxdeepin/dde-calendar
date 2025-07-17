// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "configsettings.h"
#include "commondef.h"

#include <DStandardPaths>
#include <QDebug>

DCORE_USE_NAMESPACE;

CConfigSettings::CConfigSettings()
{
    qCDebug(ClientLogger) << "CConfigSettings constructor initialized";
    init();
}

CConfigSettings::~CConfigSettings()
{
    qCDebug(ClientLogger) << "CConfigSettings destructor called";
    releaseInstance();
}

void CConfigSettings::init()
{
    qCDebug(ClientLogger) << "CConfigSettings::init - Initializing settings";
    //如果为空则创建
    if (m_settings == nullptr) {
        auto configFilepath = DStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).value(0) + "/config.ini";
        qCDebug(ClientLogger) << "Creating settings with config file:" << configFilepath;
        m_settings = new QSettings(configFilepath, QSettings::IniFormat);
    }
    initSetting();
}

void CConfigSettings::initSetting()
{
    qCDebug(ClientLogger) << "CConfigSettings::initSetting - Loading initial settings";
    m_userSidebarStatus = value("userSidebarStatus", true).toBool();
}

/**
 * @brief CConfigSettings::releaseInstance  释放配置指针
 */
void CConfigSettings::releaseInstance()
{
    qCDebug(ClientLogger) << "CConfigSettings::releaseInstance - Releasing settings instance";
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
    // qCDebug(ClientLogger) << "CConfigSettings::sync - Syncing settings to disk";
    m_settings->sync();
}

QVariant CConfigSettings::value(const QString &key, const QVariant &defaultValue)
{
    QVariant result = m_settings->value(key, defaultValue);
    // qCDebug(ClientLogger) << "CConfigSettings::value - Getting value for key:" << key << "Result:" << result;
    return result;
}

//设置对应key的值
void CConfigSettings::setOption(const QString &key, const QVariant &value)
{
    // qCDebug(ClientLogger) << "CConfigSettings::setOption - Setting key:" << key << "to value:" << value;
    m_settings->setValue(key, value);
}

//移除对应的配置信息
void CConfigSettings::remove(const QString &key)
{
    // qCDebug(ClientLogger) << "CConfigSettings::remove - Removing key:" << key;
    m_settings->remove(key);
}

//判断是否包含对应的key
bool CConfigSettings::contains(const QString &key) const
{
    bool result = m_settings->contains(key);
    // qCDebug(ClientLogger) << "CConfigSettings::contains - Checking if contains key:" << key << "Result:" << result;
    return result;
}

CConfigSettings *CConfigSettings::operator->() const
{
    return getInstance();
}

bool CConfigSettings::getUserSidebarStatus()
{
    // qCDebug(ClientLogger) << "CConfigSettings::getUserSidebarStatus - Current status:" << m_userSidebarStatus;
    return m_userSidebarStatus;
}

void CConfigSettings::setUserSidebarStatus(bool status)
{
    // qCDebug(ClientLogger) << "CConfigSettings::setUserSidebarStatus - Setting status to:" << status;
    m_userSidebarStatus = status;
    setOption("userSidebarStatus", m_userSidebarStatus);
}
