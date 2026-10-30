// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_MONTHDAYVIEW_H
#define TEST_MONTHDAYVIEW_H

#include <QObject>
#include "gtest/gtest.h"
#include "monthWidget/monthdayview.h"

class test_monthdayview : public QObject, public ::testing::Test
{
public:
    test_monthdayview();
    ~test_monthdayview() override;
protected:
    void SetUp() override;
    void TearDown() override;
    CMonthDayView *mMonthDayView = nullptr;
};

class test_monthrect : public QObject, public ::testing::Test
{
public:
    test_monthrect();
    ~test_monthrect() override;
protected:
    void SetUp() override;
    void TearDown() override;
    CMonthRect *mMonthRect = nullptr;
};

class test_monthwidget : public QObject, public ::testing::Test
{
public:
    test_monthwidget();
    ~test_monthwidget() override;
protected:
    void SetUp() override;
    void TearDown() override;
    CMonthWidget *mMonthWidget = nullptr;
};

#endif // TEST_MONTHDAYVIEW_H
