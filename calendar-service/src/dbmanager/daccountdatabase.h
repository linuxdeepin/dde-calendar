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
#ifndef DACCOUNTDATABASE_H
#define DACCOUNTDATABASE_H

#include "ddatabase.h"

class DAccountDataBase : public DDataBase
{
    Q_OBJECT
public:
    explicit DAccountDataBase(QObject *parent = nullptr);
    //初始化数据库数据，只有在创建表的时候才需要
    void initDBData();
    virtual void dbOpen(const QString &connectionName);
    QString createSchedule(const QString &schedule);
    bool updateSchedule(const QString &schedule);
    bool deleteScheduleByScheduleID(const QString &scheduleID);
    bool deleteSchedulesByScheduleTypeID(const QString &typeID);
    QString querySchedulesWithParameter(const QString &params);

    /**
     * @brief createScheduleType        创建日程类型
     * @param typeInfo
     * @return
     */
    QString createScheduleType(const QString &typeInfo);
    virtual QString getScheduleTypeByID(const QString &typeID);
    virtual QString getScheduleTypeList();
    bool scheduleTypeByUsed(const QString &typeID);
    bool deleteScheduleTypeByID(const QString &typeID);
    bool updateScheduleType(const QString &typeInfo);
    QString getAccountInfo();

private:
    bool addTypeColor(const int typeColorNo, const QString &strColorHex, const int privilege);

private:
    //初始化日程数据库
    virtual void initScheduleDB();
    virtual void initScheduleType();
    virtual void initTypeColor();
};

#endif // DACCOUNTDATABASE_H
