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
#ifndef MONTHWINDOW_H
#define MONTHWINDOW_H

#include "SchecduleRemindWidget.h"

#include <DPushButton>
#include <DFrame>
#include <DMainWindow>

#include <QWidget>
#include <QDate>
#include <QLabel>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE
class CSchceduleSearchView;
class CMonthView;
class CMonthDayView;
class QHBoxLayout;
class CTodyButton;
class CMonthWindow: public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief CMonthWindow 构造函数
     * @param parent 父类
     */
    explicit CMonthWindow(QWidget *parent = nullptr);
    /**
      * @brief ~CMonthWindow 析构函数
      */
    ~CMonthWindow() override;
    void setFirstWeekday(int weekday);
    void setDate(QDate date);
    void setLunarVisible(bool state);
    void setTheMe(int type = 0);
    void setSearchWFlag(bool flag);
    void clearSearch();
public slots:
    void previousMonth();
    void nextMonth();
    void slotsearchDateSelect(QDate date);
protected:
    void wheelEvent(QWheelEvent *) override;
private:
    void initUI();
    void initConnection();
    void initLunar();
    void slideMonth(bool next);
signals:
    /**
     * @brief dateSelected
     * @param date
     * @param detail
     */
    void dateSelected(const QDate date, const CaLunarDayInfo &detail) const;
    /**
     * @brief signalsWUpdateShcedule 更新日程的信号
     * @param w unused
     * @param id unused
     */
    void signalsWUpdateShcedule(QMainWindow *w, int id = 0);
    /**
     * @brief signalsReturnTodayUpdate 返回今天的按钮的信号
     * @param w 视图
     */
    void signalsReturnTodayUpdate(QMainWindow *w);
    /**
     * @brief signalsCurrentScheduleDate
     * @param date
     */
    void signalsCurrentScheduleDate(QDate date);
    /**
     * @brief signalsViewSelectDate 切换视图的信号
     * @param date 日期
     */
    void signalsViewSelectDate(QDate date);
    /**
     * @brief signalViewtransparentFrame 获取视图的焦点
     * @param type
     */
    void signalViewtransparentFrame(int type);
    /**
     * @brief signalCurrentDate 设置时间的信号
     * @param date 日期
     */
    void signalCurrentDate(QDate date);
public slots:
    void slotReturnTodayUpdate();
    void slotScheduleHide();
public slots:
    void slotupdateSchedule(int id = 0);
    void slotTransitSchedule(int id = 0);
    void setSelectSchedule(const ScheduleDtailInfo &scheduleInfo);
protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void slottoday();
    void slotcurrentDateLunarChanged(QDate date,  CaLunarDayInfo detail, int type = 0);
    void slotcurrentDateChanged(QDate date);
    void slotSelectedMonth(QDate date);
private:
    //一个月的视图
    CMonthView *m_monthView = nullptr;
    //12个月份视图
    CMonthDayView *m_monthDayView = nullptr;
    //背景frame
    DFrame *m_contentBackground = nullptr;
    //返回今天的按钮
    CTodyButton *m_today = nullptr;
    //当天日期
    QDate                   m_currentdate;
    //年份label
    QLabel *m_YearLabel = nullptr;
    //阴历年份label
    QLabel *m_YearLunarLabel = nullptr;
    QSpacerItem *m_spaceitem = nullptr;
    /**
     * @brief m_gridWidget 月widget
     *
     * 将月view放入widget内进行显示
     */
    DWidget *m_gridWidget = nullptr;
    //搜索标志
    bool                    m_searchfalg = false;
    //月视图最终的窗口布局
    QHBoxLayout *m_tmainLayout = nullptr;
};

#endif // YEARWINDOW_H
