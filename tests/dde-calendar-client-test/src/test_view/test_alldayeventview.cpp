// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QEvent>
#include <QFocusEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QMenu>

#include "alldayeventview.h"
#include "schedulecoormanage.h"
#include "dschedule.h"
#include "../third-party_stub/stub.h"
#include "../dialog_stub.h"

static void allday_stub_void()
{
}

static QAction *allday_QMenu_exec_stub(void *obj, const QPoint &, QAction *)
{
    Q_UNUSED(obj)
    return nullptr;
}

class test_alldayeventview : public ::testing::Test
{
protected:
    CAllDayEventWeekView *view = nullptr;
    void SetUp() override
    {
        view = new CAllDayEventWeekView();
        view->setFixedSize(QSize(800, 300));
        view->show();
        QApplication::processEvents();
    }
    void TearDown() override
    {
        delete view;
        view = nullptr;
    }
};

TEST_F(test_alldayeventview, Constructor)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(test_alldayeventview, setTheMe)
{
    view->setTheMe(0);
    view->setTheMe(1);
    view->setTheMe(2);
}

TEST_F(test_alldayeventview, MeetCreationConditions)
{
    QDateTime date = QDateTime::currentDateTime();
    EXPECT_TRUE(view->MeetCreationConditions(date));
}

TEST_F(test_alldayeventview, IsEqualtime)
{
    QDateTime t1 = QDateTime::currentDateTime();
    QDateTime t2 = QDateTime::currentDateTime();
    EXPECT_TRUE(view->IsEqualtime(t1, t2));

    QDateTime t3 = t1.addDays(1);
    EXPECT_FALSE(view->IsEqualtime(t1, t3));
}

TEST_F(test_alldayeventview, JudgeIsCreate)
{
    QPointF pos(100, 100);
    view->JudgeIsCreate(pos);
}

TEST_F(test_alldayeventview, setDayData)
{
    QVector<DSchedule::List> vlistData;
    DSchedule::List list1;
    DSchedule::Ptr sched1(new DSchedule());
    sched1->setDtStart(QDateTime(QDate(2026, 10, 1), QTime(10, 0, 0)));
    sched1->setDtEnd(QDateTime(QDate(2026, 10, 1), QTime(11, 0, 0)));
    sched1->setAllDay(true);
    list1.append(sched1);
    vlistData.append(list1);

    view->setDayData(vlistData);
    EXPECT_EQ(view->getListData().size(), 1);
}

TEST_F(test_alldayeventview, setInfo)
{
    DSchedule::List info;
    DSchedule::Ptr sched(new DSchedule());
    sched->setDtStart(QDateTime::currentDateTime());
    sched->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    info.append(sched);
    view->setInfo(info);
}

TEST_F(test_alldayeventview, getListData)
{
    auto &data = view->getListData();
    EXPECT_EQ(data.size(), 0);
}

TEST_F(test_alldayeventview, updateHeight)
{
    view->updateHeight();
}

TEST_F(test_alldayeventview, setSelectSearchSchedule)
{
    DSchedule::Ptr info(new DSchedule());
    info->setDtStart(QDateTime::currentDateTime());
    info->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    view->setSelectSearchSchedule(info);
}

TEST_F(test_alldayeventview, setMargins)
{
    view->setMargins(10, 20, 30, 40);
}

TEST_F(test_alldayeventview, updateInfo)
{
    view->updateInfo();
}

TEST_F(test_alldayeventview, changeEvent_FontChange)
{
    QEvent event(QEvent::FontChange);
    QApplication::sendEvent(view, &event);
}

TEST_F(test_alldayeventview, paintEvent)
{
    view->repaint();
    QApplication::processEvents();
}

TEST_F(test_alldayeventview, slotCreate)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    view->slotCreate(QDateTime::currentDateTime());
}

