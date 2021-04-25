/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#ifndef MONTHVIEW_H
#define MONTHVIEW_H

#include "monthweekview.h"
#include "calendardbus.h"
#include "SchecduleRemindWidget.h"
#include "monthgraphiview.h"

#include <DWidget>

#include <QList>
#include <QDate>
#include <QStyleOption>
#include <QSignalMapper>
#include <QAction>
#include <QGraphicsView>

DWIDGET_USE_NAMESPACE
/**
 * @brief The CMonthView class
 */
class CMonthView: public DWidget
{
    Q_OBJECT
public:
    void setTheMe(int type = 0);
    /**
     * @brief isDragging 是否可以拖拽
     * @return 是否可以拖拽
     */
    bool isDragging()const
    {
        return m_MonthGraphicsView->getDragStatus() !=4;
    }
public:
    explicit CMonthView(QWidget *parent = nullptr);
    ~CMonthView() override;
    void setFirstWeekday(int weekday);
signals:
    /**
     * @brief signalcurrentLunarDateChanged 当前时间改变的信号
     * @param date 改变的日期
     * @param detail 阴历信息
     * @param type 是否显示阴历信息
     */
    void signalcurrentLunarDateChanged(QDate date,  CaLunarDayInfo detail, int type = 0);
    /**
     * @brief signalcurrentDateChanged 当前日期改变的信号
     * @param date 日期
     */
    void signalcurrentDateChanged(QDate date);
    /**
     * @brief signalsSchceduleUpdate 更新日程的信号
     * @param id
     */
    void signalsSchceduleUpdate(int id = 0);
    /**
     * @brief signalsCurrentScheduleDate
     * @param date
     */
    void signalsCurrentScheduleDate(QDate date);
    /**
     * @brief signalViewtransparentFrame 设置窗口的焦点
     * @param type
     */
    void signalViewtransparentFrame(int type);
    /**
     * @brief signalsViewSelectDate 切换视图
     * @param date 日期
     */
    void signalsViewSelectDate(QDate date);
public slots:
    void setCurrentDate(const QDate date);
    void setLunarVisible(bool visible);
    void slotSchceduleUpdate();
    /**
     * @brief setSelectSchedule 设置选择的日程
     * @param scheduleInfo 日程信息
     */
    void setSelectSchedule(const ScheduleDtailInfo &scheduleInfo);
public slots:
    void slotsupdatescheduleD(QWidget *w, QVector<ScheduleDateRangeInfo> &data);
    void slotdelete(int id);
    void slotScheduleRemindWidget(const bool isShow, const ScheduleDtailInfo &out = ScheduleDtailInfo());
signals:
    /**
     * @brief signalsupdatescheduleD  更新日程的信号
     * @param w widget
     * @param begin 开始日期
     * @param end 结束日期
     */
    void signalsupdatescheduleD(QWidget *w, QDate begin, QDate end);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    int getDateIndex(const QDate &date) const;
    const QDate getCellDate(int pos);
    const CaLunarDayInfo getCaLunarDayInfo(const QDate &date);
    void updateDate();
    void updateCurrentLunar(const CaLunarDayInfo &info);
    ScheduleDtailInfo getScheduleInfo(const QDate &beginDate,const QDate &endDate);
private slots:
    void getDbusData();
private:
    //一个月的视图
    CMonthGraphiview *m_MonthGraphicsView = nullptr;
    CalendarDBus *m_DBusInter = nullptr;
    //一个月的日期
    QDate                   m_days[42];
    //当天日期
    QDate                   m_currentDate;
    QDate                   m_createDate;
    //日期对应的当天阴历信息
    QMap<QDate, CaLunarDayInfo> *lunarCache = nullptr;
    //节日列表
    QVector<FestivalInfo> m_festivallist;
    CMonthWeekView *m_weekIndicator = nullptr;
    int m_firstWeekDay = 0;
    // 创建日程
    QAction *m_createAction = nullptr;
    //月视图布局
    QVBoxLayout *m_mainLayout = nullptr;
    int                     m_leftmaagin = 0;
    int                     m_topmagin = 0;
    bool                    m_sflag = true;
    //日程浮框
    SchecduleRemindWidget *m_RemindWidget = nullptr;

    QPoint                  m_PressPoint;
    bool                    isCreate;
    QDate                   m_PressDate;
    QDate                   m_MoveDate;
};

#endif // MYCALENDARWIDGET_H
