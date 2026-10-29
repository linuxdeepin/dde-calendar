// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CPUSHBUTTON_H
#define TEST_CPUSHBUTTON_H

#include "customWidget/cpushbutton.h"
#include <QObject>
#include <gtest/gtest.h>

class test_cpushbutton : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    explicit test_cpushbutton(QObject *parent = nullptr);
    virtual void SetUp()
    {
        mWidget = new CPushButton();
        mWidget->setFixedSize(200, 34);
    }

    virtual void TearDown()
    {
        delete mWidget;
        mWidget = nullptr;
    }
protected:
    CPushButton *mWidget = nullptr;
};

#endif // TEST_CPUSHBUTTON_H
