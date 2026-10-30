// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>

#include "repeatschedulewidget.h"
#include "dschedule.h"

class TestRepeatScheduleWidget : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_schedule = DSchedule::Ptr(new DSchedule());
        m_schedule->setAllDay(false);
        m_schedule->setDtStart(QDateTime(QDate(2026, 1, 1), QTime(9, 0, 0)));
        m_schedule->setDtEnd(QDateTime(QDate(2026, 1, 1), QTime(10, 0, 0)));
        m_schedule->setSummary("Test Schedule");
    }

    void TearDown() override
    {
    }

    DSchedule::Ptr m_schedule;
};

TEST_F(TestRepeatScheduleWidget, ConstructorDefault)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Cancel,
        repeatScheduleWidget::Widget_Confirm,
        true);
    delete w;
}

TEST_F(TestRepeatScheduleWidget, ConstructorNoButton)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Cancel,
        repeatScheduleWidget::Widget_Confirm,
        false);
    delete w;
}

TEST_F(TestRepeatScheduleWidget, SetScheduleWithConfirmCancelButton)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Cancel,
        repeatScheduleWidget::Widget_Confirm,
        true);
    w->setSchedule(m_schedule);
    delete w;
}

TEST_F(TestRepeatScheduleWidget, SetScheduleWithConfirmChangeButton)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Change,
        repeatScheduleWidget::Widget_Confirm,
        true);
    w->setSchedule(m_schedule);
    delete w;
}

TEST_F(TestRepeatScheduleWidget, SetScheduleWithRepeatCancelButton)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Cancel,
        repeatScheduleWidget::Widget_Repeat,
        true);
    w->setSchedule(m_schedule);
    delete w;
}

TEST_F(TestRepeatScheduleWidget, SetScheduleWithRepeatChangeButton)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Change,
        repeatScheduleWidget::Widget_Repeat,
        true);
    w->setSchedule(m_schedule);
    delete w;
}

TEST_F(TestRepeatScheduleWidget, SetScheduleNoButton)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Cancel,
        repeatScheduleWidget::Widget_Confirm,
        false);
    w->setSchedule(m_schedule);
    delete w;
}

TEST_F(TestRepeatScheduleWidget, SlotButtonCheckNum)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Cancel,
        repeatScheduleWidget::Widget_Confirm,
        true);
    w->setSchedule(m_schedule);
    w->slotButtonCheckNum(0, QString::fromUtf8("取消"));
    w->slotButtonCheckNum(1, QString::fromUtf8("确定"));
    delete w;
}

TEST_F(TestRepeatScheduleWidget, SetScheduleMultipleCalls)
{
    repeatScheduleWidget *w = new repeatScheduleWidget(
        repeatScheduleWidget::Operation_Change,
        repeatScheduleWidget::Widget_Repeat,
        true);
    w->setSchedule(m_schedule);
    DSchedule::Ptr s2 = DSchedule::Ptr(new DSchedule());
    s2->setSummary("Second Schedule");
    w->setSchedule(s2);
    delete w;
}