TEST_F(test_alldayeventview, RightClickToCreate)
{
    Stub stub;
    stub.set((QAction *(QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), allday_QMenu_exec_stub);
    view->RightClickToCreate(nullptr, QPoint(30, 50));
}

TEST_F(test_alldayeventview, MoveInfoProcess_AllDay)
{
    DSchedule::Ptr info(new DSchedule());
    QDateTime currentTime = QDateTime::currentDateTime();
    info->setDtStart(currentTime);
    info->setDtEnd(currentTime.addDays(1));
    info->setAllDay(true);
    view->m_PressDate = currentTime;
    view->m_MoveDate = currentTime;
    view->MoveInfoProcess(info, QPointF(0, 0));
}

TEST_F(test_alldayeventview, MoveInfoProcess_NonAllDay)
{
    DSchedule::Ptr info(new DSchedule());
    QDateTime currentTime = QDateTime::currentDateTime();
    info->setDtStart(currentTime);
    info->setDtEnd(currentTime.addDays(1));
    info->setAllDay(false);
    view->m_PressDate = currentTime;
    view->m_MoveDate = currentTime;
    DSchedule::Ptr dragInfo(new DSchedule());
    dragInfo->setDtStart(currentTime);
    dragInfo->setDtEnd(currentTime.addDays(1));
    view->m_DragScheduleInfo = dragInfo;
    view->MoveInfoProcess(info, QPointF(0, 0));
}

TEST_F(test_alldayeventview, getDragScheduleInfoBeginTime)
{
    QDateTime time = QDateTime::currentDateTime();
    view->m_InfoEndTime = time;
    view->getDragScheduleInfoBeginTime(time);
}

TEST_F(test_alldayeventview, getDragScheduleInfoEndTime)
{
    QDateTime time = QDateTime::currentDateTime();
    view->m_InfoBeginTime = time;
    view->m_InfoEndTime = time.addDays(1);
    view->getDragScheduleInfoEndTime(time);
}

TEST_F(test_alldayeventview, getPosInItem)
{
    QPoint p(50, 10);
    QRectF itemRect(0, 0, 100, 20);
    view->getPosInItem(p, itemRect);
}

TEST_F(test_alldayeventview, getPosDate)
{
    QPoint p(100, 100);
    view->getPosDate(p);
}

TEST_F(test_alldayeventview, upDateInfoShow_NONE)
{
    DSchedule::Ptr info(new DSchedule());
    view->upDateInfoShow(DragInfoGraphicsView::NONE, info);
}

TEST_F(test_alldayeventview, upDateInfoShow_ChangeWhole)
{
    DSchedule::Ptr info(new DSchedule());
    view->upDateInfoShow(DragInfoGraphicsView::ChangeWhole, info);
}

TEST_F(test_alldayeventview, upDateInfoShow_IsCreate)
{
    DSchedule::Ptr info(new DSchedule());
    view->upDateInfoShow(DragInfoGraphicsView::IsCreate, info);
}

TEST_F(test_alldayeventview, slotUpdateScene)
{
    view->slotUpdateScene();
}

TEST_F(test_alldayeventview, slotStateChange)
{
    view->slotStateChange(true);
    view->slotStateChange(false);
}

TEST_F(test_alldayeventview, slotDoubleEvent)
{
    view->slotDoubleEvent();
}

TEST_F(test_alldayeventview, mouseDoubleClickEvent)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(view->viewport(), &event);
}

TEST_F(test_alldayeventview, wheelEvent)
{
    QWheelEvent event(QPointF(100, 100), QPointF(100, 100), QPoint(0, 0), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(view->viewport(), &event);
}

TEST_F(test_alldayeventview, wheelEvent_Horizontal)
{
    QWheelEvent event(QPointF(100, 100), QPointF(100, 100), QPoint(120, 0), QPoint(0, 0), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(view->viewport(), &event);
}

TEST_F(test_alldayeventview, updateDateShow)
{
    view->updateDateShow();
}

TEST_F(test_alldayeventview, createItemWidget)
{
    QVector<DSchedule::List> vlistData;
    DSchedule::List list1;
    DSchedule::Ptr sched1(new DSchedule());
    sched1->setDtStart(QDateTime(QDate(2026, 10, 1), QTime(10, 0, 0)));
    sched1->setDtEnd(QDateTime(QDate(2026, 10, 1), QTime(11, 0, 0)));
    sched1->setAllDay(true);
    list1.append(sched1);
    vlistData.append(list1);
    view->setDayData(vlistData);
    view->createItemWidget(0, false);
}

TEST_F(test_alldayeventview, updateItemHeightByFontSize)
{
    view->updateItemHeightByFontSize();
}
