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
#include "dbusaccountrequest.h"
#include <QDBusInterface>

#include <QDBusReply>
#include <QDBusInterface>
#include <QtDebug>

DbusAccountRequest::DbusAccountRequest(const QString &path, const QString &interface, QObject *parent)
    : DbusRequestBase(path, interface, QDBusConnection::sessionBus(), parent)
{
}

/**
 * @brief getAccountInfo        获取帐户信息
 * @return
 */
void DbusAccountRequest::getAccountInfo()
{
    asyncCall("getAccountInfo");
}

/**
 * @brief DbusAccountRequest::setAccountExpandStatus
 * 设置帐户列表展开状态
 * @param expandStatus 展开状态
 */
void DbusAccountRequest::setAccountExpandStatus(bool expandStatus)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("isExpand", QVariant(expandStatus));
}

void DbusAccountRequest::setAccountState(DAccount::AccountStates state)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("accountState", QVariant(state));
}

void DbusAccountRequest::setSyncFreq(DAccount::SyncFreqType freq)
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    interface.setProperty("syncFreq", QVariant(freq));
}

DAccount::AccountStates DbusAccountRequest::getAccountState()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return static_cast<DAccount::AccountStates>(interface.property("accountState").toInt());
}

bool DbusAccountRequest::getSyncState()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return interface.property("syncState").toBool();
}

DAccount::SyncFreqType DbusAccountRequest::getSyncFreq()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    return static_cast<DAccount::SyncFreqType>(interface.property("syncFreq").toInt());
}

/**
 * @brief getScheduleTypeList      获取日程类型信息集
 * @return
 */
void DbusAccountRequest::getScheduleTypeList()
{
    asyncCall("getScheduleTypeList");
}

/**
 * @brief getScheduleTypeByID        根据日程类型ID获取日程类型信息
 * @param typeID                日程类型ID
 * @return
 */
void DbusAccountRequest::getScheduleTypeByID(const QString &typeID)
{
    asyncCall("getScheduleTypeByID", QVariant(typeID));
}

/**
 * @brief createScheduleType         创建日程类型
 * @param typeInfo              类型信息
 * @return                      日程类型ID
 */
void DbusAccountRequest::createScheduleType(const DScheduleType::Ptr &typeInfo)
{
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    asyncCall("createScheduleType", QVariant(jsonStr));
}

/**
 * @brief updateScheduleType         更新日程类型
 * @param typeInfo              类型信息
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::updateScheduleType(const DScheduleType::Ptr &typeInfo)
{
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    asyncCall("updateScheduleType", QVariant(jsonStr));
}

/**
 * @brief DbusAccountRequest::updateScheduleTypeShowState
 * 更新类型显示状态
 * @param typeInfo
 */
void DbusAccountRequest::updateScheduleTypeShowState(const DScheduleType::Ptr &typeInfo)
{
    QString jsonStr;
    DScheduleType::toJsonString(typeInfo, jsonStr);
    QString callName = "updateScheduleTypeShowState";
    asyncCall("updateScheduleType", callName, QVariant(jsonStr));
}

/**
 * @brief deleteScheduleTypeByID     根据日程类型ID删除日程类型
 * @param typeID                日程类型ID
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::deleteScheduleTypeByID(const QString &typeID)
{
    QList<QVariant> argumentList;
    asyncCall("deleteScheduleTypeByID", QVariant(typeID));
}

/**
 * @brief scheduleTypeByUsed         日程类型是否被使用
 * @param typeID                日程类型ID
 * @return
 */
bool DbusAccountRequest::scheduleTypeByUsed(const QString &typeID)
{
    QDBusMessage ret = call("scheduleTypeByUsed", QVariant(typeID));
    return ret.arguments().value(0).toBool();
}

/**
 * @brief createSchedule             创建日程
 * @param ScheduleInfo               日程信息
 * @return                      返回日程ID
 */
void DbusAccountRequest::createSchedule(const DSchedule::Ptr &scheduleInfo)
{
    QString jsonStr;
    DSchedule::toJsonString(scheduleInfo, jsonStr);
    asyncCall("createSchedule", QVariant(jsonStr));
}

/**
 * @brief updateSchedule             更新日程
 * @param ScheduleInfo               日程信息
 * @return                      是否成功，true:更新成功
 */
void DbusAccountRequest::updateSchedule(const DSchedule::Ptr &scheduleInfo)
{
    QString jsonStr;
    DSchedule::toJsonString(scheduleInfo, jsonStr);
    asyncCall("updateSchedule", QVariant(jsonStr));
}

DSchedule::Ptr DbusAccountRequest::getScheduleByScheduleID(const QString &scheduleID)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(scheduleID);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("getScheduleByScheduleID"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "getScheduleTypeByID error ," << reply;
        return nullptr;
    }
    QDBusReply<QString> scheduleReply = reply;

    QString scheduleStr = scheduleReply.value();
    DSchedule::Ptr schedule;
    DSchedule::fromJsonString(schedule, scheduleStr);
    return schedule;
}

/**
 * @brief deleteScheduleByScheduleID      根据日程ID删除日程
 * @param ScheduleID                 日程ID
 * @return                      是否成功，true:删除成功
 */
void DbusAccountRequest::deleteScheduleByScheduleID(const QString &scheduleID)
{
    QList<QVariant> argumentList;
    asyncCall("deleteScheduleByScheduleID", QVariant(scheduleID));
}

