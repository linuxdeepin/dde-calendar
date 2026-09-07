// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountmanagerrequest.h"
#include "commondef.h"
#include "dcalendargeneralsettings.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

DbusAccountManagerRequest::DbusAccountManagerRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/AccountManager", "com.deepin.dataserver.Calendar.AccountManager", QDBusConnection::sessionBus(), parent)
{
    const auto connectSignal = [this](const char *signal, const char *signature) {
        if (!QDBusConnection::sessionBus().connect(this->service(), this->path(),
                                                   this->interface(), signal, signature, this,
                                                   SLOT(slotDbusCall(QDBusMessage)))) {
            qCWarning(ClientLogger) << "Failed to connect account manager DBus signal:"
                                    << signal << "path:" << this->path();
        }
    };
    connectSignal("accountUpdate", "");
    connectSignal("calDavAccountStatusChanged", "s");
    connectSignal("calDavAccountValidationFinished", "sbiss");
}

/**
 * @brief setFirstDayofWeek
 * 设置一周首日
 */
void DbusAccountManagerRequest::setFirstDayofWeek(int value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("firstDayOfWeek", QVariant(value));
}

/**
 * @brief DbusAccountManagerRequest::setTimeFormatType
 * 设置时间显示格式
 */
void DbusAccountManagerRequest::setTimeFormatType(int value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("timeFormatType", QVariant(value));
}

/**
 * @brief setFirstDayofWeekSource
 * 设置一周首日来源
 */
void DbusAccountManagerRequest::setFirstDayofWeekSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("firstDayOfWeekSource", QVariant(value));
}

/**
 * @brief setTimeFormatTypeSource
 * 设置时间显示格式来源
 */
void DbusAccountManagerRequest::setTimeFormatTypeSource(DCalendarGeneralSettings::GeneralSettingSource value)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("timeFormatTypeSource", QVariant(value));
}

/**
 * @brief getFirstDayofWeekSource
 * 获取一周首日来源
 */
DCalendarGeneralSettings::GeneralSettingSource DbusAccountManagerRequest::getFirstDayofWeekSource()
{
    QDBusInterface interface(this->service(),
                             this->path(),
                             this->interface(),
                             QDBusConnection::sessionBus(),
                             this);
    bool ok;
    auto val = interface.property("firstDayOfWeekSource").toInt(&ok);
    if (ok) {
        qCDebug(ClientLogger) << "First day of week source:" << val;
        return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val);
    } else {
        qCWarning(ClientLogger) << "Failed to get first day of week source, using default Source_Database";
        return DCalendarGeneralSettings::Source_Database;
    }
}

/**
 * @brief getTimeFormatTypeSource
 * 获取时间显示格式来源
 */
DCalendarGeneralSettings::GeneralSettingSource DbusAccountManagerRequest::getTimeFormatTypeSource()
{
    QDBusInterface interface(this->service(),
                             this->path(),
                             this->interface(),
                             QDBusConnection::sessionBus(),
                             this);
    bool ok;
    auto val = interface.property("timeFormatTypeSource").toInt(&ok);
    if (ok) {
        qCDebug(ClientLogger) << "Time format type source:" << val;
        return static_cast<DCalendarGeneralSettings::GeneralSettingSource>(val);
    } else {
        qCWarning(ClientLogger) << "Failed to get time format type source, using default Source_Database";
        return DCalendarGeneralSettings::Source_Database;
    }
}

/**
 * @brief DbusAccountManagerRequest::getAccountList
 * 请求帐户列表
 */
void DbusAccountManagerRequest::getAccountList()
{
    asyncCall("getAccountList");
}

/**
 * @brief DbusAccountManagerRequest::downloadByAccountID
 * 根据帐户id下拉数据
 * @param accountID 帐户id
 */
void DbusAccountManagerRequest::downloadByAccountID(const QString &accountID)
{
    asyncCall("downloadByAccountID", {QVariant(accountID)});
}

/**
 * @brief DbusAccountManagerRequest::uploadNetWorkAccountData
 * 更新网络帐户数据
 */
void DbusAccountManagerRequest::getCalDavAccountStatusList()
{
    asyncCall("getCalDavAccountStatusList");
}

void DbusAccountManagerRequest::validateCalDavAccount(int providerType, const QString &serverUrl,
                                                        const QString &username, const QString &credentialRef)
{
    asyncCall("validateCalDavAccount", {providerType, serverUrl, username, credentialRef});
}

