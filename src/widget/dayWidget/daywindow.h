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
#ifndef DAYWINDOW_H
#define DAYWINDOW_H

#include "calendardbus.h"
#include "schedulestructs.h"

#include <DMainWindow>
#include <DFrame>

#include <QWidget>
#include <QDate>
#include <QLabel>
DWIDGET_USE_NAMESPACE

class CDayMonthView;
class CScheduleView;
class CSchceduleSearchView;
class QHBoxLayout;
class CustomFrame;
/**
 * @brief The CDayWindow class
 *
 * 日视图主窗口
 *
 */
class CDayWindow: public QMainWindow
{
    Q_OBJECT
public:
    CDayWindow(QWidget *parent = nullptr);
    ~CDayWindow();
    void setDate(QDate date);
    void setSelectSchedule(const ScheduleDtailInfo &scheduleInfo);
    void setSearchWFlag(bool flag);
    void clearSearch();
    void setLunarVisible(bool state);
    void setTheMe(int type = 0);
    void setTime(QTime time);
    void updateHigh();
private:
    void initUI();
    void initConnection();
signals:
    /**
     * @brief dateSelected 选择日期的信号
     * @param date 日期
     * @param detail 阴历信息
     */
    void dateSelected(const QDate date, const CaLunarDayInfo &detail) const;
    /**
     * @brief signalsWUpdateShcedule 更新日程的信号
     * @param w 窗口
     * @param id
     */
    void signalsWUpdateShcedule(QMainWindow *w, int id = 0);
    /**
     * @brief signalsReturnTodayUpdate 返回今天更新
     * @param w 窗口
     */
    void signalsReturnTodayUpdate(QMainWindow *w);
    /**
     * @brief signalViewtransparentFrame 选择窗口的信号
     * @param type
     */
    void signalViewtransparentFrame(int type);
    /**
     * @brief signalCurrentDate 当前日期的信号
     * @param date 日期
     */
    void signalCurrentDate(QDate date);
public slots:
    void slotupdateSchedule(int id = 0);
    void slotTransitSchedule(int id = 0);
    void slotTransitSearchSchedule(int id = 0);
    void slotReturnTodayUpdate();
    void slotCurrentReturnDay();
    void slotScheduleHide();
protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void slotcurrentDateLunarChanged(QDate date,  CaHuangLiDayInfo detail, int type = 0);
    void slotcurrentDateChanged(QDate date);
    void slotsearchDateSelect(QDate date);
    void slotIsDragging(bool &isDragging);
private:
    //daymonthview
    CDayMonthView *m_daymonthView = nullptr;
    //背景frame
    DFrame *m_contentBackground = nullptr;
    //左边的frame
    CustomFrame *m_leftground = nullptr;
    //中间的线
    DVerticalLine *m_verline = nullptr;
    //当前时间
    QDate              m_currentdate;
    //年份label
    QLabel *m_YearLabel = nullptr;
    //阴历年份label
    QLabel *m_LunarLabel = nullptr;
    QLabel *m_SolarDay = nullptr;
    //scheduleview
    CScheduleView *m_scheduleView = nullptr;
    QString m_searchText;
    //搜索标志
    bool m_searchfalg = false;
    //主要布局
    QHBoxLayout *m_mainLayout = nullptr;
};

#endif // YEARWINDOW_H
