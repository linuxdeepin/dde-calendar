// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "doanetworkdbus.h"
#include "commondef.h"

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>

/* include dsysinfo for different OS version
#include <DSysInfo>

DCORE_USE_NAMESPACE

inline const QString getNetworkService()
{
    auto ver = DSysInfo::majorVersion().toInt();
    if (ver > 20) {
        return QString("org.deepin.dde.Network1");
    }
    return QString("com.deepin.daemon.Network");
}

inline const QString getNetworkPath()
{
    auto ver = DSysInfo::majorVersion().toInt();
    if (ver > 20) {
        return QString("/org/deepin/dde/Network1");
    }
    return QString("com/deepin/daemon/Network");
}

inline const char *getNetworkInterface()
{
    auto ver = DSysInfo::majorVersion().toInt();
    if (ver > 20) {
        return "org.deepin.dde.Network1";
    }
    return "com.deepin.daemon.Network";
}

#define NETWORK_DBUS_INTERFACE getNetworkInterface()
#define NETWORK_DBUS_SERVICE getNetworkService()
#define NETWORK_DBUS_PATH getNetworkPath()
*/

// 使用系统统一的DBus获取网络状态
#define NETWORK_DBUS_INTERFACE "org.freedesktop.NetworkManager"
#define NETWORK_DBUS_SERVICE "org.freedesktop.NetworkManager"
#define NETWORK_DBUS_PATH "/org/freedesktop/NetworkManager"

DOANetWorkDBus::DOANetWorkDBus(QObject *parent)
    : QDBusAbstractInterface(NETWORK_DBUS_SERVICE, NETWORK_DBUS_PATH, NETWORK_DBUS_INTERFACE, QDBusConnection::systemBus(), parent)
{
    // Register NetWorkState enum with Qt meta type system
    // This must be done before any signal/slot connections that use this type
    static bool metaTypeRegistered = false;
    if (!metaTypeRegistered) {
        qRegisterMetaType<DOANetWorkDBus::NetWorkState>("DOANetWorkDBus::NetWorkState");
        metaTypeRegistered = true;
    }

    if (!this->isValid()) {
        qCWarning(ClientLogger) << "Error connecting remote object, service:" << this->service() << ",path:" << this->path() << ",interface" << this->interface();
    }

    //关联后端dbus触发信号
    if (!QDBusConnection::systemBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", "sa{sv}as", this, SLOT(propertiesChanged(QDBusMessage)))) {
        qCWarning(ClientLogger) << "the connection was fail!";
    }
}

/**
 * @brief getNetWorkState           获取网络状态
 * @return
 */
DOANetWorkDBus::NetWorkState DOANetWorkDBus::getNetWorkState()
{
    int state = getPropertyByName("State").toInt();
    return (state == 70) ? DOANetWorkDBus::Active : DOANetWorkDBus::Disconnect;
}

//根据属性名称获取对应属性值
QVariant DOANetWorkDBus::getPropertyByName(const char *porpertyName)
{
    QDBusInterface dbusinterface(this->service(), this->path(), this->interface(), QDBusConnection::systemBus(), this);
    return dbusinterface.property(porpertyName);
}

//监听服务对象信号
void DOANetWorkDBus::propertiesChanged(const QDBusMessage &msg)
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
        if (prop == "State") {
            int state = changedProps[prop].toInt();
            if(70 == state){
               emit sign_NetWorkChange(DOANetWorkDBus::Active);
            }else if(20 == state){
                emit sign_NetWorkChange(DOANetWorkDBus::Disconnect);
            }

        }
    }
}
