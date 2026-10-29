// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthgraphiview.h"
#include "../third-party_stub/stub.h"
#include "../dialog_stub.h"
#include "widget/monthWidget/monthscheduleview.h"
#include <QDrag>
#include <QPixmap>
#include <QPoint>

static void monthgraphiview_stub_void()
{
}

static bool monthgraphiview_stub_addData(const DSchedule::Ptr &)
{
    return true;
}

static void monthgraphiview_stub_changeDate(const DSchedule::Ptr &)
{
}

static QPixmap monthgraphiview_stub_getPixmap(CMonthScheduleItem *)
{
    return QPixmap(1, 1);
}

static void monthScheduleView_updateDate_stub(CMonthScheduleView *, int, const DSchedule::Ptr &)
{
}

static void monthgraphiview_stub_QDrag_setPixmap(QDrag *, const QPixmap &)
{
}

static void monthgraphiview_stub_QDrag_setHotSpot(QDrag *, const QPoint &)
{
}

test_monthgraphiview::test_monthgraphiview()
{
    cMonthGraphiview = new CMonthGraphicsview();
}

test_monthgraphiview::~test_monthgraphiview()
{
    delete cMonthGraphiview;
}

// Constructor & Destructor
TEST_F(test_monthgraphiview, Constructor)
{
    CMonthGraphicsview *view = new CMonthGraphicsview();
    EXPECT_NE(view, nullptr);
    delete view;
}

TEST_F(test_monthgraphiview, Destructor)
{
    CMonthGraphicsview *view = new CMonthGraphicsview();
    delete view;
}

//void CMonthGraphiview::setTheMe(int type)
TEST_F(test_monthgraphiview, setTheMe)
{
    int type = 1;
    cMonthGraphiview->setTheMe();
    cMonthGraphiview->setTheMe(type);
}

//void CMonthGraphiview::setDate(const QVector<QDate> &showDate)
TEST_F(test_monthgraphiview, setDate)
{
    QVector<QDate> showDate;
    QDate date1(2020, 10, 1);
    QDate date2(2020, 12, 1);
    for (int i = 0; i < 42; ++i) {
        if (i % 2 == 0) {
            showDate.append(date1);
        } else {
            showDate.append(date2);
        }
    }
    std::cout << showDate.size() << std::endl;
    cMonthGraphiview->setDate(showDate);
}

//void CMonthGraphiview::setFestival(const QMap<QDate, int> &festivalInfo)
TEST_F(test_monthgraphiview, setFestival)
{
    QMap<QDate, int> festivalInfo;
    festivalInfo.insert(QDate(2020, 10, 1), 1);
    cMonthGraphiview->setFestival(festivalInfo);
}

//void CMonthGraphiview::setLunarInfo(const QMap<QDate, CaHuangLiDayInfo> &lunarCache)
TEST_F(test_monthgraphiview, setLunarInfo)
{
    QMap<QDate, CaHuangLiDayInfo> lunarCache = QMap<QDate, CaHuangLiDayInfo> {};
    cMonthGraphiview->setLunarInfo(lunarCache);
}

//void CMonthGraphiview::setLunarVisible(bool visible)
TEST_F(test_monthgraphiview, setLunarVisible)
{
    bool visible = false;
    cMonthGraphiview->setLunarVisible(visible);
}

//void CMonthGraphiview::setScheduleInfo(const QMap<QDate, DSchedule::List> &info)
TEST_F(test_monthgraphiview, setScheduleInfo)
{
    QMap<QDate, DSchedule::List> info = QMap<QDate, DSchedule::List> {};
    cMonthGraphiview->setScheduleInfo(info);
}

//void CMonthGraphiview::setSelectSearchSchedule(const DSchedule::Ptr &scheduleInfo)
TEST_F(test_monthgraphiview, setSelectSearchSchedule)
{
    DSchedule::Ptr scheduleInfo(new DSchedule());
    cMonthGraphiview->setSelectSearchSchedule(scheduleInfo);
}

//void CMonthGraphiview::setSearchScheduleInfo(const DSchedule::List &searchScheduleInfo)
TEST_F(test_monthgraphiview, setSearchScheduleInfo)
{
    DSchedule::List searchScheduleInfo;
    cMonthGraphiview->setSearchScheduleInfo(searchScheduleInfo);
}

//void CMonthGraphiview::updateSize()
TEST_F(test_monthgraphiview, updateSize)
{
    cMonthGraphiview->updateSize();
}

//void CMonthGraphiview::updateLunar()
TEST_F(test_monthgraphiview, updateLunar)
{
    cMonthGraphiview->updateLunar();
}

//void CMonthGraphiview::updateInfo()
TEST_F(test_monthgraphiview, updateInfo)
{
    cMonthGraphiview->updateInfo();
}

