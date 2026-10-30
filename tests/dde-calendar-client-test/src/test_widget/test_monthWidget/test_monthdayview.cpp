// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthdayview.h"
#include "../third-party_stub/stub.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QPainter>

static int monthdayview_stub_state = 0;

static int monthdayview_stub_getTouchState()
{
    return monthdayview_stub_state;
}

// ============================ test_monthdayview ============================

test_monthdayview::test_monthdayview()
{
    mMonthDayView = new CMonthDayView();
}

test_monthdayview::~test_monthdayview()
{
    delete mMonthDayView;
    mMonthDayView = nullptr;
}

void test_monthdayview::SetUp()
{
}

void test_monthdayview::TearDown()
{
}

// CMonthDayView::setSelectDate(const QDate &date)
TEST_F(test_monthdayview, setSelectDate)
{
    // Arrange
    QDate date = QDate::currentDate();

    // Act
    mMonthDayView->setSelectDate(date);

    // Assert
    SUCCEED();
}

// CMonthDayView::setTheMe(int type)
TEST_F(test_monthdayview, setTheMe)
{
    // Arrange
    // Act
    mMonthDayView->setTheMe(0);
    mMonthDayView->setTheMe(1);
    mMonthDayView->setTheMe(2);

    // Assert
    SUCCEED();
}

// CMonthDayView::setSearchflag(bool flag)
TEST_F(test_monthdayview, setSearchflag)
{
    // Arrange
    // Act
    mMonthDayView->setSearchflag(true);
    mMonthDayView->setSearchflag(false);

    // Assert
    SUCCEED();
}

