// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_draginfographicsview.h"
#include "../third-party_stub/stub.h"
#include "../dialog_stub.h"
#include "graphicsItem/scheduleitem.h"
#include "graphicsItem/cmonthscheduleitem.h"
#include "scheduleTask/cscheduleoperation.h"
#include <QDrag>
#include <QPixmap>
#include <QPoint>
#include <QMimeData>
#include <QFocusEvent>

test_draginfographicsview::test_draginfographicsview()
{
    cDragInfoView = new CMonthGraphicsview();
}

test_draginfographicsview::~test_draginfographicsview()
{
    delete cDragInfoView;
}

// Constructor & Destructor
TEST_F(test_draginfographicsview, Constructor)
{
    CMonthGraphicsview *view = new CMonthGraphicsview();
    EXPECT_NE(view, nullptr);
    delete view;
}

TEST_F(test_draginfographicsview, Destructor)
{
    CMonthGraphicsview *view = new CMonthGraphicsview();
    delete view;
}

// int getDragStatus() const
TEST_F(test_draginfographicsview, getDragStatus)
{
    cDragInfoView->m_DragStatus = DragInfoGraphicsView::NONE;
    EXPECT_EQ(cDragInfoView->getDragStatus(), DragInfoGraphicsView::NONE);
    cDragInfoView->m_DragStatus = DragInfoGraphicsView::IsCreate;
    EXPECT_EQ(cDragInfoView->getDragStatus(), DragInfoGraphicsView::IsCreate);
}

// void setShowRadius(bool leftShow = false, bool rightShow = false)
TEST_F(test_draginfographicsview, setShowRadius)
{
    cDragInfoView->setShowRadius(true, true);
    cDragInfoView->setShowRadius(false, false);
}

// int getSlidePos() const
TEST_F(test_draginfographicsview, getSlidePos)
{
    cDragInfoView->setSlidePos(5);
    EXPECT_EQ(cDragInfoView->getSlidePos(), 5);
}

// void setSlidePos(int pos)
TEST_F(test_draginfographicsview, setSlidePos)
{
    cDragInfoView->setSlidePos(10);
    EXPECT_EQ(cDragInfoView->getSlidePos(), 10);
}

// void stopTouchAnimation()
TEST_F(test_draginfographicsview, stopTouchAnimation)
{
    cDragInfoView->stopTouchAnimation();
}

// void setPressSelectInfo(const DSchedule::Ptr &info)
TEST_F(test_draginfographicsview, setPressSelectInfo)
{
    DSchedule::Ptr info(new DSchedule());
    cDragInfoView->setPressSelectInfo(info);
    cDragInfoView->setPressSelectInfo(DSchedule::Ptr());
}

// void setSelectSearchSchedule(const DSchedule::Ptr &scheduleInfo)
TEST_F(test_draginfographicsview, setSelectSearchSchedule)
{
    DSchedule::Ptr info(new DSchedule());
    cDragInfoView->setSelectSearchSchedule(info);
}

// void pressScheduleInit()
TEST_F(test_draginfographicsview, pressScheduleInit)
{
    cDragInfoView->pressScheduleInit();
}

// void resetPressState()
TEST_F(test_draginfographicsview, resetPressState)
{
    cDragInfoView->resetPressState();
}

// QDate getCurrentDate() const
TEST_F(test_draginfographicsview, getCurrentDate)
{
    QDate date(2020, 10, 1);
    cDragInfoView->setCurrentDate(date);
    EXPECT_EQ(cDragInfoView->getCurrentDate(), date);
}

// void setCurrentDate(const QDate &currentDate)
TEST_F(test_draginfographicsview, setCurrentDate)
{
    QDate date(2021, 5, 15);
    cDragInfoView->setCurrentDate(date);
    EXPECT_EQ(cDragInfoView->getCurrentDate(), date);
}

