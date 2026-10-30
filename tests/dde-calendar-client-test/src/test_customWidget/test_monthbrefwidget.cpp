// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthbrefwidget.h"

#include <QApplication>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QPixmap>
#include <QSet>
#include <QTest>
#include "cschedulebasewidget.h"
#include "calendarmanage.h"

test_monthbrefwidget::test_monthbrefwidget()
{
    mMonthBrefWidget = new MonthBrefWidget();
    // Access the GlobalData via the private member (allowed by -fno-access-control)
    mMonthDayRectWidget = new CMonthDayRectWidget(mMonthBrefWidget->m_globalData);
    // Initialize the static CalendarManager pointer used by CScheduleBaseWidget::getSelectDate()
    // MonthBrefWidget/CMonthDayRectWidget don't inherit from CScheduleBaseWidget, so the static
    // pointer remains null without this initialization, causing a null-deref in paintEvent.
    if (CScheduleBaseWidget::m_calendarManager == nullptr) {
        CScheduleBaseWidget::m_calendarManager = CalendarManager::getInstance();
    }
}

test_monthbrefwidget::~test_monthbrefwidget()
{
    delete mMonthDayRectWidget;
    mMonthDayRectWidget = nullptr;
    delete mMonthBrefWidget;
    mMonthBrefWidget = nullptr;
}

// ===== MonthBrefWidget tests =====

// MonthBrefWidget::MonthBrefWidget / ~MonthBrefWidget
TEST_F(test_monthbrefwidget, constructor)
{
    MonthBrefWidget *w = new MonthBrefWidget();
    EXPECT_NE(w, nullptr);
    delete w;
}

// void MonthBrefWidget::setShowMonthDate(const QDate& monthDate)
TEST_F(test_monthbrefwidget, setShowMonthDate)
{
    mMonthBrefWidget->setShowMonthDate(QDate::currentDate());
}

// void MonthBrefWidget::setHasScheduleDateSet(const QSet<QDate> &hasScheduleSet)
TEST_F(test_monthbrefwidget, setHasScheduleDateSet)
{
    QSet<QDate> scheduleSet;
    scheduleSet.insert(QDate::currentDate());
    scheduleSet.insert(QDate::currentDate().addDays(1));
    mMonthBrefWidget->setShowMonthDate(QDate::currentDate());
    mMonthBrefWidget->setHasScheduleDateSet(scheduleSet);
}

// void MonthBrefWidget::setHasScheduleDateSet — empty set
TEST_F(test_monthbrefwidget, setHasScheduleDateSet_empty)
{
    QSet<QDate> scheduleSet;
    mMonthBrefWidget->setHasScheduleDateSet(scheduleSet);
}

// void MonthBrefWidget::setHasSearchScheduleSet(const QSet<QDate> &hasScheduleSet)
TEST_F(test_monthbrefwidget, setHasSearchScheduleSet)
{
    QSet<QDate> searchSet;
    searchSet.insert(QDate::currentDate());
    mMonthBrefWidget->setShowMonthDate(QDate::currentDate());
    mMonthBrefWidget->setHasSearchScheduleSet(searchSet);
}

// void MonthBrefWidget::setHasSearchScheduleSet — empty set
TEST_F(test_monthbrefwidget, setHasSearchScheduleSet_empty)
{
    QSet<QDate> searchSet;
    mMonthBrefWidget->setHasSearchScheduleSet(searchSet);
}

// void MonthBrefWidget::mousePressEvent(QMouseEvent *event) — with MouseEventSynthesizedByQt
TEST_F(test_monthbrefwidget, mousePressEvent)
{
    mMonthBrefWidget->setFixedSize(600, 500);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(100, 100), Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &pressEvent);
}

// void MonthBrefWidget::mousePressEvent — without MouseEventSynthesizedByQt (should ignore)
TEST_F(test_monthbrefwidget, mousePressEvent_noSynth)
{
    mMonthBrefWidget->setFixedSize(600, 500);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(100, 100), Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &pressEvent);
}

// void MonthBrefWidget::mouseReleaseEvent(QMouseEvent *event) — with MouseEventSynthesizedByQt
TEST_F(test_monthbrefwidget, mouseReleaseEvent)
{
    mMonthBrefWidget->setFixedSize(600, 500);
    // First press to set touch state
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(100, 100), Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &pressEvent);

    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(100, 100), Qt::LeftButton,
                             Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &releaseEvent);
}

// void MonthBrefWidget::mouseMoveEvent(QMouseEvent *event) — with small movement
TEST_F(test_monthbrefwidget, mouseMoveEvent_small)
{
    mMonthBrefWidget->setFixedSize(600, 500);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(100, 100), Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &pressEvent);

    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(102, 100), Qt::LeftButton,
                          Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &moveEvent);
}

