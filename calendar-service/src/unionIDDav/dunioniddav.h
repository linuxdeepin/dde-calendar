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
#ifndef DUNIONIDDAV_H
#define DUNIONIDDAV_H

#include "dcalendargeneralsettings.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "dschedule.h"

/**
 * @brief The DUnionIDDav class
 * Union ID 云同步
 */
class DUnionIDDav
{
public:
    DUnionIDDav();
    //设置帐户信息
    void setAccount(const DAccount::Ptr &account);
    DAccount::Ptr getAccount() const;
    void setScheduleType(const DScheduleType::List &typeList);
    DScheduleType::List getScheduleTypes();

    void uploadSchedule(const DSchedule::Ptr &schedule);
    DSchedule::List downloadSchedule();

    //根据日程云端文件路径删除日程
    void deleteScheduleByPath(const QStringList &schedulePath);

    //设置通用设置信息
    void setCalendarGeneralSettings(const DCalendarGeneralSettings &cgSet);

    //获取云端帐户信息
    bool getCloudAccountData(const QString &accountData);
    //获取云端类型信息
    bool getCloudTypeData(const QString &typeData);

    //创建帐户信息文件
    void createAccountFile(const QString &fileName, const QString &content);
    //创建类型文件
    void createTypeFile(const QString &fileName, const QString &content);
    //创建日程文件
    void createCalendarFile(const QString &fileName, const QString &content);
    //删除文件夹
    void deleteFolder(const QString &path);
    //创建文件夹
    void createFolder(const QString &path);
    //判断文件夹是否存在
    bool exists(const QString &path);

private:
};

#endif // DUNIONIDDAV_H
