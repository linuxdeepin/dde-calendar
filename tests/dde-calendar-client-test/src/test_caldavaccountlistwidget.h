// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALDAVACCOUNTLISTWIDGET_H
#define TEST_CALDAVACCOUNTLISTWIDGET_H

#include <QObject>
#include <gtest/gtest.h>
#include "dialog/caldavaccountlistwidget.h"

class test_caldavaccountlistwidget : public QObject, public testing::Test
{
public:
    test_caldavaccountlistwidget();
    ~test_caldavaccountlistwidget();
protected:
    CalDavAccountListWidget *mWidget = nullptr;
};

#endif // TEST_CALDAVACCOUNTLISTWIDGET_H