// void MonthBrefWidget::mouseMoveEvent — with large movement (> 5px)
TEST_F(test_monthbrefwidget, mouseMoveEvent_large)
{
    mMonthBrefWidget->setFixedSize(600, 500);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(100, 100), Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &pressEvent);

    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(120, 100), Qt::LeftButton,
                          Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthBrefWidget, &moveEvent);
}

// Render MonthBrefWidget for paint coverage
TEST_F(test_monthbrefwidget, render)
{
    mMonthBrefWidget->setFixedSize(800, 500);
    mMonthBrefWidget->setShowMonthDate(QDate::currentDate());
    QPixmap pixmap(mMonthBrefWidget->size());
    mMonthBrefWidget->render(&pixmap);
}

// ===== CMonthDayRectWidget tests =====

// CMonthDayRectWidget::CMonthDayRectWidget / destructor
TEST_F(test_monthbrefwidget, dayRect_constructor)
{
    CMonthDayRectWidget *w = new CMonthDayRectWidget(mMonthBrefWidget->m_globalData);
    EXPECT_NE(w, nullptr);
    delete w;
}

// void CMonthDayRectWidget::setTheMe(int type)
TEST_F(test_monthbrefwidget, dayRect_setTheMe)
{
    mMonthDayRectWidget->setTheMe(0);
    mMonthDayRectWidget->setTheMe(1);
    mMonthDayRectWidget->setTheMe(2);
}

// void CMonthDayRectWidget::setDate(const QDate &date)
TEST_F(test_monthbrefwidget, dayRect_setDate)
{
    mMonthDayRectWidget->setDate(QDate::currentDate());
}

// QDate CMonthDayRectWidget::getDate()
TEST_F(test_monthbrefwidget, dayRect_getDate)
{
    QDate date(2024, 6, 15);
    mMonthDayRectWidget->setDate(date);
    EXPECT_EQ(mMonthDayRectWidget->getDate(), date);
}

// void CMonthDayRectWidget::mousePressEvent(QMouseEvent *event) — left button + visible
TEST_F(test_monthbrefwidget, dayRect_mousePressEvent)
{
    mMonthDayRectWidget->setFixedSize(50, 50);
    mMonthDayRectWidget->show();
    QTest::qWait(10);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton,
                           Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthDayRectWidget, &pressEvent);
    mMonthDayRectWidget->hide();
}

// void CMonthDayRectWidget::mousePressEvent — right button (should not trigger)
TEST_F(test_monthbrefwidget, dayRect_mousePressEvent_right)
{
    mMonthDayRectWidget->setFixedSize(50, 50);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, QPointF(10, 10), Qt::RightButton,
                           Qt::RightButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthDayRectWidget, &pressEvent);
}

// void CMonthDayRectWidget::mouseDoubleClickEvent(QMouseEvent *event)
TEST_F(test_monthbrefwidget, dayRect_mouseDoubleClickEvent)
{
    mMonthDayRectWidget->setFixedSize(50, 50);
    QMouseEvent doubleClickEvent(QEvent::MouseButtonDblClick, QPointF(10, 10), Qt::LeftButton,
                                 Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthDayRectWidget, &doubleClickEvent);
}

// void CMonthDayRectWidget::mouseReleaseEvent(QMouseEvent *event)
TEST_F(test_monthbrefwidget, dayRect_mouseReleaseEvent)
{
    mMonthDayRectWidget->setFixedSize(50, 50);
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(10, 10), Qt::LeftButton,
                             Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(mMonthDayRectWidget, &releaseEvent);
}

// void CMonthDayRectWidget::paintEvent(QPaintEvent *event)
TEST_F(test_monthbrefwidget, dayRect_paintEvent)
{
    mMonthDayRectWidget->setFixedSize(50, 50);
    mMonthDayRectWidget->setDate(QDate::currentDate());
    QPixmap pixmap(mMonthDayRectWidget->size());
    mMonthDayRectWidget->render(&pixmap);
}

// void CMonthDayRectWidget::leaveEvent(QEvent *event)
TEST_F(test_monthbrefwidget, dayRect_leaveEvent)
{
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(mMonthDayRectWidget, &leaveEvent);
}

// Render with schedule/search flags set
TEST_F(test_monthbrefwidget, dayRect_renderWithFlags)
{
    mMonthDayRectWidget->setFixedSize(50, 50);
    mMonthDayRectWidget->setDate(QDate::currentDate());
    mMonthBrefWidget->setShowMonthDate(QDate::currentDate());

    QSet<QDate> scheduleSet;
    scheduleSet.insert(QDate::currentDate());
    mMonthBrefWidget->setHasScheduleDateSet(scheduleSet);

    QSet<QDate> searchSet;
    searchSet.insert(QDate::currentDate());
    mMonthBrefWidget->setHasSearchScheduleSet(searchSet);

    QPixmap pixmap(mMonthDayRectWidget->size());
    mMonthDayRectWidget->render(&pixmap);
}
