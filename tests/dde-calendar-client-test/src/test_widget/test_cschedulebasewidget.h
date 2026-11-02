// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CSCHEDULEBASEWIDGET_H
#define TEST_CSCHEDULEBASEWIDGET_H

#include "widget/cschedulebasewidget.h"
#include "dschedule.h"
#include "gtest/gtest.h"
#include <QObject>

// CScheduleBaseWidget is abstract (has pure virtual methods).
// This concrete subclass implements the pure virtuals with empty bodies.
class TestScheduleBaseWidget : public CScheduleBaseWidget
{
    Q_OBJECT
public:
    explicit TestScheduleBaseWidget(QWidget *parent = nullptr)
        : CScheduleBaseWidget(parent) {}
    ~TestScheduleBaseWidget() override = default;

    void updateShowDate(const bool isUpdateBar = true) override { Q_UNUSED(isUpdateBar) }
    void updateShowSchedule() override {}
    void updateShowLunar() override {}
    void setYearData() override {}
    void setSelectSearchScheduleInfo(const DSchedule::Ptr &info) override { Q_UNUSED(info) }
};

class test_cschedulebasewidget : public QObject
    , public ::testing::Test
{
public:
    test_cschedulebasewidget();
    ~test_cschedulebasewidget();
    void SetUp() override;
    void TearDown() override;

protected:
    TestScheduleBaseWidget *mWidget = nullptr;
};

#endif // TEST_CSCHEDULEBASEWIDGET_H
