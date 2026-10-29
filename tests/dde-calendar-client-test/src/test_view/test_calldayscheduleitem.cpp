// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calldayscheduleitem.h"

#include <QPainter>
#include <QPixmap>

DSchedule::Ptr test_CAlldayscheduleitem::createTestSchedule()
{
    DSchedule::Ptr schedule = DSchedule::Ptr(new DSchedule());
    schedule->setSummary("Test Schedule");
    schedule->setScheduleTypeID("1");
    schedule->setDtStart(QDateTime::currentDateTime());
    schedule->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    return schedule;
}

// Constructor creates a valid item
TEST_F(test_CAlldayscheduleitem, Constructor)
{
    CAllDayScheduleItem *item = new CAllDayScheduleItem(QRectF(0, 0, 100, 40));
    ASSERT_NE(item, nullptr);
    delete item;
}

// hasSelectSchedule returns true when schedule matches
TEST_F(test_CAlldayscheduleitem, hasSelectSchedule_True)
{
    DSchedule::Ptr schedule = createTestSchedule();
    m_allItem->setData(schedule);
    bool result = m_allItem->hasSelectSchedule(schedule);
    EXPECT_TRUE(result);
}

// hasSelectSchedule returns false when schedule does not match
TEST_F(test_CAlldayscheduleitem, hasSelectSchedule_False)
{
    DSchedule::Ptr schedule1 = createTestSchedule();
    m_allItem->setData(schedule1);

    DSchedule::Ptr schedule2 = DSchedule::Ptr(new DSchedule());
    schedule2->setSummary("Different Schedule");
    schedule2->setScheduleTypeID("2");

    bool result = m_allItem->hasSelectSchedule(schedule2);
    EXPECT_FALSE(result);
}

// hasSelectSchedule returns false when no schedule set
TEST_F(test_CAlldayscheduleitem, hasSelectSchedule_NullData)
{
    DSchedule::Ptr schedule = createTestSchedule();
    bool result = m_allItem->hasSelectSchedule(schedule);
    EXPECT_FALSE(result);
}

// paintBackground with pixmap mode (isPixMap = true)
TEST_F(test_CAlldayscheduleitem, paintBackground_PixMap)
{
    DSchedule::Ptr schedule = createTestSchedule();
    m_allItem->setData(schedule);

    QPixmap pixmap(200, 50);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_allItem->paintBackground(&painter, m_rectF, true);
}

// paintBackground with normal mode (isPixMap = false)
TEST_F(test_CAlldayscheduleitem, paintBackground_Normal)
{
    DSchedule::Ptr schedule = createTestSchedule();
    m_allItem->setData(schedule);

    QPixmap pixmap(200, 50);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_allItem->paintBackground(&painter, m_rectF, false);
}

// paintBackground with pressed schedule (select flag)
TEST_F(test_CAlldayscheduleitem, paintBackground_PressedSchedule)
{
    DSchedule::Ptr schedule = createTestSchedule();
    m_allItem->setData(schedule);

    CAllDayScheduleItem::setPressSchedule(schedule);
    m_allItem->m_vSelectflag = true;
    m_allItem->m_vHighflag = false;
    m_allItem->m_vHoverflag = false;

    QPixmap pixmap(200, 50);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_allItem->paintBackground(&painter, m_rectF, false);
}

// paintBackground with hover flag
TEST_F(test_CAlldayscheduleitem, paintBackground_HoverFlag)
{
    DSchedule::Ptr schedule = createTestSchedule();
    m_allItem->setData(schedule);

    m_allItem->m_vHoverflag = true;
    m_allItem->m_vSelectflag = false;
    m_allItem->m_vHighflag = false;

    QPixmap pixmap(200, 50);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_allItem->paintBackground(&painter, m_rectF, false);
}

// paintBackground with high flag
TEST_F(test_CAlldayscheduleitem, paintBackground_HighFlag)
{
    DSchedule::Ptr schedule = createTestSchedule();
    m_allItem->setData(schedule);

    m_allItem->m_vHoverflag = false;
    m_allItem->m_vSelectflag = false;
    m_allItem->m_vHighflag = true;

    QPixmap pixmap(200, 50);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_allItem->paintBackground(&painter, m_rectF, false);
}

// paintBackground with long title that needs truncation
TEST_F(test_CAlldayscheduleitem, paintBackground_LongTitle)
{
    DSchedule::Ptr schedule = createTestSchedule();
    schedule->setSummary("This is a very long schedule title that should be truncated with ellipsis");
    m_allItem->setData(schedule);

    QPixmap pixmap(200, 50);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_allItem->paintBackground(&painter, m_rectF, false);
}

// paintBackground with focus and pixmap false draws frame
TEST_F(test_CAlldayscheduleitem, paintBackground_WithFocus)
{
    DSchedule::Ptr schedule = createTestSchedule();
    m_allItem->setData(schedule);
    m_allItem->setItemFocus(true);

    QPixmap pixmap(200, 50);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    m_allItem->paintBackground(&painter, m_rectF, false);
}
