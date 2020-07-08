/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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
#ifndef SCHEDULEDATAMANAGE_H
#define SCHEDULEDATAMANAGE_H

#include <QThread>
#include <QDate>
#include <QMutex>
#include <DGuiApplicationHelper>

#include "calendardbus.h"
#include "schedulestructs.h"
#include "dbusdatagetthread.h"

DGUI_USE_NAMESPACE
struct CSchedulesColor {
    int type;
    QColor gradientFromC;
    QColor gradientToC;
    QColor dotColor;
    QColor Purecolor;
    QColor shadowcolor;
    QColor textColor;
    QColor timeColor;
    QColor hightlightgradientFromC;
    QColor hightlightgradientToC;
    QColor hightlightPurecolor;
    QColor hovergradientFromC;
    QColor hovergradientToC;
    QColor hoverPurecolor;
    QColor splitColor;
    QColor pressgradientFromC;
    QColor pressgradientToC;
    QColor pressPurecolor;
};

class CSchedulesDBus;
class CScheduleDataCtrl;
class CHuangliDayDataManage;
class CaLunarMonthInfo;

class CScheduleDataManage
{
public:
    static CScheduleDataManage *getScheduleDataManage();
    CScheduleDataCtrl *getscheduleDataCtrl();
    CHuangliDayDataManage *getHuangliDayDataManage();
    CSchedulesColor getScheduleColorByType(int type);
    QColor getSystemActiveColor();
    void setSearchResult(QVector<ScheduleDateRangeInfo> &vData);
    bool getSearchResult(ScheduleDtailInfo info);//true 高亮
    bool getSearchResult(QDate date);//true 高亮
    void setTheMe(int type = 0);
    int getTheme()
    {
        return m_theme;
    }
    void clear();
    ScheduleDtailInfo getPressSelectInfo() const;
    void setPressSelectInfo(const ScheduleDtailInfo &PressSelectInfo);

    void setCurrentYear(int CurrentYear);
    QDate getFirstOfMonth(const QDate &date);
    int getFirstWeekDay() const;
    void setFirstWeekDay(int firstWeekDay);
    YearScheduleInfo *getGetAllYearScheduleInfo() const;

private:
    CScheduleDataManage();
    ~CScheduleDataManage();
private:
    CScheduleDataCtrl                               *m_scheduleDataCtrl;
    CHuangliDayDataManage                           *m_HuangliDayDataManage;
    QVector<CSchedulesColor>                        m_vScheduleColor;
    QVector<ScheduleDateRangeInfo>                  m_vScheduleInfo;
    ScheduleDtailInfo                               m_PressSelectInfo;
    int                                             m_theme = 0;
    int                                             m_CurrentYear;
    int                                             m_firstWeekDay  =0;
    YearScheduleInfo                                *m_GetAllYearScheduleInfo;

    static CScheduleDataManage                      *m_vscheduleDataManage;
};


class CDataProcessThread : public QThread
{
    Q_OBJECT

public:
    /******
    功能:构造函数
    参数:_DataManage 原始数据管理类
    返回值:无
    ******/
    CDataProcessThread(CSchedulesDBus *_DataManage);
    /******
    功能:析构函数
    参数:无
    返回值:无
    ******/
    ~CDataProcessThread();

public:
    /******
    功能:处理数据函数外部调用
    返回值:无
    ******/
    void DataProcessStart(QDate begin, QDate end);

signals:
    /******
    功能:发送信号函数
    返回值:无
    ******/
    void signalsDataProcess(QVector<ScheduleDateRangeInfo> out);

protected:
    /******
    功能:多线程处理函数
    参数:无
    返回值:无
    ******/
    void run();
private:
    CSchedulesDBus       *m_DataManage;
    //锁
    QMutex               m_mutex;
    //更新类型
    int                  m_type;
    QDate                m_begindate;
    QDate                m_enddate;
    //停止标志
    bool                 m_stopFlag;
};
class CScheduleDataCtrl: public QObject
{
    Q_OBJECT
public:
    CScheduleDataCtrl();
    ~CScheduleDataCtrl();
    qint64 addSchedule(const ScheduleDtailInfo &scheduledate);
    bool getScheduleInfoById(int id, ScheduleDtailInfo &out);
    bool getScheduleInfo(const QDate bdate, const QDate edate, QVector<ScheduleDateRangeInfo> &out);
    bool queryScheduleInfo(QString key, QDateTime starttime, QDateTime endtime, QVector<ScheduleDateRangeInfo> &out);
    bool queryScheduleInfo(QString key, QDateTime starttime, QDateTime endtime, QString &out);
    bool updateScheduleInfo(const ScheduleDtailInfo &scheduledate);
    bool deleteScheduleInfoById(int id);
    bool GetType(qint64 jobId, ScheduleType &out);
    QString InfoToJson(const ScheduleDtailInfo &scheduledate);
    ScheduleDtailInfo JsonObjectToInfo(QJsonObject &obj);
    CSchedulesDBus *getDbus() const;

public slots:
    void slotupdatescheduleD(QWidget *w, QDate begin, QDate end);
    void slotDataProcess(QVector<ScheduleDateRangeInfo> out);
signals:
    void signalsupdatescheduleD(QWidget *w, QVector<ScheduleDateRangeInfo> &data);
private:
    void clearData();
private:
    CSchedulesDBus       *m_dbus;
    QMap<QDate, ScheduleDateRangeInfo> *m_scheduleDateCache;
    QDate                m_begindate;
    QDate                m_enddate;
    QWidget              *m_currentWidget;
    CDataProcessThread   *m_thread;
};
class CalendarDBus;
class CHuangliDayDataManage: public QObject
{
public:
    CHuangliDayDataManage();
    ~CHuangliDayDataManage();
    bool getSoloDay(QDate date, QString &str);
    //获取节日状态
    QVector<bool> getDayFlag(QDate date);
private:
    CalendarDBus *m_DBusInter;
};
#endif // SCHEDULEVIEW_H
