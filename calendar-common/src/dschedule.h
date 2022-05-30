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
#ifndef DSCHEDULE_H
#define DSCHEDULE_H

#include "event.h"

#include <QString>
#include <QDebug>
#include <QSharedPointer>

//日程信息
class DSchedule : public KCalendarCore::Event
{
public:
    //提醒规则
    enum AlarmType {
        Alarm_None, //从不
        Alarm_Begin, //日程开始时
        Alarm_15Min_Front, //提前15分钟
        Alarm_30Min_Front, //提前30分钟
        Alarm_1Hour_Front, //提前1小时
        Alarm_1Day_Front, //提前1天
        Alarm_2Day_Front, //提前2天
        Alarm_1Week_Front, //提前1周
        Alarm_9Hour_After, //日程当天9点(全天)
        Alarm_15Hour_Front, //一天前（全天），为日程开始前15小时
        Alarm_39Hour_Front, //2天前（全天），为日程开始前39小时
        Alarm_159Hour_Front, //一周前(全天)，为日程开始前159小时
    };

    //重复规则
    enum RRuleType {
        RRule_None, //从不
        RRule_Day, //每天
        RRule_Work, //每工作日
        RRule_Week, //每周
        RRule_Month, //每月
        RRule_Year, //每年
    };

    typedef QSharedPointer<DSchedule> Ptr;
    typedef QVector<DSchedule::Ptr> List;

    DSchedule();
    DSchedule(const DSchedule &schedule);
    DSchedule(const KCalendarCore::Event &event);

    DSchedule *clone() const override;

    QString scheduleTypeID() const;
    void setScheduleTypeID(const QString &typeID);

    bool isValid() const;

    DSchedule &operator=(const DSchedule &schedule);

    bool operator==(const DSchedule &schedule) const;

    bool operator<(const DSchedule &schedule) const;

    bool operator>(const DSchedule &schedule) const;

    void setAlarmType(const AlarmType &alarmType);
    AlarmType getAlarmType();

    //设置重复规则，若有重复规则，规则的截止次数或日期通过RecurrenceRule::duration 判断为永不，结束与次数还是结束与日期
    void setRRuleType(const RRuleType &rtype);
    RRuleType getRRuleType();

    static bool fromJsonString(DSchedule::Ptr &schedule, const QString &json);
    static bool toJsonString(const DSchedule::Ptr &schedule, QString &json);

    static bool fromIcsString(DSchedule::Ptr &schedule, const QString &string);
    static QString toIcsString(const DSchedule::Ptr &schedule);

    //
    static QMap<QDate, DSchedule::List> fromMapString(const QString &json);
    static QString toMapString(const QMap<QDate, DSchedule::List> &scheduleMap);

private:
    QMap<int, AlarmType> getAlarmMap();

public:
    //日程信息调试打印
    friend QDebug operator<<(QDebug debug, const DSchedule &scheduleJsonData);

    QString fileName() const;
    void setFileName(const QString &fileName);

private:
    QString m_fileName; //日程对应文件名称
    //日程类型
    QString m_scheduleTypeID;
};

#endif // DSCHEDULE_H
