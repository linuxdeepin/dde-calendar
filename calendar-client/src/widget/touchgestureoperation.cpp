// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "touchgestureoperation.h"
#include "commondef.h"

#include <QEvent>
#include <QGestureEvent>
#include <QtMath>
#include <QMouseEvent>

touchGestureOperation::touchGestureOperation(QWidget *parent)
    : m_parentWidget(parent)
{
    qCDebug(ClientLogger) << "touchGestureOperation constructed";
    if (m_parentWidget) {
        m_parentWidget->setAttribute(Qt::WA_AcceptTouchEvents);
        //截获相应的gesture手势
        m_parentWidget->grabGesture(Qt::TapGesture);
        m_parentWidget->grabGesture(Qt::TapAndHoldGesture);
        m_parentWidget->grabGesture(Qt::PanGesture);
        qCDebug(ClientLogger) << "Touch gestures grabbed for widget";
    }
}

bool touchGestureOperation::event(QEvent *e)
{
    bool _result {false};
    if (e->type() == QEvent::Gesture) {
        //手势触发
        // qCDebug(ClientLogger) << "Gesture event received";
        _result = gestureEvent(dynamic_cast<QGestureEvent *>(e));
    }
    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
    if (e->type() == QEvent::MouseButtonPress && mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
        //触摸点击转换鼠标点击事件
        // qCDebug(ClientLogger) << "Touch press detected at:" << mouseEvent->pos();
        m_mouseState = M_PRESS;
        m_beginTouchPoint = mouseEvent->pos();
        _result = true;
    }
    if (e->type() == QEvent::MouseMove && mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
        //触摸移动转换鼠标移动事件
        //如果移动距离大与5则为触摸移动状态
        QPointF currentPoint = mouseEvent->pos();
        if (QLineF(m_beginTouchPoint, currentPoint).length() > 5) {
            m_mouseState = M_MOVE;
            //如果为单指点击状态则转换为滑动状态
            switch (m_touchState) {
            case T_SINGLE_CLICK: {
                // qCDebug(ClientLogger) << "Touch state changed from single click to slide";
                m_touchState = T_SLIDE;
                break;
            }
            case T_SLIDE: {
                //计算滑动方向和距离
                calculateAzimuthAngle(m_beginTouchPoint, currentPoint);
                //如果移动距离大于15则更新
                if (m_movelenght > 15) {
                    // qCDebug(ClientLogger) << "Touch slide significant movement detected, direction:" 
                    //                      << m_movingDir << "distance:" << m_movelenght;
                    m_update = true;
                    m_beginTouchPoint = currentPoint;
                }
                break;
            }
            default:
                break;
            }
        }
        _result = true;
    }
    if (e->type() == QEvent::MouseButtonRelease && mouseEvent->source() == Qt::MouseEventSynthesizedByQt) {
        // qCDebug(ClientLogger) << "Touch release detected at:" << mouseEvent->pos();
        m_mouseState = M_NONE;
        _result = true;
    }
    return _result;
}

bool touchGestureOperation::isUpdate() const
{
    return m_update;
}

void touchGestureOperation::setUpdate(bool b)
{
    // qCDebug(ClientLogger) << "Setting update state to:" << b;
    m_update = b;
}

touchGestureOperation::TouchState touchGestureOperation::getTouchState() const
{
    // qCDebug(ClientLogger) << "Getting touch state:" << m_touchState;
    return m_touchState;
}

touchGestureOperation::TouchMovingDirection touchGestureOperation::getMovingDir() const
{
    // qCDebug(ClientLogger) << "Getting moving direction:" << m_movingDir;
    return m_movingDir;
}

touchGestureOperation::TouchMovingDirection touchGestureOperation::getTouchMovingDir(QPointF &startPoint, QPointF &stopPoint, qreal &movingLine)
{
    qCDebug(ClientLogger) << "Getting touch moving direction";
    TouchMovingDirection _result {T_MOVE_NONE};
    qreal angle = 0.0000;
    qreal dx = stopPoint.rx() - startPoint.rx();
    qreal dy = stopPoint.ry() - startPoint.ry();
    //计算方位角
    angle = qAtan2(dy, dx) * 180 / M_PI;
    qreal line = qSqrt(dx * dx + dy * dy);
    //如果移动距离大于10则有效
    if (line > 10) {
        qCDebug(ClientLogger) << "Getting touch moving direction, angle:" << angle;
        if ((angle <= -45) && (angle >= -135)) {
            _result = TouchMovingDirection::T_TOP;
        } else if ((angle > -45) && (angle < 45)) {
            _result = TouchMovingDirection::T_RIGHT;
        } else if ((angle >= 45) && (angle <= 135)) {
            _result = TouchMovingDirection::T_BOTTOM;
        } else {
            _result = TouchMovingDirection::T_LEFT;
        }
    }
    movingLine = line;
    return _result;
}

bool touchGestureOperation::gestureEvent(QGestureEvent *event)
{
    // qCDebug(ClientLogger) << "Gesture event received";
    if (QGesture *tap = event->gesture(Qt::TapGesture))
        tapGestureTriggered(dynamic_cast<QTapGesture *>(tap));
    if (QGesture *pan = event->gesture(Qt::PanGesture))
        panTriggered(dynamic_cast<QPanGesture *>(pan));
    return true;
}

void touchGestureOperation::tapGestureTriggered(QTapGesture *tap)
{
    qCDebug(ClientLogger) << "Tap gesture triggered with state:" << tap->state();
    switch (tap->state()) {
    case Qt::NoGesture: {
        break;
    }
    case Qt::GestureStarted: {
        m_beginTouchTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_touchState = T_SINGLE_CLICK;
        qCDebug(ClientLogger) << "Tap gesture started, setting touch state to single click";
        break;
    }
    case Qt::GestureUpdated: {
        qCDebug(ClientLogger) << "Tap gesture updated";
        break;
    }
    case Qt::GestureFinished: {
        qCDebug(ClientLogger) << "Tap gesture finished";
        break;
    }
    default: {
        //GestureCanceled
        qCDebug(ClientLogger) << "Tap gesture canceled";
    }
    }
}

void touchGestureOperation::panTriggered(QPanGesture *pan)
{
    qCDebug(ClientLogger) << "Pan gesture triggered with state:" << pan->state();
    switch (pan->state()) {
    case Qt::NoGesture: {
        qCDebug(ClientLogger) << "Pan gesture no gesture";
        break;
    }
    case Qt::GestureStarted: {
        qCDebug(ClientLogger) << "Pan gesture started";
        break;
    }
    case Qt::GestureUpdated: {
        m_touchState = T_SLIDE;
        qCDebug(ClientLogger) << "Pan gesture updated, setting touch state to slide";
        break;
    }
    case Qt::GestureFinished: {
        qCDebug(ClientLogger) << "Pan gesture finished";
        break;
    }
    default:
        //GestureCanceled
        qCDebug(ClientLogger) << "Pan gesture canceled";
        break;
    }
}

void touchGestureOperation::calculateAzimuthAngle(QPointF &startPoint, QPointF &stopPoint)
{
    // qCDebug(ClientLogger) << "Calculating azimuth angle";
    m_movingDir = getTouchMovingDir(startPoint, stopPoint, m_movelenght);
}
