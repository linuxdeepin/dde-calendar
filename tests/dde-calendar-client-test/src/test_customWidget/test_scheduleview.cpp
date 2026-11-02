// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_scheduleview.h"
#include "../third-party_stub/stub.h"

#include <QResizeEvent>
#include <QWheelEvent>
#include <QTest>
#include <QTimer>
#include <QApplication>
#include "dschedule.h"

static QLocale::Language scheduleview_stub_language(){
    return QLocale::English;
}

static bool scheduleview_stub_event()
{
    return true;
}

static touchGestureOperation::TouchState scheduleview_getTouchState = touchGestureOperation::T_STATE_NONE;

static touchGestureOperation::TouchState scheduleview_stub_getTouchState()
{
    return scheduleview_getTouchState;
}

test_scheduleview::test_scheduleview()
{
    mScheduleView = new CScheduleView();
}

test_scheduleview::~test_scheduleview()
{
    delete mScheduleView;
    mScheduleView = nullptr;
}

DSchedule::List getScheduleData()
{
    DSchedule::Ptr schedule1(new DSchedule());
    DSchedule::Ptr schedule2(new DSchedule());

    QDateTime currentDateTime = QDateTime::currentDateTime();
    schedule1->setUid("1");
    schedule1->setDtStart(currentDateTime);
    schedule1->setDtEnd(currentDateTime.addDays(1));
    schedule1->setSummary("scheduleOne");
    schedule1->setAllDay(true);
    schedule1->setScheduleTypeID("1");

    schedule2->setUid("2");
    schedule2->setDtStart(currentDateTime.addDays(1));
    schedule2->setDtEnd(currentDateTime.addDays(1).addSecs(60 * 60));
    schedule2->setSummary("scheduleTwo");
    schedule2->setAllDay(false);
    schedule2->setScheduleTypeID("2");

    DSchedule::List scheduleList;
    scheduleList.append(schedule1);
    scheduleList.append(schedule2);
    return scheduleList;
}

//void CScheduleView::setviewMargin(int left, int top, int right, int bottom)
TEST_F(test_scheduleview, setviewMargin)
{
    mScheduleView->setviewMargin(8, 8, 4, 4);
}

//void CScheduleView::setRange(int w, int h, QDate begin, QDate end)
TEST_F(test_scheduleview, setRange)
{
    mScheduleView->setRange(4, 4, QDate::currentDate(), QDate::currentDate().addDays(1));
}

//void CScheduleView::setRange(QDate begin, QDate end)
TEST_F(test_scheduleview, setrange)
{
    mScheduleView->setRange(QDate::currentDate(), QDate::currentDate().addDays(1));
}

//void CScheduleView::setTheMe(int type)
TEST_F(test_scheduleview, setTheMe)
{
    mScheduleView->setTheMe(1);
    mScheduleView->setTheMe(2);
}

//void CScheduleView::setLunarVisible(bool state)
TEST_F(test_scheduleview, setLunarVisible)
{
    mScheduleView->setLunarVisible(true);
}

//void CScheduleView::setTime(QTime time)
TEST_F(test_scheduleview, setTime)
{
    mScheduleView->setTime(QTime::currentTime());
}

//void CScheduleView::setSelectSchedule(const DSchedule::Ptr &scheduleInfo)
TEST_F(test_scheduleview, setSelectSchedule)
{
    DSchedule::Ptr scheduleinfo = getScheduleData().first();
    mScheduleView->setSelectSchedule(scheduleinfo);
    scheduleinfo = getScheduleData().at(1);
    mScheduleView->setSelectSchedule(scheduleinfo);
}

//void CScheduleView::updateHeight()
TEST_F(test_scheduleview, updateHeight)
{
    mScheduleView->updateHeight();
}

//bool CScheduleView::IsDragging()
TEST_F(test_scheduleview, IsDragging)
{
    mScheduleView->IsDragging();
}

//void CScheduleView::setCurrentDate(const QDateTime &currentDate)
TEST_F(test_scheduleview, setCurrentDate)
{
    mScheduleView->setCurrentDate(QDateTime::currentDateTime());
}