//QPointF CMonthGraphiview::getItemPos(const QPoint &p, const QRectF &itemRect)
TEST_F(test_monthgraphiview, getItemPos)
{
    QPoint p(100, 100);
    QRectF itemRect;
    cMonthGraphiview->getItemPos(p, itemRect);
}

//PosInItem CMonthGraphiview::getPosInItem(const QPoint &p, const QRectF &itemRect)
TEST_F(test_monthgraphiview, getPosInItem_01)
{
    QPoint p(10, 10);
    QRectF rect(0, 0, 100, 100);
    cMonthGraphiview->getPosInItem(p, rect);
}

TEST_F(test_monthgraphiview, getPosInItem_02)
{
    QPoint p(90, 90);
    QRectF rect(0, 0, 100, 100);
    cMonthGraphiview->getPosInItem(p, rect);
}

//QDateTime CMonthGraphiview::getPosDate(const QPoint &p)
TEST_F(test_monthgraphiview, getPosDate_01)
{
    QPoint p(100, 100);
    cMonthGraphiview->getPosDate(p);
}

TEST_F(test_monthgraphiview, getPosDate_02)
{
    QPoint p(-100, -1);
    cMonthGraphiview->getPosDate(p);
}

TEST_F(test_monthgraphiview, getPosDate_03)
{
    QPoint p(-1, -1);
    cMonthGraphiview->setSceneRect(-2, -2, 100, 100);
    cMonthGraphiview->getPosDate(p);
}

//void CMonthGraphiview::upDateInfoShow(const CMonthGraphiview::DragStatus &status, const DSchedule::Ptr &info)
TEST_F(test_monthgraphiview, upDateInfoShow_01)
{
    Stub stub;
    stub.set(ADDR(CWeekScheduleView, addData), monthgraphiview_stub_addData);
    stub.set(ADDR(CWeekScheduleView, changeDate), monthgraphiview_stub_changeDate);
    DSchedule::Ptr info(new DSchedule());
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::IsCreate, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::ChangeBegin, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::ChangeEnd, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::ChangeWhole, info);
    cMonthGraphiview->upDateInfoShow(CMonthGraphicsview::NONE, info);
}

TEST_F(test_monthgraphiview, slideEvent_01)
{
    QPointF pos(1, 1);
    QPointF stopPos(2, 2);
    cMonthGraphiview->slideEvent(pos, stopPos);
}

TEST_F(test_monthgraphiview, mouseDoubleClickEvent_01)
{
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cMonthGraphiview->mouseDoubleClickEvent(&event);
}

TEST_F(test_monthgraphiview, mouseDoubleClickEvent_02)
{
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(0, 0), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    cMonthGraphiview->mouseDoubleClickEvent(&event);
}

TEST_F(test_monthgraphiview, mouseDoubleClickEvent_03)
{
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    cMonthGraphiview->mouseDoubleClickEvent(&event);
}

TEST_F(test_monthgraphiview, resizeEvent_01)
{
    QResizeEvent event(QSize(50, 50), QSize(100, 100));
    cMonthGraphiview->resizeEvent(&event);
}

TEST_F(test_monthgraphiview, changeEvent_01)
{
    QEvent event(QEvent::FontChange);
    cMonthGraphiview->changeEvent(&event);
}

