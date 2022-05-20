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
#ifndef ACCOUNTSERVICE_H
#define ACCOUNTSERVICE_H

#include "dservicebase.h"

#include <QSharedPointer>

class DAccountService : public DServiceBase
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Account_Loalend") //getInterface()) //"com.deepin.dataserver.Account_Loalend"
public:
    typedef QSharedPointer<DAccountService> Ptr;

    DAccountService(const QString &path, const QString &interface, QObject *parent = nullptr);

    /**
     * @brief getAccountInfo        获取帐户信息
     * @return
     */
    Q_SCRIPTABLE QString getAccountInfo();
    /**
     * @brief getScheduleTypeList      获取日程类型信息集
     * @return
     */
    Q_SCRIPTABLE QString getScheduleTypeList();

    /**
     * @brief getScheduleTypeByID        根据日程类型ID获取日程类型信息
     * @param typeID                日程类型ID
     * @return
     */
    Q_SCRIPTABLE QString getScheduleTypeByID(const QString &typeID);

    /**
     * @brief createScheduleType         创建日程类型
     * @param typeInfo              类型信息
     * @return                      日程类型ID
     */
    Q_SCRIPTABLE QString createScheduleType(const QString &typeInfo);

    /**
     * @brief updateScheduleType         更新日程类型
     * @param typeInfo              类型信息
     * @return                      是否成功，true:更新成功
     */
    Q_SCRIPTABLE bool updateScheduleType(const QString &typeInfo);

    /**
     * @brief deleteScheduleTypeByID     根据日程类型ID删除日程类型
     * @param typeID                日程类型ID
     * @return                      是否成功，true:更新成功
     */
    Q_SCRIPTABLE bool deleteScheduleTypeByID(const QString &typeID);

    /**
     * @brief scheduleTypeByUsed         日程类型是否被使用
     * @param typeID                日程类型ID
     * @return
     */
    Q_SCRIPTABLE bool scheduleTypeByUsed(const QString &typeID);

    /**
     * @brief createSchedule             创建日程
     * @param ScheduleInfo               日程信息
     * @return                      返回日程ID
     */
    Q_SCRIPTABLE QString createSchedule(const QString &scheduleInfo);

    /**
     * @brief updateSchedule             更新日程
     * @param ScheduleInfo               日程信息
     * @return                      是否成功，true:更新成功
     */
    Q_SCRIPTABLE bool updateSchedule(const QString &scheduleInfo);

    /**
     * @brief deleteScheduleByScheduleID      根据日程ID删除日程
     * @param ScheduleID                 日程ID
     * @return                      是否成功，true:删除成功
     */
    Q_SCRIPTABLE bool deleteScheduleByScheduleID(const QString &scheduleID);

    /**
     * @brief deleteSchedulesByScheduleTypeID 根据日程类型ID删除日程
     * @param typeID                日程类型ID
     * @return                      是否成功，true:删除成功
     */
    Q_SCRIPTABLE bool deleteSchedulesByScheduleTypeID(const QString &typeID);

    /**
     * @brief querySchedulesWithParameter        根据查询参数查询日程
     * @param params                        具体的查询参数
     * @return                              查询到的日程集
     */
    Q_SCRIPTABLE QString querySchedulesWithParameter(const QString &params);

public:
    /**
     * @brief updateAccountInfo     更新帐户信息
     * @param accountInfo           帐户信息
     */
    void updateAccountInfo(const QString &accountInfo);
signals:
    //TODO: 日程更新信号，日程颜色更新信号

public slots:
};

#endif // ACCOUNTSERVICE_H