void DbusAccountManagerRequest::getCalDavAccountConfig(const QString &accountID)
{
    asyncCall("getCalDavAccountConfig", {accountID});
}

void DbusAccountManagerRequest::validateCalDavAccountForUpdate(
    const QString &accountID, int providerType, const QString &serverUrl,
    const QString &username, const QString &credentialRef)
{
    asyncCall("validateCalDavAccountForUpdate",
              {accountID, providerType, serverUrl, username, credentialRef});
}

void DbusAccountManagerRequest::updateCalDavCredentialReference(const QString &accountID, const QString &credentialRef)
{
    asyncCall("updateCalDavCredentialReference", {accountID, credentialRef});
}

void DbusAccountManagerRequest::createCalDavAccount(int providerType, const QString &serverUrl,
                                                     const QString &username, const QString &credentialRef,
                                                     const QString &displayName)
{
    asyncCall("createCalDavAccount", {providerType, serverUrl, username, credentialRef, displayName});
}

void DbusAccountManagerRequest::updateCalDavAccount(const QString &accountID, int providerType,
                                                     const QString &serverUrl, const QString &username,
                                                     const QString &credentialRef, const QString &displayName)
{
    asyncCall("updateCalDavAccount", {accountID, providerType, serverUrl, username, credentialRef,
                                        displayName});
}

void DbusAccountManagerRequest::deleteCalDavAccount(const QString &accountID)
{
    asyncCall("deleteCalDavAccount", {accountID});
}

void DbusAccountManagerRequest::deleteCalDavAccountWithLocalDataOption(const QString &accountID,
                                                                    bool deleteLocalData)
{
    asyncCall("deleteCalDavAccountWithLocalDataOption", {accountID, deleteLocalData});
}

void DbusAccountManagerRequest::resolveAllCalDavConflicts(const QString &accountID, bool keepLocal)
{
    asyncCall("resolveAllCalDavConflicts", {accountID, keepLocal});
}

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
 * @brief DbusAccountManagerRequest::login
 * 帐户登录
 */
void DbusAccountManagerRequest::login()
{
    asyncCall("login");
}

/**
 * @brief DbusAccountManagerRequest::loginout
 * 帐户登出
 */
void DbusAccountManagerRequest::logout()
{
    asyncCall("logout");
}

void DbusAccountManagerRequest::clientIsShow(bool isShow)
{
    QList<QVariant> argumentList;
    argumentList << isShow;
    //不需要返回结果，发送完直接结束
    callWithArgumentList(QDBus::NoBlock, QStringLiteral("calendarIsShow"), argumentList);
}

bool DbusAccountManagerRequest::getIsSupportUid()
{
    qCDebug(ClientLogger) << "Checking UID support";
    QList<QVariant> argumentList;
    QDBusMessage msg = callWithArgumentList(QDBus::Block, QStringLiteral("isSupportUid"), argumentList);
    if (msg.type() == QDBusMessage::ReplyMessage) {
        QVariant variant = msg.arguments().first();
        return variant.toBool();
    } else {
        qCWarning(ClientLogger) << "Failed to get UID support status, defaulting to false";
        return false;
    }
}

