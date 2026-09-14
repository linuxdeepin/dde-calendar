// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbustimedate.h"
#include "ddlogCommon.h"

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>
#include <QElapsedTimer>

using namespace DDLOG_COMMON;

#define NETWORK_DBUS_INTEERFACENAME "com.deepin.daemon.Timedate"
#define NETWORK_DBUS_NAME "com.deepin.daemon.Timedate"
#define NETWORK_DBUS_PATH "/com/deepin/daemon/Timedate"

DBusTimedate::DBusTimedate(QObject *parent)
    : QDBusAbstractInterface(NETWORK_DBUS_NAME, NETWORK_DBUS_PATH, NETWORK_DBUS_INTEERFACENAME, QDBusConnection::sessionBus(), parent)
{
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(NETWORK_DBUS_NAME,
                                               NETWORK_DBUS_PATH,
                                               "org.freedesktop.DBus.Properties",
                                               QLatin1String("PropertiesChanged"), this,
                                               SLOT(propertiesChanged(QDBusMessage)))) {
        qCWarning(calendar_common) << "the PropertiesChanged was fail!";
        qCWarning(calendar_common) << this->lastError();
    }

    initPropertyFlags();
}

int DBusTimedate::shortTimeFormat()
{
    //如果存在对应的时间设置则获取，否则默认为4
    return m_hasDateTimeFormat ? getPropertyByName("ShortTimeFormat").toInt() : 4;
}

int DBusTimedate::shortDateFormat()
{
    //如果存在对应的时间设置则获取，否则默认为1
    return m_hasDateTimeFormat ? getPropertyByName("ShortDateFormat").toInt() : 1;
}

Qt::DayOfWeek DBusTimedate::weekBegins()
{
    // dcc 一周首日支持周一至周日七选一，写入值为 0 基（0=周一 … 6=周日，dcc 5.10 实测），
    // 加一转为 Qt::DayOfWeek（1=周一 … 7=周日）；视图层支持任意一周首日，跟随系统时原样透传
    if (m_hasDateTimeFormat) {
        const int day = getPropertyByName("WeekBegins").toInt();
        if (day >= 0 && day <= 6) {
            return static_cast<Qt::DayOfWeek>(day + 1);
        }
        return Qt::Monday;
    }
    return Qt::Monday;
}

bool DBusTimedate::hasUse24HourFormat()
{
    return m_hasUse24HourFormat;
}

bool DBusTimedate::use24HourFormat()
{
    // dcc 的 12/24 小时制开关实际写入 Use24HourFormat；ShortTimeFormat 在切换时保持不变
    return m_hasUse24HourFormat ? getPropertyByName("Use24HourFormat").toBool() : true;
}

void DBusTimedate::propertiesChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    // 参数固定长度
    if (3 != arguments.count())
        return;
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != this->interface())
        return;
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach (const QString &prop, keys) {
        if (prop == "ShortTimeFormat") {
            emit ShortTimeFormatChanged(changedProps[prop].toInt());
        } else if (prop == "ShortDateFormat") {
            emit ShortDateFormatChanged(changedProps[prop].toInt());
        } else if (prop == "WeekBegins") {
            emit WeekBeginsChanged(changedProps[prop].toInt());
        } else if (prop == "Use24HourFormat") {
            emit use24HourFormatChanged(changedProps[prop].toBool());
        }
    }
}

QVariant DBusTimedate::getPropertyByName(const char *porpertyName)
{
    QDBusInterface dbusinterface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return dbusinterface.property(porpertyName);
}

void DBusTimedate::initPropertyFlags()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(NETWORK_DBUS_NAME,
                                                      NETWORK_DBUS_PATH,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));

    QDBusMessage reply =  QDBusConnection::sessionBus().call(msg);

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QString xml = reply.arguments().first().toString();
        m_hasDateTimeFormat = xml.contains("\"ShortDateFormat\"");
        m_hasUse24HourFormat = xml.contains("\"Use24HourFormat\"");
    }
}
