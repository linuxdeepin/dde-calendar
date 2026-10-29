// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_yearscheduleview.h"
#include "../third-party_stub/stub.h"
#include "constants.h"
#include <QDebug>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QPixmap>
#include <QTest>

// free functions defined in yearscheduleview.cpp (external linkage)
extern bool YScheduleDateThan(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2);
extern bool YScheduleDaysThan(const DSchedule::Ptr &s1, const DSchedule::Ptr &s2);

test_yearscheduleview::test_yearscheduleview()
{
}

test_yearscheduleview::~test_yearscheduleview()
{
}

void test_yearscheduleview::SetUp()
{
    cYearScheduleView = new CYearScheduleView();
    zYearScheduleOutView = new CYearScheduleOutView();
}

void test_yearscheduleview::TearDown()
{
    delete cYearScheduleView;
    cYearScheduleView = nullptr;
    delete zYearScheduleOutView;
    zYearScheduleOutView = nullptr;
}

static DSchedule::Ptr makeSchedule(const QDate &begin, const QDate &end,
                                   const QString &title, bool allDay = false)
{
    DSchedule::Ptr ptr(new DSchedule());
    ptr->setSummary(title);
    ptr->setDtStart(QDateTime(begin, QTime(0, 0, 0)));
    ptr->setDtEnd(QDateTime(end, QTime(23, 59, 59)));
    ptr->setAllDay(allDay);
    return ptr;
}

static DSchedule::List getScheduleList()
{
    DSchedule::List list;
    list.append(makeSchedule(QDate(2021, 1, 1), QDate(2021, 1, 2), "all-day one", true));
    list.append(makeSchedule(QDate(2021, 1, 3), QDate(2021, 1, 5), "all-day two", true));
    list.append(makeSchedule(QDate(2021, 1, 6), QDate(2021, 1, 9), "timed three", false));
    list.append(makeSchedule(QDate(2021, 1, 10), QDate(2021, 1, 14), "timed four", false));
    list.append(makeSchedule(QDate(2021, 1, 15), QDate(2021, 1, 20), "timed five", false));
    return list;
}

// CYearScheduleView constructor
TEST_F(test_yearscheduleview, constructor)
{
    ASSERT_NE(cYearScheduleView, nullptr);
}

// CYearScheduleView destructor (covered via TearDown, explicit smoke test)
TEST_F(test_yearscheduleview, destructor)
{
    CYearScheduleView *view = new CYearScheduleView();
    delete view;
}

// void CYearScheduleView::setData(DSchedule::List &vListData)
TEST_F(test_yearscheduleview, setData)
{
    DSchedule::List list = getScheduleList();
    cYearScheduleView->setData(list);
}

// setData with empty list
TEST_F(test_yearscheduleview, setData_empty)
{
    DSchedule::List list;
    cYearScheduleView->setData(list);
}

// setData with more than max count (truncation path)
TEST_F(test_yearscheduleview, setData_truncate)
{
    DSchedule::List list;
    for (int i = 0; i < 10; ++i) {
        list.append(makeSchedule(QDate(2021, 1, 1).addDays(i),
                                 QDate(2021, 1, 2).addDays(i),
                                 QString("item %1").arg(i), i % 2 == 0));
    }
    cYearScheduleView->setData(list);
}

// void CYearScheduleView::clearData()
TEST_F(test_yearscheduleview, clearData)
{
    DSchedule::List list = getScheduleList();
    cYearScheduleView->setData(list);
    cYearScheduleView->clearData();
}

// void CYearScheduleView::showWindow()
TEST_F(test_yearscheduleview, showWindow_empty)
{
    cYearScheduleView->showWindow();
    cYearScheduleView->repaint();
    QCoreApplication::processEvents();
}

TEST_F(test_yearscheduleview, showWindow_withData)
{
    DSchedule::List list = getScheduleList();
    cYearScheduleView->setData(list);
    cYearScheduleView->showWindow();
    cYearScheduleView->repaint();
    QCoreApplication::processEvents();
}

// void CYearScheduleView::setTheMe(int type)
TEST_F(test_yearscheduleview, setTheMe)
{
    cYearScheduleView->setTheMe(1);
    cYearScheduleView->setTheMe(2);
    cYearScheduleView->setTheMe(0);
}

// void CYearScheduleView::setCurrentDate(const QDate &cdate)
TEST_F(test_yearscheduleview, setCurrentDate)
{
    cYearScheduleView->setCurrentDate(QDate(2021, 6, 15));
}

// QDate CYearScheduleView::getCurrentDate()
TEST_F(test_yearscheduleview, getCurrentDate)
{
    QDate d(2021, 6, 15);
    cYearScheduleView->setCurrentDate(d);
    EXPECT_EQ(cYearScheduleView->getCurrentDate(), d);
}

// void CYearScheduleView::setTimeFormat(const QString &format)
TEST_F(test_yearscheduleview, setTimeFormat)
{
    cYearScheduleView->setTimeFormat("hh:mm");
}

// int CYearScheduleView::getPressScheduleIndex()
TEST_F(test_yearscheduleview, getPressScheduleIndex)
{
    // cursor not inside any draw rect -> -1
    int idx = cYearScheduleView->getPressScheduleIndex();
    EXPECT_EQ(idx, -1);
}

