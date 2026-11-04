// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_WEEKWINDOW_H
#define TEST_WEEKWINDOW_H

#include "weekWidget/weekwindow.h"
#include "gtest/gtest.h"
#include <QObject>

class test_weekWindow : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_weekWindow = new CWeekWindow(nullptr);
    }
    void TearDown() override
    {
        delete m_weekWindow;
        m_weekWindow = nullptr;
    }
    CWeekWindow *m_weekWindow = nullptr;
};

#endif // TEST_WEEKWINDOW_H
