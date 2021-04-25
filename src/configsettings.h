/*
 * Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
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

#pragma once

#include "singleton.h"

#include <QSettings>

class CConfigSettings : public QObject, public DCalendar::DSingleton<CConfigSettings>
{
    Q_OBJECT
public:
    explicit CConfigSettings(QObject *parent = 0);
    ~CConfigSettings();

    static void init();
    static QPointer<QSettings> settings();

    static void sync();
    static QVariant value(const QString &key);
    static void setOption(const QString &key, const QVariant &value);

private:
    static QPointer<QSettings> m_settings;
    friend class DCalendar::DSingleton<CConfigSettings>;
};
