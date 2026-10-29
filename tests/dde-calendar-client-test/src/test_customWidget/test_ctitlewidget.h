// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CTITLEWIDGET_H
#define TEST_CTITLEWIDGET_H

#include "customWidget/ctitlewidget.h"
#include <QObject>
#include <QTest>
#include <gtest/gtest.h>

class test_ctitlewidget: public QObject, public::testing::Test
{
    Q_OBJECT
public:
    test_ctitlewidget();

    virtual void SetUp()
    {
        mWidget = new CTitleWidget();
        mWidget->resize(800, 40);
    }

    virtual void TearDown()
    {
        delete mWidget;
        mWidget = nullptr;
    }
protected:
    CTitleWidget *mWidget = nullptr;
};

#endif // TEST_CTITLEWIDGET_H
