// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "schedulelistwidget.h"

#include <QApplication>
#include <QVBoxLayout>

static DSchedule::Ptr makeSchedule(const QDateTime &start, const QDateTime &end,
                                   const QString &title = QStringLiteral("item"))
{
    DSchedule::Ptr s(new DSchedule);
    s->setDtStart(start);
    s->setDtEnd(end);
    s->setAllDay(false);
    s->setSummary(title);
    return s;
}

static DSchedule::List makeList(int count)
{
    DSchedule::List list;
    QDateTime base(QDate(2026, 9, 1), QTime(8, 0));
    for (int i = 0; i < count; ++i) {
        list << makeSchedule(base.addSecs(i * 3600), base.addSecs(i * 3600 + 1800),
                             QStringLiteral("item%1").arg(i));
    }
    return list;
}

TEST(ScheduleListWidgetTest, Construct)
{
    scheduleListWidget w;
    EXPECT_EQ(nullptr, w.parentWidget());
    scheduleListWidget *w2 = new scheduleListWidget(nullptr);
    EXPECT_EQ(nullptr, w2->parentWidget());
    delete w2;
}

TEST(ScheduleListWidgetTest, SetEmptyScheduleInfoVector)
{
    scheduleListWidget w;
    DSchedule::List empty;
    w.setScheduleInfoVector(empty);
    w.updateUI();
    SUCCEED();
}

TEST(ScheduleListWidgetTest, SetSingleScheduleInfoVector)
{
    scheduleListWidget w;
    w.setScheduleInfoVector(makeList(1));
    w.updateUI();
    SUCCEED();
}

TEST(ScheduleListWidgetTest, SetMultipleScheduleInfoVector)
{
    scheduleListWidget w;
    w.setScheduleInfoVector(makeList(5));
    w.updateUI();
    SUCCEED();
}

TEST(ScheduleListWidgetTest, SetScheduleInfoVectorExceedingShowNum)
{
    scheduleListWidget w;
    w.setScheduleInfoVector(makeList(15));
    w.updateUI();
    w.show();
    w.repaint();
    qApp->processEvents();
    SUCCEED();
}

TEST(ScheduleListWidgetTest, SetScheduleInfoVectorExactlyShowNum)
{
    scheduleListWidget w;
    w.setScheduleInfoVector(makeList(10));
    w.updateUI();
    SUCCEED();
}

TEST(ScheduleListWidgetTest, SetScheduleInfoVectorOneOverShowNum)
{
    scheduleListWidget w;
    w.setScheduleInfoVector(makeList(11));
    w.updateUI();
    SUCCEED();
}