// DSchedule::Ptr getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate)
TEST_F(test_draginfographicsview, getScheduleInfo_01)
{
    QDateTime begin(QDate(2020, 10, 1), QTime(8, 0, 0));
    QDateTime end(QDate(2020, 10, 5), QTime(10, 0, 0));
    DSchedule::Ptr info = cDragInfoView->getScheduleInfo(begin, end);
    EXPECT_NE(info, nullptr);
    EXPECT_TRUE(info->allDay());
}

TEST_F(test_draginfographicsview, getScheduleInfo_02)
{
    QDateTime begin(QDate(2020, 10, 5), QTime(8, 0, 0));
    QDateTime end(QDate(2020, 10, 1), QTime(10, 0, 0));
    DSchedule::Ptr info = cDragInfoView->getScheduleInfo(begin, end);
    EXPECT_NE(info, nullptr);
    EXPECT_TRUE(info->allDay());
}

// void ShowSchedule(DragInfoItem *infoitem)
TEST_F(test_draginfographicsview, ShowSchedule)
{
    cDragInfoView->ShowSchedule(nullptr);
}

// void setTheMe(int type = 0)
TEST_F(test_draginfographicsview, setTheMe)
{
    cDragInfoView->setTheMe(0);
    cDragInfoView->setTheMe(1);
    cDragInfoView->setTheMe(2);
}

// bool isCanDragge(const DSchedule::Ptr &info)
TEST_F(test_draginfographicsview, isCanDragge_01)
{
    EXPECT_FALSE(cDragInfoView->isCanDragge(DSchedule::Ptr()));
}

TEST_F(test_draginfographicsview, isCanDragge_02)
{
    DSchedule::Ptr info(new DSchedule());
    EXPECT_FALSE(cDragInfoView->isCanDragge(info));
}

// bool DeleteItem(const DSchedule::Ptr &info)
TEST_F(test_draginfographicsview, DeleteItem)
{
    EXPECT_FALSE(cDragInfoView->DeleteItem(DSchedule::Ptr()));
}

// stub for CScheduleOperation::changeSchedule (avoid null account item crash in test env)
bool changeSchedule_stub(CScheduleOperation *obj, const DSchedule::Ptr &newInfo, const DSchedule::Ptr &oldInfo)
{
    Q_UNUSED(obj)
    Q_UNUSED(newInfo)
    Q_UNUSED(oldInfo)
    return true;
}

// void updateScheduleInfo(const DSchedule::Ptr &info)
TEST_F(test_draginfographicsview, updateScheduleInfo)
{
    Stub stub;
    stub.set(ADDR(CScheduleOperation, changeSchedule), changeSchedule_stub);
    DSchedule::Ptr info(new DSchedule());
    cDragInfoView->updateScheduleInfo(info);
}

// void updateInfo()
TEST_F(test_draginfographicsview, updateInfo)
{
    cDragInfoView->updateInfo();
}

// void slideEvent(QPointF &startPoint, QPointF &stopPort)
TEST_F(test_draginfographicsview, slideEvent_01)
{
    QPointF start(0, 0);
    QPointF stop(0, 100);
    cDragInfoView->slideEvent(start, stop);
}

TEST_F(test_draginfographicsview, slideEvent_02)
{
    QPointF start(0, 100);
    QPointF stop(0, 0);
    cDragInfoView->slideEvent(start, stop);
}

TEST_F(test_draginfographicsview, slideEvent_03)
{
    QPointF start(0, 0);
    QPointF stop(100, 0);
    cDragInfoView->slideEvent(start, stop);
}

TEST_F(test_draginfographicsview, slideEvent_04)
{
    QPointF start(100, 0);
    QPointF stop(0, 0);
    cDragInfoView->slideEvent(start, stop);
}

// void slotCreate(const QDateTime &date)
TEST_F(test_draginfographicsview, slotCreate)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    QDateTime date(QDate(2020, 10, 1), QTime(8, 0, 0));
    cDragInfoView->slotCreate(date);
}

// void slotDeleteItem()
TEST_F(test_draginfographicsview, slotDeleteItem)
{
    cDragInfoView->slotDeleteItem();
}

// void slotContextMenu(CFocusItem *item)
TEST_F(test_draginfographicsview, slotContextMenu)
{
    cDragInfoView->slotContextMenu(nullptr);
}

