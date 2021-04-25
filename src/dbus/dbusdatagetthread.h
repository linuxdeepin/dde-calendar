/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#ifndef DBUSDATAGETTHREAD_H
#define DBUSDATAGETTHREAD_H

#include "calendardbus.h"
#include "schedulestructs.h"

#include <QObject>
#include <QThread>

class CSchedulesDBus;
class DataGetWork;
class CaLunarMonthInfo;

struct YearScheduleInfo {
    int                             m_year;
    QMap<int,QDate>                 m_firstDay;
    QMap<int,QVector<ScheduleDateRangeInfo> >     m_monthInfo;
};
Q_DECLARE_METATYPE(YearScheduleInfo);

struct YearLunar {
    int m_year;
    QMap<int,CaLunarMonthInfo *>          m_monthLunar;
};

struct YearFestival {
    int                 m_year;
    QMap<QDate,int>     m_dayFestival;      //1:xiu 2:ban
};


class DbusDataGetThread : public QObject
{
    Q_OBJECT
public:
    explicit DbusDataGetThread(CSchedulesDBus *_DataManage,QObject *parent = nullptr);
    ~DbusDataGetThread();
signals:
    //获取日程信息
    void getScheduleInfo(YearScheduleInfo *ScheduleInfo);
    //获取阴历信息
    void getLunarInfo(YearLunar *lunarInfo);
    //获取节假日信息
    void getFestivalInfo(YearFestival *festivalInfo);

    void signalsgetInfo(const QString &str);

signals:
    void signalGetScheduleSuccess();

public slots:
private:
    QThread                 workerThread;
    DataGetWork *m_work = nullptr;
};


class DataGetWork :public QObject
{
    Q_OBJECT
public:
    explicit DataGetWork(CSchedulesDBus *_DataManage);
    ~DataGetWork();
public:
    //获取日程信息
    void getScheduleInfo(YearScheduleInfo *ScheduleInfo);
    //获取阴历信息
    void getLunarInfo(YearLunar *lunarInfo);
    //获取节假日信息
    void getFestivalInfo(YearFestival *festivalInfo);

signals:
    void signalGetScheduleSuccess();

private:
    CSchedulesDBus *m_DataManage = nullptr;
};

#endif // DBUSDATAGETTHREAD_H
