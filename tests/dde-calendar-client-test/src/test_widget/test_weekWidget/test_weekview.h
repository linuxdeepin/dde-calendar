// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_WEEKVIEW_H
#define TEST_WEEKVIEW_H

#include "weekWidget/weekview.h"
#include "gtest/gtest.h"
#include <QObject>

static int testGetWeekNumOfYear(const QDate &)
{
    return 1;
}

class test_weekview : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mWeekView = new CWeekView(&testGetWeekNumOfYear);
    }
    void TearDown() override
    {
        delete mWeekView;
        mWeekView = nullptr;
    }
    CWeekView *mWeekView = nullptr;
};

// Separate fixture for CWeekNumWidget (needs function pointer in constructor)
class test_weeknumwidget : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mWeekNumWidget = new CWeekNumWidget(&testGetWeekNumOfYear, nullptr);
    }
    void TearDown() override
    {
        delete mWeekNumWidget;
        mWeekNumWidget = nullptr;
    }
    CWeekNumWidget *mWeekNumWidget = nullptr;
};

#endif // TEST_WEEKVIEW_H