//void CScheduleView::setShowScheduleInfo(const QMap<QDate, DSchedule::List> &scheduleInfo)
TEST_F(test_scheduleview, setShowScheduleInfo)
{
    QMap<QDate, DSchedule::List> dateInfoMap;
    dateInfoMap.insert(QDate::currentDate(), getScheduleData());
    mScheduleView->setShowScheduleInfo(dateInfoMap);
}

//void CScheduleView::setDate(QDate date)
TEST_F(test_scheduleview, setDate)
{
    mScheduleView->setDate(QDate::currentDate());
}

//void CScheduleView::slotupdateSchedule()
TEST_F(test_scheduleview, slotupdateSchedule)
{
    mScheduleView->slotupdateSchedule();
}

//void CScheduleView::slotPosHours(QVector<int> vPos, QVector<int> vHours, int currentTimeType)
TEST_F(test_scheduleview, slotPosHours)
{
    QVector<int> pos;
    pos.append(1);
    pos.append(2);
    pos.append(3);
    pos.append(4);

    QVector<int> hours;
    hours.append(4);
    hours.append(8);
    hours.append(16);
    hours.append(20);

    mScheduleView->slotPosHours(pos, hours, 1);
}

//void CScheduleView::slotCurrentScheduleDate(QDate date)
TEST_F(test_scheduleview, slotCurrentScheduleDate)
{
    mScheduleView->slotCurrentScheduleDate(QDate::currentDate());
}

//void CScheduleView::slotScheduleShow(const bool isShow, const DSchedule::Ptr &out)
TEST_F(test_scheduleview, slotScheduleShow)
{
    mScheduleView->slotScheduleShow(false, getScheduleData().first());
    mScheduleView->slotScheduleShow(true, getScheduleData().at(1));
}

//void CScheduleView::slotUpdatePaint(const int topM)
TEST_F(test_scheduleview, slotUpdatePaint)
{
    const int top = 4;
    mScheduleView->slotUpdatePaint(top);
}

//void CScheduleView::slotUpdateScene()
TEST_F(test_scheduleview, slotUpdateScene)
{
    mScheduleView->slotUpdateScene();
}

//void CScheduleView::updateSchedule()
TEST_F(test_scheduleview, updateSchedule)
{
    mScheduleView->updateSchedule();
}

//void CScheduleView::updateAllday()
TEST_F(test_scheduleview, updateAllday)
{
    mScheduleView->updateAllday();
}

//int CScheduleView::scheduleViewHeight()
TEST_F(test_scheduleview, scheduleViewHeight)
{
    mScheduleView->scheduleViewHeight();
}

//getPixmap
TEST_F(test_scheduleview, getPixmap)
{
    QVector<int> pos;
    pos.append(1);
    pos.append(2);
    pos.append(3);
    pos.append(4);

    QVector<int> hours;
    hours.append(4);
    hours.append(8);
    hours.append(16);
    hours.append(20);

    mScheduleView->slotPosHours(pos, hours, 1);

    mScheduleView->setFixedSize(800, 500);
    QPixmap pixmap(mScheduleView->size());
    pixmap.fill(Qt::white);
}

TEST_F(test_scheduleview, paintEvent_01)
{
    QPaintEvent e(QRect(0, 0, 100, 100));
    mScheduleView->paintEvent(&e);
}

TEST_F(test_scheduleview, paintEvent_02)
{
    mScheduleView->m_currentTimeType = 1;
    QPaintEvent e(QRect(0, 0, 100, 100));
    mScheduleView->paintEvent(&e);
}

TEST_F(test_scheduleview, paintEvent_03)
{
    Stub stub;
    stub.set(ADDR(QLocale, language), scheduleview_stub_language);

    QPaintEvent e(QRect(0, 0, 100, 100));
    CScheduleView view;
    view.paintEvent(&e);
}

TEST_F(test_scheduleview, paintEvent_04)
{
    Stub stub;
    stub.set(ADDR(QLocale, language), scheduleview_stub_language);

    QPaintEvent e(QRect(0, 0, 100, 100));
    CScheduleView view;
    view.m_currentTimeType = 1;
    view.paintEvent(&e);
}

