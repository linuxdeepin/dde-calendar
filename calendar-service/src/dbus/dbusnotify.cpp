// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusnotify.h"

Q_LOGGING_CATEGORY(dbusNotifyLog, "calendar.dbus.notify")

DBusNotify::DBusNotify(const QString &service, const QString &path, const QString &interface, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, interface.toLatin1().data(), connection, parent)
{
    qCDebug(dbusNotifyLog) << "DBusNotify initialized with service:" << service << "path:" << path << "interface:" << interface;
}

//根据通知ID关闭桌面顶部通知
void DBusNotify::closeNotification(quint32 notifyID)
{
    qCDebug(dbusNotifyLog) << "Closing notification with ID:" << notifyID;
    QList<QVariant> argumentList;
    argumentList << notifyID;
    callWithArgumentList(QDBus::NoBlock, QStringLiteral("CloseNotification"), argumentList);
    qCDebug(dbusNotifyLog) << "Close notification request sent for ID:" << notifyID;
}
