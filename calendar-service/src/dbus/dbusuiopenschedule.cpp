// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusuiopenschedule.h"

Q_LOGGING_CATEGORY(dbusUIOpenScheduleLog, "calendar.dbus.uiopenschedule")

DbusUIOpenSchedule::DbusUIOpenSchedule(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    qCDebug(dbusUIOpenScheduleLog) << "DbusUIOpenSchedule initialized with service:" << service << "path:" << path;
}