// void slotSwitchPrePage(const QDate &focusDate, bool isSwitchView = false)
TEST_F(test_draginfographicsview, slotSwitchPrePage)
{
    QDate date(2020, 10, 1);
    cDragInfoView->slotSwitchPrePage(date, false);
    cDragInfoView->slotSwitchPrePage(date, true);
}

// void slotSwitchNextPage(const QDate &focusDate, bool isSwitchView = false)
TEST_F(test_draginfographicsview, slotSwitchNextPage)
{
    QDate date(2020, 10, 1);
    cDragInfoView->slotSwitchNextPage(date, false);
    cDragInfoView->slotSwitchNextPage(date, true);
}

// void slotsetNextFocus()
TEST_F(test_draginfographicsview, slotsetNextFocus)
{
    cDragInfoView->slotsetNextFocus();
}

// void setSceneCurrentItemFocus(const QDate &focusDate)
TEST_F(test_draginfographicsview, setSceneCurrentItemFocus)
{
    QDate date(2020, 10, 1);
    cDragInfoView->setSceneCurrentItemFocus(date);
}

// void setDragPixmap(QDrag *drag, DragInfoItem *item)
// stubs for QDrag methods that crash under offscreen QPA platform
void QDrag_setPixmap_stub(QDrag *obj, const QPixmap &pixmap) { Q_UNUSED(obj) Q_UNUSED(pixmap) }
void QDrag_setHotSpot_stub(QDrag *obj, const QPoint &hotspot) { Q_UNUSED(obj) Q_UNUSED(hotspot) }

TEST_F(test_draginfographicsview, setDragPixmap)
{
    Stub stub;
    stub.set(ADDR(QDrag, setPixmap), QDrag_setPixmap_stub);
    stub.set(ADDR(QDrag, setHotSpot), QDrag_setHotSpot_stub);
    QDrag drag(cDragInfoView);
    // setDragPixmap ignores the item param (Q_UNUSED), pass nullptr to avoid
    // CMonthScheduleItem construction crash in headless test env
    cDragInfoView->DragInfoGraphicsView::setDragPixmap(&drag, nullptr);
}

// void mousePressEvent(QMouseEvent *event)
TEST_F(test_draginfographicsview, mousePressEvent_01)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cDragInfoView->mousePressEvent(&event);
}

TEST_F(test_draginfographicsview, mousePressEvent_02)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    cDragInfoView->mousePressEvent(&event);
}

// void mouseReleaseEvent(QMouseEvent *event)
TEST_F(test_draginfographicsview, mouseReleaseEvent_01)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cDragInfoView->mouseReleaseEvent(&event);
}

TEST_F(test_draginfographicsview, mouseReleaseEvent_02)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    cDragInfoView->mouseReleaseEvent(&event);
}

// void mouseMoveEvent(QMouseEvent *event)
TEST_F(test_draginfographicsview, mouseMoveEvent_01)
{
    QMouseEvent event(QEvent::MouseMove, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cDragInfoView->mouseMoveEvent(&event);
}

TEST_F(test_draginfographicsview, mouseMoveEvent_02)
{
    cDragInfoView->m_touchState = DragInfoGraphicsView::TS_NONE;
    QMouseEvent event(QEvent::MouseMove, QPointF(1, 1), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    cDragInfoView->mouseMoveEvent(&event);
}

// void wheelEvent(QWheelEvent *event)
TEST_F(test_draginfographicsview, wheelEvent)
{
    QWheelEvent event(QPointF(1, 1), QPointF(1, 1), QPoint(0, 120), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    cDragInfoView->wheelEvent(&event);
}

// void contextMenuEvent(QContextMenuEvent *event)
TEST_F(test_draginfographicsview, contextMenuEvent)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    calendarQMenuExecStub(stub);
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(1, 1));
    cDragInfoView->contextMenuEvent(&event);
}

// void keyPressEvent(QKeyEvent *event)
TEST_F(test_draginfographicsview, keyPressEvent)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    cDragInfoView->keyPressEvent(&event);
}

