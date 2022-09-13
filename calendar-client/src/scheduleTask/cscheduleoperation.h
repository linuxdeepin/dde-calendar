// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
class CScheduleOperation : public QObject
{
    Q_OBJECT
public:
    explicit CScheduleOperation(QWidget *parent = nullptr);
    //创建日程
    bool createSchedule(const ScheduleDataInfo &scheduleInfo);
    //修改日程
    bool changeSchedule(const ScheduleDataInfo &newInfo, const ScheduleDataInfo &oldInfo);
    //删除日程  false :取消删除  true: 确定删除
    bool deleteSchedule(const ScheduleDataInfo &scheduleInfo);
    //查询日程
    QString queryScheduleStr(const QString &key, const QDateTime &startTime, const QDateTime &endTime);
    //获取查询日程
    bool queryScheduleInfo(const QString &key, const QDateTime &startTime, const QDateTime &endTime, QMap<QDate, QVector<ScheduleDataInfo> > &info);
    //删除日程 仅删除此日程 不弹框提醒
    void deleteOnlyInfo(const ScheduleDataInfo &scheduleInfo);

    //创建日程类型
    bool createJobType(JobTypeInfo &jobTypeInfo);//新增时，颜色可能是：自定义/默认类型。以“自定义颜色编码默认为0”来区分.
    //更新日程类型
    //更新名称和颜色，颜色可能是：自定义-自定义、自定义-默认类型、默认类型-默认类型
    bool updateJobType(JobTypeInfo &oldJobTypeInfo, JobTypeInfo &newJobTypeInfo);
    //获取日程类型列表
    bool getJobTypeList(QList<JobTypeInfo> &lstJobTypeInfo);
    //删除日程类型
    bool deleteJobType(const int iJobTypeNo);
    //获取日程类型是否被使用
    bool isJobTypeUsed(const int iJobTypeNo);
    //获取颜色类型列表
    bool getColorTypeList(QList<JobTypeColorInfo> &lstColorTypeInfo);
private:
    //修改重复日程
    bool changeRecurInfo(const ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo);
    //修改重复日程,仅修改此日程
    bool changeOnlyInfo(const ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo);
    //修改重复规则
    void changeRepetitionRule(ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo);
    //更新日程类型(这里的接口是私有的，供updateJobType同名接口调用，在另一接口中做修改的逻辑，这里实现功能)
    bool updateJobType(const JobTypeInfo &jobTypeInfo);
    //农历每年闰月提示信息
    void lunarMessageDialogShow(const ScheduleDataInfo &newinfo);
    //根据新旧日程信息判断是否提示
    void showLunarMessageDialog(const ScheduleDataInfo &newinfo, const ScheduleDataInfo &oldinfo);
signals:
public slots:
private:
    CScheduleDBus                   *m_DBusManager = nullptr;
    QWidget                         *m_widget;
};

#endif // CSCHEDULEOPERATION_H
