// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicebase.h"
#include "commondef.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QFile>
#include <QRegularExpression>
#include <QtDebug>

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
    if (whiteList.size() < index) {
        qCWarning(ServiceLogger) << "Index" << index << "out of range for whitelist, denying access";
        return false;
    }
    QString clientName = getClientName();
    qCDebug(ServiceLogger) << "Checking client" << clientName << "against whitelist" << index;
    for (int i = 0; i < whiteList.at(index).size(); ++i) {
        if (whiteList.at(index).at(i).contains(clientName)) {
            qCDebug(ServiceLogger) << "Client" << clientName << "found in whitelist, allowing access";
            return true;
        }
    }
    qCDebug(ServiceLogger) << "Client" << clientName << "not found in whitelist, denying access";
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
