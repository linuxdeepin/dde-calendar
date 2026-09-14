// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicebase.h"
#include "commondef.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusReply>
#include <QFile>
#include <QRegularExpression>
#include <QtDebug>

namespace {

bool isCallerServiceOwner(const QString &wellKnownName, const QString &caller)
{
    if (wellKnownName.isEmpty() || caller.isEmpty()) {
        return false;
    }

    const QDBusReply<QString> owner = QDBusConnection::sessionBus().interface()->serviceOwner(wellKnownName);
    return owner.isValid() && owner.value() == caller;
}

} // namespace

DServiceBase::DServiceBase(const QString &path, const QString &interface, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_interface(interface)
{
    qCDebug(ServiceLogger) << "Initializing ServiceBase with path:" << path << "interface:" << interface;
}

QString DServiceBase::getPath() const
{
    qCDebug(ServiceLogger) << "Getting service path:" << m_path;
    return m_path;
}

QString DServiceBase::getInterface() const
{
    qCDebug(ServiceLogger) << "Getting service interface:" << m_interface;
    return m_interface;
}

QString DServiceBase::getClientName()
{
    qCDebug(ServiceLogger) << "Getting client name for service:" << message().service();
    uint pid = QDBusConnection::sessionBus().interface()->servicePid(message().service());
    qCDebug(ServiceLogger) << "Client PID:" << pid;
    QString name;
    QFile file(QString("/proc/%1/status").arg(pid));
    if (file.open(QFile::ReadOnly)) {
        qCDebug(ServiceLogger) << "Successfully opened proc status file for PID:" << pid;
        name = QString(file.readLine()).section(QRegularExpression("([\\t ]*:[\\t ]*|\\n)"), 1, 1);
        file.close();
        qCDebug(ServiceLogger) << "Retrieved client name:" << name;
    } else {
        qCWarning(ServiceLogger) << "Failed to open proc status file for PID:" << pid;
    }
    return name;
}

bool DServiceBase::clientWhite(const int index)
{
    qCDebug(ServiceLogger) << "Checking client whitelist for index:" << index;
//    DeepinAIAssista
#ifdef CALENDAR_SERVICE_AUTO_EXIT
    qCDebug(ServiceLogger) << "Auto-exit mode enabled, checking whitelist";
    //根据编号,获取不同到白名单
    static QVector<QStringList> whiteList {{"dde-calendar", "DeepinAIAssistant"}, {"dde-calendar"}, {"dde-calendar"}};
    if (index < 0 || index >= whiteList.size()) {
        qCWarning(ServiceLogger) << "Index" << index << "out of range for whitelist, denying access";
        sendErrorReply(QDBusError::AccessDenied,
                       QStringLiteral("Invalid calendar account type."));
        return false;
    }
    const QString clientName = getClientName();
    // Keep /proc-based client identification. In a separate PID namespace,
    // fall back to the verified owner of the client's well-known D-Bus name.
    if (clientName.isEmpty()) {
        const QString caller = message().service();
        const bool calendarClient = isCallerServiceOwner(QStringLiteral("com.deepin.Calendar"), caller);
        const bool assistantClient = index == 0
            && isCallerServiceOwner(QStringLiteral("com.iflytek.aiassistant"), caller);
        if (calendarClient || assistantClient) {
            qCDebug(ServiceLogger) << "Verified client by D-Bus name owner:" << caller;
            return true;
        }

        qCWarning(ServiceLogger) << "Cannot verify client identity, denying access for:" << caller;
        sendErrorReply(QDBusError::AccessDenied,
                       QStringLiteral("Cannot verify client identity."));
        return false;
    }

    qCDebug(ServiceLogger) << "Checking client" << clientName << "against whitelist" << index;
    const QStringList &allowedNames = whiteList.at(index);
    int truncatedNameMatches = 0;
    for (const QString &allowedName : allowedNames) {
        if (clientName == allowedName) {
            qCDebug(ServiceLogger) << "Client" << clientName << "found in whitelist, allowing access";
            return true;
        }
        // Linux truncates /proc/<pid>/status Name to 15 characters. Accept a
        // truncated name only when it identifies exactly one allowed client.
        if (clientName.size() == 15 && allowedName.size() > clientName.size()
            && allowedName.startsWith(clientName)) {
            ++truncatedNameMatches;
        }
    }
    if (truncatedNameMatches == 1) {
        qCDebug(ServiceLogger) << "Client" << clientName
                               << "matched one truncated whitelist name, allowing access";
        return true;
    }
    qCDebug(ServiceLogger) << "Client" << clientName << "not found in whitelist, denying access";
    // Reply with a D-Bus error instead of an empty return value: an empty
    // string looks like a valid (empty) result to clients and can wipe their
    // loaded schedule data.
    sendErrorReply(QDBusError::AccessDenied,
                   QStringLiteral("Client is not allowed to access calendar data."));
    return false;
#else
    qCDebug(ServiceLogger) << "Auto-exit mode disabled, allowing all clients";
    Q_UNUSED(index)
    return true;
#endif
}

void DServiceBase::notifyPropertyChanged(const QString &interface, const QString &propertyName)
{
    qCDebug(ServiceLogger) << "Notifying property change for interface:" << interface << "property:" << propertyName;
    QDBusMessage signal = QDBusMessage::createSignal(
        getPath(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");
    signal << interface;
    QVariantMap changedProps;
    QVariant propertyValue = property(propertyName.toUtf8());
    changedProps.insert(propertyName, propertyValue);
    qCDebug(ServiceLogger) << "Property" << propertyName << "changed to value:" << propertyValue;
    signal << changedProps;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
    qCDebug(ServiceLogger) << "Property change notification sent for" << propertyName;
}
