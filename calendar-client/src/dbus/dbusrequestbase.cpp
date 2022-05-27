/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "dbusrequestbase.h"
#include <QDebug>

DbusRequestBase::DbusRequestBase(const QString &path, const QString &interface, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(DBUS_SERVER_NAME, path, interface.toStdString().c_str(), connection, parent)
{
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), this->interface(), "", this, SLOT(slotDbusCall(QDBusMessage)))) {
        qWarning() << "the connection was fail!" << "path: " << this->path() << "interface: " << this->interface();
    };
}

void DbusRequestBase::setCallbackFunc(CallbackFunc func)
{
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
    QDBusPendingCall async = QDBusAbstractInterface::asyncCall(method, args);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(async, method, this);
    //将回调函数放进CallWatcher中，随CallWatcher调用结果返回
    watcher->setCallbackFunc(m_callbackFunc);
    //清楚回调函数，防止多方法调用时混淆
    setCallbackFunc(nullptr);
    connect(watcher, &CDBusPendingCallWatcher::signalCallFinished, this, &DbusRequestBase::slotCallFinished);
}

/**
 * @brief DbusRequestBase::asyncCall
 * 异步访问dbus接口
 * @param method    dbus方法名
 * @param args  参数
 */
void DbusRequestBase::asyncCall(const QString &method,
                                const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4,
                                const QVariant &arg5, const QVariant &arg6, const QVariant &arg7, const QVariant &arg8)
{
    QDBusPendingCall async = QDBusAbstractInterface::asyncCall(method, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(async, method, this);
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
    qInfo() << msg.member();
}
