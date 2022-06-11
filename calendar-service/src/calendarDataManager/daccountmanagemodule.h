/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#ifndef DACCOUNTMANAGEMODULE_H
#define DACCOUNTMANAGEMODULE_H

#include "syncfilemanage.h"
#include "daccount.h"
#include "daccountmodule.h"
#include "daccountmanagerdatabase.h"
#include "daccountservice.h"

#include <QObject>
#include <QSharedPointer>

//帐户类型总数，若支持的类型增加则需要修改
const int accountTypeCount = 3;

//帐户管理模块
class DAccountManageModule : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAccountManageModule> Ptr;

    explicit DAccountManageModule(QObject *parent = nullptr);

    //获取所有帐户信息
    QString getAccountList();

    //获取通用设置
    QString getCalendarGeneralSettings();
    //设置通用设置
    void setCalendarGeneralSettings(const QString &cgSet);

    int getfirstDayOfWeek();
    void setFirstDayOfWeek(const int firstday);
    int getTimeFormatType();
    void setTimeFormatType(const int timeType);

    void remindJob(const QString &accountID, const QString &alarmID);

    /**
     * @brief updateRemindSchedules     更新未来10分钟的提醒任务
     * @param isClear                   是否清空提醒任务数据库
     */
    void updateRemindSchedules(bool isClear);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param accountID             帐户id
     * @param alarmID               提醒任务id
     * @param operationNum          操作编号 ， 1：打开日历，2：稍后提醒 3： 明天提醒 4： 提前1天提醒 5:关闭按钮
     */
    void notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum);

    void downloadByAccountID(const QString &accountID);
    void uploadNetWorkAccountData();

    //账户登录
    void login();
    //账户登出
    void logout();

    void calendarOpen(bool isOpen);

private:
    void unionIDDataMerging();
    void initAccountDBusInfo(const DAccount::Ptr &account);
signals:
    void firstDayOfWeekChange();
    void timeFormatTypeChange();
    void signalLoginStatusChange();

public slots:
    void slotFirstDayOfWeek(const int firstDay);
    void slotTimeFormatType(const int timeType);
    //TODO：监听网络帐户管理信号和Union ID登陆退出状态
    void slotUidLoginStatueChange(const bool staus);
    //控制中心的同步开关
    void slotSwitcherChange(const bool state);
    //通用设置发生改变
    void slotSettingChange();

private:
    SyncFileManage *m_syncFileManage = nullptr;
    DAccountManagerDataBase::Ptr m_accountManagerDB;
    DAccount::List m_accountList;
    QMap<QString, DAccountModule::Ptr> m_accountModuleMap;
    QMap<QString, DAccountService::Ptr> m_AccountServiceMap[accountTypeCount];
    DCalendarGeneralSettings::Ptr m_generalSetting;
};

#endif // DACCOUNTMANAGEMODULE_H
