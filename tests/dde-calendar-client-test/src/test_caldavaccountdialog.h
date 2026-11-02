// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALDAVACCOUNTDIALOG_H
#define TEST_CALDAVACCOUNTDIALOG_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/caldavaccountdialog.h"

class test_caldavaccountdialog : public QObject, public testing::Test
{
public:
    test_caldavaccountdialog();
    ~test_caldavaccountdialog();
protected:
    CalDavAccountDialog *mDialog = nullptr;
};

#endif // TEST_CALDAVACCOUNTDIALOG_H
