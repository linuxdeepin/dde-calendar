// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "viewschedulewidget.h"

#include <QApplication>
#include <QDateTime>

static DSchedule::Ptr makeSchedule(const QDateTime &start, const QDateTime &end,
                                   const QString &title = QStringLiteral("vitem"))
{
    DSchedule::Ptr s(new DSchedule);
    s->setDtStart(start);
    s->setDtEnd(end);
    s->setAllDay(false);
    s->setSummary(title);
    return s;
}

static DSchedule::Map makeMap(int days, int perDay)
{
    DSchedule::Map map;
    QDateTime base(QDate(2026, 10, 1), QTime(8, 0));
    for (int d = 0; d < days; ++d) {
        DSchedule::List list;
        for (int i = 0; i < perDay; ++i) {
            list << makeSchedule(base.addDays(d).addSecs(i * 3600),
                                 base.addDays(d).addSecs(i * 3600 + 1800),
                                 QStringLiteral("d%1-i%2").arg(d).arg(i));
        }
        map.insert(base.addDays(d).date(), list);
    }
    return map;
}

TEST(ViewScheduleWidgetTest, ConstructAndDestruct)
{
    viewschedulewidget w;
    EXPECT_EQ(nullptr, w.parentWidget());
    viewschedulewidget *w2 = new viewschedulewidget(nullptr);
    EXPECT_EQ(nullptr, w2->parentWidget());
    delete w2;
}

TEST(ViewScheduleWidgetTest, SetScheduleDateRangeInfo)
{
    viewschedulewidget w;
    DSchedule::Map map = makeMap(2, 3);
    w.setScheduleDateRangeInfo(map);
    SUCCEED();
}

TEST(ViewScheduleWidgetTest, SetQueryDateTimes)
{
    viewschedulewidget w;
    QDateTime begin(QDate(2026, 10, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 10, 31), QTime(23, 59));
    w.setQueryBeginDateTime(begin);
    w.setQueryEndDateTime(end);
    EXPECT_EQ(begin, w.m_beginDateTime);
    EXPECT_EQ(end, w.m_endDateTime);
}

TEST(ViewScheduleWidgetTest, GetScheduleNumEmpty)
{
    viewschedulewidget w;
    DSchedule::Map empty;
    EXPECT_EQ(0, w.getScheduleNum(empty));
}

TEST(ViewScheduleWidgetTest, GetScheduleNumNonEmpty)
{
    viewschedulewidget w;
    DSchedule::Map map = makeMap(3, 2);
    EXPECT_EQ(6, w.getScheduleNum(map));
}

TEST(ViewScheduleWidgetTest, ViewScheduleInfoShowEmpty)
{
    viewschedulewidget w;
    DSchedule::Map empty;
    w.viewScheduleInfoShow(empty);
    SUCCEED();
}

TEST(ViewScheduleWidgetTest, ViewScheduleInfoShowFew)
{
    viewschedulewidget w;
    DSchedule::Map map = makeMap(2, 2);
    w.viewScheduleInfoShow(map);
    w.show();
    w.repaint();
    qApp->processEvents();
    SUCCEED();
}

TEST(ViewScheduleWidgetTest, ViewScheduleInfoShowExceedsLimit)
{
    viewschedulewidget w;
    DSchedule::Map map = makeMap(15, 1);
    w.viewScheduleInfoShow(map);
    w.show();
    w.repaint();
    qApp->processEvents();
    SUCCEED();
}

TEST(ViewScheduleWidgetTest, SlotItemPress)
{
    viewschedulewidget w;
    auto s = makeSchedule(QDateTime(QDate(2026, 11, 1), QTime(8, 0)),
                          QDateTime(QDate(2026, 11, 1), QTime(9, 0)));
    w.slotItemPress(s);
    SUCCEED();
}
