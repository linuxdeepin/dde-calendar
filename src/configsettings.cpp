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

#include <DSettingsOption>
#include <DStandardPaths>

#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

#include <qsettingbackend.h>

DCORE_USE_NAMESPACE;

QPointer<QSettings> CConfigSettings::m_settings = nullptr;

CConfigSettings::CConfigSettings(QObject *parent)
    : QObject(parent)
{

}

CConfigSettings::~CConfigSettings()
{
    qDebug() << "destory";
}

void CConfigSettings::init()
{
    auto configFilepath = DStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).value(0) + "/config.ini";
    m_settings = new QSettings(configFilepath, QSettings::IniFormat);;
}

QPointer<QSettings> CConfigSettings::settings()
{
    return m_settings;
}

void CConfigSettings::sync()
{
    m_settings->sync();
}

QVariant CConfigSettings::value(const QString &key)
{
    return m_settings->value(key);
}

void CConfigSettings::setOption(const QString &key, const QVariant &value)
{
    m_settings->setValue(key, value);
}
