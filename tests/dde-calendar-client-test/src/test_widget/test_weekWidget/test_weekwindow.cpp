// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_weekwindow.h"

#include "../third-party_stub/stub.h"
#include "customWidget/scheduleview.h"

#include <QTest>
#include <QMouseEvent>
#include <QResizeEvent>

// ============ Stubs for CScheduleView methods ============

namespace WeekWindowStub {

void slotDeleteitem_Stub(void *obj) { Q_UNUSED(obj) }
bool IsDragging_Stub(void *obj) { Q_UNUSED(obj); return false; }
void slotScheduleShow_Stub(void *obj, const bool isShow, const DSchedule::Ptr &out) { Q_UNUSED(obj) Q_UNUSED(isShow) Q_UNUSED(out) }
void slotUpdateScene_Stub(void *obj) { Q_UNUSED(obj) }
void setSelectSchedule_Stub(void *obj, const DSchedule::Ptr &scheduleInfo) { Q_UNUSED(obj) Q_UNUSED(scheduleInfo) }
void updateHeight_Stub(void *obj) { Q_UNUSED(obj) }
void setTheMe_Stub(void *obj, int type) { Q_UNUSED(obj) Q_UNUSED(type) }
void setLunarVisible_Stub(void *obj, bool state) { Q_UNUSED(obj) Q_UNUSED(state) }
void setTime_Stub(void *obj, QTime time) { Q_UNUSED(obj) Q_UNUSED(time) }
void setShowScheduleInfo_Stub(void *obj, const QMap<QDate, DSchedule::List> &scheduleInfo) { Q_UNUSED(obj) Q_UNUSED(scheduleInfo) }
void setRange_Stub(void *obj, QDate begin, QDate end) { Q_UNUSED(obj) Q_UNUSED(begin) Q_UNUSED(end) }
void setTimeFormat_Stub(void *obj, QString timeformat) { Q_UNUSED(obj) Q_UNUSED(timeformat) }

} // namespace WeekWindowStub

// void CWeekWindow::setTheMe(int type)
TEST_F(test_weekWindow, setTheMe)
{
    m_weekWindow->setTheMe(0);
    m_weekWindow->setTheMe(1);
    m_weekWindow->setTheMe(2);
}

// void CWeekWindow::setLunarVisible(bool state)
TEST_F(test_weekWindow, setLunarVisible)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, setLunarVisible), WeekWindowStub::setLunarVisible_Stub);
    m_weekWindow->setLunarVisible(true);
    m_weekWindow->setLunarVisible(false);
}

// void CWeekWindow::initUI() / initConnection() — covered by constructor
TEST_F(test_weekWindow, initUI_initConnection)
{
    // Already called in SetUp(); just verify no crash
    SUCCEED();
}

// void CWeekWindow::setTime(QTime time)
TEST_F(test_weekWindow, setTime)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, setTime), WeekWindowStub::setTime_Stub);
    m_weekWindow->setTime(QTime::currentTime());
}

// void CWeekWindow::setSearchWFlag(bool flag)
TEST_F(test_weekWindow, setSearchWFlag)
{
    m_weekWindow->setSearchWFlag(true);
    m_weekWindow->setSearchWFlag(false);
}

// void CWeekWindow::updateHeight()
TEST_F(test_weekWindow, updateHeight)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, updateHeight), WeekWindowStub::updateHeight_Stub);
    m_weekWindow->updateHeight();
}

// void CWeekWindow::setYearData()
TEST_F(test_weekWindow, setYearData)
{
    m_weekWindow->setYearData();
}

// void CWeekWindow::updateShowDate(const bool isUpdateBar)
TEST_F(test_weekWindow, updateShowDate)
{
    Stub stub;
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->updateShowDate(true);
    m_weekWindow->updateShowDate(false);
}

// void CWeekWindow::updateShowSchedule()
TEST_F(test_weekWindow, updateShowSchedule)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    m_weekWindow->updateShowSchedule();
}

// void CWeekWindow::updateShowLunar()
TEST_F(test_weekWindow, updateShowLunar)
{
    m_weekWindow->updateShowLunar();
}

// void CWeekWindow::updateSearchScheduleInfo()
TEST_F(test_weekWindow, updateSearchScheduleInfo)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotUpdateScene), WeekWindowStub::slotUpdateScene_Stub);
    m_weekWindow->updateSearchScheduleInfo();
}

// void CWeekWindow::setSelectSearchScheduleInfo(const DSchedule::Ptr &info)
TEST_F(test_weekWindow, setSelectSearchScheduleInfo)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, setSelectSchedule), WeekWindowStub::setSelectSchedule_Stub);
    DSchedule::Ptr info;
    m_weekWindow->setSelectSearchScheduleInfo(info);
}

