// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_yearwindow.h"
#include "customWidget/customframe.h"
#include "widget/yearWidget/yearscheduleview.h"
#include "calendarglobalenv.h"
#include "constants.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QGestureEvent>
#include <QTapGesture>
#include <QPanGesture>
#include <QResizeEvent>
#include <QTest>
#include <QPixmap>
#include <QSet>
#include <QtWidgets/private/qgesture_p.h>

test_yearwindow::test_yearwindow()
{
    CalendarGlobalEnv::getGlobalEnv()->registerKey(DDECalendar::CursorPointKey, QPoint(20, 20));
    mYearWindow = new CYearWindow();
    // Let the deferred setYearData timer fire
    QCoreApplication::processEvents();
}

test_yearwindow::~test_yearwindow()
{
    delete mYearWindow;
    mYearWindow = nullptr;
}

// ===== CYearWindow constructor / destructor =====

TEST_F(test_yearwindow, constructor)
{
    ASSERT_NE(mYearWindow, nullptr);
}

TEST_F(test_yearwindow, destructor)
{
    CYearWindow *win = new CYearWindow();
    QCoreApplication::processEvents();
    delete win;
}

// initUI / initConnection — called by constructor, exercise explicitly
TEST_F(test_yearwindow, initUI)
{
    mYearWindow->initUI();
}

TEST_F(test_yearwindow, initConnection)
{
    mYearWindow->initConnection();
}

// ===== setTheMe =====

TEST_F(test_yearwindow, setTheMe)
{
    mYearWindow->setTheMe(0);
    mYearWindow->setTheMe(1);
    mYearWindow->setTheMe(2);
}

// ===== setSearchWFlag =====

TEST_F(test_yearwindow, setSearchWFlag)
{
    mYearWindow->setSearchWFlag(true);
    mYearWindow->setSearchWFlag(false);
}

// ===== setYearData =====

TEST_F(test_yearwindow, setYearData)
{
    mYearWindow->setYearData();
}

// ===== updateShowDate =====

TEST_F(test_yearwindow, updateShowDate)
{
    mYearWindow->updateShowDate(true);
    mYearWindow->updateShowDate(false);
}

// ===== updateShowSchedule =====

TEST_F(test_yearwindow, updateShowSchedule)
{
    mYearWindow->updateShowSchedule();
}

// ===== updateShowLunar =====

TEST_F(test_yearwindow, updateShowLunar)
{
    mYearWindow->updateShowLunar();
}

// ===== updateSearchScheduleInfo =====

TEST_F(test_yearwindow, updateSearchScheduleInfo)
{
    mYearWindow->updateSearchScheduleInfo();
}

// ===== setSelectSearchScheduleInfo =====

TEST_F(test_yearwindow, setSelectSearchScheduleInfo)
{
    DSchedule::Ptr ptr(new DSchedule());
    ptr->setSummary("test schedule");
    mYearWindow->setSelectSearchScheduleInfo(ptr);
}

// ===== slotprev / slotnext / slottoday =====

TEST_F(test_yearwindow, slotprev)
{
    mYearWindow->slotprev();
}

TEST_F(test_yearwindow, slotnext)
{
    mYearWindow->slotnext();
}

TEST_F(test_yearwindow, slottoday)
{
    mYearWindow->slottoday();
}

// ===== switchYear (private, accessible via -fno-access-control) =====

TEST_F(test_yearwindow, switchYear)
{
    mYearWindow->switchYear(1);
    mYearWindow->switchYear(-1);
}

// ===== onLunarInfoReady =====

TEST_F(test_yearwindow, onLunarInfoReady)
{
    CaHuangLiDayInfo info;
    // pass current select date so the method does not early-return
    mYearWindow->onLunarInfoReady(mYearWindow->getSelectDate(), info);
    // pass a different date to exercise the early-return path
    mYearWindow->onLunarInfoReady(QDate(1999, 1, 1), info);
}

// ===== setLunarShow (private) =====

