// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUERYSCHEDULETASK_H
#define QUERYSCHEDULETASK_H

#include "schedulebasetask.h"
#include "../data/queryjsondata.h"
#include "../widget/viewschedulewidget.h"

class queryScheduleTask : public scheduleBaseTask
{
    Q_OBJECT
public:
    explicit queryScheduleTask(CSchedulesDBus *dbus);
    Reply SchedulePress(semanticAnalysisTask &semanticTask) override;

private:
    /**
     * @brief setDateTime 设置查询的开始结束时间
     * @param queryJsonData jsondata句柄
     */
    void setDateTime(QueryJsonData *queryJsonData);
    /**
     * @brief getHalfAYearSchedule 获取半年的日程信息
     * @param titleName 日程titlename
     * @param beginDatetime 查询开始时间
     * @param endDateTime 查询结束时间
     * @return 日程信息
     */
    QVector<ScheduleDateRangeInfo> getSchedule(QString titleName, QDateTime beginDatetime, QDateTime endDateTime);
    /**
     * @brief getEveryDayOrWorkDaySchedule 获取每天或者工作日的日程信息
     * @param queryJsonData jsondatajubing
     * @param repeat 重复类型
     * @return 日程信息
     */
    QVector<ScheduleDateRangeInfo> getEveryDayOrWorkDaySchedule(QueryJsonData *queryJsonData, int repeat);
    /**
     * @brief getRestDaySchedule 获取休息日的日程信息
     * @param queryJsonData jsondata句柄
     * @param queryWeek 需要查询周几的容器
     * @return 日程信息
     */
    QVector<ScheduleDateRangeInfo> getRestDaySchedule(QueryJsonData *queryJsonData, QVector<int> queryWeek);
    /**
     * @brief getEveryWeekSchedule 获取每周的日程信息
     * @param queryJsonData jsondata句柄
     * @param repeatNum 需要查询每周几-周几的容器
     * @return 日程信息
     */
    QVector<ScheduleDateRangeInfo> getEveryWeekSchedule(QueryJsonData *queryJsonData, QVector<int> repeatNum);
    /**
     * @brief getEveryMonthSchedule 获取每月的日程信息
     * @param queryJsonData jsondata句柄
     * @param repeatNum 需要查询每月几号-几号的容器
     * @return 日程信息
     */
    QVector<ScheduleDateRangeInfo> getEveryMonthSchedule(QueryJsonData *queryJsonData, QVector<int> repeatNum);
    /**
     * @brief getEveryYearSchedule 获取每年的日程信息
     * @param queryJsonData jsondata句柄
     * @return 日程信息
     */
    QVector<ScheduleDateRangeInfo> getEveryYearSchedule(QueryJsonData *queryJsonData);
    /**
     * @brief getNonePropertyStatusSchedule 查询不是重复类型的日程
     * @param queryJsonData jsondata句柄
     * @param schedule 获取到的半年的日程信息
     * @return 日程信息
     */
    QVector<ScheduleDateRangeInfo> getNonePropertyStatusSchedule(QueryJsonData *queryJsonData, QVector<ScheduleDateRangeInfo> schedule);
    /**
     * @brief queryOverDueDate 是否查询的是过去的日程
     * @param queryJsonData jsondata句柄
     * @return bool
     */
    bool queryOverDueDate(QueryJsonData *queryJsonData);

private:
    QDateTime       m_BeginDateTime;
    QDateTime       m_EndDateTime;
    viewschedulewidget      *viewWidget = nullptr;


protected:
};

#endif // QUERYSCHEDULETASK_H
