// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_graphicsview.h"
#include "graphicsItem/scheduleitem.h"
#include "../third-party_stub/stub.h"
#include "../dialog_stub.h"

test_graphicsview::test_graphicsview()
{
    cGraphicsView = new CGraphicsView(nullptr, CGraphicsView::WeekPos);
}

test_graphicsview::~test_graphicsview()
{
    delete cGraphicsView;
}

// Constructor & Destructor
TEST_F(test_graphicsview, Constructor)
{
    CGraphicsView *view = new CGraphicsView(nullptr, CGraphicsView::WeekPos);
    EXPECT_NE(view, nullptr);
    delete view;
}

TEST_F(test_graphicsview, Destructor)
{
    CGraphicsView *view = new CGraphicsView(nullptr, CGraphicsView::DayPos);
    delete view;
}

// void setMargins(int left, int top, int right, int bottom)
TEST_F(test_graphicsview, setMargins)
{
    cGraphicsView->setMargins(50, 50, 50, 50);
    EXPECT_EQ(cGraphicsView->getMargins().left(), 50);
}

// void setTheMe(int type = 0)
TEST_F(test_graphicsview, setTheMe)
{
    cGraphicsView->setTheMe(0);
    cGraphicsView->setTheMe(2);
}

// bool MeetCreationConditions(const QDateTime &date)
TEST_F(test_graphicsview, MeetCreationConditions_01)
{
    cGraphicsView->m_PressDate = QDateTime::currentDateTime();
    QDateTime date = QDateTime::currentDateTime();
    EXPECT_TRUE(cGraphicsView->MeetCreationConditions(date));
}

TEST_F(test_graphicsview, MeetCreationConditions_02)
{
    cGraphicsView->m_PressDate = QDateTime(QDate(2020, 1, 1), QTime(0, 0, 0));
    QDateTime date(QDate(2020, 1, 20), QTime(0, 0, 0));
    EXPECT_FALSE(cGraphicsView->MeetCreationConditions(date));
}

// void updateHeight()
TEST_F(test_graphicsview, updateHeight)
{
    cGraphicsView->updateHeight();
}

// void setRange(int w, int h, QDate begindate, QDate enddate, int rightMargin)
TEST_F(test_graphicsview, setRange_01)
{
    cGraphicsView->setRange(20, 20, QDate(2020, 11, 1), QDate(2020, 11, 7), 10);
}

TEST_F(test_graphicsview, setRange_02)
{
    cGraphicsView->setRange(QDate(2020, 11, 1), QDate(2020, 11, 7));
}

// void setCurrentDate(const QDateTime &currentDate)
TEST_F(test_graphicsview, setCurrentDate)
{
    QDateTime date = QDateTime::currentDateTime();
    cGraphicsView->setCurrentDate(date);
}

// void setInfo(const DSchedule::List &info)
TEST_F(test_graphicsview, setInfo)
{
    DSchedule::List infos;
    cGraphicsView->setInfo(infos);
}

// void upDateInfoShow(const DragStatus &status, const DSchedule::Ptr &info)
TEST_F(test_graphicsview, upDateInfoShow_01)
{
    DSchedule::Ptr info(new DSchedule());
    cGraphicsView->upDateInfoShow(CGraphicsView::IsCreate, info);
}

TEST_F(test_graphicsview, upDateInfoShow_02)
{
    DSchedule::Ptr info(new DSchedule());
    cGraphicsView->upDateInfoShow(CGraphicsView::ChangeEnd, info);
}

TEST_F(test_graphicsview, upDateInfoShow_03)
{
    DSchedule::Ptr info(new DSchedule());
    cGraphicsView->upDateInfoShow(CGraphicsView::ChangeBegin, info);
}

TEST_F(test_graphicsview, upDateInfoShow_04)
{
    DSchedule::Ptr info(new DSchedule());
    cGraphicsView->upDateInfoShow(CGraphicsView::ChangeWhole, info);
}

TEST_F(test_graphicsview, upDateInfoShow_05)
{
    cGraphicsView->upDateInfoShow(CGraphicsView::NONE, DSchedule::Ptr());
}

