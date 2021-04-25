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

#include "dbusdatagetthread.h"

#include <DPushButton>
#include <DTitlebar>
#include <DLabel>
#include <DButtonBox>
#include <DFrame>

#include <QStackedLayout>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QMainWindow>
#include <QObject>
#include <DMainWindow>

#include <dstackwidget.h>
#include <dsearchedit.h>

DWIDGET_USE_NAMESPACE
class CYearWindow;
class CMonthWindow;
class CWeekWindow;
class CDayWindow;
class __Scheduler;
class CSchceduleSearchView;
class AnimationStackedWidget;
class CScheduleDataManage;
class Calendarmainwindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit Calendarmainwindow(QWidget *w = nullptr);
    ~Calendarmainwindow() override;
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
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
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
    DLabel *m_icon = nullptr;
    AnimationStackedWidget *m_stackWidget = nullptr;
    DButtonBox *m_buttonBox = nullptr;
    DButtonBoxButton *m_yearButton = nullptr;
    DButtonBoxButton *m_monthButton = nullptr;
    DButtonBoxButton *m_weekButton = nullptr;
    DButtonBoxButton *m_dayButton = nullptr;
    DSearchEdit *m_searchEdit = nullptr;
    CYearWindow *m_yearwindow = nullptr;
    CMonthWindow *m_monthWindow = nullptr;
    CWeekWindow *m_weekWindow = nullptr;
    CDayWindow *m_DayWindow = nullptr;
    __Scheduler *m_dbus = nullptr;
    bool m_searchflag = false;
    CSchceduleSearchView *m_schceduleSearchView = nullptr;
    DFrame *m_contentBackground = nullptr;
    int m_priindex = 3; //默认打开日视图双击
    DFrame *m_transparentFrame = nullptr;
    bool m_opensearchflag = false;
    QDate m_currentdate;
    int m_scheduleSearchViewMaxWidth;
    QPropertyAnimation *m_animation = nullptr;
    DbusDataGetThread *m_DataGetThread = nullptr;
    QTimer *m_DynamicIconUpdateTimer = nullptr;
};

#endif // CALENDARMAINWINDOW_H
