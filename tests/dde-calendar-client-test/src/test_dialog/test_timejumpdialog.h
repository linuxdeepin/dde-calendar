// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_TIMEJUMPDIALOG_H
#define TEST_TIMEJUMPDIALOG_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/timejumpdialog.h"

class test_timejumpdialog : public QObject, public testing::Test
{
public:
    test_timejumpdialog();
    ~test_timejumpdialog();
protected:
    TimeJumpDialog *mDialog = nullptr;
};

#endif // TEST_TIMEJUMPDIALOG_H
