// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QSignalSpy>
#include "test_daymonthview.h"
#include <QTest>
#include <QEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>

static QVector<QDate> dayMonthviewGetDayList()
{
    QVector<QDate> dateList;
    QDate currentDate = QDate::currentDate();
    QDate firstOfMonth(currentDate.year(), currentDate.month(), 1);
    int offset = firstOfMonth.dayOfWeek() - 1;
    QDate startDate = firstOfMonth.addDays(-offset);
    for (int i = 0; i < 42; i++) {
        dateList.append(startDate.addDays(i));
    }
    return dateList;
}

static QMap<QDate, CaHuangLiDayInfo> dayMonthViewGetHuangLiDayInfo()
{
    QMap<QDate, CaHuangLiDayInfo> huangLiDayInfo;
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "辛丑牛年";
    info.mGanZhiMonth = "庚寅月";
    info.mGanZhiDay = "辛卯日";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    info.mLunarLeapMonth = 4;
    info.mZodiac = "牛";
    info.mTerm = "";
    info.mSolarFestival = "国庆节";
    info.mLunarFestival = "除夕";
    info.mSuit = "嫁娶.入土";
    info.mAvoid = "动土.破土";
    huangLiDayInfo.insert(QDate::currentDate(), info);
    return huangLiDayInfo;
}

static QVector<bool> dayMonthViewGetLineFlag()
{
    QVector<bool> flags;
    for (int i = 0; i < 42; i++) {
        flags.append(true);
    }
    return flags;
}

test_daymonthview::test_daymonthview()
{
    mDayMonthView = new CDayMonthView();
    mDayMonthWidget = new CDayMonthWidget();
}

test_daymonthview::~test_daymonthview()
{
    delete mDayMonthView;
    mDayMonthView = nullptr;
    delete mDayMonthWidget;
    mDayMonthWidget = nullptr;
}

// === CDayMonthView tests ===

//CDayMonthView::CDayMonthView(QWidget *parent)
TEST_F(test_daymonthview, view_constructor)
{
    CDayMonthView *view = new CDayMonthView();
    ASSERT_NE(view, nullptr);
    delete view;
}

//void CDayMonthView::setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate)
TEST_F(test_daymonthview, view_setShowDate)
{
    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
}

//void CDayMonthView::setLunarVisible(bool visible)
TEST_F(test_daymonthview, view_setLunarVisible)
{
    mDayMonthView->setLunarVisible(true);
    mDayMonthView->setLunarVisible(false);
}

//void CDayMonthView::setTheMe(int type)
TEST_F(test_daymonthview, view_setTheMe)
{
    mDayMonthView->setTheMe(0);
    mDayMonthView->setTheMe(1);
    mDayMonthView->setTheMe(2);
}

//void CDayMonthView::setSearchFlag(bool flag)
TEST_F(test_daymonthview, view_setSearchFlag)
{
    mDayMonthView->setSearchFlag(true);
    mDayMonthView->setSearchFlag(false);
}

//void CDayMonthView::setHuangLiInfo(const CaHuangLiDayInfo &huangLiInfo)
TEST_F(test_daymonthview, view_setHuangLiInfo)
{
    mDayMonthView->setHuangLiInfo(dayMonthViewGetHuangLiDayInfo().value(QDate::currentDate()));
}

//void CDayMonthView::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
TEST_F(test_daymonthview, view_setHasScheduleFlag)
{
    mDayMonthView->setHasScheduleFlag(dayMonthViewGetLineFlag());
}

//void CDayMonthView::updateDateLunarDay()
TEST_F(test_daymonthview, view_updateDateLunarDay)
{
    mDayMonthView->setHuangLiInfo(dayMonthViewGetHuangLiDayInfo().value(QDate::currentDate()));
}

//void CDayMonthView::changeSelectDate(const QDate &date)
TEST_F(test_daymonthview, view_changeSelectDate)
{
    QSignalSpy spy(mDayMonthView, &CDayMonthView::signalChangeSelectDate);
    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    // Emit via slotprev to trigger changeSelectDate
    mDayMonthView->slotprev();
    EXPECT_GT(spy.count(), 0);
}

//void CDayMonthView::slotprev()
TEST_F(test_daymonthview, view_slotprev)
{
    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    mDayMonthView->slotprev();
}

//void CDayMonthView::slotnext()
TEST_F(test_daymonthview, view_slotnext)
{
    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    mDayMonthView->slotnext();
}

//void CDayMonthView::slottoday()
TEST_F(test_daymonthview, view_slottoday)
{
    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    mDayMonthView->slottoday();
}