TEST_F(test_yearwindow, setLunarShow)
{
    mYearWindow->setLunarShow();
}

// ===== slotSetScheduleHide =====

TEST_F(test_yearwindow, slotSetScheduleHide)
{
    mYearWindow->slotSetScheduleHide();
}

// ===== slotMousePress (various press types) =====

TEST_F(test_yearwindow, slotMousePress)
{
    CalendarGlobalEnv::getGlobalEnv()->reviseValue(DDECalendar::CursorPointKey, QPoint(20, 20));
    QDate currentDate = QDate::currentDate();
    mYearWindow->slotMousePress(currentDate, 0);
    ASSERT_EQ(mYearWindow->getSelectDate(), currentDate);
    mYearWindow->slotMousePress(currentDate, 1);
    mYearWindow->slotMousePress(currentDate, 2);
    mYearWindow->slotMousePress(currentDate, 3);
    mYearWindow->slotMousePress(currentDate, 4);
}

// ===== wheelEvent (down → slotnext, up → slotprev, horizontal → return) =====

TEST_F(test_yearwindow, wheelEvent)
{
    // Vertical scroll down (y < 0 → slotnext)
    QWheelEvent wheelDown(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0),
                          QPoint(0, -120), Qt::NoButton, Qt::NoModifier,
                          Qt::ScrollUpdate, false);
    QApplication::sendEvent(mYearWindow, &wheelDown);

    // Vertical scroll up (y > 0 → slotprev)
    QWheelEvent wheelUp(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0),
                        QPoint(0, 120), Qt::NoButton, Qt::NoModifier,
                        Qt::ScrollUpdate, false);
    QApplication::sendEvent(mYearWindow, &wheelUp);

    // Horizontal scroll (x != 0 → early return)
    QWheelEvent wheelH(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0),
                       QPoint(120, 0), Qt::NoButton, Qt::NoModifier,
                       Qt::ScrollUpdate, false);
    QApplication::sendEvent(mYearWindow, &wheelH);
}

// ===== eventFilter (today button + year widget) =====