TEST_F(test_scheduleview, event_01)
{
    Stub stub;
    stub.set(ADDR(touchGestureOperation, event), scheduleview_stub_event);
    stub.set(ADDR(touchGestureOperation, getTouchState), scheduleview_stub_getTouchState);
    QPaintEvent e(QRect(0, 0, 100, 100));
    scheduleview_getTouchState = touchGestureOperation::T_STATE_NONE;
    CScheduleView view;
    view.event(&e);
}

TEST_F(test_scheduleview, event_02)
{
    Stub stub;
    stub.set(ADDR(touchGestureOperation, event), scheduleview_stub_event);
    stub.set(ADDR(touchGestureOperation, getTouchState), scheduleview_stub_getTouchState);
    QPaintEvent e(QRect(0, 0, 100, 100));
    scheduleview_getTouchState = touchGestureOperation::T_SINGLE_CLICK;
    CScheduleView view;
    view.event(&e);
}

TEST_F(test_scheduleview, event_03)
{
    Stub stub;
    stub.set(ADDR(touchGestureOperation, event), scheduleview_stub_event);
    stub.set(ADDR(touchGestureOperation, getTouchState), scheduleview_stub_getTouchState);
    QPaintEvent e(QRect(0, 0, 100, 100));
    scheduleview_getTouchState = touchGestureOperation::T_SLIDE;
    CScheduleView view;
    view.m_touchGesture.setUpdate(true);
    view.m_touchGesture.m_movingDir = touchGestureOperation::T_LEFT;

    view.event(&e);
}

TEST_F(test_scheduleview, event_04)
{
    Stub stub;
    stub.set(ADDR(touchGestureOperation, event), scheduleview_stub_event);
    stub.set(ADDR(touchGestureOperation, getTouchState), scheduleview_stub_getTouchState);
    QPaintEvent e(QRect(0, 0, 100, 100));
    scheduleview_getTouchState = touchGestureOperation::T_SLIDE;
    CScheduleView view;
    view.m_touchGesture.setUpdate(true);
    view.m_touchGesture.m_movingDir = touchGestureOperation::T_RIGHT;

    view.event(&e);
}

//void CScheduleView::setTimeFormat(QString timeformat)
TEST_F(test_scheduleview, setTimeFormat)
{
    mScheduleView->setTimeFormat("hh:mm");
    mScheduleView->setTimeFormat("AP hh:mm:ss");
}

//void CScheduleView::initConnection()
TEST_F(test_scheduleview, initConnection)
{
    mScheduleView->initConnection();
}

//void CScheduleView::initUI()
TEST_F(test_scheduleview, initUI)
{
    mScheduleView->initUI();
}

//void CScheduleView::resizeEvent(QResizeEvent *event)
TEST_F(test_scheduleview, resizeEvent)
{
    mScheduleView->setFixedSize(800, 600);
    QResizeEvent event(QSize(800, 600), QSize(400, 300));
    mScheduleView->resizeEvent(&event);
}

//void CScheduleView::wheelEvent(QWheelEvent *e)
TEST_F(test_scheduleview, wheelEvent)
{
    QWheelEvent event(QPointF(10, 10), QPointF(10, 10), QPoint(), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    mScheduleView->wheelEvent(&event);
}

//void CScheduleView::slotDeleteitem()
TEST_F(test_scheduleview, slotDeleteitem)
{
    QTimer::singleShot(100, []() {
        QWidgetList widgets = QApplication::topLevelWidgets();
        for (QWidget *w : widgets) {
            if (w->inherits("DDialog") || w->inherits("QDialog")) {
                w->close();
                break;
            }
        }
    });
    mScheduleView->slotDeleteitem();
}

//void CScheduleView::slotSwitchView(const QDate &focusDate, CWeekDayGraphicsview::ViewType viewtype, bool setItemFocus)
TEST_F(test_scheduleview, slotSwitchView_01)
{
    mScheduleView->slotSwitchView(QDate::currentDate(), CWeekDayGraphicsview::ALLDayView, true);
}

TEST_F(test_scheduleview, slotSwitchView_02)
{
    mScheduleView->slotSwitchView(QDate::currentDate(), CWeekDayGraphicsview::PartTimeView, false);
}
