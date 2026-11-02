// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEXT_COLORSLIDER_H
#define TEXT_COLORSLIDER_H

#include "customWidget/colorWidget/colorslider.h"
#include <QObject>
#include <QTest>
#include <gtest/gtest.h>

class test_colorslider : public QObject, public::testing::Test
{
    Q_OBJECT
public:
    test_colorslider() {}

    virtual void SetUp()
    {
        mWidget = new ColorSlider();
        mWidget->resize(100, 20);
    }

    virtual void TearDown()
    {
        delete mWidget;
        mWidget = nullptr;
    }
protected:
    ColorSlider *mWidget = nullptr;
};

#endif // TEXT_COLORSLIDER_H
