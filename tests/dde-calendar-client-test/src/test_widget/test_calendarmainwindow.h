// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALENDARMAINWINDOW_H
#define TEST_CALENDARMAINWINDOW_H

#include "widget/calendarmainwindow.h"
#include "gtest/gtest.h"
#include <QObject>

class test_calendarmainwindow : public QObject
    , public ::testing::Test
{
public:
    test_calendarmainwindow();
    ~test_calendarmainwindow();
    void SetUp() override;
    void TearDown() override;

protected:
    Calendarmainwindow *mMainWindow = nullptr;
};

#endif // TEST_CALENDARMAINWINDOW_H
