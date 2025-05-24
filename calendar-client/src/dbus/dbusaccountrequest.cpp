// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountrequest.h"
#include "commondef.h"
#include <QDBusInterface>

#include <QDBusReply>
#include <QDBusInterface>
#include <QtDebug>

// Add logging category
Q_LOGGING_CATEGORY(accountRequest, "calendar.dbus.account")

DbusAccountRequest::DbusAccountRequest(const QString &path, const QString &interface, QObject *parent)
    : DbusRequestBase(path, interface, QDBusConnection::sessionBus(), parent)
{
    qCDebug(accountRequest) << "Initializing DBus Account Request with path:" << path << "interface:" << interface;
}

/**
 * @brief getAccountInfo        获取帐户信息
 * @return
 */
void DbusAccountRequest::getAccountInfo()
{
    qCDebug(accountRequest) << "Getting account info";
    asyncCall("getAccountInfo");
}

/**
 * @brief DbusAccountRequest::setAccountExpandStatus
 * 设置帐户列表展开状态
 * @param expandStatus 展开状态
 */
void DbusAccountRequest::setAccountExpandStatus(bool expandStatus)
{
    qCDebug(accountRequest) << "Setting account expand status to:" << expandStatus;
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("isExpand", QVariant(expandStatus));
}

void DbusAccountRequest::setAccountState(DAccount::AccountStates state)
{
    qCDebug(accountRequest) << "Setting account state to:" << state;
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("accountState", QVariant(state));
}

void DbusAccountRequest::setSyncFreq(const QString &freq)
{
    qCDebug(accountRequest) << "Setting sync frequency to:" << freq;
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("syncFreq", QVariant(freq));
}

DAccount::AccountStates DbusAccountRequest::getAccountState()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    auto state = static_cast<DAccount::AccountStates>(interface.property("accountState").toInt());
    qCDebug(accountRequest) << "Current account state:" << state;
    return state;
}

DAccount::AccountSyncState DbusAccountRequest::getSyncState()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    auto state = static_cast<DAccount::AccountSyncState>(interface.property("syncState").toInt());
    qCDebug(accountRequest) << "Current sync state:" << state;
    return state;
}

QString DbusAccountRequest::getSyncFreq()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    QString freq = interface.property("syncFreq").toString();
    qCDebug(accountRequest) << "Current sync frequency:" << freq;
    return freq;
}

/**
 * @brief getScheduleTypeList      获取日程类型信息集
 * @return
 */
void DbusAccountRequest::getScheduleTypeList()
{
    qCDebug(accountRequest) << "Getting schedule type list";
    asyncCall("getScheduleTypeList");
}

/**
 * @brief getScheduleTypeByID        根据日程类型ID获取日程类型信息
 * @param typeID                日程类型ID
 * @return
 */
void DbusAccountRequest::getScheduleTypeByID(const QString &typeID)
{
    qCDebug(accountRequest) << "Getting schedule type by ID:" << typeID;
    asyncCall("getScheduleTypeByID", {QVariant(typeID)});
}

/**
 * @brief createScheduleType         创建日程类型
 * @param typeInfo              类型信息
 * @return                      日程类型ID
 */
void DbusAccountRequest::createScheduleType(const DScheduleType::Ptr &typeInfo)
{
    qCDebug(accountRequest) << "Creating new schedule type";
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    asyncCall("createScheduleType", {QVariant(jsonStr)});
}

/**
 * @brief updateScheduleType         更新日程类型
 * @param typeInfo              类型信息
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::updateScheduleType(const DScheduleType::Ptr &typeInfo)
{
    qCDebug(accountRequest) << "Updating schedule type";
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    asyncCall("updateScheduleType", {QVariant(jsonStr)});
}

/**
 * @brief DbusAccountRequest::updateScheduleTypeShowState
 * 更新类型显示状态
 * @param typeInfo
 */
void DbusAccountRequest::updateScheduleTypeShowState(const DScheduleType::Ptr &typeInfo)
{
    qCDebug(accountRequest) << "Updating schedule type show state";
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    QString callName = "updateScheduleTypeShowState";
    asyncCall("updateScheduleType", callName, {QVariant(jsonStr)});
}

/**
 * @brief deleteScheduleTypeByID     根据日程类型ID删除日程类型
 * @param typeID                日程类型ID
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::deleteScheduleTypeByID(const QString &typeID)
{
    qCDebug(accountRequest) << "Deleting schedule type with ID:" << typeID;
    QList<QVariant> argumentList;
    asyncCall("deleteScheduleTypeByID", {QVariant(typeID)});
}

/**
 * @brief scheduleTypeByUsed         日程类型是否被使用
 * @param typeID                日程类型ID
 * @return
 */
