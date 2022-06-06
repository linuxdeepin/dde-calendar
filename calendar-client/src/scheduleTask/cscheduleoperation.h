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
#include "accountitem.h"

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
    explicit CScheduleOperation(const AccountItem::Ptr &accountItem = nullptr, QWidget *parent = nullptr);
    explicit CScheduleOperation(const QString &scheduleTypeID, QWidget *parent = nullptr);
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
    void deleteOnlyInfo(const DSchedule::Ptr &scheduleInfo);


    //是否为节假日日程
    static bool isFestival(const DSchedule::Ptr &schedule);
    //日程是不可修改的
    static bool scheduleIsInvariant(const DSchedule::Ptr &schedule);

private:
    //修改重复日程
    bool changeRecurInfo(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);
    //修改重复日程,仅修改此日程
    bool changeOnlyInfo(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);
    //修改重复规则
    void changeRepetitionRule(DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);

    //农历每年闰月提示信息
    void lunarMessageDialogShow(const DSchedule::Ptr &newinfo);
    //根据新旧日程信息判断是否提示
    void showLunarMessageDialog(const DSchedule::Ptr &newinfo, const DSchedule::Ptr &oldinfo);
signals:
public slots:
private:
    AccountItem::Ptr m_accountItem;
    QWidget                         *m_widget;
};

#endif // CSCHEDULEOPERATION_H