TEST_F(test_monthgraphiview, wheelEvent_01)
{
    QWheelEvent event(QPointF(1, 1), QPointF(1, 1), QPoint(0, 120), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    cMonthGraphiview->wheelEvent(&event);
}

TEST_F(test_monthgraphiview, updateBackgroundShowItem_01)
{
    cMonthGraphiview->updateBackgroundShowItem();
}

TEST_F(test_monthgraphiview, setDragPixmap_01)
{
    Stub stub;
    stub.set(ADDR(CMonthScheduleItem, getPixmap), monthgraphiview_stub_getPixmap);
    stub.set(ADDR(QDrag, setPixmap), monthgraphiview_stub_QDrag_setPixmap);
    stub.set(ADDR(QDrag, setHotSpot), monthgraphiview_stub_QDrag_setHotSpot);
    QDrag drag(cMonthGraphiview);
    CMonthScheduleItem item(QRect(0, 0, 10, 10));
    cMonthGraphiview->setDragPixmap(&drag, &item);
}

TEST_F(test_monthgraphiview, MeetCreationConditions_01)
{
    QDateTime date;
    EXPECT_TRUE(cMonthGraphiview->MeetCreationConditions(date));
}

TEST_F(test_monthgraphiview, IsEqualtime_01)
{
    QDateTime date;
    EXPECT_TRUE(cMonthGraphiview->IsEqualtime(date, date));
}

TEST_F(test_monthgraphiview, JudgeIsCreate_01)
{
    EXPECT_FALSE(cMonthGraphiview->JudgeIsCreate(QPointF(1, 1)));
}

TEST_F(test_monthgraphiview, RightClickToCreate_01)
{
    QPoint pos(1, 1);
    cMonthGraphiview->RightClickToCreate(nullptr, pos);
}

TEST_F(test_monthgraphiview, MoveInfoProcess_01)
{
    Stub stub;
    void (CMonthScheduleView::*updateDate_ptr)(const int, const DSchedule::Ptr &) = &CMonthScheduleView::updateDate;
    typedef void (*fptr_updateDate)(CMonthScheduleView *, int, const DSchedule::Ptr &);
    stub.set((fptr_updateDate)(updateDate_ptr), monthScheduleView_updateDate_stub);

    QPointF pos(1, 1);
    DSchedule::Ptr info(new DSchedule());
    cMonthGraphiview->MoveInfoProcess(info, pos);
}

TEST_F(test_monthgraphiview, MoveInfoProcess_02)
{
    Stub stub;
    void (CMonthScheduleView::*updateDate_ptr)(const int, const DSchedule::Ptr &) = &CMonthScheduleView::updateDate;
    typedef void (*fptr_updateDate)(CMonthScheduleView *, int, const DSchedule::Ptr &);
    stub.set((fptr_updateDate)(updateDate_ptr), monthScheduleView_updateDate_stub);

    QPointF pos(1, -1);
    DSchedule::Ptr info(new DSchedule());
    cMonthGraphiview->MoveInfoProcess(info, pos);
}

TEST_F(test_monthgraphiview, getDragScheduleInfoBeginTime_01)
{
    QDateTime moveDateTime(QDate(2020, 10, 5), QTime(8, 0, 0));
    cMonthGraphiview->m_InfoBeginTime = QDateTime(QDate(2020, 10, 1), QTime(8, 0, 0));
    cMonthGraphiview->m_InfoEndTime = QDateTime(QDate(2020, 10, 10), QTime(10, 0, 0));
    QDateTime result = cMonthGraphiview->getDragScheduleInfoBeginTime(moveDateTime);
    EXPECT_EQ(result.date(), QDate(2020, 10, 5));
}

TEST_F(test_monthgraphiview, getDragScheduleInfoBeginTime_02)
{
    QPointF pos(1, -1);
    QDateTime time;
    cMonthGraphiview->m_InfoBeginTime = time;
    cMonthGraphiview->m_InfoEndTime = time;
    EXPECT_EQ(cMonthGraphiview->getDragScheduleInfoBeginTime(time), time);
}

TEST_F(test_monthgraphiview, getDragScheduleInfoEndTime_01)
{
    QDateTime moveDateTime(QDate(2020, 10, 5), QTime(10, 0, 0));
    cMonthGraphiview->m_InfoBeginTime = QDateTime(QDate(2020, 10, 1), QTime(8, 0, 0));
    cMonthGraphiview->m_InfoEndTime = QDateTime(QDate(2020, 10, 10), QTime(10, 0, 0));
    QDateTime result = cMonthGraphiview->getDragScheduleInfoEndTime(moveDateTime);
    EXPECT_EQ(result.date(), QDate(2020, 10, 5));
}

TEST_F(test_monthgraphiview, getDragScheduleInfoEndTime_02)
{
    QPointF pos(1, -1);
    QDateTime time;
    cMonthGraphiview->m_InfoBeginTime = time;
    cMonthGraphiview->m_InfoEndTime = time;
    EXPECT_EQ(cMonthGraphiview->getDragScheduleInfoEndTime(time), time);
}

//void CMonthGraphicsview::setSceneCurrentItemFocus(const QDate &focusDate)
TEST_F(test_monthgraphiview, setSceneCurrentItemFocus_01)
{
    QVector<QDate> showDate;
    for (int i = 0; i < 42; ++i) {
        showDate.append(QDate(2020, 10, 1).addDays(i));
    }
    cMonthGraphiview->setDate(showDate);
    QDate focusDate(2020, 10, 5);
    cMonthGraphiview->setSceneCurrentItemFocus(focusDate);
}

TEST_F(test_monthgraphiview, setSceneCurrentItemFocus_02)
{
    QDate focusDate(2020, 10, 5);
    cMonthGraphiview->setSceneCurrentItemFocus(focusDate);
}

TEST_F(test_monthgraphiview, slotCreate_02)
{
    Stub stub;
    calendarDDialogExecStub(stub);

    QPointF pos(1, -1);
    QDateTime time;
    cMonthGraphiview->m_InfoEndTime = time;
    cMonthGraphiview->slotCreate(time);
}
