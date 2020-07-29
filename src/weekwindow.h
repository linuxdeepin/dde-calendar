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

#include <QWidget>
#include <DMainWindow>
#include <QDate>
#include <QLabel>
#include <DImageButton>
#include <DPushButton>
#include <QLabel>
#include <DFrame>
#include "calendardbus.h"
#include <DIconButton>
#include <QHBoxLayout>
#include "customframe.h"
#include "schedulestructs.h"
DWIDGET_USE_NAMESPACE

class CWeekHeadView;
class CaLunarDayInfo;
class CWeekView;
class CScheduleView;
class CSchceduleSearchView;
class CTodyButton;
class CWeekWindow: public QMainWindow
{
    Q_OBJECT
public:
    CWeekWindow(QWidget *parent = nullptr);
    ~CWeekWindow() Q_DECL_OVERRIDE;
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
    void setSearchText(QString str);
    void updateHigh();
signals:
    void dateSelected(const QDate date, const CaLunarDayInfo &detail) const;
    void signalsWUpdateShcedule(QMainWindow *w, int id = 0);
    void signalsReturnTodayUpdate(QMainWindow *w);
    void signalsCurrentScheduleDate(QDate date);
    void signalViewtransparentFrame(int type);
    void signalCurrentDate(QDate date);
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
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    CWeekHeadView     *m_weekHeadView; //周视图头
    DFrame *m_contentBackground = nullptr;
    DIconButton      *m_prevButton = nullptr;
    DIconButton      *m_nextButton = nullptr;
    CTodyButton       *m_today = nullptr;
    QDate              m_currentdate;
    QLabel            *m_YearLabel;
    QLabel            *m_YearLunarLabel;
    QLabel            *m_weekLabel;
    CWeekView         *m_weekview = nullptr;
    CustomFrame            *m_todayframe;
    CScheduleView     *m_scheduleView;
    QSpacerItem       *m_spaceitem;
    QVBoxLayout       *m_mainhLayout;
    // CSchceduleSearchView *m_schceduleSearchView;
    QString           m_searchText;
    QHBoxLayout *m_tmainLayout;
    bool m_searchfalg = false;
};

#endif // YEARWINDOW_H
