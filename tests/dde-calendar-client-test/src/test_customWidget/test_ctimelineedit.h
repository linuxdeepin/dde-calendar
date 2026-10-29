// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CTIMELINEEDIT_H
#define TEST_CTIMELINEEDIT_H

#include "customWidget/ctimelineedit.h"
#include <QObject>
#include <QTest>
#include <gtest/gtest.h>

class test_ctimelineedit : public::testing::Test
{
public:
    test_ctimelineedit();

    virtual void SetUp()
    {
        mWidget = new CTimeLineEdit(5);
    }

    virtual void TearDown()
    {
        delete mWidget;
        mWidget = nullptr;
    }
protected:
    CTimeLineEdit *mWidget = nullptr;
};

#endif // TEST_CTIMELINEEDIT_H
