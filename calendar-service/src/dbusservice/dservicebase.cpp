// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicebase.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDebug>

DServiceBase::DServiceBase(const QString &path, const QString &interface, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_interface(interface)
{
}

QString DServiceBase::getPath() const
{
    return m_path;
}

QString DServiceBase::getInterface() const
{
    return m_interface;
}

void DServiceBase::notifyPropertyChanged(const QString &interface, const QString &propertyName)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        getPath(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");
    signal << interface;
    QVariantMap changedProps;
    changedProps.insert(propertyName, property(propertyName.toUtf8()));
    signal << changedProps;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
}
