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
#ifndef DALARMMANAGER_H
#define DALARMMANAGER_H

#include "dreminddata.h"
#include "dschedule.h"

#include <QObject>

class DbusUIOpenSchedule;
class DBusNotify;
//日程提醒管理类
class DAlarmManager : public QObject
{
    Q_OBJECT
public:
    explicit DAlarmManager(QObject *parent = nullptr);
    //更新提醒任务
    void updateRemind(const DRemindData::List &remindList);
    void notifyJobsChanged(const DRemindData::List &remindList);

    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param remindData            提醒日程相关信息
     * @param operationNum          操作编号
     *                              1：打开日历，
     *                              2：稍后提醒 21：15min后提醒 22：一个小时后提醒 23：四个小时后提醒
     *                              3：明天提醒 4： 提前1天提醒
     */
    void notifyMsgHanding(const DRemindData::Ptr &remindData, const int operationNum);

    void remindLater(const DRemindData::Ptr &remindData, const int operationNum);

    int remindJob(const DRemindData::Ptr &remindData, const DSchedule::Ptr &schedule);

private:
    int getRemindLaterDuration(int count, qint64 &duration);

    //获取日程提醒内容
    QString getRemindBody(const DSchedule::Ptr &schedule);

    QString getBodyTimePart(const QDateTime &nowtime, const QDateTime &jobtime, bool allday, bool isstart);

private:
    DbusUIOpenSchedule *m_dbusuiopen; //打开日历前端dbus操作相关
    DBusNotify *m_dbusnotify; //日程提醒dbus操作相关
};

#endif // DALARMMANAGER_H
