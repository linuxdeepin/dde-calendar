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
#include "cdbuspendingcallwatcher.h"

CDBusPendingCallWatcher::CDBusPendingCallWatcher(const QDBusPendingCall &call, QString member, QObject *parent)
    : QDBusPendingCallWatcher(call, parent)
    , m_member(member)
{
    connect(this, &QDBusPendingCallWatcher::finished, this, [this]() {
        //转发调用完成事件
        emit this->signalCallFinished(this);
    });
}

/**
 * @brief CDBusPendingCallWatcher::setCallbackFunc
 * 设置回调函数
 * @param func 回调函数
 */
void CDBusPendingCallWatcher::setCallbackFunc(CallbackFunc func)
{
    m_func = func;
}

/**
 * @brief CDBusPendingCallWatcher::getCallbackFunc
 * 获取回调函数
 * @return 回调函数
 */
CallbackFunc CDBusPendingCallWatcher::getCallbackFunc()
{
    return m_func;
}

/**
 * @brief CDBusPendingCallWatcher::getmember
 * 设置调用方法名
 * @return 方法名
 */
QString CDBusPendingCallWatcher::getmember()
{
    return m_member;
}
