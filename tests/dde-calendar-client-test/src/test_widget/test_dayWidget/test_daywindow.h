// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_DAYWINDOW_H
#define TEST_DAYWINDOW_H

#include <QObject>
#include <gtest/gtest.h>
#include "dayWidget/daywindow.h"

class test_daywindow : public QObject
    , public ::testing::Test
{
public:
    test_daywindow();
    void SetUp() override;
    void TearDown() override;

protected:
    CDayWindow *m_dayWindow = nullptr;
};

#endif // TEST_DAYWINDOW_H
