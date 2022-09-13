// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VIEWSCHEDULEWIDGET_H
#define VIEWSCHEDULEWIDGET_H

#include <QWidget>
#include "icondframe.h"
#include "../data/schedulestructs.h"
#include "scheduleitemwidget.h"

class viewschedulewidget : public IconDFrame
{
    Q_OBJECT
public:
    explicit viewschedulewidget(QWidget *parent = nullptr);
    ~viewschedulewidget();
    void setScheduleDateRangeInfo(QVector<ScheduleDateRangeInfo> &scheduleDateInfo);
    void viewScheduleInfoShow(QVector<ScheduleDateRangeInfo> m_showdate);
    int getScheduleNum(QVector<ScheduleDateRangeInfo> m_showdate);
    void setQueryBeginDateTime(QDateTime begindatetime);
    void setQueryEndDateTime(QDateTime enddatetime);

    QVector<ScheduleDateRangeInfo> queryScheduleWithTime(QVector<ScheduleDateRangeInfo> &scheduleinfo, QTime beginT, QTime endT);
    QVector<ScheduleDateRangeInfo> queryScheduleWithDate(QVector<ScheduleDateRangeInfo> &scheduleinfo, QDate beginD, QDate endD);
    QVector<ScheduleDateRangeInfo> queryScheduleWithWeek(QVector<ScheduleDateRangeInfo> &scheduleinfo, QVector<int> weekDay, int dayofweek = 0, QTime beginT = QTime(0, 0, 0), QTime endT = QTime(0, 0, 0));
    QVector<ScheduleDateRangeInfo> queryScheduleWithMonth(QVector<ScheduleDateRangeInfo> &scheduleinfo, QVector<int> monthDay, int dayofmonth = 0, QTime beginT = QTime(0, 0, 0), QTime endT = QTime(0, 0, 0));

    QVector<ScheduleDateRangeInfo> getAllScheduleInfo();
    QVector<ScheduleDateRangeInfo> getAllRpeatScheduleInfo(int rpeat);
    /**
     * @brief getNextScheduleInfo 获取下一个日程
     * @return 下一个日程信息
     */
    QVector<ScheduleDateRangeInfo> getNextScheduleInfo();
public slots:
    void slotItemPress(const ScheduleDtailInfo &info);

private:
    QVector<ScheduleDtailInfo> m_scheduleInfo;
    QVector<ScheduleDateRangeInfo> m_scheduleDateInfo;
    QVector<ScheduleDateRangeInfo> m_showdate;
    QDateTime m_beginDateTime;
    QDateTime m_endDateTime;
};

#endif // QUERYSCHEDULEWIDGET_H
