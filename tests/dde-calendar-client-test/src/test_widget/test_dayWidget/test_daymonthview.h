// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_DAYMONTHVIEW_H
#define TEST_DAYMONTHVIEW_H

#include <QObject>
#include <gtest/gtest.h>
#include "dayWidget/daymonthview.h"

class test_daymonthview : public QObject
    , public ::testing::Test
{
public:
    test_daymonthview();
    ~test_daymonthview();

protected:
    CDayMonthView *mDayMonthView = nullptr;
    CDayMonthWidget *mDayMonthWidget = nullptr;
};

#endif // TEST_DAYMONTHVIEW_H
