// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbustimedate.h"
#include "commondef.h"

#include <DSysInfo>

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>

DCORE_USE_NAMESPACE

inline const char *getTimedateService()
{
    auto ver = DSysInfo::majorVersion().toInt();
    if (ver > 20) {
        return "org.deepin.dde.Timedate1";
    }
    return "com.deepin.daemon.Timedate";
}

inline const char *getTimedatePath()
{
    auto ver = DSysInfo::majorVersion().toInt();
    if (ver > 20) {
        return "/org/deepin/dde/Timedate1";
    }
    return "/com/deepin/daemon/Timedate";
}

inline const char *getTimedateInterface()
{
    auto ver = DSysInfo::majorVersion().toInt();
    if (ver > 20) {
        return "org.deepin.dde.Timedate1";
    }
    return "com.deepin.daemon.Timedate";
}

#define TIMEDATE_DBUS_INTERFACE getTimedateInterface()
#define TIMEDATE_DBUS_SERVICE getTimedateService()
#define TIMEDATE_DBUS_PATH getTimedatePath()

DBusTimedate::DBusTimedate(QObject *parent)
    : QDBusAbstractInterface(TIMEDATE_DBUS_SERVICE, TIMEDATE_DBUS_PATH, TIMEDATE_DBUS_INTERFACE, QDBusConnection::sessionBus(), parent)
{
    qCDebug(CommonLogger) << "DBusTimedate::DBusTimedate";
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(TIMEDATE_DBUS_SERVICE,
                                               TIMEDATE_DBUS_PATH,
                                               "org.freedesktop.DBus.Properties",
                                               QLatin1String("PropertiesChanged"), this,
                                               SLOT(propertiesChanged(QDBusMessage)))) {
        qCWarning(CommonLogger) << "Failed to connect to PropertiesChanged signal:" << this->lastError().message();
    }

    m_hasDateTimeFormat = getHasDateTimeFormat();
}

int DBusTimedate::shortTimeFormat()
{
    qCDebug(CommonLogger) << "DBusTimedate::shortTimeFormat";
    //如果存在对应的时间设置则获取，否则默认为4
    return m_hasDateTimeFormat ? getPropertyByName("ShortTimeFormat").toInt() : 4;
}

int DBusTimedate::shortDateFormat()
{
    qCDebug(CommonLogger) << "DBusTimedate::shortDateFormat";
    //如果存在对应的时间设置则获取，否则默认为1
    return m_hasDateTimeFormat ? getPropertyByName("ShortDateFormat").toInt() : 1;
}

Qt::DayOfWeek DBusTimedate::weekBegins()
{
    qCDebug(CommonLogger) << "DBusTimedate::weekBegins";
    if (m_hasDateTimeFormat) {
        // WeekBegins是从0开始的，加1才能对应DayOfWeek
        return Qt::DayOfWeek(getPropertyByName("WeekBegins").toInt() + 1);
    }
    return Qt::Monday;
}

void DBusTimedate::propertiesChanged(const QDBusMessage &msg)
{
    qCDebug(CommonLogger) << "DBusTimedate::propertiesChanged";
    QList<QVariant> arguments = msg.arguments();
    // 参数固定长度
    if (3 != arguments.count()) {
        qCWarning(CommonLogger) << "Invalid number of arguments in PropertiesChanged signal:" << arguments.count();
        return;
    }

    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != this->interface()) {
        qCDebug(CommonLogger) << "Ignoring PropertiesChanged for interface:" << interfaceName;
        return;
    }

    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach (const QString &prop, keys) {
        if (prop == "ShortTimeFormat") {
            qCDebug(CommonLogger) << "ShortTimeFormat changed";
            emit ShortTimeFormatChanged(changedProps[prop].toInt());
        } else if (prop == "ShortDateFormat") {
            qCDebug(CommonLogger) << "ShortDateFormat changed";
            emit ShortDateFormatChanged(changedProps[prop].toInt());
        }
    }
}

QVariant DBusTimedate::getPropertyByName(const char *porpertyName)
{
    qCDebug(CommonLogger) << "DBusTimedate::getPropertyByName, propertyName:" << porpertyName;
    QDBusInterface dbusinterface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return dbusinterface.property(porpertyName);
}

bool DBusTimedate::getHasDateTimeFormat()
{
    qCDebug(CommonLogger) << "DBusTimedate::getHasDateTimeFormat";
    QDBusMessage msg = QDBusMessage::createMethodCall(TIMEDATE_DBUS_SERVICE,
                                                      TIMEDATE_DBUS_PATH,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));

    QDBusMessage reply =  QDBusConnection::sessionBus().call(msg);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QVariant variant = reply.arguments().first();
        return variant.toString().contains("\"ShortDateFormat\"");
    } else {
        qCWarning(CommonLogger) << "Failed to check DateTime format support:" << reply.errorMessage();
        return false;
    }
}
