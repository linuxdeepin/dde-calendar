// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_DCALENDARDIALOG_H
#define TEST_DCALENDARDIALOG_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/dcalendarddialog.h"

class test_dcalendarddialog : public::QObject, public::testing::Test
{
public:
    test_dcalendarddialog();
    ~test_dcalendarddialog();
protected:
    DCalendarDDialog *mDialog = nullptr;
};

#endif // TEST_DCALENDARDIALOG_H