/**
 * @brief deleteSchedulesByScheduleTypeID 根据日程类型ID删除日程
 * @param typeID                日程类型ID
 * @return                      是否成功，true:删除成功
 */
void DbusAccountRequest::deleteSchedulesByScheduleTypeID(const QString &typeID)
{
    QList<QVariant> argumentList;
    asyncCall("deleteSchedulesByScheduleTypeID", QVariant(typeID));
}

/**
 * @brief querySchedulesWithParameter        根据查询参数查询日程
 * @param params                        具体的查询参数
 * @return                              查询到的日程集
 */
void DbusAccountRequest::querySchedulesWithParameter(const DScheduleQueryPar::Ptr &params)
{
    //key为空为正常日程获取，不为空则为搜索日程
    QString callName = "searchSchedulesWithParameter";
    if (params->key().isEmpty()) {
        callName = "querySchedulesWithParameter";
        m_priParams = params;
    }
    QString jsonStr = DScheduleQueryPar::toJsonString(params);
    asyncCall("querySchedulesWithParameter", callName, QVariant(jsonStr));
}

bool DbusAccountRequest::querySchedulesByExternal(const DScheduleQueryPar::Ptr &params, QString &json)
{
    QDBusPendingReply<QString> reply = call("querySchedulesWithParameter", QVariant(params));
    if (reply.isError()) {
        qWarning() << reply.error().message();
        return false;
    }
    json = reply.argumentAt<0>();
    return true;
}

void DbusAccountRequest::getSysColors()
{
    asyncCall("getSysColors");
}

QString DbusAccountRequest::getDtLastUpdate()
{
    QDBusInterface interface(this->service(), this->path(), this->interface(), QDBusConnection::sessionBus(), this);
    QString datetime = interface.property("dtLastUpdate").toString();
    return datetime;
}

void DbusAccountRequest::slotCallFinished(CDBusPendingCallWatcher *call)
{
    int ret = 0;
    bool canCall = true;

    if (call->isError()) {
        qWarning() << call->reply().member() << call->error().message();
        ret = 1;
    } else {
        if (call->getmember() == "getAccountInfo") {
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            DAccount::Ptr ptr;
            ptr.reset(new DAccount());
            if (DAccount::fromJsonString(ptr, str)) {
                emit signalGetAccountInfoFinish(ptr);
            } else {
                qWarning() << "AccountInfo Parsing failed!";
                ret = 2;
            }
        } else if (call->getmember() == "getScheduleTypeList") {
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            DScheduleType::List stList;
            if (DScheduleType::fromJsonListString(stList, str)) {
                emit signalGetScheduleTypeListFinish(stList);
            } else {
                qWarning() << "ScheduleTypeList Parsing failed!";
                ret = 2;
            }
        } else if (call->getmember() == "querySchedulesWithParameter") {
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            QMap<QDate, DSchedule::List> map = DSchedule::fromMapString(str);
            emit signalGetScheduleListFinish(map);
        } else if (call->getmember() == "searchSchedulesWithParameter") {
            QDBusPendingReply<QString> reply = *call;
            QString str = reply.argumentAt<0>();
            QMap<QDate, DSchedule::List> map = DSchedule::fromMapString(str);
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
            canCall = false;
            //在发起数据获取刷新数据，并将本回调函数和数据传到下一个事件中
            CallbackFunc func = call->getCallbackFunc();
            if(func) {
                setCallbackFunc([=](CallMessge) {
                    func({0, scheduleTypeId});
                });
            }
            getScheduleTypeList();
        } else if (call->getmember() == "createSchedule") {
            //创建日程结束
            canCall = false;
            //重新读取日程数据
            setCallbackFunc(call->getCallbackFunc());
            querySchedulesWithParameter(m_priParams);
        } else if (call->getmember() == "deleteScheduleByScheduleID" || call->getmember() ==  "updateSchedule") {
            //删除或修改日程结束
            canCall = false;
            //重新读取日程数据
            setCallbackFunc(call->getCallbackFunc());
            querySchedulesWithParameter(m_priParams);
        } else if (call->getmember() == "updateScheduleTypeShowState") {
            //更新日程类型显示状态结束
            canCall = false;
            //重新读取日程数据
            setCallbackFunc(call->getCallbackFunc());
            querySchedulesWithParameter(m_priParams);
        } else if(call->getmember() == "updateScheduleType"
                  || call->getmember() == "deleteScheduleTypeByID") {
            getScheduleTypeList();
        }

        if (canCall && call->getCallbackFunc() != nullptr) {
            call->getCallbackFunc()({ret, ""});
        }
    }
    call->deleteLater();
}

void DbusAccountRequest::slotDbusCall(const QDBusMessage &msg)
{
    if (msg.member() == "PropertiesChanged") {
        QDBusPendingReply<QString, QVariantMap, QStringList> reply = msg;
        onPropertiesChanged(reply.argumentAt<0>(), reply.argumentAt<1>(), reply.argumentAt<2>());
    }
}

void DbusAccountRequest::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    for (QVariantMap::const_iterator it = changedProperties.cbegin(), end = changedProperties.cend(); it != end; ++it) {
        if (it.key() == "syncState") {
            int state = it.value().toInt();
            emit signalSyncStateChange(static_cast<DAccount::AccountSyncState>(state));
        }
        if(it.key() == "dtLastUpdate") {
            emit signalDtLastUpdate(getDtLastUpdate());
        }
        if(it.key() == "accountState") {
            emit signalAccountStateChange(getAccountState());
        }
    }
}
