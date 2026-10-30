// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_JOBTYPECOMBOBOX_H
#define TEST_JOBTYPECOMBOBOX_H

#include "customWidget/jobtypecombobox.h"
#include <QObject>
#include <gtest/gtest.h>

class test_jobtypecombobox: public QObject, public testing::Test
{
public:
    test_jobtypecombobox();
    ~test_jobtypecombobox();
protected:
    JobTypeComboBox *mWidget = nullptr;
};

#endif // TEST_JOBTYPECOMBOBOX_H
