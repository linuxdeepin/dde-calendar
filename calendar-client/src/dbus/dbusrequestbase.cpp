// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusrequestbase.h"
#include "commondef.h"
#include <QDebug>

// Add logging category
Q_LOGGING_CATEGORY(dbusRequestBase, "calendar.dbus.base")

DbusRequestBase::DbusRequestBase(const QString &path, const QString &interface, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(DBUS_SERVER_NAME, path, interface.toStdString().c_str(), connection, parent)
{
    qCDebug(dbusRequestBase) << "Initializing DBus Request Base with path:" << path << "interface:" << interface;
    
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), this->interface(), "", this, SLOT(slotDbusCall(QDBusMessage)))) {
        qCWarning(dbusRequestBase) << "Failed to connect DBus signal for path:" << this->path() << "interface:" << this->interface();
    };
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties", "", this, SLOT(slotDbusCall(QDBusMessage)))) {
        qCWarning(dbusRequestBase) << "Failed to connect DBus Properties signal for path:" << this->path() << "interface:" << this->interface();
    };
}

void DbusRequestBase::setCallbackFunc(CallbackFunc func)
{
    qCDebug(dbusRequestBase) << "Setting callback function for DBus request";
    m_callbackFunc = func;
}

/**
 * @brief DbusRequestBase::asyncCall
 * 异步访问dbus接口
 * @param method    dbus方法名
 * @param args  参数
 */
void DbusRequestBase::asyncCall(const QString &method, const QList<QVariant> &args)
{
    qCDebug(dbusRequestBase) << "Making async DBus call for method:" << method << "with" << args.size() << "arguments";
    QDBusPendingCall async = QDBusAbstractInterface::asyncCallWithArgumentList(method, args);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(async, method, this);
    //将回调函数放进CallWatcher中，随CallWatcher调用结果返回
    watcher->setCallbackFunc(m_callbackFunc);
    //清楚回调函数，防止多方法调用时混淆
    setCallbackFunc(nullptr);
    connect(watcher, &CDBusPendingCallWatcher::signalCallFinished, this, &DbusRequestBase::slotCallFinished);
}

void DbusRequestBase::asyncCall(const QString &method, const QString &callName, const QList<QVariant> &args)
{
    qCDebug(dbusRequestBase) << "Making async DBus call for method:" << method << "with call name:" << callName << "and" << args.size() << "arguments";
    QDBusPendingCall async = QDBusAbstractInterface::asyncCallWithArgumentList(method, args);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(async, callName, this);
    //将回调函数放进CallWatcher中，随CallWatcher调用结果返回
    watcher->setCallbackFunc(m_callbackFunc);
    //清楚回调函数，防止多方法调用时混淆
    setCallbackFunc(nullptr);
    connect(watcher, &CDBusPendingCallWatcher::signalCallFinished, this, &DbusRequestBase::slotCallFinished);
}

/**
 * @brief slotDbusCall
 * dbus服务端调用
 * @param msg 调用消息
 */
void DbusRequestBase::slotDbusCall(const QDBusMessage &msg)
{
    qCDebug(dbusRequestBase) << "Received DBus call with member:" << msg.member();
}
