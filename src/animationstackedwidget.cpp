/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "animationstackedwidget.h"

#include <QPainter>

AnimationStackedWidget::AnimationStackedWidget(const AnimationOri ori, QWidget *parent)
    : QStackedWidget(parent),
      m_animationOri(ori)
    , animation(new QPropertyAnimation(this, QByteArray()))
{
    setDuration(1000);
    connect(animation
            , &QPropertyAnimation::valueChanged
            , this
            , &AnimationStackedWidget::valueChanged_slot);
    connect(animation
            , &QPropertyAnimation::finished
            , this
            , &AnimationStackedWidget::animationFinished);
}

AnimationStackedWidget::~AnimationStackedWidget()
{
    delete animation;
}

void AnimationStackedWidget::setDuration(int duration)
{
    this->duration = duration;
    animation->setEasingCurve(QEasingCurve::InOutQuad);
}


void AnimationStackedWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if ( isAnimation ) {
        QPainter paint(this);
        paintPrevious(paint, currentIndex());
        paintNext(paint, nextIndex);
    }
}

void AnimationStackedWidget::valueChanged_slot(QVariant value)
{
    currentValue = value;
    update();
}

void AnimationStackedWidget::animationFinished()
{
    isAnimation = false;
    widget(currentIndex())->show();
    setCurrentIndex(nextIndex);
    emit signalIsFinished();
}


void AnimationStackedWidget::setCurrent(int index)
{
    //如果正在动画，那么return
    if ( isAnimation ) {
        return;
    }

    //开始动画并设置间隔和开始、结束值
    QRect g = geometry();
    int value = 0;
    if (index < currentIndex()) {
        switch (m_animationOri) {
        case LR: {
            m_moveOri = LeftToRight;
            value = g.width();
        }
        break;
        case TB: {
            m_moveOri = TopToBottom;
            value = g.height();
        }
        break;
        }
    } else if (index > currentIndex()) {
        switch (m_animationOri) {
        case LR: {
            m_moveOri = RightToLeft;
            value = g.width();
        }
        break;
        case TB: {
            m_moveOri = BottomToTop;
            value = g.height();
        }
        break;
        }
    } else {
        emit signalIsFinished();
        return;
    }
    isAnimation = true;
    widgetCount = count();
    int c = currentIndex();
    nextIndex = index;

    //隐藏当前的widget
    widget(c)->hide();
    animation->setStartValue(value);
    animation->setEndValue(0);
    animation->setDuration(duration);
    animation->start();
}

void AnimationStackedWidget::setPre()
{
    //如果正在动画，那么return
    if ( isAnimation ) {
        return;
    }

    //开始动画并设置间隔和开始、结束值
    QRect g = geometry();
    int value = 0;
    int index = currentIndex();
    index = qAbs(index - 1);
    switch (m_animationOri) {
    case LR: {
        m_moveOri = LeftToRight;
        value = g.width();
    }
    break;
    case TB: {
        m_moveOri = TopToBottom;
        value = g.height();
    }
    break;
    }

    isAnimation = true;
    widgetCount = count();
    int c = currentIndex();
    nextIndex = index;

    //隐藏当前的widget
    widget(c)->hide();
    animation->setStartValue(value);
    animation->setEndValue(0);
    animation->setDuration(duration);
    animation->start();
}

void AnimationStackedWidget::setNext()
{
    //如果正在动画，那么return
    if ( isAnimation ) {
        return;
    }

    //开始动画并设置间隔和开始、结束值
    QRect g = geometry();
    int value = 0;
    int index = currentIndex();
    index = (index + 1) % 2;

    switch (m_animationOri) {
    case LR: {
        m_moveOri = RightToLeft;
        value = g.width();
    }
    break;
    case TB: {
        m_moveOri = BottomToTop;
        value = g.height();
    }
    break;
    }


    isAnimation = true;
    widgetCount = count();
    int c = currentIndex();
    nextIndex = index;

    //隐藏当前的widget
    widget(c)->hide();
    animation->setStartValue(value);
    animation->setEndValue(0);
    animation->setDuration(duration);
    animation->start();
}

void AnimationStackedWidget::paintPrevious(QPainter &paint, int currentIndex)
{
    //获得当前页面的Widget
    QWidget *w = widget(currentIndex);
    QPixmap pixmap(w->size());
    //将Widget的内容渲染到QPixmap对象中，即将Widget变成一张图片
    w->render(&pixmap);
    QRect r = w->geometry();
    //绘制当前的Widget
    double value = currentValue.toDouble();
    QRectF r1(0.0, 0.0, value, r.height());
    QRectF r2(r.width() - value, 0, value, r.height());
    switch (m_moveOri) {
    case LeftToRight:
        paint.drawPixmap(QRectF(r.width() - value, 0, value, r.height()),
                         pixmap,
                         QRectF(0.0, 0.0, value, r.height()));
        break;
    case RightToLeft:
        paint.drawPixmap(QRectF(0.0, 0.0, value, r.height()),
                         pixmap,
                         QRectF(r.width() - value, 0, value, r.height()));
        break;
    case TopToBottom:
        paint.drawPixmap(QRectF(0.0, r.height() - value, r.width(), value),
                         pixmap,
                         QRectF(0.0, 0.0, r.width(), value));
        break;
    case BottomToTop:
        paint.drawPixmap(QRectF(0.0, 0.0, r.width(), value),
                         pixmap,
                         QRectF( 0, r.height() - value, r.width(), value));
        break;
    }
}

void AnimationStackedWidget::paintNext(QPainter &paint, int nextIndex)
{
    QWidget *nextWidget = widget(nextIndex);
    QRect r = geometry();
    //这行代码不加会有bug，第一次切换的时候，QStackedWidget并没有为child分配大小
    nextWidget->resize(r.width(), r.height());
    QPixmap nextPixmap(nextWidget->size());
    nextWidget->render(&nextPixmap);
    double value = currentValue.toDouble();
    switch (m_moveOri) {
    case LeftToRight:
        paint.drawPixmap(QRectF(0.0, 0.0, r.width() - value, r.height()),
                         nextPixmap,
                         QRectF(value, 0.0, r.width() - value, r.height()));
        break;
    case RightToLeft:
        paint.drawPixmap(QRectF(value, 0.0, r.width() - value, r.height()),
                         nextPixmap,
                         QRectF(0.0, 0.0, r.width() - value, r.height()));
        break;
    case TopToBottom:
        paint.drawPixmap(QRectF(0.0, 0.0, r.width(), r.height() - value),
                         nextPixmap,
                         QRectF(0.0, value,  r.width(), r.height() - value));
        break;
    case BottomToTop:
        paint.drawPixmap(QRectF(0.0, value,  r.width(), r.height() - value),
                         nextPixmap,
                         QRectF(0.0, 0.0, r.width(), r.height() - value));
        break;
    }
}