TEST_F(test_yearwindow, eventFilter)
{
    // Today button — MouseButtonPress
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPointF(5, 5),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mYearWindow->eventFilter(mYearWindow->m_today, &mousePress);

    // Today button — KeyPress Key_Return
    QKeyEvent keyReturn(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    mYearWindow->eventFilter(mYearWindow->m_today, &keyReturn);

    // Year widget — KeyPress Key_Up → slotprev
    QKeyEvent keyUp(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    mYearWindow->eventFilter(mYearWindow->m_yearWidget, &keyUp);

    // Year widget — KeyPress Key_Down → slotnext
    QKeyEvent keyDown(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    mYearWindow->eventFilter(mYearWindow->m_yearWidget, &keyDown);

    // Non-matching watched object
    QEvent otherEvent(QEvent::Move);
    mYearWindow->eventFilter(mYearWindow, &otherEvent);
}

// ===== mousePressEvent (synthesized + normal) =====

TEST_F(test_yearwindow, mousePressEvent)
{
    // Synthesized touch event — records m_touchBeginPoint
    QMouseEvent synthPress(QEvent::MouseButtonPress, QPointF(10, 10),
                           QPointF(10, 10), QPointF(10, 10),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                           Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(mYearWindow, &synthPress);

    // Normal mouse event
    QMouseEvent normalPress(QEvent::MouseButtonPress, QPointF(20, 20),
                            Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mYearWindow, &normalPress);
}

// ===== resizeEvent =====

TEST_F(test_yearwindow, resizeEvent)
{
    QResizeEvent resizeEvent(QSize(400, 300), mYearWindow->size());
    QApplication::sendEvent(mYearWindow, &resizeEvent);
}

// ===== mouseMoveEvent (synthesized + normal) =====

TEST_F(test_yearwindow, mouseMoveEvent)
{
    // Synthesized touch move — early return
    QMouseEvent synthMove(QEvent::MouseMove, QPointF(15, 15),
                          QPointF(15, 15), QPointF(15, 15),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                          Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(mYearWindow, &synthMove);

    // Normal mouse move
    QMouseEvent normalMove(QEvent::MouseMove, QPointF(25, 25),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mYearWindow, &normalMove);
}

// ===== mouseReleaseEvent (synthesized with touchState + normal) =====

TEST_F(test_yearwindow, mouseReleaseEvent)
{
    // Set m_touchState = 2 (as if tap gesture updated) to exercise
    // the calculateAzimuthAngle path in mouseReleaseEvent
    mYearWindow->m_touchState = 2;

    // Synthesized release with upward movement → T_TOP → slotnext
    mYearWindow->m_touchBeginPoint = QPointF(50, 50);
    QMouseEvent synthReleaseUp(QEvent::MouseButtonRelease, QPointF(50, 10),
                               QPointF(50, 10), QPointF(50, 10),
                               Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                               Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(mYearWindow, &synthReleaseUp);

    // Reset and test downward movement → T_BOTTOM → slotprev
    mYearWindow->m_touchState = 2;
    mYearWindow->m_touchBeginPoint = QPointF(50, 10);
    QMouseEvent synthReleaseDown(QEvent::MouseButtonRelease, QPointF(50, 50),
                                 QPointF(50, 50), QPointF(50, 50),
                                 Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                                 Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(mYearWindow, &synthReleaseDown);

    // Normal (non-synthesized) release
    mYearWindow->m_touchState = 0;
    QMouseEvent normalRelease(QEvent::MouseButtonRelease, QPointF(30, 30),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mYearWindow, &normalRelease);
}

// ===== event (gesture + non-gesture) =====

TEST_F(test_yearwindow, event_gesture)
{
    QTapGesture tap;
    tap.d_func()->state = Qt::GestureStarted;
    QList<QGesture *> gestures;
    gestures << &tap;
    QGestureEvent gestureEvent(gestures);
    mYearWindow->event(&gestureEvent);
}

TEST_F(test_yearwindow, event_nonGesture)
{
    QEvent moveEvent(QEvent::Move);
    mYearWindow->event(&moveEvent);
}

// ===== gestureEvent (private) =====

TEST_F(test_yearwindow, gestureEvent)
{
    QTapGesture tap;
    tap.d_func()->state = Qt::GestureStarted;
    QPanGesture pan;
    pan.d_func()->state = Qt::GestureFinished;
    pan.setOffset(QPointF(0, -50));

    QList<QGesture *> gestures;
    gestures << &tap << &pan;
    QGestureEvent gestureEvent(gestures);
    mYearWindow->gestureEvent(&gestureEvent);
}

// ===== tapGestureTriggered (private — all states) =====

TEST_F(test_yearwindow, tapGestureTriggered)
{
    QTapGesture tap;

    tap.d_func()->state = Qt::NoGesture;
    mYearWindow->tapGestureTriggered(&tap);

    tap.d_func()->state = Qt::GestureStarted;
    mYearWindow->tapGestureTriggered(&tap);

    tap.d_func()->state = Qt::GestureUpdated;
    mYearWindow->tapGestureTriggered(&tap);

    tap.d_func()->state = Qt::GestureFinished;
    mYearWindow->tapGestureTriggered(&tap);

    tap.d_func()->state = static_cast<Qt::GestureState>(999);
    mYearWindow->tapGestureTriggered(&tap);
}

// ===== panTriggered (private — finished + non-finished) =====

TEST_F(test_yearwindow, panTriggered)
{
    // GestureFinished with upward offset → T_TOP → slotnext
    QPanGesture panUp;
    panUp.d_func()->state = Qt::GestureFinished;
    panUp.setOffset(QPointF(0, -50));
    mYearWindow->panTriggered(&panUp);

    // GestureFinished with downward offset → T_BOTTOM → slotprev
    QPanGesture panDown;
    panDown.d_func()->state = Qt::GestureFinished;
    panDown.setOffset(QPointF(0, 50));
    mYearWindow->panTriggered(&panDown);

    // GestureFinished with rightward offset → T_RIGHT → default (no switch)
    QPanGesture panRight;
    panRight.d_func()->state = Qt::GestureFinished;
    panRight.setOffset(QPointF(50, 0));
    mYearWindow->panTriggered(&panRight);

    // Non-finished state → default
    QPanGesture panCanceled;
    panCanceled.d_func()->state = Qt::GestureCanceled;
    mYearWindow->panTriggered(&panCanceled);
}

// ===== calculateAzimuthAngle (private — all directions) =====

TEST_F(test_yearwindow, calculateAzimuthAngle)
{
    QPointF origin(0, 0);

    // T_TOP — upward (angle ≈ -90°)
    QPointF top(0, -50);
    mYearWindow->calculateAzimuthAngle(origin, top);

    // T_RIGHT — rightward (angle ≈ 0°)
    QPointF right(50, 0);
    mYearWindow->calculateAzimuthAngle(origin, right);

    // T_BOTTOM — downward (angle ≈ 90°)
    QPointF bottom(0, 50);
    mYearWindow->calculateAzimuthAngle(origin, bottom);

    // T_LEFT — leftward (angle ≈ 180°)
    QPointF left(-50, 0);
    mYearWindow->calculateAzimuthAngle(origin, left);

    // T_NONE — distance < 10
    QPointF tiny(1, 1);
    mYearWindow->calculateAzimuthAngle(origin, tiny);
}

// ===== YearFrame tests =====

TEST_F(test_yearwindow, yearFrame_constructor)
{
    YearFrame *yf = new YearFrame();
    ASSERT_NE(yf, nullptr);
    delete yf;
}

TEST_F(test_yearwindow, yearFrame_setShowDate)
{
    YearFrame yf;
    yf.setShowDate(QDate(2021, 6, 15));
}

TEST_F(test_yearwindow, yearFrame_setLunarYearDate)
{
    YearFrame yf;
    yf.setLunarYearDate();
    yf.setLunarYearDate("农历辛丑年");
}

TEST_F(test_yearwindow, yearFrame_setDateHasScheduleSign)
{
    YearFrame yf;
    yf.setShowDate(QDate(2021, 6, 15));
    QSet<QDate> scheduleSet;
    scheduleSet << QDate(2021, 1, 1) << QDate(2021, 6, 15) << QDate(2021, 12, 31);
    yf.setDateHasScheduleSign(scheduleSet);
}

TEST_F(test_yearwindow, yearFrame_setTheMe)
{
    YearFrame yf;
    yf.setTheMe(0);
    yf.setTheMe(1);
    yf.setTheMe(2);
}

TEST_F(test_yearwindow, yearFrame_setSearchSchedule)
{
    YearFrame yf;
    yf.setShowDate(QDate(2021, 6, 15));
    QSet<QDate> searchSet;
    searchSet << QDate(2021, 1, 1) << QDate(2021, 6, 15);
    yf.setSearchSchedule(searchSet);
}

TEST_F(test_yearwindow, yearFrame_setViewFocus)
{
    YearFrame yf;
    yf.setShowDate(QDate(2021, 6, 15));
    // Valid index
    yf.setViewFocus(0);
    yf.setViewFocus(5);
    // Invalid indices
    yf.setViewFocus(-1);
    yf.setViewFocus(99);
}

TEST_F(test_yearwindow, yearFrame_getViewFocusIndex)
{
    YearFrame yf;
    // Default should be -1
    EXPECT_EQ(yf.getViewFocusIndex(), -1);
}

TEST_F(test_yearwindow, yearFrame_eventFilter)
{
    YearFrame yf;
    yf.setShowDate(QDate(2021, 6, 15));
    QEvent event(QEvent::FocusIn);
    yf.eventFilter(&yf, &event);
}
