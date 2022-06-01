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
#ifndef DBUSREQUESTBASE_H
#define DBUSREQUESTBASE_H

#include "cdbuspendingcallwatcher.h"
#include <QDBusAbstractInterface>
#include <QDBusPendingReply>

#define DBUS_SERVER_NAME "com.deepin.dataserver.Calendar"

//继承QDbus接口类，包装自定义接口访问
class DbusRequestBase : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit DbusRequestBase(const QString &path, const QString &interface,
                             const QDBusConnection &connection = QDBusConnection::sessionBus(),
                             QObject *parent = nullptr);

    //设置回调函数
    void setCallbackFunc(CallbackFunc func);

signals:

public slots:
    //dbus服务端调用
    virtual void slotDbusCall(const QDBusMessage &msg);
    //dbus调用完成事件
    virtual void slotCallFinished(CDBusPendingCallWatcher*) = 0;

protected:
    //异步调用，包装异步调用事件
    void asyncCall(const QString &method, const QList<QVariant> &args);
    void asyncCall(const QString &method,
                   const QVariant &arg1 = QVariant(),
                   const QVariant &arg2 = QVariant(),
                   const QVariant &arg3 = QVariant(),
                   const QVariant &arg4 = QVariant(),
                   const QVariant &arg5 = QVariant(),
                   const QVariant &arg6 = QVariant(),
                   const QVariant &arg7 = QVariant(),
                   const QVariant &arg8 = QVariant());
    void asyncCall(const QString &method, QString callName,
                   const QVariant &arg1 = QVariant(),
                   const QVariant &arg2 = QVariant(),
                   const QVariant &arg3 = QVariant(),
                   const QVariant &arg4 = QVariant(),
                   const QVariant &arg5 = QVariant(),
                   const QVariant &arg6 = QVariant(),
                   const QVariant &arg7 = QVariant(),
                   const QVariant &arg8 = QVariant());

private:
    CallbackFunc m_callbackFunc = nullptr; //回调函数
};

#endif // DBUSREQUESTBASE_H
