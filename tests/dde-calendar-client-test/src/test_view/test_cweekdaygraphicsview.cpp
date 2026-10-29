// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QDate>
#include <QWheelEvent>

#include "cweekdaygraphicsview.h"
#include "schedulecoormanage.h"
#include "draginfographicsview.h"
#include "../third-party_stub/stub.h"
#include "../dialog_stub.h"

// Concrete subclass to instantiate the abstract CWeekDayGraphicsview
class TestableWeekDayGraphicsview : public CWeekDayGraphicsview
{
public:
    using CWeekDayGraphicsview::CWeekDayGraphicsview;
    bool MeetCreationConditions(const QDateTime &) override { return false; }
    void upDateInfoShow(const DragInfoGraphicsView::DragStatus &, const DSchedule::Ptr &) override {}
    QDateTime getPosDate(const QPoint &) override { return QDateTime::currentDateTime(); }
    void MoveInfoProcess(DSchedule::Ptr &, const QPointF &) override {}
    DragInfoGraphicsView::PosInItem getPosInItem(const QPoint &, const QRectF &) override { return DragInfoGraphicsView::MIDDLE; }
    bool IsEqualtime(const QDateTime &, const QDateTime &) override { return false; }
    bool JudgeIsCreate(const QPointF &) override { return false; }
    void RightClickToCreate(QGraphicsItem *, const QPoint &) override {}
    QDateTime getDragScheduleInfoBeginTime(const QDateTime &) override { return QDateTime::currentDateTime(); }
    QDateTime getDragScheduleInfoEndTime(const QDateTime &) override { return QDateTime::currentDateTime(); }
};

class test_cweekdaygraphicsview : public ::testing::Test
{
protected:
    TestableWeekDayGraphicsview *view = nullptr;
    void SetUp() override
    {
        view = new TestableWeekDayGraphicsview();
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

TEST_F(test_cweekdaygraphicsview, Constructor)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(test_cweekdaygraphicsview, Constructor_DayPos)
{
    TestableWeekDayGraphicsview *dayView = new TestableWeekDayGraphicsview(nullptr, CWeekDayGraphicsview::DayPos);
    EXPECT_NE(dayView, nullptr);
    delete dayView;
}

TEST_F(test_cweekdaygraphicsview, Constructor_PartTimeView)
{
    TestableWeekDayGraphicsview *ptView = new TestableWeekDayGraphicsview(nullptr, CWeekDayGraphicsview::WeekPos, CWeekDayGraphicsview::PartTimeView);
    EXPECT_NE(ptView, nullptr);
    delete ptView;
}

TEST_F(test_cweekdaygraphicsview, setRange_wh)
{
    QDate begin(2026, 10, 1);
    QDate end(2026, 10, 7);
    view->setRange(800, 300, begin, end, 0);
}

TEST_F(test_cweekdaygraphicsview, setRange_dates)
{
    QDate begin(2026, 10, 1);
    QDate end(2026, 10, 7);
    view->setRange(begin, end);
}

TEST_F(test_cweekdaygraphicsview, setTheMe)
{
    view->setTheMe(0);
    view->setTheMe(1);
    view->setTheMe(2);
}

TEST_F(test_cweekdaygraphicsview, getCoorManage)
{
    CScheduleCoorManage *coor = view->getCoorManage();
    EXPECT_NE(coor, nullptr);
}

TEST_F(test_cweekdaygraphicsview, setCurrentFocusItem)
{
    QDate date(2026, 10, 1);
    view->setRange(800, 300, date, date.addDays(6), 0);
    view->setCurrentFocusItem(date, false);
    view->setCurrentFocusItem(date, true);
}

TEST_F(test_cweekdaygraphicsview, setSceneRect)
{
    view->setSceneRect(0, 0, 800, 300);
}

TEST_F(test_cweekdaygraphicsview, createBackgroundItem)
{
    view->createBackgroundItem();
}

TEST_F(test_cweekdaygraphicsview, setSceneCurrentItemFocus)
{
    QDate date(2026, 10, 1);
    view->setRange(800, 300, date, date.addDays(6), 0);
    view->setSceneCurrentItemFocus(date);
}

TEST_F(test_cweekdaygraphicsview, updateBackgroundShowItem)
{
    view->updateBackgroundShowItem();
}

TEST_F(test_cweekdaygraphicsview, setBackgroundDate)
{
    QDate begin(2026, 10, 1);
    QDate end(2026, 10, 7);
    view->setRange(800, 300, begin, end, 0);
    view->setBackgroundDate();
}

TEST_F(test_cweekdaygraphicsview, slotSwitchView)
{
    QDate date(2026, 10, 1);
    view->setRange(800, 300, date, date.addDays(6), 0);
    view->slotSwitchView(date, false);
    view->slotSwitchView(date, true);
}

TEST_F(test_cweekdaygraphicsview, slotViewInit)
{
    view->slotViewInit();
}

TEST_F(test_cweekdaygraphicsview, slotPosOnView)
{
    view->slotPosOnView(50.0);
    view->slotPosOnView(0.0);
}
