// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dunioniddbus.h"
#include "commondef.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(dunionIDDbusLog, "calendar.dbus.unionid")

DUnionIDDbus::DUnionIDDbus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    qCDebug(dunionIDDbusLog) << "DUnionIDDbus initialized with service:" << service << "path:" << path;
    auto reply = this->SwitcherDump();
    reply.waitForFinished();
    if (!reply.isValid()) {
        qCWarning(dunionIDDbusLog) << "Error connecting remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
        qCWarning(dunionIDDbusLog) << reply.error();
    } else {
        qCInfo(dunionIDDbusLog) << "Connected remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
        qCInfo(dunionIDDbusLog) << "Switcher dump" << reply.value();
    }
}

DUnionIDDbus::~DUnionIDDbus()
{
    qCDebug(dunionIDDbusLog) << "DUnionIDDbus destroyed";
}