void DbusAccountManagerRequest::getIsSupportUidAsync()
{
    asyncCall("isSupportUid");
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
        qCWarning(ClientLogger) << "DBus call error - Method:" << call->getmember()
                                << "Error:" << call->error().message();
        if (call->getmember() == QStringLiteral("validateCalDavAccount")
            || call->getmember() == QStringLiteral("validateCalDavAccountForUpdate")
            || call->getmember() == QStringLiteral("createCalDavAccount")
            || call->getmember() == QStringLiteral("updateCalDavAccount")) {
            emit signalCalDavAccountRequestFailed(call->getmember(), call->error().message());
        }
        // Retry critical calls synchronously on failure
        if (call->getmember() == "getAccountList") {
            qCInfo(ClientLogger) << "Retrying getAccountList synchronously";
            QDBusReply<QString> reply = callWithArgumentList(QDBus::Block, "getAccountList", QList<QVariant>());
            if (reply.isValid()) {
                DAccount::List accountList;
                if (DAccount::fromJsonListString(accountList, reply.value())) {
                    emit signalGetAccountListFinish(accountList);
                    canCall = false;
                }
            }
            if (canCall) ret = 1;
        } else if (call->getmember() == "getCalendarGeneralSettings") {
            qCInfo(ClientLogger) << "Retrying getCalendarGeneralSettings synchronously";
            QDBusReply<QString> reply = callWithArgumentList(QDBus::Block, "getCalendarGeneralSettings", QList<QVariant>());
            if (reply.isValid()) {
                DCalendarGeneralSettings::Ptr ptr;
                ptr.reset(new DCalendarGeneralSettings());
                if (DCalendarGeneralSettings::fromJsonString(ptr, reply.value())) {
                    emit signalGetGeneralSettingsFinish(ptr);
                    canCall = false;
                }
            }
            if (canCall) ret = 1;
        } else if (call->getmember() == "isSupportUid") {
            qCInfo(ClientLogger) << "Retrying isSupportUid synchronously";
            QDBusReply<bool> reply = callWithArgumentList(QDBus::Block, "isSupportUid", QList<QVariant>());
            if (reply.isValid()) {
                emit signalGetIsSupportUidFinish(reply.value());
                canCall = false;
            }
            if (canCall) ret = 1;
        } else {
            ret = 1;
        }
    } else if (call->getmember() == "getAccountList") {
        qCDebug(ClientLogger) << "Processing getAccountList response";
        QDBusPendingReply<QString> reply = *call;
        //获取返回值
        QString str = reply.argumentAt<0>();
        DAccount::List accountList;
        //解析字符串
        if (DAccount::fromJsonListString(accountList, str)) {
            qCDebug(ClientLogger) << "Successfully parsed" << accountList.size() << "accounts";
            emit signalGetAccountListFinish(accountList);
        } else {
            qCWarning(ClientLogger) << "Failed to parse account list JSON";
            ret = 2;
        }
    } else if (call->getmember() == "getCalendarGeneralSettings") {
        qCDebug(ClientLogger) << "Processing getCalendarGeneralSettings response";
        QDBusPendingReply<QString> reply = *call;
        QString str = reply.argumentAt<0>();
        DCalendarGeneralSettings::Ptr ptr;
        ptr.reset(new DCalendarGeneralSettings());
        if (DCalendarGeneralSettings::fromJsonString(ptr, str)) {
            qCDebug(ClientLogger) << "Successfully parsed calendar general settings";
            emit signalGetGeneralSettingsFinish(ptr);
        } else {
            qCWarning(ClientLogger) << "Failed to parse calendar general settings JSON";
            ret = 2;
        }
    } else if (call->getmember() == "isSupportUid") {
        QDBusPendingReply<bool> reply = *call;
        const bool supported = reply.argumentAt<0>();
        emit signalGetIsSupportUidFinish(supported);
    } else if (call->getmember() == "validateCalDavAccount") {
        QDBusPendingReply<QString> reply = *call;
        const QString requestID = reply.argumentAt<0>();
        qCDebug(ClientLogger) << "CalDAV validation DBus method returned"
                                << "requestIdPresent:" << !requestID.isEmpty();
        emit signalValidateCalDavAccountStart(requestID);
        ret = requestID.isEmpty() ? 1 : 0;
    } else if (call->getmember() == "getCalDavAccountConfig") {
        QDBusPendingReply<QString> reply = *call;
        emit signalGetCalDavAccountConfigFinish(reply.argumentAt<0>());
        ret = reply.argumentAt<0>().isEmpty() ? 1 : 0;
    } else if (call->getmember() == "validateCalDavAccountForUpdate") {
        QDBusPendingReply<QString> reply = *call;
        const QString requestID = reply.argumentAt<0>();
        qCDebug(ClientLogger) << "CalDAV update validation DBus method returned"
                                << "requestIdPresent:" << !requestID.isEmpty();
        emit signalValidateCalDavAccountForUpdateStart(requestID);
        ret = requestID.isEmpty() ? 1 : 0;
    } else if (call->getmember() == "getCalDavAccountStatusList") {
        QDBusPendingReply<QString> reply = *call;
        DCalDavAccountStatus::List statusList;
        if (DCalDavAccountStatus::fromJsonListString(statusList, reply.argumentAt<0>())) {
            emit signalGetCalDavAccountStatusListFinish(statusList);
        } else {
            qCWarning(ClientLogger) << "Failed to parse CalDAV account status list";
            ret = 2;
        }
    } else if (call->getmember() == "updateCalDavCredentialReference") {
        QDBusPendingReply<bool> reply = *call;
        ret = reply.argumentAt<0>() ? 0 : 1;
    } else if (call->getmember() == "createCalDavAccount") {
        QDBusPendingReply<QString> reply = *call;
        const QString accountID = reply.argumentAt<0>();
        qCDebug(ClientLogger) << "CalDAV account creation DBus method returned"
                                << "accountIdPresent:" << !accountID.isEmpty();
        emit signalCreateCalDavAccountFinish(accountID);
        ret = accountID.isEmpty() ? 1 : 0;
    } else if (call->getmember() == "updateCalDavAccount") {
        QDBusPendingReply<bool> reply = *call;
        const bool success = reply.argumentAt<0>();
        emit signalUpdateCalDavAccountFinish(success);
        ret = success ? 0 : 1;
    } else if (call->getmember() == "deleteCalDavAccount") {
        QDBusPendingReply<bool> reply = *call;
        const bool success = reply.argumentAt<0>();
        emit signalDeleteCalDavAccountFinish(success);
        ret = success ? 0 : 1;
    } else if (call->getmember() == "deleteCalDavAccountWithLocalDataOption") {
        QDBusPendingReply<bool> reply = *call;
        const bool success = reply.argumentAt<0>();
        emit signalDeleteCalDavAccountFinish(success);
        ret = success ? 0 : 1;
    } else if (call->getmember() == "resolveAllCalDavConflicts") {
        QDBusPendingReply<bool> reply = *call;
        ret = reply.argumentAt<0>() ? 0 : 1;
    } else if (call->getmember() == "setCalendarGeneralSettings") {
        qCDebug(ClientLogger) << "Processing setCalendarGeneralSettings response";
        canCall = false;
        setCallbackFunc(call->getCallbackFunc());
        getCalendarGeneralSettings();
    }

    //执行回调函数
    if (canCall && call->getCallbackFunc() != nullptr) {
        qCDebug(ClientLogger) << "Executing callback for method:" << call->getmember() << "with ret:" << ret;
        call->getCallbackFunc()({ret, ""});
    }
    //释放内存
    call->deleteLater();
}