// CMonthDayView::wheelEvent(QWheelEvent *e)
TEST_F(test_monthdayview, wheelEvent)
{
    // Arrange
    QWheelEvent e(QPointF(1, 1), QPointF(1, 1), QPoint(), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);

    // Act
    mMonthDayView->wheelEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthDayView::event(QEvent *e) — high priority
TEST_F(test_monthdayview, event_01)
{
    // Arrange
    Stub stub;
    monthdayview_stub_state = 1;
    stub.set(ADDR(touchGestureOperation, getTouchState), monthdayview_stub_getTouchState);
    QMouseEvent e(QEvent::MouseButtonPress, QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    bool result = mMonthDayView->event(&e);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(test_monthdayview, event_02)
{
    // Arrange — non-mouse event
    QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);

    // Act
    mMonthDayView->event(&e);

    // Assert
    SUCCEED();
}

TEST_F(test_monthdayview, event_03)
{
    // Arrange — slide state via stub
    Stub stub;
    monthdayview_stub_state = 2; // T_SLIDE
    stub.set(ADDR(touchGestureOperation, getTouchState), monthdayview_stub_getTouchState);
    QMouseEvent e(QEvent::MouseMove, QPointF(5, 5), Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    // Act
    mMonthDayView->event(&e);

    // Assert
    SUCCEED();
}

// ============================ test_monthrect ============================

test_monthrect::test_monthrect()
{
    mMonthRect = new CMonthRect();
}

test_monthrect::~test_monthrect()
{
    delete mMonthRect;
    mMonthRect = nullptr;
}

void test_monthrect::SetUp()
{
}

void test_monthrect::TearDown()
{
}

// CMonthRect::setDate / getDate
TEST_F(test_monthrect, setDate_getDate)
{
    // Arrange
    QDate date(2024, 6, 15);

    // Act
    mMonthRect->setDate(date);

    // Assert
    EXPECT_EQ(mMonthRect->getDate(), date);
}

// CMonthRect::setRect(x,y,w,h) / rect
TEST_F(test_monthrect, setRect_xywh)
{
    // Arrange
    // Act
    mMonthRect->setRect(1.0, 2.0, 3.0, 4.0);

    // Assert
    EXPECT_EQ(mMonthRect->rect(), QRectF(1.0, 2.0, 3.0, 4.0));
}

// CMonthRect::setRect(const QRectF &rect) / rect
TEST_F(test_monthrect, setRect_qrectf)
{
    // Arrange
    QRectF rect(10, 20, 30, 40);

    // Act
    mMonthRect->setRect(rect);

    // Assert
    EXPECT_EQ(mMonthRect->rect(), rect);
}

// CMonthRect::setDevicePixelRatio
TEST_F(test_monthrect, setDevicePixelRatio)
{
    // Arrange
    // Act
    CMonthRect::setDevicePixelRatio(2.5);

    // Assert
    SUCCEED();
}

// CMonthRect::setTheMe — light
TEST_F(test_monthrect, setTheMe_light)
{
    // Arrange
    // Act
    CMonthRect::setTheMe(0);

    // Assert
    EXPECT_TRUE(CMonthRect::m_defaultTextColor.isValid());
}

// CMonthRect::setTheMe — dark
TEST_F(test_monthrect, setTheMe_dark)
{
    // Arrange
    // Act
    CMonthRect::setTheMe(2);

    // Assert
    EXPECT_TRUE(CMonthRect::m_defaultTextColor.isValid());
}

// CMonthRect::setSelectRect / getSelectRect
TEST_F(test_monthrect, setSelectRect_getSelectRect)
{
    // Arrange
    // Act
    CMonthRect::setSelectRect(mMonthRect);

    // Assert
    EXPECT_EQ(CMonthRect::getSelectRect(), mMonthRect);
}

// CMonthRect::paintItem — non-selected
TEST_F(test_monthrect, paintItem_normal)
{
    // Arrange
    mMonthRect->m_Date = QDate(2024, 6, 15);
    QImage img(100, 50, QImage::Format_ARGB32);
    QPainter painter(&img);

    // Act
    mMonthRect->paintItem(&painter, QRectF(0, 0, 100, 50), false);

    // Assert
    SUCCEED();
}

// CMonthRect::paintItem — selected (drawFocus=true)
TEST_F(test_monthrect, paintItem_focus)
{
    // Arrange
    mMonthRect->m_Date = QDate::currentDate();
    CMonthRect::setSelectRect(mMonthRect);
    QImage img(100, 50, QImage::Format_ARGB32);
    QPainter painter(&img);

    // Act
    mMonthRect->paintItem(&painter, QRectF(0, 0, 100, 50), true);

    // Assert
    SUCCEED();
}

// ============================ test_monthwidget ============================

test_monthwidget::test_monthwidget()
{
    mMonthWidget = new CMonthWidget();
}

test_monthwidget::~test_monthwidget()
{
    delete mMonthWidget;
    mMonthWidget = nullptr;
}

void test_monthwidget::SetUp()
{
}

void test_monthwidget::TearDown()
{
}

// CMonthWidget::setDate(const QDate date[12])
TEST_F(test_monthwidget, setDate)
{
    // Arrange
    QDate days[12];
    QDate current = QDate::currentDate();
    for (int i = 0; i < 12; ++i) {
        days[i] = current.addMonths(i);
    }

    // Act
    mMonthWidget->setDate(days);

    // Assert
    SUCCEED();
}

// CMonthWidget::updateSize — mid priority
TEST_F(test_monthwidget, updateSize)
{
    // Arrange
    mMonthWidget->resize(200, 100);

    // Act
    mMonthWidget->updateSize();

    // Assert
    SUCCEED();
}

// CMonthWidget::getMousePosItem
TEST_F(test_monthwidget, getMousePosItem)
{
    // Arrange
    mMonthWidget->resize(200, 100);

    // Act
    int idx = mMonthWidget->getMousePosItem(QPointF(10, 10));

    // Assert
    SUCCEED();
}

// CMonthWidget::updateShowDate
TEST_F(test_monthwidget, updateShowDate)
{
    // Arrange
    // Act
    mMonthWidget->updateShowDate(QDate::currentDate());

    // Assert
    SUCCEED();
}

// CMonthWidget::mousePress(const QPoint &point)
TEST_F(test_monthwidget, mousePress)
{
    // Arrange
    mMonthWidget->resize(200, 100);

    // Act
    mMonthWidget->mousePress(QPoint(10, 10));

    // Assert
    SUCCEED();
}

// CMonthWidget::keyPressEvent — Left
TEST_F(test_monthwidget, keyPressEvent_Left)
{
    // Arrange
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);

    // Act
    mMonthWidget->keyPressEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::keyPressEvent — Right
TEST_F(test_monthwidget, keyPressEvent_Right)
{
    // Arrange
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);

    // Act
    mMonthWidget->keyPressEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::keyPressEvent — Up
TEST_F(test_monthwidget, keyPressEvent_Up)
{
    // Arrange
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);

    // Act
    mMonthWidget->keyPressEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::keyPressEvent — Down
TEST_F(test_monthwidget, keyPressEvent_Down)
{
    // Arrange
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);

    // Act
    mMonthWidget->keyPressEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::focusInEvent — TabFocusReason
TEST_F(test_monthwidget, focusInEvent_Tab)
{
    // Arrange
    QFocusEvent e(QEvent::FocusIn, Qt::TabFocusReason);

    // Act
    mMonthWidget->focusInEvent(&e);

    // Assert
    EXPECT_TRUE(mMonthWidget->m_isFocus);
}

// CMonthWidget::focusInEvent — other reason
TEST_F(test_monthwidget, focusInEvent_Other)
{
    // Arrange
    QFocusEvent e(QEvent::FocusIn, Qt::OtherFocusReason);

    // Act
    mMonthWidget->focusInEvent(&e);

    // Assert
    EXPECT_FALSE(mMonthWidget->m_isFocus);
}

// CMonthWidget::focusOutEvent
TEST_F(test_monthwidget, focusOutEvent)
{
    // Arrange
    QFocusEvent e(QEvent::FocusOut, Qt::OtherFocusReason);

    // Act
    mMonthWidget->focusOutEvent(&e);

    // Assert
    EXPECT_FALSE(mMonthWidget->m_isFocus);
}

// CMonthWidget::mousePressEvent
TEST_F(test_monthwidget, mousePressEvent)
{
    // Arrange
    QMouseEvent e(QEvent::MouseButtonPress, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    mMonthWidget->mousePressEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::mouseReleaseEvent
TEST_F(test_monthwidget, mouseReleaseEvent)
{
    // Arrange
    QMouseEvent e(QEvent::MouseButtonRelease, QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    // Act
    mMonthWidget->mouseReleaseEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::mouseMoveEvent
TEST_F(test_monthwidget, mouseMoveEvent)
{
    // Arrange
    QMouseEvent e(QEvent::MouseMove, QPointF(10, 10), Qt::NoButton, Qt::NoButton, Qt::NoModifier);

    // Act
    mMonthWidget->mouseMoveEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::paintEvent
TEST_F(test_monthwidget, paintEvent)
{
    // Arrange
    QPaintEvent e(QRect(0, 0, 1, 1));

    // Act
    mMonthWidget->paintEvent(&e);

    // Assert
    SUCCEED();
}

// CMonthWidget::resizeEvent
TEST_F(test_monthwidget, resizeEvent)
{
    // Arrange
    QResizeEvent e(QSize(200, 100), QSize(100, 50));

    // Act
    mMonthWidget->resizeEvent(&e);

    // Assert
    SUCCEED();
}
