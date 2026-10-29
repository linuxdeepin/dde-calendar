// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_yearview.h"

#include "../third-party_stub/stub.h"
#include <QTest>
#include <QPixmap>
#include <QSet>

test_yearview::test_yearview()
{
}

test_yearview::~test_yearview()
{
}

void test_yearview::SetUp()
{
    cYearView = new CYearView();
}

void test_yearview::TearDown()
{
    delete cYearView;
    cYearView = nullptr;
}

// void CYearView::slotDoubleClickDate(const QDate &date)
TEST_F(test_yearview, slotDoubleClickDate)
{
    cYearView->slotDoubleClickDate(QDate(2021, 6, 15));
}

// void CYearView::slotPressClickDate(const QDate &date)
TEST_F(test_yearview, slotPressClickDate)
{
    cYearView->slotPressClickDate(QDate(2021, 6, 15));
}

// void CYearView::setTheMe(int type)
TEST_F(test_yearview, setTheMe)
{
    cYearView->setTheMe(1);
    cYearView->setTheMe(2);
    cYearView->setTheMe(0);
}

// void CYearView::setShowMonthDate(const QDate &showMonth)
TEST_F(test_yearview, setShowMonthDate)
{
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
}

// QDate CYearView::getShowMonthDate()
TEST_F(test_yearview, getShowMonthDate)
{
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    QDate result = cYearView->getShowMonthDate();
    EXPECT_EQ(result, QDate(2021, 6, 1));
}

// void CYearView::setHasScheduleSet(const QSet<QDate> &)
TEST_F(test_yearview, setHasScheduleSet)
{
    QSet<QDate> scheduleSet;
    scheduleSet << QDate(2021, 6, 1) << QDate(2021, 6, 15);
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    cYearView->setHasScheduleSet(scheduleSet);
}

// void CYearView::setHasSearchScheduleSet(const QSet<QDate> &)
TEST_F(test_yearview, setHasSearchScheduleSet)
{
    QSet<QDate> searchSet;
    searchSet << QDate(2021, 6, 1) << QDate(2021, 6, 15);
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    cYearView->setHasSearchScheduleSet(searchSet);
}

// bool CYearView::getStartAndStopDate(QDate &startDate, QDate &stopDate)
TEST_F(test_yearview, getStartAndStopDate)
{
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    QDate startDate;
    QDate stopDate;
    bool result = cYearView->getStartAndStopDate(startDate, stopDate);
    EXPECT_TRUE(result);
    EXPECT_EQ(startDate, QDate(2021, 6, 1));
    EXPECT_EQ(stopDate, QDate(2021, 6, 30));
}

// bool CYearView::eventFilter(QObject *o, QEvent *e) — double-click on currentMouth
TEST_F(test_yearview, eventFilter)
{
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    QWidget *currentMonth = cYearView->findChild<QWidget *>("currentMouth");
    if (currentMonth) {
        QMouseEvent dclick(QEvent::MouseButtonDblClick, QPointF(5, 5), Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
        cYearView->eventFilter(currentMonth, &dclick);
    }
    // also feed a non-matching object/event
    QEvent e(QEvent::Move);
    cYearView->eventFilter(cYearView, &e);
}

// void CYearView::resizeEvent(QResizeEvent *event)
TEST_F(test_yearview, resizeEvent)
{
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    QResizeEvent resizeEvent(QSize(400, 300), cYearView->size());
    QApplication::sendEvent(cYearView, &resizeEvent);
}

// void CYearView::paintEvent(QPaintEvent *e)
TEST_F(test_yearview, paintEvent)
{
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    cYearView->resize(400, 300);
    QPaintEvent paintEvent(QRect(0, 0, 400, 300));
    QApplication::sendEvent(cYearView, &paintEvent);
}

// GUI test: double-click currentMouth label
TEST_F(test_yearview, guitest)
{
    cYearView->setShowMonthDate(QDate(2021, 6, 15));
    QWidget *currentMonth = cYearView->findChild<QWidget *>("currentMouth");
    if (currentMonth) {
        QTest::mouseDClick(currentMonth, Qt::LeftButton);
    }
}
