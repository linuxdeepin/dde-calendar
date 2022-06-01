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
#ifndef CDBUSPENDINGCALLWATCHER_H
#define CDBUSPENDINGCALLWATCHER_H

#include <QDBusPendingCallWatcher>

/**
 * @brief The CallMessge struct
 * dbus请求回调数据
 */
struct CallMessge{
    int code;       //返回码，0:请求成功，大于0:请求失败
    QString msg;    //返回码说明，根据实际需求可返回任意字符串
};

/**
 * @brief CallbackFunc
 * dbus请求回调函数类型
 */
typedef std::function<void(CallMessge)> CallbackFunc;

//继承QDbus回调观察者，将部分自定义的数据包装在回调类中
class CDBusPendingCallWatcher : public QDBusPendingCallWatcher
{
    Q_OBJECT
public:
    explicit CDBusPendingCallWatcher(const QDBusPendingCall &call, QString member, QObject *parent = nullptr);

    //设置回调函数
    void setCallbackFunc(CallbackFunc func);
    //获取回调函数
    CallbackFunc getCallbackFunc();

    //设置调用方法名
    QString getmember();

signals:
    void signalCallFinished(CDBusPendingCallWatcher*);

public slots:

private:
    QString m_member;       //调用的dbus方法名
    CallbackFunc m_func = nullptr;  //回调函数指针
};

#endif // CDBUSPENDINGCALLWATCHER_H
