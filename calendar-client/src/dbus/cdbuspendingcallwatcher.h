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

//继承QDbus回调观察者，将部分自定义的数据包装在回调类中
class CDBusPendingCallWatcher : public QDBusPendingCallWatcher
{
    Q_OBJECT
public:
    explicit CDBusPendingCallWatcher(const QDBusPendingCall &call, QString member, QObject *parent = nullptr);

    typedef std::function<void(bool)> CallbackFunc;

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