// void paintEvent(QPaintEvent *event)
TEST_F(test_draginfographicsview, paintEvent)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    cDragInfoView->paintEvent(&event);
}

// void showEvent(QShowEvent *event)
TEST_F(test_draginfographicsview, showEvent)
{
    QShowEvent event;
    cDragInfoView->showEvent(&event);
}

// void dragEnterEvent(QDragEnterEvent *event)
TEST_F(test_draginfographicsview, dragEnterEvent)
{
    QMimeData mimeData;
    QDragEnterEvent event(QPoint(1, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    cDragInfoView->dragEnterEvent(&event);
}

// void dragLeaveEvent(QDragLeaveEvent *event)
TEST_F(test_draginfographicsview, dragLeaveEvent)
{
    QDragLeaveEvent event;
    cDragInfoView->dragLeaveEvent(&event);
}

// void dragMoveEvent(QDragMoveEvent *event)
TEST_F(test_draginfographicsview, dragMoveEvent)
{
    QMimeData mimeData;
    QDragMoveEvent event(QPoint(1, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    cDragInfoView->dragMoveEvent(&event);
}

// void dropEvent(QDropEvent *event)
TEST_F(test_draginfographicsview, dropEvent)
{
    QMimeData mimeData;
    QDropEvent event(QPoint(1, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    cDragInfoView->dropEvent(&event);
}

// bool event(QEvent *e)
TEST_F(test_draginfographicsview, event_01)
{
    QEvent event(QEvent::Leave);
    cDragInfoView->event(&event);
}

TEST_F(test_draginfographicsview, event_02)
{
    QFocusEvent event(QEvent::FocusIn);
    cDragInfoView->event(&event);
}

// void mousePress(const QPoint &point)
TEST_F(test_draginfographicsview, mousePress)
{
    QPoint point(1, 1);
    cDragInfoView->mousePress(point);
}

// void DragPressEvent(const QPoint &pos, DragInfoItem *item)
TEST_F(test_draginfographicsview, DragPressEvent)
{
    QPoint pos(1, 1);
    cDragInfoView->DragPressEvent(pos, nullptr);
}

// void mouseReleaseScheduleUpdate()
TEST_F(test_draginfographicsview, mouseReleaseScheduleUpdate_01)
{
    cDragInfoView->m_DragStatus = DragInfoGraphicsView::NONE;
    cDragInfoView->mouseReleaseScheduleUpdate();
}

TEST_F(test_draginfographicsview, mouseReleaseScheduleUpdate_02)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    cDragInfoView->m_DragStatus = DragInfoGraphicsView::IsCreate;
    cDragInfoView->m_DragScheduleInfo = DSchedule::Ptr(new DSchedule());
    cDragInfoView->m_PressScheduleInfo = DSchedule::Ptr(new DSchedule());
    cDragInfoView->mouseReleaseScheduleUpdate();
}

TEST_F(test_draginfographicsview, mouseReleaseScheduleUpdate_03)
{
    cDragInfoView->m_DragStatus = DragInfoGraphicsView::ChangeBegin;
    cDragInfoView->m_DragScheduleInfo = DSchedule::Ptr(new DSchedule());
    cDragInfoView->m_PressScheduleInfo = DSchedule::Ptr(new DSchedule());
    cDragInfoView->m_InfoBeginTime = cDragInfoView->m_MoveDate;
    cDragInfoView->mouseReleaseScheduleUpdate();
}

TEST_F(test_draginfographicsview, mouseReleaseScheduleUpdate_04)
{
    cDragInfoView->m_DragStatus = DragInfoGraphicsView::ChangeEnd;
    cDragInfoView->m_DragScheduleInfo = DSchedule::Ptr(new DSchedule());
    cDragInfoView->m_PressScheduleInfo = DSchedule::Ptr(new DSchedule());
    cDragInfoView->m_InfoEndTime = cDragInfoView->m_MoveDate;
    cDragInfoView->mouseReleaseScheduleUpdate();
}