bool DbusAccountRequest::scheduleTypeByUsed(const QString &typeID)
{
    qCDebug(accountRequest) << "Checking if schedule type is used:" << typeID;
    QDBusMessage ret = call("scheduleTypeByUsed", QVariant(typeID));
    bool used = ret.arguments().value(0).toBool();
    qCDebug(accountRequest) << "Schedule type used status:" << used;
    return used;
}

/**
 * @brief createSchedule             创建日程
 * @param ScheduleInfo               日程信息
 * @return                      返回日程ID
 */
void DbusAccountRequest::createSchedule(const DSchedule::Ptr &scheduleInfo)
{
    qCDebug(accountRequest) << "Creating new schedule";
    QString jsonStr;
    DSchedule::toJsonString(scheduleInfo, jsonStr);
    asyncCall("createSchedule", {QVariant(jsonStr)});
}

/**
 * @brief updateSchedule             更新日程
 * @param ScheduleInfo               日程信息
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::updateSchedule(const DSchedule::Ptr &scheduleInfo)
{
    qCDebug(accountRequest) << "Updating schedule";
    QString jsonStr;
    DSchedule::toJsonString(scheduleInfo, jsonStr);
    asyncCall("updateSchedule", {QVariant(jsonStr)});
}

DSchedule::Ptr DbusAccountRequest::getScheduleByScheduleID(const QString &scheduleID)
{
    qCDebug(accountRequest) << "Getting schedule by ID:" << scheduleID;
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(scheduleID);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("getScheduleByScheduleID"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qCWarning(accountRequest) << "Failed to get schedule by ID:" << scheduleID << "Error:" << reply;
        return nullptr;
    }
    QDBusReply<QString> scheduleReply = reply;

    QString scheduleStr = scheduleReply.value();
    DSchedule::Ptr schedule;
    DSchedule::fromJsonString(schedule, scheduleStr);
    qCDebug(accountRequest) << "Successfully retrieved schedule";
    return schedule;
}

/**
 * @brief deleteScheduleByScheduleID      根据日程ID删除日程
 * @param ScheduleID                 日程ID
 * @return                      是否成功，true:删除成功
 */
void DbusAccountRequest::deleteScheduleByScheduleID(const QString &scheduleID)
{
    qCDebug(accountRequest) << "Deleting schedule with ID:" << scheduleID;
    QList<QVariant> argumentList;
    asyncCall("deleteScheduleByScheduleID", {QVariant(scheduleID)});
}

/**
 * @brief deleteSchedulesByScheduleTypeID 根据日程类型ID删除日程
 * @param typeID                日程类型ID
 * @return                      是否成功，true:删除成功
 */
void DbusAccountRequest::deleteSchedulesByScheduleTypeID(const QString &typeID)
{
    qCDebug(accountRequest) << "Deleting schedules by type ID:" << typeID;
    QList<QVariant> argumentList;
    asyncCall("deleteSchedulesByScheduleTypeID", {QVariant(typeID)});
}

/**
 * @brief querySchedulesWithParameter        根据查询参数查询日程
 * @param params                        具体的查询参数
 * @return                              查询到的日程集
 */
void DbusAccountRequest::querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params)
{
    qCDebug(accountRequest) << "Querying schedules with parameters";
    //key为空为正常日程获取，不为空则为搜索日程
    QString callName = "searchSchedulesWithParameter";
    if (params->key().isEmpty()) {
        callName = "querySchedulesWithParameter";
        m_priParams = params;
    }
    QString jsonStr = DScheduleQueryPar::toJsonString(params);
    asyncCall("querySchedulesWithParameter", callName, {QVariant(jsonStr)});
}

bool DbusAccountRequest::querySchedulesByExternal(const DScheduleQueryPar::Ptr &params, QString &json)
{
    qCDebug(accountRequest) << "Querying schedules by external parameters";
    QDBusPendingReply<QString> reply = call("querySchedulesWithParameter", QVariant::fromValue(params));
    if (reply.isError()) {
        qCWarning(accountRequest) << "Failed to query schedules:" << reply.error().message();
        return false;
    }
    json = reply.argumentAt<0>();
    qCDebug(accountRequest) << "Successfully queried schedules";
    return true;
}

void DbusAccountRequest::getSysColors()
{
    qCDebug(accountRequest) << "Getting system colors";
    asyncCall("getSysColors");
}

QString DbusAccountRequest::getDtLastUpdate()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    QString datetime = interface.property("dtLastUpdate").toString();
    qCDebug(accountRequest) << "Last update time:" << datetime;
    return datetime;
}

