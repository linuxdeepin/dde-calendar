// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountmanagerrequest.h"
#include "commondef.h"
#include <QDebug>

DbusAccountManagerRequest::DbusAccountManagerRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/AccountManager", "com.deepin.dataserver.Calendar.AccountManager", QDBusConnection::sessionBus(), parent)
{
}

/**
 * @brief DbusAccountManagerRequest::getAccountList
 * 请求帐户列表
 */
DAccount::List DbusAccountManagerRequest::getAccountList()
{
    DAccount::List accountList;
    QList<QVariant> argumentList;
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("getAccountList"), argumentList);
    pCall.waitForFinished();
    QDBusPendingReply<QString> reply = pCall.reply();

    //获取返回值
    QString str = reply.argumentAt<0>();
    //解析字符串
    DAccount::fromJsonListString(accountList, str);
    return accountList;
}

/**
 * @brief DbusAccountManagerRequest::downloadByAccountID
 * 根据帐户id下拉数据
 * @param accountID 帐户id
 */
void DbusAccountManagerRequest::downloadByAccountID(const QString &accountID)
{
    qCInfo(CommonLogger) << "Requesting download for account ID:" << accountID;
    QList<QVariant> argumentList;
    argumentList << QVariant(accountID);
    asyncCall("downloadByAccountID", argumentList);
}

/**
 * @brief DbusAccountManagerRequest::uploadNetWorkAccountData
 * 更新网络帐户数据
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
    asyncCall("setCalendarGeneralSettings", {QVariant(jsonStr)});
}

void DbusAccountManagerRequest::clientIsShow(bool isShow)
{
    QList<QVariant> argumentList;
    argumentList << isShow;
    //不需要返回结果，发送完直接结束
    callWithArgumentList(QDBus::NoBlock, QStringLiteral("calendarIsShow"), argumentList);
}

/**
 * @brief DbusAccountManagerRequest::slotCallFinished
 * dbus调用完成事件
 * @param call 回调类
 */
void DbusAccountManagerRequest::slotCallFinished(CDBusPendingCallWatcher *call)
{
    int ret = 0;
    bool canCall = true;
    //错误处理
    if (call->isError()) {
        //打印错误信息
        qCWarning(CommonLogger) << "DBus call failed - Method:" << call->reply().member() 
                               << "Error:" << call->error().message();
        ret = 1;
    } else if (call->getmember() == "getAccountList") {
        //"getAccountList"方法回调事件
        QDBusPendingReply<QString> reply = *call;
        //获取返回值
        QString str = reply.argumentAt<0>();
        DAccount::List accountList;
        //解析字符串
        if (DAccount::fromJsonListString(accountList, str)) {
            qCDebug(CommonLogger) << "Successfully processed account list with" << accountList.size() << "accounts";
            emit signalGetAccountListFinish(accountList);
        } else {
            qCWarning(CommonLogger) << "Failed to parse account list from JSON response";
            ret = 2;
        }
    } else if (call->getmember() == "getCalendarGeneralSettings") {
        qCDebug(CommonLogger) << "Processing calendar general settings response";
        QDBusPendingReply<QString> reply = *call;
        QString str = reply.argumentAt<0>();
        DCalendarGeneralSettings::Ptr ptr;
        ptr.reset(new DCalendarGeneralSettings());
        if (DCalendarGeneralSettings::fromJsonString(ptr, str)) {
            qCDebug(CommonLogger) << "Successfully parsed calendar general settings";
            emit signalGetGeneralSettingsFinish(ptr);
        } else {
            qCWarning(CommonLogger) << "Failed to parse calendar general settings from JSON";
            ret = 2;
        }
    } else if (call->getmember() == "setCalendarGeneralSettings") {
        qCDebug(CommonLogger) << "Calendar settings updated, refreshing general settings";
        canCall = false;
        setCallbackFunc(call->getCallbackFunc());
        getCalendarGeneralSettings();
    }

    //执行回调函数
    if (canCall && call->getCallbackFunc() != nullptr) {
        qCDebug(CommonLogger) << "Executing callback for method:" << call->getmember() << "with ret:" << ret;
        call->getCallbackFunc()({ret, ""});
    }
    //释放内存
    call->deleteLater();
}
