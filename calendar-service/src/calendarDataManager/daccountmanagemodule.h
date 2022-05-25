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

#include "dunioniddbus.h"
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

private:
    void unionIDDataMerging();
signals:

public slots:
    //TODO：监听网络帐户管理信号和Union ID登陆退出状态

private:
    DAccountManagerDataBase::Ptr m_accountManagerDB;
    DUnionIDDbus::Ptr m_unionIDDbus;
    DAccount::List m_accountList;
    QMap<QString, DAccountService::Ptr> m_AccountServiceMap[accountTypeCount];
};

#endif // DACCOUNTMANAGEMODULE_H
