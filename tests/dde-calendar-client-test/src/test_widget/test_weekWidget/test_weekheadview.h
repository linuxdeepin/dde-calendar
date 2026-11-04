// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_WEEKHEADVIEW_H
#define TEST_WEEKHEADVIEW_H

#include "weekWidget/weekheadview.h"
#include "gtest/gtest.h"
#include <QObject>

class test_weekheadview : public ::testing::Test
{
protected:
    void SetUp() override
    {
        mWeekHeadView = new CWeekHeadView();
    }
    void TearDown() override
    {
        delete mWeekHeadView;
        mWeekHeadView = nullptr;
    }
    CWeekHeadView *mWeekHeadView = nullptr;
};

#endif // TEST_WEEKHEADVIEW_H
