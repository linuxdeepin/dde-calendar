// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_animationstackedwidget.h"

#include <QTest>
#include <QMouseEvent>

test_animationstackedwidget::test_animationstackedwidget()
{

}

test_animationstackedwidget::~test_animationstackedwidget()
{

}

//void AnimationStackedWidget::setDuration(int duration)
TEST_F(test_animationstackedwidget, setDuration)
{
    mAnimationStackedWidget->setDuration(400);
}

//void AnimationStackedWidget::animationFinished()
TEST_F(test_animationstackedwidget, animationfinished)
{
    mAnimationStackedWidget->animationFinished();
}

//void AnimationStackedWidget::setCurrent(int index)
TEST_F(test_animationstackedwidget, setCurrent)
{
    mAnimationStackedWidget->setCurrent(1);
}

//void AnimationStackedWidget::setPre()
TEST_F(test_animationstackedwidget, setPre)
{
    mAnimationStackedWidget->setPre();
}

//void AnimationStackedWidget::setNext()
TEST_F(test_animationstackedwidget, setNext)
{
    mAnimationStackedWidget->setNext();
}

//void AnimationStackedWidget::setCurrentWidget(int &index, int beginWidth)
TEST_F(test_animationstackedwidget, setCurrentWidget)
{
    int index = 1;
    mAnimationStackedWidget->setCurrentWidget(index, 8);
}

//void AnimationStackedWidget::setCurrentWidget — same index triggers signalIsFinished
TEST_F(test_animationstackedwidget, setCurrentWidget_sameIndex)
{
    int index = 0;
    mAnimationStackedWidget->setCurrentWidget(index, 8);
}

//int AnimationStackedWidget::getBeginValue()
TEST_F(test_animationstackedwidget, getBeginValue)
{
    mAnimationStackedWidget->getBeginValue();
}

//AnimationStackedWidget::MoveOrientation AnimationStackedWidget::getMoveOrientation(const int currIndex, const int nextIndex)
TEST_F(test_animationstackedwidget, getMoveOrientation)
{
    mAnimationStackedWidget->getMoveOrientation(0, 1);
    mAnimationStackedWidget->getMoveOrientation(1, 0);
}

//double AnimationStackedWidget::offset() const
TEST_F(test_animationstackedwidget, offset)
{
    mAnimationStackedWidget->offset();
}

//void AnimationStackedWidget::setOffset(double offset)
TEST_F(test_animationstackedwidget, setOffset)
{
    mAnimationStackedWidget->setOffset(2.0);
}

//void AnimationStackedWidget::mouseMoveEvent(QMouseEvent *e)
TEST_F(test_animationstackedwidget, mouseMoveEvent)
{
    QMouseEvent mouseEvent(QEvent::MouseMove, QPointF(10, 10), QPointF(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(mAnimationStackedWidget, &mouseEvent);
    SUCCEED();
}

//paintEvent via grab — exercises paintCurrentWidget & paintNextWidget
TEST_F(test_animationstackedwidget, animationtest)
{
    mAnimationStackedWidget->setCurrentIndex(0);
    mAnimationStackedWidget->setDuration(250);
    mAnimationStackedWidget->setCurrent(1);
    mAnimationStackedWidget->setFixedSize(500, 800);
    QPixmap pixmap(mAnimationStackedWidget->size());
    for (int i = 0; i < 10; ++i) {
        mAnimationStackedWidget->setOffset(i);
        pixmap = mAnimationStackedWidget->grab();
    }

    mAnimationStackedWidget->setCurrentIndex(1);
    mAnimationStackedWidget->setCurrent(0);
    for (int i = 0; i < 10; ++i) {
        mAnimationStackedWidget->setOffset(i);
        pixmap = mAnimationStackedWidget->grab();
    }
}

//TB orientation tests — exercises TB branch in getBeginValue, setPre, setNext, getMoveOrientation
TEST_F(test_animationstackedwidget, TB_setDuration)
{
    AnimationStackedWidget w(AnimationStackedWidget::TB);
    w.setDuration(300);
    EXPECT_TRUE(w.IsRunning() == false);
}

TEST_F(test_animationstackedwidget, TB_getBeginValue)
{
    AnimationStackedWidget w(AnimationStackedWidget::TB);
    w.setFixedSize(400, 600);
    w.addWidget(new QWidget(&w));
    w.addWidget(new QWidget(&w));
    w.getBeginValue();
}

TEST_F(test_animationstackedwidget, TB_setPre)
{
    AnimationStackedWidget w(AnimationStackedWidget::TB);
    w.setFixedSize(400, 600);
    w.addWidget(new QWidget(&w));
    w.addWidget(new QWidget(&w));
    w.setPre();
}

TEST_F(test_animationstackedwidget, TB_setNext)
{
    AnimationStackedWidget w(AnimationStackedWidget::TB);
    w.setFixedSize(400, 600);
    w.addWidget(new QWidget(&w));
    w.addWidget(new QWidget(&w));
    w.setNext();
}

TEST_F(test_animationstackedwidget, TB_getMoveOrientation)
{
    AnimationStackedWidget w(AnimationStackedWidget::TB);
    w.getMoveOrientation(0, 1);
    w.getMoveOrientation(1, 0);
}

TEST_F(test_animationstackedwidget, TB_animationTest)
{
    AnimationStackedWidget w(AnimationStackedWidget::TB);
    w.setFixedSize(400, 600);
    w.addWidget(new QWidget(&w));
    w.addWidget(new QWidget(&w));
    w.setCurrentIndex(0);
    w.setDuration(250);
    w.setCurrent(1);
    for (int i = 0; i < 10; ++i) {
        w.setOffset(i);
        w.grab();
    }
}

TEST_F(test_animationstackedwidget, TB_mouseMoveEvent)
{
    AnimationStackedWidget w(AnimationStackedWidget::TB);
    w.setFixedSize(400, 600);
    QMouseEvent tbMouseEvent(QEvent::MouseMove, QPointF(50, 50), QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &tbMouseEvent);
    SUCCEED();
}
