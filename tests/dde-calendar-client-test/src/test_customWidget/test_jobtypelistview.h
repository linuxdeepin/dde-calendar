// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_JOBTYPELISTVIEW_H
#define TEST_JOBTYPELISTVIEW_H

#include "customWidget/jobtypelistview.h"
#include <QObject>
#include <gtest/gtest.h>

class test_jobtypelistview: public QObject, public testing::Test
{
public:
    test_jobtypelistview();
    ~test_jobtypelistview();
protected:
    JobTypeListView *mWidget = nullptr;
};

#endif // TEST_JOBTYPELISTVIEW_H
