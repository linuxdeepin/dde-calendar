// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_LABELWIDGET_H
#define TEST_LABELWIDGET_H

#include "customWidget/labelwidget.h"
#include <QObject>
#include <gtest/gtest.h>

class test_labelwidget: public QObject, public testing::Test
{
public:
    test_labelwidget();
    ~test_labelwidget();
protected:
    LabelWidget *mWidget = nullptr;
};

#endif // TEST_LABELWIDGET_H