// bool CYearScheduleView::event(QEvent *event) — ToolTip path
TEST_F(test_yearscheduleview, event_ToolTip)
{
    DSchedule::List list = getScheduleList();
    cYearScheduleView->setData(list);
    cYearScheduleView->resize(300, 300);
    cYearScheduleView->showWindow();
    cYearScheduleView->repaint();
    QCoreApplication::processEvents();
    QHelpEvent helpEvent(QEvent::ToolTip, QPoint(40, 20), cYearScheduleView->mapToGlobal(QPoint(40, 20)));
    QApplication::sendEvent(cYearScheduleView, &helpEvent);

    QHelpEvent helpEvent2(QEvent::ToolTip, QPoint(500, 500), cYearScheduleView->mapToGlobal(QPoint(500, 500)));
    QApplication::sendEvent(cYearScheduleView, &helpEvent2);
}

// bool CYearScheduleView::event(QEvent *event) — non-ToolTip path
TEST_F(test_yearscheduleview, event_nonToolTip)
{
    QEvent e(QEvent::Move);
    cYearScheduleView->event(&e);
}

// void CYearScheduleView::paintEvent(QPaintEvent *event)
TEST_F(test_yearscheduleview, paintEvent_empty)
{
    cYearScheduleView->setFixedSize(300, 200);
    QPaintEvent paintEvent(QRect(0, 0, 300, 200));
    QApplication::sendEvent(cYearScheduleView, &paintEvent);
}

TEST_F(test_yearscheduleview, paintEvent_withData)
{
    DSchedule::List list = getScheduleList();
    cYearScheduleView->setData(list);
    cYearScheduleView->setFixedSize(300, 300);
    cYearScheduleView->showWindow();
    QPaintEvent paintEvent(QRect(0, 0, 300, 300));
    QApplication::sendEvent(cYearScheduleView, &paintEvent);
}

// void CYearScheduleView::updateDateShow() — private, exercised via showWindow
TEST_F(test_yearscheduleview, updateDateShow)
{
    DSchedule::List list = getScheduleList();
    cYearScheduleView->setData(list);
    cYearScheduleView->showWindow();
    cYearScheduleView->repaint();
    QCoreApplication::processEvents();
}

// ===== CYearScheduleOutView =====

// constructor
TEST_F(test_yearscheduleview, outView_constructor)
{
    ASSERT_NE(zYearScheduleOutView, nullptr);
}

// void CYearScheduleOutView::clearData()
TEST_F(test_yearscheduleview, outView_clearData)
{
    zYearScheduleOutView->clearData();
}

// void CYearScheduleOutView::setData(DSchedule::List &vListData)
TEST_F(test_yearscheduleview, outView_setData)
{
    DSchedule::List list = getScheduleList();
    zYearScheduleOutView->setData(list);
}

// void CYearScheduleOutView::setTheMe(int type)
TEST_F(test_yearscheduleview, outView_setTheMe)
{
    zYearScheduleOutView->setTheMe(1);
    zYearScheduleOutView->setTheMe(2);
}

// void CYearScheduleOutView::setCurrentDate(const QDate &cDate)
TEST_F(test_yearscheduleview, outView_setCurrentDate)
{
    zYearScheduleOutView->setCurrentDate(QDate(2021, 6, 15));
}

// void CYearScheduleOutView::setDirection(DArrowRectangle::ArrowDirection)
TEST_F(test_yearscheduleview, outView_setDirection)
{
    zYearScheduleOutView->setDirection(DArrowRectangle::ArrowDirection::ArrowTop);
    zYearScheduleOutView->setDirection(DArrowRectangle::ArrowDirection::ArrowBottom);
}

// void CYearScheduleOutView::setTimeFormat(const QString &format)
TEST_F(test_yearscheduleview, outView_setTimeFormat)
{
    zYearScheduleOutView->setTimeFormat("hh:mm");
}

// void CYearScheduleOutView::mousePressEvent(QMouseEvent *event)
TEST_F(test_yearscheduleview, outView_mousePressEvent)
{
    // empty data -> getPressScheduleIndex returns -1 -> early return, safe
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(zYearScheduleOutView, &press);
}

// ===== Free functions =====

TEST_F(test_yearscheduleview, YScheduleDateThan)
{
    DSchedule::Ptr s1 = makeSchedule(QDate(2021, 1, 1), QDate(2021, 1, 2), "aaa");
    DSchedule::Ptr s2 = makeSchedule(QDate(2021, 1, 3), QDate(2021, 1, 4), "bbb");
    // s1 starts before s2 -> s1 < s2
    EXPECT_TRUE(YScheduleDateThan(s1, s2));
    EXPECT_FALSE(YScheduleDateThan(s2, s1));

    // same begin/end dates -> compare by summary
    DSchedule::Ptr s3 = makeSchedule(QDate(2021, 1, 1), QDate(2021, 1, 1), "aaa");
    DSchedule::Ptr s4 = makeSchedule(QDate(2021, 1, 1), QDate(2021, 1, 1), "bbb");
    EXPECT_TRUE(YScheduleDateThan(s3, s4));
    EXPECT_FALSE(YScheduleDateThan(s4, s3));
}

TEST_F(test_yearscheduleview, YScheduleDaysThan)
{
    DSchedule::Ptr s1 = makeSchedule(QDate(2021, 1, 1), QDate(2021, 1, 5), "short");  // 4 days
    DSchedule::Ptr s2 = makeSchedule(QDate(2021, 1, 1), QDate(2021, 1, 10), "long");  // 9 days
    EXPECT_FALSE(YScheduleDaysThan(s1, s2));  // 4 > 9 is false
    EXPECT_TRUE(YScheduleDaysThan(s2, s1));   // 9 > 4 is true
}
