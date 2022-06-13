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
#ifndef DBUSACCOUNTMANAGERREQUEST_H
#define DBUSACCOUNTMANAGERREQUEST_H

#include "dbusrequestbase.h"
#include "daccount.h"
#include "dcalendargeneralsettings.h"

//所有帐户信息管理类
class DbusAccountManagerRequest : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit DbusAccountManagerRequest(QObject *parent = nullptr);

    //设置一周首日
    void setFirstDayofWeek(int);
    //设置时间显示格式
    void setTimeFormatType(int);

    //获取帐户列表
    void getAccountList();
    //根据帐户id下拉数据
    void downloadByAccountID(const QString &accountID);
    //更新网络帐户数据
    void uploadNetWorkAccountData();
    //获取通用设置
    void getCalendarGeneralSettings();
    //设置通用设置
    void setCalendarGeneralSettings(DCalendarGeneralSettings::Ptr ptr);
    //
    void clientIsShow(bool isShow);

    //帐户登录
    void login();
    //帐户登出
    void logout();

signals:
    //获取帐户列表数据完成信号
    void signalGetAccountListFinish(DAccount::List accountList);
    //获取通用设置完成信号
    void signalGetGeneralSettingsFinish(DCalendarGeneralSettings::Ptr ptr);

public slots:
    //dbus调用完成事件
    void slotCallFinished(CDBusPendingCallWatcher *) override;
    //后端发送信号事件
    void slotDbusCall(const QDBusMessage &msg) override;

private:
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);
};

#endif // DBUSACCOUNTMANAGERREQUEST_H
