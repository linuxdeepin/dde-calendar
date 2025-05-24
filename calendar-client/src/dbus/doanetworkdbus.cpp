// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "doanetworkdbus.h"
#include "commondef.h"

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QtDebug>
#include <QDBusInterface>

// Add logging category
Q_LOGGING_CATEGORY(networkDBus, "calendar.dbus.network")

DOANetWorkDBus::DOANetWorkDBus(QObject *parent)
    : QDBusAbstractInterface(NETWORK_DBUS_NAME, NETWORK_DBUS_PATH, NETWORK_DBUS_INTEERFACENAME, QDBusConnection::sessionBus(), parent)
{
    qCDebug(networkDBus) << "Initializing Network DBus interface";
    
    if (!this->isValid()) {
        qCWarning(networkDBus) << "Failed to connect to remote object - Service:" << this->service() 
                              << "Path:" << this->path() 
                              << "Interface:" << this->interface();
    }

    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", "sa{sv}as", this, SLOT(propertiesChanged(QDBusMessage)))) {
        qCWarning(networkDBus) << "Failed to connect to DBus Properties signal";
    }
}

/**
 * @brief getUserName           获取用户名
 * @return
 */
DOANetWorkDBus::NetWorkState DOANetWorkDBus::getNetWorkState()
{
    auto state = getPropertyByName("State").toInt();
    qCDebug(networkDBus) << "Current network state:" << state;
    return state == 70 ? DOANetWorkDBus::Active : DOANetWorkDBus::Disconnect;
}

//根据属性名称获取对应属性值
QVariant DOANetWorkDBus::getPropertyByName(const char *porpertyName)
{
    qCDebug(networkDBus) << "Getting property value for:" << porpertyName;
    QDBusInterface dbusinterface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return dbusinterface.property(porpertyName);
}

//监听服务对象信号
void DOANetWorkDBus::propertiesChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    // 参数固定长度
    if (3 != arguments.count()) {
        qCWarning(networkDBus) << "Invalid number of arguments in properties changed signal";
        return;
    }
    
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != this->interface()) {
        qCDebug(networkDBus) << "Ignoring properties changed for different interface:" << interfaceName;
        return;
    }
    
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    QStringList keys = changedProps.keys();
    foreach (const QString &prop, keys) {
        if (prop == "State") {
            int state = changedProps[prop].toInt();
            qCDebug(networkDBus) << "Network state changed to:" << state;
            if(70 == state){
               emit sign_NetWorkChange(DOANetWorkDBus::Active);
            }else if(20 == state){
                emit sign_NetWorkChange(DOANetWorkDBus::Disconnect);
            }
        }
    }
}
