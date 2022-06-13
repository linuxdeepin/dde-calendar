/*
* Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     wangyou <wangyou@uniontech.com>
*
* Maintainer: wangyou <wangyou@uniontech.com>
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
#ifndef DOANETWORKDBUS_H
#define DOANETWORKDBUS_H

#include <QDBusAbstractInterface>

#define NETWORK_DBUS_INTEERFACENAME "com.deepin.daemon.Network"
#define NETWORK_DBUS_NAME "com.deepin.daemon.Network"
#define NETWORK_DBUS_PATH "/com/deepin/daemon/Network"

class DOANetWorkDBus : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit DOANetWorkDBus(QObject *parent = nullptr);



    enum NetWorkState {
        Active = 1, //已连接
        Disconnect, //已断开
        Connecting, //连接中
        unknow      //未知
    };
    Q_ENUM(NetWorkState)

    /**
     * @brief getNetWorkState           获取网络状态
     * @return
     */
    DOANetWorkDBus::NetWorkState getNetWorkState();

signals:
    void sign_NetWorkChange(DOANetWorkDBus::NetWorkState);

public slots:
    void propertiesChanged(const QDBusMessage &msg);
private:
    QVariant getPropertyByName(const char *porpertyName);
};

#endif // DOANETWORKDBUS_H
