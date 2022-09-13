// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARSERVICE_H
#define CALENDARSERVICE_H

#include "src/commondef.h"
#include "src/dbusdatastruct.h"
#include "calendarscheduler.h"
#include "calendarhuangli.h"

#include <QObject>
#include <QDBusContext>

class CalendarService : public QObject
    , protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar")
public:
    explicit CalendarService(QObject *parent = nullptr);

private:
    void initConnections();
Q_SIGNALS:
    Q_SCRIPTABLE void JobsUpdated(const QList<qlonglong> &Ids);
    //日程类型发生改变
    Q_SCRIPTABLE void JobTypeOrColorUpdated();

public Q_SLOTS:
    //LunarCalendar
    Q_SCRIPTABLE QString GetFestivalMonth(quint32 year, quint32 month);
    Q_SCRIPTABLE QString GetHuangLiDay(quint32 year, quint32 month, quint32 day);
    Q_SCRIPTABLE QString GetHuangLiMonth(quint32 year, quint32 month, bool fill);
    Q_SCRIPTABLE CaLunarDayInfo GetLunarInfoBySolar(quint32 year, quint32 month, quint32 day);
    Q_SCRIPTABLE CaLunarMonthInfo GetLunarMonthCalendar(quint32 year, quint32 month, bool fill);

    //CalendarScheduler
    Q_SCRIPTABLE qint64 CreateJob(const QString &jobInfo);
    Q_SCRIPTABLE qint64 CreateType(const QString &typeInfo);
    Q_SCRIPTABLE void DeleteJob(qint64 id);
    Q_SCRIPTABLE QString GetJob(qint64 id);
    Q_SCRIPTABLE QString GetJobs(quint32 startYear, quint32 startMonth, quint32 startDay, quint32 endYear, quint32 endMonth, quint32 endDay);
    Q_SCRIPTABLE QString QueryJobs(const QString &params);
    Q_SCRIPTABLE void UpdateJob(const QString &jobInfo);
    Q_SCRIPTABLE QString QueryJobsWithLimit(const QString &params, qint32 maxNum);
    Q_SCRIPTABLE QString QueryJobsWithRule(const QString &params, const QString &rules);
    //CalendarScheduler: jobtype & color
    Q_SCRIPTABLE bool CreateJobType(const QString &jobTypeInfo);//
    Q_SCRIPTABLE bool DeleteJobType(const int &typeNo);
    Q_SCRIPTABLE bool UpdateJobType(const QString &jobTypeInfo);
    Q_SCRIPTABLE QString GetJobTypeList();
    Q_SCRIPTABLE bool isJobTypeUsed(const int &typeNo);

    Q_SCRIPTABLE QString GetColorTypeList();
    //稍后提醒相关接口
    Q_SCRIPTABLE void remindJob(const qint64 jobID, const qint64 recurID);

    /**
     * @brief updateRemindJob       每隔10分钟更新提醒日程
     * @param isClear               是否情况日程定时任务数据库
     */
    Q_SCRIPTABLE void updateRemindJob(bool isClear);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param jobID                 日程id
     * @param operationNum          操作编号 ， 1：打开日历，2：稍后提醒 3： 明天提醒 4： 提前1天提醒 5:关闭按钮
     */
    Q_SCRIPTABLE void notifyMsgHanding(const qint64 jobID, const qint64 recurID, const qint32 operationNum);

private:
    CalendarScheduler *m_scheduler;
    CalendarHuangLi *m_huangli;
};

#endif // CALENDARSERVICE_H