// void CWeekWindow::deleteselectSchedule()
TEST_F(test_weekWindow, deleteselectSchedule)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotDeleteitem), WeekWindowStub::slotDeleteitem_Stub);
    m_weekWindow->deleteselectSchedule();
}

// void CWeekWindow::slotIsDragging(bool &isDragging)
TEST_F(test_weekWindow, slotIsDragging)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, IsDragging), WeekWindowStub::IsDragging_Stub);
    bool isDragging = true;
    m_weekWindow->slotIsDragging(isDragging);
    ASSERT_EQ(isDragging, false);
}

// void CWeekWindow::slotViewSelectDate(const QDate &date)
TEST_F(test_weekWindow, slotViewSelectDate)
{
    Stub stub;
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slotViewSelectDate(QDate::currentDate());
}

// void CWeekWindow::slotSwitchPrePage()
TEST_F(test_weekWindow, slotSwitchPrePage)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slotSwitchPrePage();
    QTest::qWait(50); // wait for QTimer::singleShot(5ms) to fire
}

// void CWeekWindow::slotSwitchNextPage()
TEST_F(test_weekWindow, slotSwitchNextPage)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slotSwitchNextPage();
    QTest::qWait(50); // wait for QTimer::singleShot(5ms) to fire
}

// void CWeekWindow::slotprev()
TEST_F(test_weekWindow, slotprev)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slotprev();
    QTest::qWait(50); // wait for QTimer::singleShot(5ms) to fire
}

// void CWeekWindow::slotnext()
TEST_F(test_weekWindow, slotnext)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slotnext();
    QTest::qWait(50); // wait for QTimer::singleShot(5ms) to fire
}

// void CWeekWindow::slottoday()
TEST_F(test_weekWindow, slottoday)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slottoday();
}

// void CWeekWindow::slotSelectDate(const QDate &date)
TEST_F(test_weekWindow, slotSelectDate)
{
    Stub stub;
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slotSelectDate(QDate::currentDate());
}

// void CWeekWindow::slotAngleDelta(int delta)
TEST_F(test_weekWindow, slotAngleDelta)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, IsDragging), WeekWindowStub::IsDragging_Stub);
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    // delta > 0 → slotprev
    m_weekWindow->slotAngleDelta(120);
    QTest::qWait(50);
    // delta < 0 → slotnext
    m_weekWindow->slotAngleDelta(-120);
    QTest::qWait(50);
    // delta == 0 → no action
    m_weekWindow->slotAngleDelta(0);
}

// void CWeekWindow::switchDate(const QDate &date)
TEST_F(test_weekWindow, switchDate)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    stub.set(static_cast<void(CScheduleView::*)(QDate, QDate)>(&CScheduleView::setRange), WeekWindowStub::setRange_Stub);
    stub.set(ADDR(CScheduleView, setTimeFormat), WeekWindowStub::setTimeFormat_Stub);
    stub.set(ADDR(CScheduleView, setShowScheduleInfo), WeekWindowStub::setShowScheduleInfo_Stub);
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    m_weekWindow->slotScheduleHide(); // also covers slotScheduleHide indirectly
    // switchDate is private, but accessible via -fno-access-control; call via slottoday which calls switchDate
    m_weekWindow->slottoday();
}

// void CWeekWindow::slotScheduleHide()
TEST_F(test_weekWindow, slotScheduleHide)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    m_weekWindow->slotScheduleHide();
}

// void CWeekWindow::resizeEvent(QResizeEvent *event)
TEST_F(test_weekWindow, resizeEvent)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    // Directly invoke resizeEvent since setFixedSize may not trigger it
    // when the widget is not shown
    QResizeEvent event1(QSize(1000, 600), QSize(100, 100));
    m_weekWindow->resizeEvent(&event1);
    // Also test with search flag on
    m_weekWindow->setSearchWFlag(true);
    QResizeEvent event2(QSize(800, 500), QSize(1000, 600));
    m_weekWindow->resizeEvent(&event2);
    m_weekWindow->setSearchWFlag(false);
}

// void CWeekWindow::mousePressEvent(QMouseEvent *event)
TEST_F(test_weekWindow, mousePressEvent)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, slotScheduleShow), WeekWindowStub::slotScheduleShow_Stub);
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPointF(100, 100),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_weekWindow, &mousePress);
}

// ~CWeekWindow() — destructor coverage
TEST_F(test_weekWindow, destructor)
{
    Stub stub;
    stub.set(ADDR(CScheduleView, setTheMe), WeekWindowStub::setTheMe_Stub);
    stub.set(ADDR(CScheduleView, setLunarVisible), WeekWindowStub::setLunarVisible_Stub);
    CWeekWindow *window = new CWeekWindow(nullptr);
    delete window;
}
