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
#include "dbusaccountmanagerrequest.h"
#include <QDebug>

DbusAccountManagerRequest::DbusAccountManagerRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/AccountManager", "com.deepin.dataserver.Calendar.AccountManager", QDBusConnection::sessionBus(), parent)
{

}

/**
 * @brief DbusAccountManagerRequest::getAccountList
 * 请求账户列表
 */
void DbusAccountManagerRequest::getAccountList()
{
    asyncCall("getAccountList");
}

/**
 * @brief DbusAccountManagerRequest::downloadByAccountID
 * 根据账户id下拉数据
 * @param accountID 账户id
 */
void DbusAccountManagerRequest::downloadByAccountID(const QString &accountID)
{
    QList<QVariant> argumentList;
    argumentList << QVariant(accountID);
    asyncCall("downloadByAccountID", argumentList);
}

/**
 * @brief DbusAccountManagerRequest::uploadNetWorkAccountData
 * 更新网络账户数据
 */
void DbusAccountManagerRequest::uploadNetWorkAccountData()
{
    asyncCall("uploadNetWorkAccountData");
}

/**
 * @brief DbusAccountManagerRequest::getCalendarGeneralSettings
 * 获取通用设置
 */
void DbusAccountManagerRequest::getCalendarGeneralSettings()
{
    asyncCall("getCalendarGeneralSettings");
}

/**
 * @brief DbusAccountManagerRequest::setCalendarGeneralSettings
 * 设置通用设置
 * @param ptr   通用设置
 */
void DbusAccountManagerRequest::setCalendarGeneralSettings(DCalendarGeneralSettings::Ptr ptr)
{
    QString jsonStr;
    DCalendarGeneralSettings::toJsonString(ptr, jsonStr);

    QList<QVariant> argumentList;
    argumentList << QVariant(jsonStr);
    asyncCall("setCalendarGeneralSettings", argumentList);
}

/**
 * @brief DbusAccountManagerRequest::slotCallFinished
 * dbus调用完成事件
 * @param call 回调类
 */
void DbusAccountManagerRequest::slotCallFinished(CDBusPendingCallWatcher* call)
{
    bool ret = true;
    //错误处理
    if (call->isError()) {
        //打印错误信息
        qWarning() << call->reply().member() << call->error().message();
        ret = false;
    } else if (call->getmember() == "getAccountList") {
        //"getAccountList"方法回调事件
        QDBusPendingReply<QString> reply = *call;
        //获取返回值
        QString str = reply.argumentAt<0>();
        DAccount::List accountList;
        //解析字符串
        if (DAccount::fromJsonListString(accountList, str)) {
            emit signalGetAccountListFinish(accountList);
        } else {
            qWarning() << "AccountList Parsing failed!";
            ret = false;
        }
    } else if (call->getmember() == "getCalendarGeneralSettings") {
        QDBusPendingReply<QString> reply = *call;
        QString str = reply.argumentAt<0>();
        DCalendarGeneralSettings::Ptr ptr;
        ptr.reset(new DCalendarGeneralSettings());
        if (DCalendarGeneralSettings::fromJsonString(ptr, str)) {
            emit signalGetGeneralSettingsFinish(ptr);
        } else {
            qWarning() << "AccountList Parsing failed!";
            ret = false;
        }
    }

    //执行回调函数
    if (call->getCallbackFunc() != nullptr) {
        call->getCallbackFunc()(ret);
    }
    //释放内存
    call->deleteLater();
}
