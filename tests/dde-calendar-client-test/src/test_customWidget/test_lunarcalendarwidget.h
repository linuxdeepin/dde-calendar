// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_LUNARCALENDARWIDGET_H
#define TEST_LUNARCALENDARWIDGET_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/lunarcalendarwidget.h"

class test_lunarcalendarwidget : public QObject, public testing::Test
{
public:
    test_lunarcalendarwidget();
    ~test_lunarcalendarwidget();
protected:
    LunarCalendarWidget *mWidget = nullptr;
};

#endif // TEST_LUNARCALENDARWIDGET_H
