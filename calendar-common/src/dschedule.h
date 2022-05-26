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
    //TODO:提醒规则

    //TODO:重复规则

    typedef QSharedPointer<DSchedule> Ptr;
    typedef QVector<DSchedule::Ptr> List;

    DSchedule();
    DSchedule(const DSchedule &schedule);

    DSchedule *clone() const override;

    QString scheduleTypeID() const;
    void setScheduleTypeID(const QString &typeID);

    bool isValid() const;

    DSchedule &operator=(const DSchedule &schedule);

    bool operator==(const DSchedule &schedule) const;

    bool operator<(const DSchedule &schedule) const;

    bool operator>(const DSchedule &schedule) const;

    static bool fromJsonString(DSchedule::Ptr &schedule, const QString &json);
    static bool toJsonString(const DSchedule::Ptr &schedule, QString &json);

    static bool fromIcsString(DSchedule::Ptr &schedule, const QString &string);
    static QString toIcsString(const DSchedule::Ptr &schedule);

    //
    static QMap<QDate, DSchedule::List> fromMapString(const QString &json);
    static QString toMapString(const QMap<QDate, DSchedule::List> &scheduleMap);

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
