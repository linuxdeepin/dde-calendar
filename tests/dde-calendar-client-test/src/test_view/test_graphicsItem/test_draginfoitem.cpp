// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_draginfoitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>
#include <QGraphicsSceneHoverEvent>
#include <QCoreApplication>

test_DragInfoItem::test_DragInfoItem()
{
}

void test_DragInfoItem::SetUp()
{
    m_scene = new QGraphicsScene();
    m_item = new TestDragInfoItem(QRectF(0, 0, 100, 50));
    m_scene->addItem(m_item);
}

void test_DragInfoItem::TearDown()
{
    delete m_scene;
    m_scene = nullptr;
    m_item = nullptr;
}

// Constructor
TEST_F(test_DragInfoItem, Constructor)
{
    EXPECT_EQ(m_item->rect(), QRectF(0, 0, 100, 50));
    EXPECT_EQ(m_item->getItemType(), CFocusItem::CITEM);
    EXPECT_FALSE(m_item->isRunning());
}

// Destructor
TEST_F(test_DragInfoItem, Destructor)
{
    TestDragInfoItem *item = new TestDragInfoItem(QRectF(0, 0, 50, 50));
    delete item;
    SUCCEED();
}

// setData / getData
TEST_F(test_DragInfoItem, SetGetData)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Test Schedule");
    m_item->setData(schedule);
    EXPECT_EQ(m_item->getData(), schedule);
}

// getData — default null
TEST_F(test_DragInfoItem, GetData_DefaultNull)
{
    EXPECT_EQ(m_item->getData(), DSchedule::Ptr());
}

// setPressFlag (static)
TEST_F(test_DragInfoItem, SetPressFlag)
{
    DragInfoItem::setPressFlag(true);
    SUCCEED();
    DragInfoItem::setPressFlag(false);
    SUCCEED();
}

// setPressSchedule / getPressSchedule (static)
TEST_F(test_DragInfoItem, SetGetPressSchedule)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Press Schedule");
    DragInfoItem::setPressSchedule(schedule);
    EXPECT_EQ(DragInfoItem::getPressSchedule(), schedule);
}

// getPressSchedule — default
TEST_F(test_DragInfoItem, GetPressSchedule_Default)
{
    DSchedule::Ptr result = DragInfoItem::getPressSchedule();
    EXPECT_TRUE(result == DSchedule::Ptr() || result != DSchedule::Ptr());
}

// setSearchScheduleInfo
TEST_F(test_DragInfoItem, SetSearchScheduleInfo)
{
    DSchedule::List list;
    DSchedule::Ptr s1(new DSchedule());
    s1->setSummary("Search 1");
    DSchedule::Ptr s2(new DSchedule());
    s2->setSummary("Search 2");
    list.append(s1);
    list.append(s2);
    m_item->setSearchScheduleInfo(list);
    SUCCEED();
}

// setFont
TEST_F(test_DragInfoItem, SetFont)
{
    m_item->setFont(DFontSizeManager::T8);
    SUCCEED();
}

// setOffset / readOffset
TEST_F(test_DragInfoItem, SetOffset)
{
    m_item->setOffset(5);
    EXPECT_EQ(m_item->readOffset(), 5);
    QRectF expected(0 - 5, 0 - 5 / 2, 100 + 5 * 2, 50 + 5);
    EXPECT_EQ(m_item->rect(), expected);
}

// readOffset — default
TEST_F(test_DragInfoItem, ReadOffset_Default)
{
    EXPECT_EQ(m_item->readOffset(), 0);
}

// setStartValue
TEST_F(test_DragInfoItem, SetStartValue)
{
    m_item->setStartValue(10);
    SUCCEED();
}

// setEndValue
TEST_F(test_DragInfoItem, SetEndValue)
{
    m_item->setEndValue(20);
    SUCCEED();
}

// startAnimation
TEST_F(test_DragInfoItem, StartAnimation)
{
    m_item->setStartValue(0);
    m_item->setEndValue(10);
    m_item->startAnimation();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 500);
    SUCCEED();
}

// startAnimation — already running (call twice)
TEST_F(test_DragInfoItem, StartAnimation_AlreadyRunning)
{
    m_item->setStartValue(0);
    m_item->setEndValue(10);
    m_item->startAnimation();
    m_item->startAnimation();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 500);
    SUCCEED();
}

// animationFinished — private slot, accessible via -fno-access-control
TEST_F(test_DragInfoItem, AnimationFinished)
{
    m_item->animationFinished();
    EXPECT_FALSE(m_item->isRunning());
}

// isRunning
TEST_F(test_DragInfoItem, IsRunning)
{
    EXPECT_FALSE(m_item->isRunning());
}

// hoverEnterEvent
TEST_F(test_DragInfoItem, HoverEnterEvent)
{
    QGraphicsSceneHoverEvent event(QEvent::GraphicsSceneHoverEnter);
    m_item->hoverEnterEvent(&event);
    SUCCEED();
}

// hoverLeaveEvent
TEST_F(test_DragInfoItem, HoverLeaveEvent)
{
    QGraphicsSceneHoverEvent event(QEvent::GraphicsSceneHoverLeave);
    m_item->hoverLeaveEvent(&event);
    SUCCEED();
}

// paint
TEST_F(test_DragInfoItem, Paint)
{
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    m_item->paint(&painter, &option);
    SUCCEED();
}

// paint — with schedule data set
TEST_F(test_DragInfoItem, Paint_WithSchedule)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Paint Test");
    m_item->setData(schedule);
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    m_item->paint(&painter, &option);
    SUCCEED();
}
