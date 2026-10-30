// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CWEEKWIDGET_H
#define TEST_CWEEKWIDGET_H

#include "customWidget/cweekwidget.h"
#include <QObject>
#include <gtest/gtest.h>

class test_cweekwidget: public QObject, public testing::Test
{
public:
    test_cweekwidget();
    ~test_cweekwidget();
protected:
    CWeekWidget *mWidget = nullptr;
};

#endif // TEST_CWEEKWIDGET_H