void DbusAccountRequest::slotCallFinished(CDBusPendingCallWatcher *call)
{
    int ret = 0;
    bool canCall = true;
    QVariant msg;

    if (call->isError()) {
        qCWarning(accountRequest) << "DBus call error for member:" << call->getmember() << "Error:" << call->error().message();
        ret = 1;
    } else {
        if (call->getmember() == "getAccountInfo") {
            qCDebug(accountRequest) << "Processing getAccountInfo response";
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            DAccount::Ptr ptr;
            ptr.reset(new DAccount());
            if (DAccount::fromJsonString(ptr, str)) {
                qCDebug(accountRequest) << "Successfully parsed account info";
                emit signalGetAccountInfoFinish(ptr);
            } else {
                qCWarning(accountRequest) << "Failed to parse account info";
                ret = 2;
            }
        } else if (call->getmember() == "getScheduleTypeList") {
            qCDebug(accountRequest) << "Processing getScheduleTypeList response";
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            DScheduleType::List stList;
            if (DScheduleType::fromJsonListString(stList, str)) {
                qCDebug(accountRequest) << "Successfully parsed schedule type list with" << stList.size() << "items";
                emit signalGetScheduleTypeListFinish(stList);
            } else {
                qCWarning(accountRequest) << "Failed to parse schedule type list";
                ret = 2;
            }
        } else if (call->getmember() == "querySchedulesWithParameter") {
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            QMap<QDate, DSchedule::List> map = DSchedule::fromQueryResult(str);
            emit signalGetScheduleListFinish(map);
        } else if (call->getmember() == "searchSchedulesWithParameter") {
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            QMap<QDate, DSchedule::List> map = DSchedule::fromQueryResult(str);
            emit signalSearchScheduleListFinish(map);
        } else if (call->getmember() == "getSysColors") {
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            DTypeColor::List list = DTypeColor::fromJsonString(str);
            emit signalGetSysColorsFinish(list);
        } else if (call->getmember() == "createScheduleType") {
            //创建日程类型结束
            QDBusPendingReply<QString> reply = *call;
            QString scheduleTypeId = reply.argumentAt<0>();
            msg = scheduleTypeId;
        } else if (call->getmember() == "updateScheduleTypeShowState") {
            //更新日程类型显示状态结束
            canCall = false;
            //重新读取日程数据
            setCallbackFunc(call->getCallbackFunc());
            querySchedulesWithParameter(m_priParams);
        }
    }
    if (canCall && call->getCallbackFunc() != nullptr) {
        call->getCallbackFunc()({ret, msg});
    }
    call->deleteLater();
}

void DbusAccountRequest::slotDbusCall(const QDBusMessage &msg)
{
    if (msg.member() == "accountUpdate") {
        qCDebug(accountRequest) << "Received account update signal";
        getAccountList();
    } else if (msg.member() == "PropertiesChanged") {
        qCDebug(accountRequest) << "Received properties changed signal";
        QDBusPendingReply<QString, QVariantMap, QStringList> reply = msg;
        onPropertiesChanged(reply.argumentAt<0>(), reply.argumentAt<1>(), reply.argumentAt<2>());
    } else if (msg.member() == "scheduleTypeUpdate") {
        getScheduleTypeList();
    } else if (msg.member() == "scheduleUpdate") {
        //更新全局数据
        querySchedulesWithParameter(m_priParams);
        //更新搜索数据
        emit signalSearchUpdate();
    }
}

void DbusAccountRequest::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    qCDebug(accountRequest) << "Processing properties changed for interface:" << interfaceName;
    for (QVariantMap::const_iterator it = changedProperties.cbegin(), end = changedProperties.cend(); it != end; ++it) {
        if (it.key() == "syncState") {
            int state = it.value().toInt();
            emit signalSyncStateChange(static_cast<DAccount::AccountSyncState>(state));
        } else if (it.key() == "accountState") {
            int state = it.value().toInt();
            emit signalAccountStateChange(static_cast<DAccount::AccountStates>(state));
        }
        if (it.key() == "dtLastUpdate") {
            emit signalDtLastUpdate(getDtLastUpdate());
        }
        if (it.key() == "accountState") {
            emit signalAccountStateChange(getAccountState());
        }
        if (it.key() == "dtLastUpdate") {
            emit signalDtLastUpdate(getDtLastUpdate());
        }
        if (it.key() == "accountState") {
            emit signalAccountStateChange(getAccountState());
        }
        if (it.key() == "firstDayOfWeek") {
            qCDebug(accountRequest) << "First day of week property changed";
            getCalendarGeneralSettings();
        } else if (it.key() == "timeFormatType") {
            qCDebug(accountRequest) << "Time format type property changed";
            getCalendarGeneralSettings();
        }
    }
}

void DbusAccountRequest::importSchedule(QString icsFilePath, QString typeID, bool cleanExists)
{
    qCDebug(accountRequest) << "Importing schedule from:" << icsFilePath << "Type ID:" << typeID << "Clean exists:" << cleanExists;
    asyncCall("importSchedule", {QVariant(icsFilePath), QVariant(typeID), QVariant(cleanExists)});
}

void DbusAccountRequest::exportSchedule(QString icsFilePath, QString typeID)
{
    qCDebug(accountRequest) << "Exporting schedule to:" << icsFilePath << "Type ID:" << typeID;
    asyncCall("exportSchedule", {QVariant(icsFilePath), QVariant(typeID)});
}
