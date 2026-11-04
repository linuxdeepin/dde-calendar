// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_weekview.h"

#include <QTest>
#include <QWheelEvent>
#include <QEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QResizeEvent>

// ===================== CWeekView tests =====================

// void CWeekView::setSelectDate(const QDate date)
TEST_F(test_weekview, setSelectDate)
{
    mWeekView->setSelectDate(QDate::currentDate());
    SUCCEED();
}

// void CWeekView::setCurrent(const QDateTime &dateTime)
TEST_F(test_weekview, setCurrent)
{
    mWeekView->setCurrent(QDateTime::currentDateTime());
    SUCCEED();
}

// void CWeekView::setTheMe(int type)
TEST_F(test_weekview, setTheMe)
{
    mWeekView->setTheMe(0);
    mWeekView->setTheMe(1);
    mWeekView->setTheMe(2);
}

// bool CWeekView::event(QEvent *e)
TEST_F(test_weekview, event)
{
    // Send a non-touch event to exercise the else branch (DWidget::event)
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(mWeekView, &enterEvent);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(mWeekView, &leaveEvent);
}

// void CWeekView::wheelEvent(QWheelEvent *event)
TEST_F(test_weekview, wheelEvent)
{
    mWeekView->setFixedSize(600, 50);
    // Vertical wheel up (y > 0) → previous week
    QPoint angleDelta1(0, 120);
    QWheelEvent wheelEvent1(QPointF(300, 25), QPointF(300, 25), QPoint(), angleDelta1,
                            Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(mWeekView, &wheelEvent1);

    // Vertical wheel down (y < 0) → next week
    QPoint angleDelta2(0, -120);
    QWheelEvent wheelEvent2(QPointF(300, 25), QPointF(300, 25), QPoint(), angleDelta2,
                            Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(mWeekView, &wheelEvent2);
}

// ~CWeekView() — destructor coverage
TEST_F(test_weekview, destructor)
{
    CWeekView *view = new CWeekView(&testGetWeekNumOfYear);
    delete view;
}

// ===================== CWeekNumWidget tests =====================

// void CWeekNumWidget::setSelectDate(const QDate date)
TEST_F(test_weeknumwidget, setSelectDate)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    SUCCEED();
}

// void CWeekNumWidget::setCurrent(const QDateTime &dateTime)
TEST_F(test_weeknumwidget, setCurrent)
{
    mWeekNumWidget->setCurrent(QDateTime::currentDateTime());
    SUCCEED();
}

// void CWeekNumWidget::setTheMe(int type)
TEST_F(test_weeknumwidget, setTheMe)
{
    mWeekNumWidget->setTheMe(0);
    mWeekNumWidget->setTheMe(1);
    mWeekNumWidget->setTheMe(2);
}

// void CWeekNumWidget::setSelectedCell(int index) — private, accessible via -fno-access-control
TEST_F(test_weeknumwidget, setSelectedCell)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    // Select a different cell
    mWeekNumWidget->setSelectedCell(3);
    // Select the same cell again (early return branch)
    mWeekNumWidget->setSelectedCell(3);
    // Select another cell
    mWeekNumWidget->setSelectedCell(5);
}

// void CWeekNumWidget::updateDate() — private, accessible via -fno-access-control
TEST_F(test_weeknumwidget, updateDate)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    mWeekNumWidget->updateDate();
    SUCCEED();
}

// void CWeekNumWidget::cellClicked(QWidget *cell)
TEST_F(test_weeknumwidget, cellClicked)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    mWeekNumWidget->setFixedSize(600, 50);
    // cellClicked with a valid cell (the first cell in m_cellList)
    // Access m_cellList via -fno-access-control; it's private
    // Use the public slot directly with nullptr first (invalid branch)
    mWeekNumWidget->cellClicked(nullptr);
}

// bool CWeekNumWidget::event(QEvent *e)
TEST_F(test_weeknumwidget, event)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    mWeekNumWidget->setFixedSize(600, 50);
    mWeekNumWidget->setFocus();

    // Key_Left when focusWidget() == this
    QKeyEvent leftEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(mWeekNumWidget, &leftEvent);

    // Key_Right when focusWidget() == this
    QKeyEvent rightEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(mWeekNumWidget, &rightEvent);

    // Non-key event (exercises else branch → DWidget::event)
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(mWeekNumWidget, &enterEvent);
}

// bool CWeekNumWidget::eventFilter(QObject *o, QEvent *e)
TEST_F(test_weeknumwidget, eventFilter)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    mWeekNumWidget->setFixedSize(600, 50);

    // Trigger Paint events via render which exercises eventFilter
    QPixmap pixmap(mWeekNumWidget->size());
    mWeekNumWidget->render(&pixmap);
}

// void CWeekNumWidget::focusInEvent(QFocusEvent *event)
TEST_F(test_weeknumwidget, focusInEvent)
{
    mWeekNumWidget->setFocus();
    // Trigger focus in
    QFocusEvent focusIn(QEvent::FocusIn, Qt::MouseFocusReason);
    QApplication::sendEvent(mWeekNumWidget, &focusIn);
}

// void CWeekNumWidget::focusOutEvent(QFocusEvent *event)
TEST_F(test_weeknumwidget, focusOutEvent)
{
    QFocusEvent focusOut(QEvent::FocusOut);
    QApplication::sendEvent(mWeekNumWidget, &focusOut);
}

// void CWeekNumWidget::mousePressEvent(QMouseEvent *event)
TEST_F(test_weeknumwidget, mousePressEvent)
{
    mWeekNumWidget->setFixedSize(600, 50);
    QMouseEvent mousePress(QEvent::MouseButtonPress, QPointF(100, 10),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mWeekNumWidget, &mousePress);
}

// void CWeekNumWidget::paintCell(QWidget *cell) — private, triggered via Paint event in eventFilter
TEST_F(test_weeknumwidget, paintCell)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    mWeekNumWidget->setFixedSize(600, 50);
    QPixmap pixmap(mWeekNumWidget->size());
    mWeekNumWidget->render(&pixmap);
}

// void CWeekNumWidget::resizeEvent(QResizeEvent *event)
TEST_F(test_weeknumwidget, resizeEvent)
{
    mWeekNumWidget->setSelectDate(QDate::currentDate());
    // Large size — all cells visible
    mWeekNumWidget->setFixedSize(800, 50);
    // Small size — some cells hidden
    mWeekNumWidget->setFixedSize(100, 50);
}

// ~CWeekNumWidget() — destructor coverage
TEST_F(test_weeknumwidget, destructor)
{
    CWeekNumWidget *widget = new CWeekNumWidget(&testGetWeekNumOfYear, nullptr);
    delete widget;
}
