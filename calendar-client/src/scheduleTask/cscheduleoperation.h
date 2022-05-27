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
#ifndef CSCHEDULEOPERATION_H
#define CSCHEDULEOPERATION_H

#include "scheduledatamanage.h"

#include <QObject>
#include <QWidget>

class CScheduleDBus;
/**
 * @brief The CScheduleOperation class
 * 日程操作类,创建,修改和删除日程
 */
//TODO:此类需要修改
class CScheduleOperation : public QObject
{
    Q_OBJECT
public:
    explicit CScheduleOperation(QWidget *parent = nullptr);
    //创建日程
    bool createSchedule(const DSchedule::Ptr &scheduleInfo);
    //修改日程
    bool changeSchedule(const DSchedule::Ptr &newInfo, const DSchedule::Ptr &oldInfo);
    //删除日程  false :取消删除  true: 确定删除
    bool deleteSchedule(const DSchedule::Ptr &scheduleInfo);
    //查询日程
    QString queryScheduleStr(const QString &key, const QDateTime &startTime, const QDateTime &endTime);
    //获取查询日程
    bool queryScheduleInfo(const QString &key, const QDateTime &startTime, const QDateTime &endTime, QMap<QDate, QVector<DSchedule>> &info);
    //删除日程 仅删除此日程 不弹框提醒
    void deleteOnlyInfo(const DSchedule &scheduleInfo);

    //创建日程类型
    bool createJobType(DScheduleType &jobTypeInfo); //新增时，颜色可能是：自定义/默认类型。以“自定义颜色编码默认为0”来区分.
    //更新日程类型
    //更新名称和颜色，颜色可能是：自定义-自定义、自定义-默认类型、默认类型-默认类型
    bool updateJobType(DScheduleType &oldJobTypeInfo, DScheduleType &newJobTypeInfo);
    //获取日程类型列表
    bool getJobTypeList(QList<DScheduleType> &lstJobTypeInfo);
    //删除日程类型
    bool deleteJobType(const QString &iJobTypeNo);
    //    //获取日程类型是否被使用
    bool isJobTypeUsed(const QString &iJobTypeNo);
    //获取颜色类型列表
    bool getColorTypeList(QList<DTypeColor> &lstColorTypeInfo);

private:
    //修改重复日程
    bool changeRecurInfo(const DSchedule &newinfo, const DSchedule &oldinfo);
    //修改重复日程,仅修改此日程
    bool changeOnlyInfo(const DSchedule &newinfo, const DSchedule &oldinfo);
    //修改重复规则
    void changeRepetitionRule(DSchedule &newinfo, const DSchedule &oldinfo);
    //更新日程类型(这里的接口是私有的，供updateJobType同名接口调用，在另一接口中做修改的逻辑，这里实现功能)
    bool updateJobType(const DScheduleType &jobTypeInfo);
    //农历每年闰月提示信息
    void lunarMessageDialogShow(const DSchedule &newinfo);
    //根据新旧日程信息判断是否提示
    void showLunarMessageDialog(const DSchedule &newinfo, const DSchedule &oldinfo);
signals:
public slots:
private:
    CScheduleDBus                   *m_DBusManager = nullptr;
    QWidget                         *m_widget;
};

#endif // CSCHEDULEOPERATION_H
