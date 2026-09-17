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

    //异步检查是否支持DateTimeFormat，不阻塞启动关键路径
    //初始使用默认值，待结果返回后通过信号更新
    asyncCheckFormatSupport();
}

int DBusTimedate::shortTimeFormat()
{
    qCDebug(CommonLogger) << "DBusTimedate::shortTimeFormat";
    //返回缓存值或默认值，不阻塞
    return m_shortTimeFormat;
}

int DBusTimedate::shortDateFormat()
{
    qCDebug(CommonLogger) << "DBusTimedate::shortDateFormat";
    //返回缓存值或默认值，不阻塞
    return m_shortDateFormat;
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
            m_shortTimeFormat = changedProps[prop].toInt();
            emit ShortTimeFormatChanged(m_shortTimeFormat);
        } else if (prop == "ShortDateFormat") {
            qCDebug(CommonLogger) << "ShortDateFormat changed";
            m_shortDateFormat = changedProps[prop].toInt();
            emit ShortDateFormatChanged(m_shortDateFormat);
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

void DBusTimedate::asyncCheckFormatSupport()
{
    qCDebug(CommonLogger) << "DBusTimedate::asyncCheckFormatSupport";
    QDBusMessage msg = QDBusMessage::createMethodCall(TIMEDATE_DBUS_SERVICE,
                                                      TIMEDATE_DBUS_PATH,
                                                      "org.freedesktop.DBus.Introspectable",
                                                      QStringLiteral("Introspect"));

    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &DBusTimedate::onIntrospectFinished);
}

void DBusTimedate::onIntrospectFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QString> reply(*watcher);
    if (reply.isError()) {
        qCWarning(CommonLogger) << "Failed to check DateTime format support:" << reply.error().message();
        m_hasDateTimeFormat = false;
    } else {
        m_hasDateTimeFormat = reply.value().contains("\"ShortDateFormat\"");
        if (m_hasDateTimeFormat) {
            asyncFetchFormatValues();
        }
    }
    watcher->deleteLater();
}

void DBusTimedate::asyncFetchFormatValues()
{
    qCDebug(CommonLogger) << "DBusTimedate::asyncFetchFormatValues";

    //异步获取ShortTimeFormat
    QDBusMessage timeMsg = QDBusMessage::createMethodCall(
        TIMEDATE_DBUS_SERVICE, TIMEDATE_DBUS_PATH,
        "org.freedesktop.DBus.Properties", QStringLiteral("Get"));
    timeMsg << this->interface() << QStringLiteral("ShortTimeFormat");
    QDBusPendingCall timeCall = QDBusConnection::sessionBus().asyncCall(timeMsg);
    QDBusPendingCallWatcher *timeWatcher = new QDBusPendingCallWatcher(timeCall, this);
    timeWatcher->setProperty("propertyName", QStringLiteral("ShortTimeFormat"));
    connect(timeWatcher, &QDBusPendingCallWatcher::finished, this, &DBusTimedate::onPropertyFetched);

    //异步获取ShortDateFormat
    QDBusMessage dateMsg = QDBusMessage::createMethodCall(
        TIMEDATE_DBUS_SERVICE, TIMEDATE_DBUS_PATH,
        "org.freedesktop.DBus.Properties", QStringLiteral("Get"));
    dateMsg << this->interface() << QStringLiteral("ShortDateFormat");
    QDBusPendingCall dateCall = QDBusConnection::sessionBus().asyncCall(dateMsg);
    QDBusPendingCallWatcher *dateWatcher = new QDBusPendingCallWatcher(dateCall, this);
    dateWatcher->setProperty("propertyName", QStringLiteral("ShortDateFormat"));
    connect(dateWatcher, &QDBusPendingCallWatcher::finished, this, &DBusTimedate::onPropertyFetched);
}

void DBusTimedate::onPropertyFetched(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusVariant> reply(*watcher);
    if (reply.isError()) {
        qCWarning(CommonLogger) << "Failed to fetch property:" << reply.error().message();
    } else {
        QString propName = watcher->property("propertyName").toString();
        int value = reply.value().variant().toInt();
        if (propName == "ShortTimeFormat") {
            m_shortTimeFormat = value;
            emit ShortTimeFormatChanged(value);
        } else if (propName == "ShortDateFormat") {
            m_shortDateFormat = value;
            emit ShortDateFormatChanged(value);
        }
    }
    watcher->deleteLater();
}
