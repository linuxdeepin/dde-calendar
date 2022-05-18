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
#ifndef ACCOUNTMANAGERSERVICE_H
#define ACCOUNTMANAGERSERVICE_H

#include "dservicebase.h"
#include "daccountservice.h"

#include <QString>

/**
 * @brief The DAccountManagerService class      帐户管理服务
 */
class DAccountManagerService : public DServiceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.AccountManager")
public:
    explicit DAccountManagerService(QObject *parent = nullptr);

    /**
     * @brief getAccountList        获取帐户列表
     * @return
     */
    Q_SCRIPTABLE QString getAccountList();

    //稍后提醒相关接口
    Q_SCRIPTABLE void remindJob(const QString &scheduleID, const qint64 recurID);

    /**
     * @brief updateRemindJob       每隔10分钟更新提醒日程
     * @param isClear               是否情况日程定时任务数据库
     */
    Q_SCRIPTABLE void updateRemindJob(bool isClear);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param accountID             帐户id
     * @param jobID                 日程id
     * @param jobID                 日程重复id
     * @param operationNum          操作编号 ， 1：打开日历，2：稍后提醒 3： 明天提醒 4： 提前1天提醒 5:关闭按钮
     */
    Q_SCRIPTABLE void notifyMsgHanding(const QString &accountID, const QString &scheduleID, const qint64 recurID, const qint32 operationNum);
    Q_SCRIPTABLE void downloadByAccountID(const QString &accountID);
    Q_SCRIPTABLE void uploadNetWorkAccountData();

signals:
    Q_SCRIPTABLE void accountUpdate(const QStringList &accountIDs);
public slots:
    //TODO：监听网络帐户管理信号和Union ID登陆退出状态
private:
    QString getUnionIDAccount();

private:
    QMap<QString, DAccountService *> m_AccountServiceMap;
};

#endif // ACCOUNTMANAGERSERVICE_H
