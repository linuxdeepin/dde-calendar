// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbustimedate.h"
#include "commondef.h"

#include <DSysInfo>

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QTimer>

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

    // Async initialization: defer format detection and property reads
    // to the event loop to avoid blocking startup with synchronous DBus calls.
    QTimer::singleShot(0, this, &DBusTimedate::asyncInitProperties);
}

int DBusTimedate::shortTimeFormat()
{
    qCDebug(CommonLogger) << "DBusTimedate::shortTimeFormat";
    return m_shortTimeFormat;
}

int DBusTimedate::shortDateFormat()
{
    qCDebug(CommonLogger) << "DBusTimedate::shortDateFormat";
    return m_shortDateFormat;
}

Qt::DayOfWeek DBusTimedate::weekBegins()
{
    qCDebug(CommonLogger) << "DBusTimedate::weekBegins";
    return Qt::DayOfWeek(m_weekBegins);
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
            m_shortTimeFormat = changedProps[prop].toInt();
            emit ShortTimeFormatChanged(m_shortTimeFormat);
        } else if (prop == "ShortDateFormat") {
            qCDebug(CommonLogger) << "ShortDateFormat changed";
            m_shortDateFormat = changedProps[prop].toInt();
            emit ShortDateFormatChanged(m_shortDateFormat);
        } else if (prop == "WeekBegins") {
            qCDebug(CommonLogger) << "WeekBegins changed";
            m_weekBegins = changedProps[prop].toInt() + 1;
            emit WeekBeginsChanged(m_weekBegins);
        }
    }
}

void DBusTimedate::asyncInitProperties()
{
    qCDebug(CommonLogger) << "DBusTimedate::asyncInitProperties";

    // Check format support asynchronously
    QDBusMessage msg = QDBusMessage::createMethodCall(TIMEDATE_DBUS_SERVICE,
                                                      TIMEDATE_DBUS_PATH,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));
    QDBusPendingCall introspectCall = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *introspectWatcher = new QDBusPendingCallWatcher(introspectCall, this);
    connect(introspectWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QString> reply(*watcher);
        if (reply.isError()) {
            qCWarning(CommonLogger) << "Failed to check DateTime format support:" << reply.error().message();
        } else {
            m_hasDateTimeFormat = reply.value().contains("ShortDateFormat");
        }
        watcher->deleteLater();

        if (m_hasDateTimeFormat) {
            // Read properties asynchronously and emit signals on update
            QDBusInterface dbusinterface(service(), path(), interface(), QDBusConnection::sessionBus(), this);
            int newTimeFormat = dbusinterface.property("ShortTimeFormat").toInt();
            int newDateFormat = dbusinterface.property("ShortDateFormat").toInt();
            int newWeekBegins = dbusinterface.property("WeekBegins").toInt() + 1;

            if (newTimeFormat != m_shortTimeFormat) {
                m_shortTimeFormat = newTimeFormat;
                emit ShortTimeFormatChanged(m_shortTimeFormat);
            }
            if (newDateFormat != m_shortDateFormat) {
                m_shortDateFormat = newDateFormat;
                emit ShortDateFormatChanged(m_shortDateFormat);
            }
            if (newWeekBegins != m_weekBegins) {
                m_weekBegins = newWeekBegins;
                emit WeekBeginsChanged(m_weekBegins);
            }
        }
    });
}
