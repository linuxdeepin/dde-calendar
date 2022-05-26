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
#ifndef DACCOUNTMODULE_H
#define DACCOUNTMODULE_H

#include "daccount.h"
#include "daccountdatabase.h"
#include "dschedule.h"

#include <QObject>
#include <QSharedPointer>

//帐户模块
//处理后端数据获取，提醒，上传下载等
class DAccountModule : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<DAccountModule> Ptr;
    typedef QList<Ptr> List;

    explicit DAccountModule(const DAccount::Ptr &account, QObject *parent = nullptr);

    //获取帐户信息
    QString getAccountInfo();

    //获取类型
    QString getScheduleTypeList();
    QString getScheduleTypeByID(const QString &typeID);
    QString createScheduleType(const QString &typeInfo);
    bool deleteScheduleTypeByID(const QString &typeID);
    bool scheduleTypeByUsed(const QString &typeID);

    //日程信息
    QString createSchedule(const QString &scheduleInfo);
    bool updateSchedule(const QString &scheduleInfo);
    QString getScheduleByScheduleID(const QString &scheduleID);
    bool deleteScheduleByScheduleID(const QString &scheduleID);
    QString querySchedulesWithParameter(const QString &params);

    DAccount::Ptr account() const;

private:
    QMap<QDate, DSchedule::List> getScheduleTimesOn(const QDateTime &dtStart, const QDateTime &dtEnd, const DSchedule::List &scheduleList, bool extend = true);

signals:
    //

public slots:
private:
    DAccount::Ptr m_account;
    DAccountDataBase::Ptr m_accountDB;
};

#endif // DACCOUNTMODULE_H