// QDateTime getPosDate(const QPoint &p)
TEST_F(test_graphicsview, getPosDate)
{
    cGraphicsView->setRange(20, 20, QDate(2020, 11, 1), QDate(2020, 11, 7), 10);
    QPoint point(50, 40);
    cGraphicsView->getPosDate(point);
}

// void MoveInfoProcess(DSchedule::Ptr &info, const QPointF &pos)
TEST_F(test_graphicsview, MoveInfoProcess_01)
{
    DSchedule::Ptr info(new DSchedule());
    info->setAllDay(true);
    QPointF pos(1, 1);
    cGraphicsView->MoveInfoProcess(info, pos);
    EXPECT_FALSE(info->allDay());
}

TEST_F(test_graphicsview, MoveInfoProcess_02)
{
    DSchedule::Ptr info(new DSchedule());
    info->setAllDay(false);
    QPointF pos(1, 1);
    cGraphicsView->MoveInfoProcess(info, pos);
    EXPECT_FALSE(info->allDay());
}

// void addScheduleItem(const DSchedule::Ptr &info, QDate date, int index, int totalNum, int type, int viewtype, int maxnum)
TEST_F(test_graphicsview, addScheduleItem_01)
{
    cGraphicsView->setRange(20, 20, QDate(2020, 11, 1), QDate(2020, 11, 7), 10);
    DSchedule::Ptr info(new DSchedule());
    cGraphicsView->addScheduleItem(info, QDate(2020, 11, 3), 1, 2, 1, 1, 3);
}

// void clearSchedule()
TEST_F(test_graphicsview, clearSchedule)
{
    cGraphicsView->clearSchedule();
    EXPECT_TRUE(cGraphicsView->m_updateDflag);
}

// void setSelectSearchSchedule(const DSchedule::Ptr &info)
TEST_F(test_graphicsview, setSelectSearchSchedule)
{
    DSchedule::Ptr info(new DSchedule());
    cGraphicsView->setSelectSearchSchedule(info);
}

// void scheduleClassificationType(DSchedule::List &scheduleInfolist, QList<ScheduleclassificationInfo> &info)
TEST_F(test_graphicsview, scheduleClassificationType)
{
    DSchedule::List scheduleInfolist;
    QList<ScheduleclassificationInfo> info;
    cGraphicsView->scheduleClassificationType(scheduleInfolist, info);
}

// void slotDoubleEvent(int type)
TEST_F(test_graphicsview, slotDoubleEvent)
{
    cGraphicsView->slotDoubleEvent(1);
}

// void slotScrollBar()
TEST_F(test_graphicsview, slotScrollBar)
{
    cGraphicsView->slotScrollBar();
}

// void slotUpdateScene()
TEST_F(test_graphicsview, slotUpdateScene)
{
    cGraphicsView->slotUpdateScene();
}

// void slotStateChange(bool bState)
TEST_F(test_graphicsview, slotStateChange)
{
    cGraphicsView->slotStateChange(true);
    cGraphicsView->slotStateChange(false);
}

// void scrollBarValueChangedSlot()
TEST_F(test_graphicsview, scrollBarValueChangedSlot)
{
    cGraphicsView->scrollBarValueChangedSlot();
}

// DSchedule::Ptr getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate)
TEST_F(test_graphicsview, getScheduleInfo_01)
{
    QDateTime begin(QDate(2020, 12, 1), QTime(8, 0, 0));
    QDateTime end(QDate(2020, 12, 1), QTime(10, 0, 0));
    DSchedule::Ptr info = cGraphicsView->getScheduleInfo(begin, end);
    EXPECT_NE(info, nullptr);
    EXPECT_FALSE(info->allDay());
}

TEST_F(test_graphicsview, getScheduleInfo_02)
{
    QDateTime begin(QDate(2020, 12, 1), QTime(10, 0, 0));
    QDateTime end(QDate(2020, 12, 1), QTime(8, 0, 0));
    DSchedule::Ptr info = cGraphicsView->getScheduleInfo(end, begin);
    EXPECT_NE(info, nullptr);
    EXPECT_FALSE(info->allDay());
}

// bool IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
TEST_F(test_graphicsview, IsEqualtime)
{
    QDateTime begin(QDate(2020, 12, 1), QTime(8, 0, 0));
    QDateTime end(QDate(2020, 12, 21), QTime(8, 0, 0));
    cGraphicsView->IsEqualtime(begin, end);
}

