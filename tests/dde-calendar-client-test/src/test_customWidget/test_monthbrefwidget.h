// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_MONTHBREFWIDGET_H
#define TEST_MONTHBREFWIDGET_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/monthbrefwidget.h"

class test_monthbrefwidget : public QObject, public testing::Test
{
public:
    test_monthbrefwidget();
    ~test_monthbrefwidget();
protected:
    MonthBrefWidget *mMonthBrefWidget = nullptr;
    CMonthDayRectWidget *mMonthDayRectWidget = nullptr;
};

#endif // TEST_MONTHBREFWIDGET_H
