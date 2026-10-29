// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_YEARSCHEDULEVIEW_H
#define TEST_YEARSCHEDULEVIEW_H

#include <QObject>
#include <gtest/gtest.h>
#include "widget/yearWidget/yearscheduleview.h"

class test_yearscheduleview : public QObject, public ::testing::Test
{
public:
    test_yearscheduleview();
    ~test_yearscheduleview();
    void SetUp() override;
    void TearDown() override;

protected:
    CYearScheduleView *cYearScheduleView = nullptr;
    CYearScheduleOutView *zYearScheduleOutView = nullptr;
};

#endif // TEST_YEARSCHEDULEVIEW_H
