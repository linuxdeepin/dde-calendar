// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cscheduleitem.h"

#include <QPainter>
#include <QPixmap>
#include <QCoreApplication>

test_CScheduleItem::test_CScheduleItem()
{
}

void test_CScheduleItem::SetUp()
{
    m_scene = new QGraphicsScene();
    m_item = new CScheduleItem(QRectF(0, 0, 100, 50));
    m_scene->addItem(m_item);
}

void test_CScheduleItem::TearDown()
{
    delete m_scene;
    m_scene = nullptr;
    m_item = nullptr;
}

// Constructor
TEST_F(test_CScheduleItem, Constructor)
{
    EXPECT_EQ(m_item->rect(), QRectF(0, 0, 100, 50));
    EXPECT_EQ(m_item->getType(), 0);
    EXPECT_FALSE(m_item->getData());
}

// Constructor — with type
TEST_F(test_CScheduleItem, Constructor_WithType)
{
    CScheduleItem *item = new CScheduleItem(QRectF(0, 0, 80, 40), nullptr, 1);
    EXPECT_EQ(item->getType(), 1);
    delete item;
}

// Destructor
TEST_F(test_CScheduleItem, Destructor)
{
    CScheduleItem *item = new CScheduleItem(QRectF(0, 0, 50, 50));
    delete item;
    SUCCEED();
}

// setData / getData
TEST_F(test_CScheduleItem, SetGetData)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Test Schedule");
    QDate date(2026, 10, 28);
    m_item->setData(schedule, date, 3);
    EXPECT_EQ(m_item->getData(), schedule);
    EXPECT_EQ(m_item->m_totalNum, 3);
    EXPECT_EQ(m_item->getDate(), date);
}

// hasSelectSchedule — true
TEST_F(test_CScheduleItem, HasSelectSchedule_True)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Select Test");
    m_item->setData(schedule, QDate(2026, 10, 28), 1);
    EXPECT_TRUE(m_item->hasSelectSchedule(schedule));
}

// hasSelectSchedule — false
TEST_F(test_CScheduleItem, HasSelectSchedule_False)
{
    DSchedule::Ptr schedule1(new DSchedule());
    schedule1->setSummary("Schedule 1");
    m_item->setData(schedule1, QDate(2026, 10, 28), 1);

    DSchedule::Ptr schedule2(new DSchedule());
    schedule2->setSummary("Schedule 2");
    EXPECT_FALSE(m_item->hasSelectSchedule(schedule2));
}

// getType
TEST_F(test_CScheduleItem, GetType)
{
    EXPECT_EQ(m_item->getType(), 0);
}

// getData — default null
TEST_F(test_CScheduleItem, GetData_DefaultNull)
{
    EXPECT_FALSE(m_item->getData());
}

// splitText — empty string
TEST_F(test_CScheduleItem, SplitText_Empty)
{
    QFont font;
    QStringList listStr;
    QFontMetrics fontM(font);
    m_item->splitText(font, 50, 30, "", listStr, fontM);
    EXPECT_TRUE(listStr.isEmpty());
}

// splitText — normal width
TEST_F(test_CScheduleItem, SplitText_NormalWidth)
{
    QFont font;
    QStringList listStr;
    QFontMetrics fontM(font);
    m_item->splitText(font, 100, 30, "Hello World Test String", listStr, fontM);
    EXPECT_FALSE(listStr.isEmpty());
}

// splitText — small width (< 30)
TEST_F(test_CScheduleItem, SplitText_SmallWidth)
{
    QFont font;
    QStringList listStr;
    QFontMetrics fontM(font);
    m_item->splitText(font, 20, 30, "ABC", listStr, fontM);
    SUCCEED();
}

// splitText — small width, small height (< 23)
TEST_F(test_CScheduleItem, SplitText_SmallWidthSmallHeight)
{
    QFont font;
    QStringList listStr;
    QFontMetrics fontM(font);
    m_item->splitText(font, 20, 15, "ABC", listStr, fontM);
    SUCCEED();
}

// timeFormatChanged — value = 1 (12-hour format)
TEST_F(test_CScheduleItem, TimeFormatChanged_12Hour)
{
    m_item->timeFormatChanged(1);
    SUCCEED();
}

// timeFormatChanged — value = 0 (24-hour format)
TEST_F(test_CScheduleItem, TimeFormatChanged_24Hour)
{
    m_item->timeFormatChanged(0);
    SUCCEED();
}

// paintBackground — type 0, default schedule (no crash)
TEST_F(test_CScheduleItem, PaintBackground_Type0_DefaultSchedule)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Paint Test");
    schedule->setDtStart(QDateTime(QDate(2026, 10, 28), QTime(9, 0)));
    schedule->setScheduleTypeID("1");
    schedule->setAccountColor("#FF0000");
    m_item->setData(schedule, QDate(2026, 10, 28), 1);

    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    m_item->paintBackground(&painter, QRectF(0, 0, 100, 50));
    SUCCEED();
}

// paintBackground — type 1 (more item indicator)
TEST_F(test_CScheduleItem, PaintBackground_Type1)
{
    CScheduleItem *item = new CScheduleItem(QRectF(0, 0, 100, 50), nullptr, 1);
    m_scene->addItem(item);
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("More Item");
    schedule->setScheduleTypeID("1");
    schedule->setAccountColor("#00FF00");
    item->setData(schedule, QDate(2026, 10, 28), 5);

    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    item->paintBackground(&painter, QRectF(0, 0, 100, 50));
    SUCCEED();

    delete item;
}

// paintBackground — with hover and select flags
TEST_F(test_CScheduleItem, PaintBackground_HoverSelect)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Hover Test");
    schedule->setDtStart(QDateTime(QDate(2026, 10, 28), QTime(10, 30)));
    schedule->setScheduleTypeID("1");
    schedule->setAccountColor("#0000FF");
    m_item->setData(schedule, QDate(2026, 10, 28), 1);

    m_item->m_vHoverflag = true;
    m_item->m_vSelectflag = false;

    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    m_item->paintBackground(&painter, QRectF(0, 0, 100, 50));
    SUCCEED();
}

// paintBackground — with select flag and press
TEST_F(test_CScheduleItem, PaintBackground_SelectPress)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Select Test");
    schedule->setDtStart(QDateTime(QDate(2026, 10, 28), QTime(14, 0)));
    schedule->setScheduleTypeID("1");
    schedule->setAccountColor("#FFAA00");
    schedule->setMoved(false);
    m_item->setData(schedule, QDate(2026, 10, 28), 2);

    DragInfoItem::setPressSchedule(schedule);
    DragInfoItem::setPressFlag(true);
    m_item->m_vSelectflag = true;
    m_item->m_vHighflag = false;

    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    m_item->paintBackground(&painter, QRectF(0, 0, 100, 50));
    SUCCEED();

    DragInfoItem::setPressFlag(false);
}

// paintBackground — with focus
TEST_F(test_CScheduleItem, PaintBackground_WithFocus)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Focus Test");
    schedule->setDtStart(QDateTime(QDate(2026, 10, 28), QTime(8, 0)));
    schedule->setScheduleTypeID("1");
    schedule->setAccountColor("#AA00FF");
    m_item->setData(schedule, QDate(2026, 10, 28), 1);

    m_item->setItemFocus(true);

    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    m_item->paintBackground(&painter, QRectF(0, 0, 100, 50));
    SUCCEED();
}
