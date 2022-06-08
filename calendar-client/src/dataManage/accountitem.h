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
#ifndef ACCOUNTITEM_H
#define ACCOUNTITEM_H

#include "dbus/dbusaccountrequest.h"
#include <QObject>

//单项账户信息管理类
class AccountItem : public QObject
{
    Q_OBJECT
public:
    explicit AccountItem(const DAccount::Ptr &account, QObject *parent = nullptr);

    typedef QSharedPointer<AccountItem> Ptr;

    void resetAccount();

    //获取账户数据
    DAccount::Ptr getAccount();

    //获取日程
    QMap<QDate, DSchedule::List> getScheduleMap();

    // 获取日程类型信息集
    DScheduleType::List getScheduleTypeList();

    //根据类型ID获取日程类型
    DScheduleType::Ptr getScheduleTypeByID(const QString &typeID);
    DScheduleType::Ptr getScheduleTypeByName(const QString &typeName);

    //获取颜色类型列表
    DTypeColor::List getColorTypeList();

    //更新账户列表展开状态
    void setAccountExpandStatus(bool expandStatus);

    //设置账号状态
    void setAccountState(DAccount::AccountStates state);
    //设置同步频率
    void setSyncFreq(DAccount::SyncFreqType freq);
    //获取账号状态
    DAccount::AccountStates getAccountState();
    //获取同步状态
    bool getSyncState();
    //获取同步频率
    DAccount::SyncFreqType getSyncFreq();

    //创建日程类型
    void createJobType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback = nullptr);

    //更新类型信息
    void updateScheduleType(const DScheduleType::Ptr &typeInfo, CallbackFunc callback = nullptr);

    //更新类型显示状态
    void updateScheduleTypeShowState(const DScheduleType::Ptr &scheduleInfo, CallbackFunc callback = nullptr);

    //根据类型ID删除日程类型
    void deleteScheduleTypeByID(const QString &typeID, CallbackFunc callback = nullptr);

    //类型是否被日程使用
    bool scheduleTypeIsUsed(const QString &typeID);

    //创建日程
    void createSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback = nullptr);

    //更新日程
    void updateSchedule(const DSchedule::Ptr &scheduleInfo, CallbackFunc callback = nullptr);

    //根据日程ID获取日程
    DSchedule::Ptr getScheduleByScheduleID(const QString &scheduleID);

    //根据日程ID删除日程
    void deleteScheduleByID(const QString &scheduleID, CallbackFunc callback = nullptr);

    //根据类型ID删除日程
    void deleteSchedulesByTypeID(const QString &typeID, CallbackFunc callback = nullptr);

    //根据查询条件查询数据
    void querySchedulesWithParameter(const int year, CallbackFunc callback = nullptr);
    void querySchedulesWithParameter(const QDateTime &start, const QDateTime &end, CallbackFunc callback = nullptr);
    void querySchedulesWithParameter(const QString &key, const QDateTime &start, const QDateTime &end, CallbackFunc callback = nullptr);
    void querySchedulesWithParameter(const DScheduleQueryPar::Ptr &, CallbackFunc callback = nullptr);

    //对外请求接口，同步
    QString querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end);
    bool querySchedulesByExternal(const QString &key, const QDateTime &start, const QDateTime &end, QMap<QDate, DSchedule::List>& out);

signals:
    void signalAccountDataUpdate();
    void signalScheduleUpdate();
    void signalScheduleTypeUpdate();
    void signalSearchScheduleUpdate();
    void signalLogout();

public slots:
    //获取账户信息完成事件
    void slotGetAccountInfoFinish(DAccount::Ptr);
    //获取日程类型数据完成事件
    void slotGetScheduleTypeListFinish(DScheduleType::List);
    //获取日程数据完成事件
    void slotGetScheduleListFinish(QMap<QDate, DSchedule::List>);
    //搜索日程数据完成事件
    void slotSearchScheduleListFinish(QMap<QDate, DSchedule::List>);
    //获取系统颜色完成
    void slotGetSysColorsFinish(DTypeColor::List);

private:
    void initConnect();

private:
    DAccount::Ptr m_account;    //账户数据
    DScheduleType::List m_scheduleTypeList; //日程类型数据
    DTypeColor::List m_typeColorList;    //颜色数据
    DbusAccountRequest *m_dbusRequest = nullptr; //dbus请求实例
    //一年的日程信息
    QMap<QDate, DSchedule::List> m_scheduleMap{};
    //搜索的日程信息
    QMap<QDate, DSchedule::List> m_searchedScheduleMap{};

    QMap<QString, bool> m_dataStatus;   //数据状态

};

#endif // ACCOUNTITEM_H