void DbusAccountManagerRequest::slotDbusCall(const QDBusMessage &msg)
{
    qCDebug(ClientLogger) << "Received DBus signal:" << msg.member();
    if (msg.member() == "accountUpdate") {
        qCDebug(ClientLogger) << "Account update signal received, refreshing account list";
        getAccountList();
    } else if (msg.member() == "calDavAccountStatusChanged") {
        getCalDavAccountStatusList();
    } else if (msg.member() == "calDavAccountValidationFinished") {
        const QString requestID = msg.arguments().value(0).toString();
        const bool success = msg.arguments().value(1).toBool();
        const int validationError = msg.arguments().value(2).toInt();
        const QString errorMessage = msg.arguments().value(3).toString();
        const QString principalDisplayName = msg.arguments().value(4).toString();
        qCDebug(ClientLogger) << "Received CalDAV validation result signal"
                                << "requestIdPresent:" << !requestID.isEmpty()
                                << "success:" << success
                                << "validationError:" << validationError
                                << "errorPresent:" << !errorMessage.isEmpty()
                                << "principalDisplayNamePresent:" << !principalDisplayName.isEmpty();
        emit signalCalDavAccountValidationFinished(requestID, success, validationError, errorMessage,
                                                    principalDisplayName);
    } else if (msg.member() == "PropertiesChanged") {
        qCDebug(ClientLogger) << "Properties changed signal received";
        QDBusPendingReply<QString, QVariantMap, QStringList> reply = msg;
        onPropertiesChanged(reply.argumentAt<0>(), reply.argumentAt<1>(), reply.argumentAt<2>());
    }
}

void DbusAccountManagerRequest::onPropertiesChanged(const QString &, const QVariantMap &changedProperties, const QStringList &)
{
    qCDebug(ClientLogger) << "Processing properties changed with" << changedProperties.size() << "changes";
    for (QVariantMap::const_iterator it = changedProperties.cbegin(), end = changedProperties.cend(); it != end; ++it) {
        if (it.key() == "firstDayOfWeek") {
            qCDebug(ClientLogger) << "First day of week property changed, refreshing general settings";
            getCalendarGeneralSettings();
        } else if (it.key() == "timeFormatType") {
            qCDebug(ClientLogger) << "Time format type property changed, refreshing general settings";
            getCalendarGeneralSettings();
        }
    }
}
