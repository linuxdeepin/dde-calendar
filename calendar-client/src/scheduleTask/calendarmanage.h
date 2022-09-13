// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CSCHEDULEMANAGE_H
#define CSCHEDULEMANAGE_H

#include "calendardatedatamanage.h"
#include "scheduletask.h"

#include <com_deepin_daemon_timedate.h>

#include <QObject>

typedef  com::deepin::daemon::Timedate DaemonTimeDate;
class CScheduleBaseWidget;
/**
 * @brief The CalendarManage class
 *  日历数据管理类
 */
class CalendarManager : public QObject
{
    Q_OBJECT
public:
    static CalendarManager *getInstance();
    static void releaseInstance();
    //添加显示界面
    void addShowWidget(CScheduleBaseWidget *showWidget);
    //移除显示界面
    void removeShowWidget(CScheduleBaseWidget *showWidget);
    //根据编号获取显示界面
    CScheduleBaseWidget *getShowWidget(const int index);
    //获取显示窗口的数目
    int  getShowWidgetSize();
    //获取日历时间数据管理
    CalendarDateDataManager *getCalendarDateDataManage() const;
    //获取日程任务
    CScheduleTask *getScheduleTask() const;
    //获取是否显示农历信息
    bool getShowLunar() const;
signals:
    void signalTimeFormatChanged(int value);
    void signalDateFormatChanged(int value);
private:
    explicit CalendarManager(QObject *parent = nullptr);
    ~CalendarManager();
private:
    //初始化数据
    void initData();
    //初始化关联
    void initConnection();
    //设置每周首日
    void setWeekBegins(const int value);
    //更新日程信息
    void updateJobs();
    //更新日程类型颜色
    void updateJobTypeColor();
public slots:
    //关联dbus信号，每周首日改变事触发
    void WeekBeginsChanged(int  value);
    //日程更新成功刷新界面
    void slotGetScheduleSuccess();
    //农历更新成功刷新界面
    void slotGetLunarSuccess();
    //接收dbus信号更新日程日程信息
    void slotJobsUpdated();
    //
    void slotUpdateSearchSchedule();
    //更新日程类型颜色
    void slotUpdateJobTypeColor();

    void slotTimeFormatChanged(int value);
    void slotDateFormatChanged(int value);
private:
    static CalendarManager      *m_scheduleManager;
    CalendarDateDataManager     *m_dateManage;
    DaemonTimeDate              *m_timeDateDbus;
    CScheduleTask               *m_scheduleTask;
    QList<CScheduleBaseWidget *> m_showWidget;
    bool                        m_showLunar;
};

#endif // CSCHEDULEMANAGE_H