// bool JudgeIsCreate(const QPointF &pos)
TEST_F(test_graphicsview, JudgeIsCreate)
{
    QPointF pos(20, 20);
    cGraphicsView->JudgeIsCreate(pos);
}

// void RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos)
TEST_F(test_graphicsview, RightClickToCreate)
{
    QPoint pos(20, 20);
    Stub stub;
    calendarQMenuExecStub(stub);
    cGraphicsView->RightClickToCreate(nullptr, pos);
}

// QDateTime getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
TEST_F(test_graphicsview, getDragScheduleInfoBeginTime)
{
    QDateTime begin(QDate(2020, 12, 1), QTime(8, 0, 0));
    cGraphicsView->getDragScheduleInfoBeginTime(begin);
}

// QDateTime getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
TEST_F(test_graphicsview, getDragScheduleInfoEndTime)
{
    QDateTime end(QDate(2020, 12, 21), QTime(8, 0, 0));
    cGraphicsView->getDragScheduleInfoEndTime(end);
}

// void keepCenterOnScene()
TEST_F(test_graphicsview, keepCenterOnScene)
{
    cGraphicsView->keepCenterOnScene();
}

// void setTime(QTime time)
TEST_F(test_graphicsview, setTime)
{
    QTime time(7, 30, 5, 100);
    cGraphicsView->setTime(time);
}

// void updateInfo()
TEST_F(test_graphicsview, updateInfo)
{
    cGraphicsView->updateInfo();
}

// void ShowSchedule(DragInfoItem *infoitem)
TEST_F(test_graphicsview, ShowSchedule_01)
{
    CScheduleItem *item = new CScheduleItem(QRectF(0, 0, 100, 100), nullptr, 1);
    cGraphicsView->ShowSchedule(item);
    delete item;
}

TEST_F(test_graphicsview, ShowSchedule_02)
{
    CScheduleItem *item = new CScheduleItem(QRectF(0, 0, 100, 100), nullptr, 0);
    cGraphicsView->ShowSchedule(item);
    delete item;
}

// PosInItem getPosInItem(const QPoint &p, const QRectF &itemRect)
TEST_F(test_graphicsview, getPosInItem_01)
{
    QPoint p(10, 10);
    QRectF rect(0, 0, 100, 100);
    cGraphicsView->getPosInItem(p, rect);
}

// void mouseDoubleClickEvent(QMouseEvent *event)
TEST_F(test_graphicsview, mouseDoubleClickEvent_01)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cGraphicsView->mouseDoubleClickEvent(&event);
}

TEST_F(test_graphicsview, mouseDoubleClickEvent_02)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    cGraphicsView->mouseDoubleClickEvent(&event);
}

// void mousePressEvent(QMouseEvent *event)
TEST_F(test_graphicsview, mousePressEvent_01)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cGraphicsView->mousePressEvent(&event);
}

TEST_F(test_graphicsview, mousePressEvent_02)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    cGraphicsView->mousePressEvent(&event);
}

// void mouseMoveEvent(QMouseEvent *event)
TEST_F(test_graphicsview, mouseMoveEvent_01)
{
    QMouseEvent event(QEvent::MouseMove, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cGraphicsView->mouseMoveEvent(&event);
}

// void wheelEvent(QWheelEvent *event)
TEST_F(test_graphicsview, wheelEvent)
{
    QWheelEvent event(QPointF(1, 1), QPointF(1, 1), QPoint(0, 120), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    cGraphicsView->wheelEvent(&event);
}

// void resizeEvent(QResizeEvent *event)
TEST_F(test_graphicsview, resizeEvent)
{
    QResizeEvent event(QSize(50, 50), QSize(100, 100));
    cGraphicsView->resizeEvent(&event);
}

// void paintEvent(QPaintEvent *event)
TEST_F(test_graphicsview, paintEvent)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    cGraphicsView->paintEvent(&event);
}

// void slotCreate(const QDateTime &date)
TEST_F(test_graphicsview, slotCreate)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    QDateTime date(QDate(2020, 10, 1), QTime(8, 0, 0));
    cGraphicsView->slotCreate(date);
}
