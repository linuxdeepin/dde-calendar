// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_touchgestureoperation.h"

test_touchgestureoperation::test_touchgestureoperation()
{
    mTouchGestureOperation = new touchGestureOperation();
}

test_touchgestureoperation::~test_touchgestureoperation()
{
    delete mTouchGestureOperation;
    mTouchGestureOperation = nullptr;
}

// Constructor without parent
TEST_F(test_touchgestureoperation, ConstructorNoParent)
{
    touchGestureOperation *tgo = new touchGestureOperation();
    delete tgo;
}

// Constructor with parent widget
TEST_F(test_touchgestureoperation, ConstructorWithParent)
{
    QWidget parent;
    touchGestureOperation *tgo = new touchGestureOperation(&parent);
    delete tgo;
}

//void touchGestureOperation::setUpdate(bool b)
TEST_F(test_touchgestureoperation, setUpdate)
{
    mTouchGestureOperation->setUpdate(true);
}

//bool touchGestureOperation::isUpdate() const
TEST_F(test_touchgestureoperation, isUpdate)
{
    mTouchGestureOperation->setUpdate(false);
    bool res = mTouchGestureOperation->isUpdate();
    EXPECT_FALSE(res);
    mTouchGestureOperation->setUpdate(true);
    res = mTouchGestureOperation->isUpdate();
    EXPECT_TRUE(res);
}

//touchGestureOperation::TouchState touchGestureOperation::getTouchState() const
TEST_F(test_touchgestureoperation, getTouchState)
{
    mTouchGestureOperation->getTouchState();
}

//touchGestureOperation::TouchMovingDirection touchGestureOperation::getMovingDir() const
TEST_F(test_touchgestureoperation, getMovingDir)
{
    mTouchGestureOperation->getMovingDir();
}

//touchGestureOperation::TouchMovingDirection touchGestureOperation::getTouchMovingDir(QPointF &startPoint, QPointF &stopPoint, qreal &movingLine)
TEST_F(test_touchgestureoperation, getTouchMovingDir)
{
    QPointF point1;
    QPointF point2;
    qreal num = 2;
    mTouchGestureOperation->getTouchMovingDir(point1, point2, num);
}

// Test getTouchMovingDir with directional movement
TEST_F(test_touchgestureoperation, getTouchMovingDirDirectional)
{
    QPointF start(0, 0);
    QPointF stopRight(20, 0);
    qreal line = 0;
    auto dir = mTouchGestureOperation->getTouchMovingDir(start, stopRight, line);
    EXPECT_EQ(touchGestureOperation::T_RIGHT, dir);

    QPointF stopLeft(-20, 0);
    dir = mTouchGestureOperation->getTouchMovingDir(start, stopLeft, line);
    EXPECT_EQ(touchGestureOperation::T_LEFT, dir);

    QPointF stopTop(0, -20);
    dir = mTouchGestureOperation->getTouchMovingDir(start, stopTop, line);
    EXPECT_EQ(touchGestureOperation::T_TOP, dir);

    QPointF stopBottom(0, 20);
    dir = mTouchGestureOperation->getTouchMovingDir(start, stopBottom, line);
    EXPECT_EQ(touchGestureOperation::T_BOTTOM, dir);
}

//void touchGestureOperation::calculateAzimuthAngle(QPointF &startPoint, QPointF &stopPoint)
TEST_F(test_touchgestureoperation, calculateAzimuthAngle)
{
    QPointF point1;
    QPointF point2;
    mTouchGestureOperation->calculateAzimuthAngle(point1, point2);
}

// Test calculateAzimuthAngle with actual movement to set m_movingDir
TEST_F(test_touchgestureoperation, calculateAzimuthAngleWithMovement)
{
    QPointF start(0, 0);
    QPointF stop(20, 0);
    mTouchGestureOperation->calculateAzimuthAngle(start, stop);
    EXPECT_EQ(touchGestureOperation::T_RIGHT, mTouchGestureOperation->getMovingDir());
}

//bool touchGestureOperation::event(QEvent *e)
TEST_F(test_touchgestureoperation, eventGesture)
{
    QTapGesture *tap = new QTapGesture;
    QPanGesture *pan = new QPanGesture;
    QList<QGesture *> gestures;
    gestures << tap << pan;
    QGestureEvent gEvent(gestures);
    mTouchGestureOperation->event(&gEvent);
}

TEST_F(test_touchgestureoperation, eventMousePress)
{
    QPointF local(10, 10);
    QPointF global(10, 10);
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, local, global,
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mTouchGestureOperation->event(&mouseEvent);
    SUCCEED();
}

TEST_F(test_touchgestureoperation, eventMouseMove)
{
    // First press to set m_beginTouchPoint
    QPointF pressLocal(0, 0);
    QPointF pressGlobal(0, 0);
    QMouseEvent pressEvent(QEvent::MouseButtonPress, pressLocal, pressGlobal,
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mTouchGestureOperation->event(&pressEvent);

    // Move with small distance (<=5) - no state change
    QPointF moveLocalSmall(2, 2);
    QMouseEvent moveSmallEvent(QEvent::MouseMove, moveLocalSmall, moveLocalSmall,
                               Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mTouchGestureOperation->event(&moveSmallEvent);

    // Move with large distance (>5) - triggers state change
    QPointF moveLocalLarge(20, 0);
    QMouseEvent moveLargeEvent(QEvent::MouseMove, moveLocalLarge, moveLocalLarge,
                               Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mTouchGestureOperation->event(&moveLargeEvent);

    // Move again with large distance in slide state
    QPointF moveLocalLarge2(40, 0);
    QMouseEvent moveLargeEvent2(QEvent::MouseMove, moveLocalLarge2, moveLocalLarge2,
                                Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mTouchGestureOperation->event(&moveLargeEvent2);
    SUCCEED();
}

TEST_F(test_touchgestureoperation, eventMouseRelease)
{
    QPointF local(10, 10);
    QPointF global(10, 10);
    QMouseEvent mouseEvent(QEvent::MouseButtonRelease, local, global,
                           Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    mTouchGestureOperation->event(&mouseEvent);
    SUCCEED();
}

TEST_F(test_touchgestureoperation, eventOther)
{
    QEvent otherEvent(QEvent::None);
    bool result = mTouchGestureOperation->event(&otherEvent);
    EXPECT_FALSE(result);
}

//bool touchGestureOperation::gestureEvent(QGestureEvent *event)
TEST_F(test_touchgestureoperation, gestureEvent)
{
    QTapGesture *tap = new QTapGesture;
    QPanGesture *pan = new QPanGesture;
    QList<QGesture *> gestures;
    gestures << tap << pan;
    QGestureEvent gEvent(gestures);
    mTouchGestureOperation->gestureEvent(&gEvent);
}

//void touchGestureOperation::tapGestureTriggered(QTapGesture *tap)
TEST_F(test_touchgestureoperation, tapGestureTriggered)
{
    QTapGesture tap;
    mTouchGestureOperation->tapGestureTriggered(&tap);
}

//void touchGestureOperation::panTriggered(QPanGesture *pan)
TEST_F(test_touchgestureoperation, panTriggered)
{
    QPanGesture pan;
    mTouchGestureOperation->panTriggered(&pan);
}
