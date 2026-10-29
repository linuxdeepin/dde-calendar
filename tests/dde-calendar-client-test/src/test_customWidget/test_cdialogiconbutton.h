// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CDIALOGICONBUTTON_H
#define TEST_CDIALOGICONBUTTON_H

#include "customWidget/cdialogiconbutton.h"
#include <QObject>
#include <gtest/gtest.h>

class test_cdialogiconbutton : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    explicit test_cdialogiconbutton(QObject *parent = nullptr);
    virtual void SetUp()
    {
        mWidget = new CDialogIconButton();
    }
    virtual void TearDown()
    {
        delete mWidget;
        mWidget = nullptr;
    }
protected:
    CDialogIconButton *mWidget = nullptr;
};

#endif // TEST_CDIALOGICONBUTTON_H
