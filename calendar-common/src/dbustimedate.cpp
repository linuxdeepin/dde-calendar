// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbustimedate.h"
#include "commondef.h"

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>

#define NETWORK_DBUS_INTEERFACENAME "com.deepin.daemon.Timedate"
#define NETWORK_DBUS_NAME "com.deepin.daemon.Timedate"
#define NETWORK_DBUS_PATH "/com/deepin/daemon/Timedate"

DBusTimedate::DBusTimedate(QObject *parent)
    : QDBusAbstractInterface(NETWORK_DBUS_NAME, NETWORK_DBUS_PATH, NETWORK_DBUS_INTEERFACENAME, QDBusConnection::sessionBus(), parent)
{
    qCDebug(ClientLogger) << "Initializing DBusTimedate interface";
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(NETWORK_DBUS_NAME,
                                               NETWORK_DBUS_PATH,
                                               "org.freedesktop.DBus.Properties",
                                               QLatin1String("PropertiesChanged"), this,
                                               SLOT(propertiesChanged(QDBusMessage)))) {
        qCWarning(ClientLogger) << "Failed to connect to DBus PropertiesChanged signal";
        qCWarning(ClientLogger) << "DBus error:" << this->lastError().message();
    }

    m_hasDateTimeFormat = getHasDateTimeFormat();
    qCDebug(ClientLogger) << "DBusTimedate initialized with hasDateTimeFormat:" << m_hasDateTimeFormat;
}

int DBusTimedate::shortTimeFormat()
{
    //如果存在对应的时间设置则获取，否则默认为4
    int format = m_hasDateTimeFormat ? getPropertyByName("ShortTimeFormat").toInt() : 4;
    qCDebug(ClientLogger) << "Getting shortTimeFormat:" << format;
    return format;
}

int DBusTimedate::shortDateFormat()
{
    //如果存在对应的时间设置则获取，否则默认为1
    int format = m_hasDateTimeFormat ? getPropertyByName("ShortDateFormat").toInt() : 1;
    qCDebug(ClientLogger) << "Getting shortDateFormat:" << format;
    return format;
}

Qt::DayOfWeek DBusTimedate::weekBegins()
{
    Qt::DayOfWeek day;
    if (m_hasDateTimeFormat) {
        // WeekBegins是从0开始的，加1才能对应DayOfWeek
        day = Qt::DayOfWeek(getPropertyByName("WeekBegins").toInt() + 1);
    } else {
        day = Qt::Monday;
    }
    qCDebug(ClientLogger) << "Getting weekBegins:" << day;
    return day;
}

void DBusTimedate::propertiesChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    // 参数固定长度
    if (3 != arguments.count()) {
        qCWarning(ClientLogger) << "Invalid DBus message: expected 3 arguments, got" << arguments.count();
        return;
    }
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != this->interface()) {
        qCWarning(ClientLogger) << "Unexpected interface name:" << interfaceName;
        return;
    }
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach (const QString &prop, keys) {
        if (prop == "ShortTimeFormat") {
            qCDebug(ClientLogger) << "ShortTimeFormat changed to:" << changedProps[prop].toInt();
            emit ShortTimeFormatChanged(changedProps[prop].toInt());
        } else if (prop == "ShortDateFormat") {
            qCDebug(ClientLogger) << "ShortDateFormat changed to:" << changedProps[prop].toInt();
            emit ShortDateFormatChanged(changedProps[prop].toInt());
        }
    }
}

QVariant DBusTimedate::getPropertyByName(const char *porpertyName)
{
    QDBusInterface dbusinterface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    QVariant value = dbusinterface.property(porpertyName);
    if (!value.isValid()) {
        qCWarning(ClientLogger) << "Failed to get property:" << porpertyName;
    }
    return value;
}

bool DBusTimedate::getHasDateTimeFormat()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(NETWORK_DBUS_NAME,
                                                      NETWORK_DBUS_PATH,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));

    QDBusMessage reply =  QDBusConnection::sessionBus().call(msg);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QVariant variant = reply.arguments().first();
        bool hasFormat = variant.toString().contains("\"ShortDateFormat\"");
        qCDebug(ClientLogger) << "Checked for DateTimeFormat support:" << hasFormat;
        return hasFormat;
    } else {
        qCWarning(ClientLogger) << "Failed to introspect DBus interface";
        return false;
    }
}