//void CDayMonthView::wheelEvent(QWheelEvent *event)
TEST_F(test_daymonthview, view_wheelEvent)
{
    mDayMonthView->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    mDayMonthView->setFixedSize(400, 300);
    QPoint pos(100, 100);
    QWheelEvent wheelDown(QPointF(100, 100), QPointF(100, 100), QPoint(), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(mDayMonthView, &wheelDown);

    QWheelEvent wheelUp(QPointF(100, 100), QPointF(100, 100), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(mDayMonthView, &wheelUp);
}

//void CDayMonthView::paintEvent(QPaintEvent *e)
TEST_F(test_daymonthview, view_paintEvent)
{
    mDayMonthView->setSearchFlag(false);
    mDayMonthView->setFixedSize(400, 300);
    mDayMonthView->show();
    QTest::qWait(50);
    mDayMonthView->setSearchFlag(true);
    QTest::qWait(50);
    mDayMonthView->hide();
}

// === CDayMonthWidget tests ===

//CDayMonthWidget::CDayMonthWidget(QWidget *parent)
TEST_F(test_daymonthview, widget_constructor)
{
    CDayMonthWidget *widget = new CDayMonthWidget();
    ASSERT_NE(widget, nullptr);
    delete widget;
}

//void CDayMonthWidget::setTheMe(int type)
TEST_F(test_daymonthview, widget_setTheMe)
{
    mDayMonthWidget->setTheMe(0);
    mDayMonthWidget->setTheMe(1);
    mDayMonthWidget->setTheMe(2);
}

//void CDayMonthWidget::setShowDate(const QVector<QDate> &showDate, const QDate &selectDate, const QDate &currentDate)
TEST_F(test_daymonthview, widget_setShowDate)
{
    mDayMonthWidget->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
}

//void CDayMonthWidget::setHasScheduleFlag(const QVector<bool> &hasScheduleFlag)
TEST_F(test_daymonthview, widget_setHasScheduleFlag)
{
    mDayMonthWidget->setHasScheduleFlag(dayMonthViewGetLineFlag());
}

//void CDayMonthWidget::cellClicked(QWidget *cell)
TEST_F(test_daymonthview, widget_cellClicked)
{
    mDayMonthWidget->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    mDayMonthWidget->setFixedSize(400, 250);
    // Trigger paintEvent via eventFilter for each cell to populate internal state
    mDayMonthWidget->show();
    QTest::qWait(50);
    mDayMonthWidget->hide();
}

//void CDayMonthWidget::setSelectedCell(int index)
TEST_F(test_daymonthview, widget_setSelectedCell)
{
    mDayMonthWidget->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    QSignalSpy spy(mDayMonthWidget, &CDayMonthWidget::signalChangeSelectDate);
    mDayMonthWidget->setSelectedCell(10);
    EXPECT_GT(spy.count(), 0);
    spy.clear();
    mDayMonthWidget->setSelectedCell(20);
    EXPECT_GT(spy.count(), 0);
}

//void CDayMonthWidget::keyPressEvent(QKeyEvent *event)
TEST_F(test_daymonthview, widget_keyPressEvent)
{
    mDayMonthWidget->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    QSignalSpy spy(mDayMonthWidget, &CDayMonthWidget::signalChangeSelectDate);

    QKeyEvent keyLeft(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QCoreApplication::sendEvent(mDayMonthWidget, &keyLeft);

    QKeyEvent keyRight(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QCoreApplication::sendEvent(mDayMonthWidget, &keyRight);

    QKeyEvent keyUp(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QCoreApplication::sendEvent(mDayMonthWidget, &keyUp);

    QKeyEvent keyDown(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QCoreApplication::sendEvent(mDayMonthWidget, &keyDown);

    QKeyEvent keyOther(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    QCoreApplication::sendEvent(mDayMonthWidget, &keyOther);
}

//void CDayMonthWidget::focusInEvent(QFocusEvent *event)
TEST_F(test_daymonthview, widget_focusInEvent)
{
    QFocusEvent focusIn(QEvent::FocusIn, Qt::TabFocusReason);
    QCoreApplication::sendEvent(mDayMonthWidget, &focusIn);

    QFocusEvent focusInBacktab(QEvent::FocusIn, Qt::BacktabFocusReason);
    QCoreApplication::sendEvent(mDayMonthWidget, &focusInBacktab);

    QFocusEvent focusInActive(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
    QCoreApplication::sendEvent(mDayMonthWidget, &focusInActive);

    QFocusEvent focusInOther(QEvent::FocusIn, Qt::MouseFocusReason);
    QCoreApplication::sendEvent(mDayMonthWidget, &focusInOther);
}

//void CDayMonthWidget::focusOutEvent(QFocusEvent *event)
TEST_F(test_daymonthview, widget_focusOutEvent)
{
    QFocusEvent focusOut(QEvent::FocusOut, Qt::ActiveWindowFocusReason);
    QCoreApplication::sendEvent(mDayMonthWidget, &focusOut);
}

//void CDayMonthWidget::mousePressEvent(QMouseEvent *event)
TEST_F(test_daymonthview, widget_mousePressEvent)
{
    mDayMonthWidget->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(mDayMonthWidget, &mousePress);
}

//void CDayMonthWidget::resizeEvent(QResizeEvent *event)
TEST_F(test_daymonthview, widget_resizeEvent)
{
    mDayMonthWidget->setFixedSize(400, 250);
    mDayMonthWidget->setFixedSize(700, 300);
}

//void CDayMonthWidget::eventFilter(QObject *o, QEvent *e)
TEST_F(test_daymonthview, widget_eventFilter)
{
    mDayMonthWidget->setShowDate(dayMonthviewGetDayList(), QDate::currentDate(), QDate::currentDate());
    mDayMonthWidget->setFixedSize(400, 250);
    mDayMonthWidget->show();
    QTest::qWait(50);
    mDayMonthWidget->hide();
}
