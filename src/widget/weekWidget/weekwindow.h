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
#ifndef WEEKWINDOW_H
#define WEEKWINDOW_H

#include "customframe.h"
#include "schedulestructs.h"
#include "calendardbus.h"

#include <DFrame>
#include <DIconButton>
#include <DImageButton>
#include <DPushButton>
#include <DMainWindow>

#include <QWidget>
#include <QDate>
#include <QLabel>
#include <QLabel>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class CWeekHeadView;
class CaLunarDayInfo;
class CWeekView;
class CScheduleView;
class CSchceduleSearchView;
class CTodyButton;
/**
 * @brief The CWeekWindow class
 */
class CWeekWindow: public QMainWindow
{
    Q_OBJECT
public:
    CWeekWindow(QWidget *parent = nullptr);
    ~CWeekWindow() override;
    void setDate(QDate date);
    void setSelectSchedule(const ScheduleDtailInfo &scheduleInfo);
    void setFirstWeekday(int weekday);
    void setLunarVisible(bool state);
    void initUI();
    void initConnection();
    void setTheMe(int type = 0);
    void setTime(QTime time);
    void setSearchWFlag(bool flag);
    void clearSearch();
    /**
     * @brief updateHigh CScheduleView updateHigh
     */
    void updateHigh();
signals:
    /**
     * @brief dateSelected
     * @param date
     * @param detail
     */
    void dateSelected(const QDate date, const CaLunarDayInfo &detail) const;
    /**
     * @brief signalsWUpdateShcedule 更新日程信号
     * @param w
     * @param id
     */
    void signalsWUpdateShcedule(QMainWindow *w, int id = 0);
    /**
     * @brief signalsReturnTodayUpdate 返回今天的信号
     * @param w 视图窗口
     */
    void signalsReturnTodayUpdate(QMainWindow *w);
    /**
     * @brief signalsCurrentScheduleDate 切换窗口的信号
     * @param date 时间
     */
    void signalsCurrentScheduleDate(QDate date);
    /**
     * @brief signalViewtransparentFrame 窗口转换信号
     * @param type
     */
    void signalViewtransparentFrame(int type);
    /**
     * @brief signalCurrentDate 设置时间信号
     * @param date 时间
     */
    void signalCurrentDate(QDate date);
    /**
     * @brief signalsViewSelectDate 选择时间的信号
     * @param date 时间
     */
    void signalsViewSelectDate(QDate date);
public slots:
    void slotReturnTodayUpdate();
    void slotIsDragging(bool &isDragging);
public slots:
    void slotupdateSchedule(int id = 0);
    void slotTransitSchedule(int id = 0);
    void slotTransitSearchSchedule(int id = 0);
    void slotScheduleHide();
private slots:
    void slotprev();
    void slotnext();
    void slottoday();
    void slotCurrentWeek(QDate date, QDate currentDate);
    void slotcurrentDateLunarChanged(QVector<QDate> vdate, QVector<CaLunarDayInfo> vdetail, int type = 0);
    void slotcurrentDateChanged(QDate date);
    void slotsearchDateSelect(QDate date);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    //周视图头weekheadview
    CWeekHeadView *m_weekHeadView = nullptr;
    //背景
    DFrame *m_contentBackground = nullptr;
    //上一周按钮
    DIconButton *m_prevButton = nullptr;
    //下一周按钮
    DIconButton *m_nextButton = nullptr;
    //返回今天按钮
    CTodyButton       *m_today = nullptr;
    //当前日期
    QDate              m_currentdate;
    //年份label
    QLabel *m_YearLabel = nullptr;
    //阴历年份label
    QLabel *m_YearLunarLabel = nullptr;
    //周lable
    QLabel *m_weekLabel = nullptr;
    //weekview
    CWeekView         *m_weekview = nullptr;
    //title中间的背景
    CustomFrame *m_todayframe = nullptr;
    //日程view
    CScheduleView *m_scheduleView = nullptr;
    QSpacerItem *m_spaceitem = nullptr;
    //周视图布局
    QVBoxLayout *m_mainhLayout = nullptr;
    //搜索文字
    QString m_searchText;
    //总体布局
    QHBoxLayout *m_tmainLayout = nullptr;
    //搜索标志
    bool m_searchfalg = false;
};

#endif // YEARWINDOW_H
