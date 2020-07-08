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

#ifndef CALENDARMAINWINDOW_H
#define CALENDARMAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <DMainWindow>
#include <dstackwidget.h>
#include <DPushButton>
#include <dsearchedit.h>
#include <QButtonGroup>
#include <DTitlebar>
#include <DLabel>
#include <QStackedWidget>
#include <DButtonBox>
#include <QStackedLayout>
#include <QPropertyAnimation>
#include <DFrame>


#include "scheduledatamanage.h"
#include "animationstackedwidget.h"
#include "dbusdatagetthread.h"

DWIDGET_USE_NAMESPACE
class CYearWindow;
class CMonthWindow;
class CWeekWindow;
class CDayWindow;
class __Scheduler;
class CSchceduleSearchView;
class Calendarmainwindow : public DMainWindow
{
    Q_OBJECT
//    Q_PROPERTY(int schedulesearchWidth WRITE setSearchWidth)
public:
    Calendarmainwindow(QWidget *w = nullptr);
    ~Calendarmainwindow() override;
    //void Invoke(const QString &mothodName, const QString &content);
    bool analysisCreate(const QString &content, ScheduleDtailInfo &info);
    void viewWindow(int type, QDateTime datetime);
    void UpdateJob();
    void updateHigh();
    void setSearchWidth(int w);
public slots:
    void slotTheme(int type);
    void OpenSchedule(QString job);
    void ActiveWindow();
    void RaiseWindow();
    void onViewShortcut();
    void slotmaxminViewShortcut();
    void slotGetScheduleInfoSuccess();
    void slotDynamicIconUpdate();
private:
    void initUI();
    void initConnection();
    void initLunar();
    void createview();
    DPushButton *createButon(QString name);

    void getScheduleInfo();
protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private slots:
    void setScheduleHide();
    void slotstackWClicked(QAbstractButton *bt);
    void slotWUpdateShcedule(QMainWindow *w, int id = 0);
    void slotReturnTodyUpdate(QMainWindow *w);
    void slotSreturnPressed();
    void slotStextChanged();
    void slotStextfocusChanged(bool onFocus);
    void slotJobsUpdated(const QList<qlonglong> &Ids);
    void slotSearchEdit();
    void slotTransitSearchSchedule(int id = 0);
    void slotsearchDateSelect(QDate date);
    void slotSearchSelectSchedule(const ScheduleDtailInfo &scheduleInfo);

    void slotdoubleclickDate(QDate date);
    void slotselectMonth(QDate date);
    void slotselectWeek(QDate date);

    void slotCurrentScheduleDate(QDate date);
    void slotViewSelectDate(QDate date);
    void slotViewtransparentFrame(int type);
    void slotCurrentDate(QDate date);
    void slotSetButtonBox();
private:
    DLabel                    *m_icon;
    AnimationStackedWidget              *m_stackWidget;
    //DSegmentedControl         *m_segmentedControl;
    DButtonBox                *m_buttonBox;
    DButtonBoxButton          *m_yearButton;
    DButtonBoxButton          *m_monthButton;
    DButtonBoxButton          *m_weekButton;
    DButtonBoxButton          *m_dayButton;
    DSearchEdit               *m_searchEdit;
    CYearWindow               *m_yearwindow;
    CMonthWindow              *m_monthWindow;
    CWeekWindow               *m_weekWindow;
    CDayWindow                *m_DayWindow;
    __Scheduler               *m_dbus;
    bool                      m_searchflag = false;
    CSchceduleSearchView *m_schceduleSearchView;
    DFrame *m_contentBackground = nullptr;
    int                       m_priindex = 3; //默认打开日视图双击
    DFrame                    *m_transparentFrame;
    bool                      m_opensearchflag = false;
    QDate                     m_currentdate;
    int                         m_scheduleSearchViewMaxWidth;
    QPropertyAnimation          *m_animation;

    DbusDataGetThread          *m_DataGetThread;

    QTimer                      *m_DynamicIconUpdateTimer;

};

#endif // CALENDARMAINWINDOW_H
