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

///**
// * @brief DbusAccountRequest::updateAccountInfo
// * 更新账户信息
// * @param account
// */
//void DbusAccountRequest::updateAccountInfo(const DAccount::Ptr& account)
//{
//    QString jsonStr;
//    DAccount::toJsonString(account, jsonStr);
//    asyncCall("updateAccountInfo", QVariant(jsonStr));
//}

/**
 * @brief DbusAccountRequest::updateAccountExpandStatus
 * 更新账户列表展开状态
 * @param accountInfo
 */
void DbusAccountRequest::updateAccountExpandStatus(const DAccount::Ptr &accountInfo)
{
    //TODO:通过帐户属性设置展开状态
    //    QString jsonStr;
    //    DAccount::toJsonString(accountInfo, jsonStr);
    //    QString callName = "updateAccountExpandStatus";
    //    asyncCall("updateAccountInfo", callName, QVariant(jsonStr));
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
void DbusAccountRequest::scheduleTypeByUsed(const QString &typeID)
{
    asyncCall("scheduleTypeByUsed", QVariant(typeID));
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

void DbusAccountRequest::getSysColors()
{
    asyncCall("getSysColors");
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
            setCallbackFunc([=](CallMessge) {
                func({0, scheduleTypeId});
            });
            getScheduleTypeList();
        } else if (call->getmember() == "createSchedule") {
            //创建日程结束
            canCall = false;
            //重新读取日程数据
            setCallbackFunc(call->getCallbackFunc());
            querySchedulesWithParameter(m_priParams);
        } else if (call->getmember() == "deleteScheduleByScheduleID") {
            //删除日程结束
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
        }

        if (canCall && call->getCallbackFunc() != nullptr) {
            call->getCallbackFunc()({ret, ""});
        }
    }
    call->deleteLater();
}
