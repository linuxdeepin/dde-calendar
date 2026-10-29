// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_DRAGINFOGRAPHICSVIEW_H
#define TEST_DRAGINFOGRAPHICSVIEW_H

#include "draginfographicsview.h"
#include "monthgraphiview.h"
#include "gtest/gtest.h"
#include <QObject>

class test_draginfographicsview : public QObject, public::testing::Test
{
public:
    test_draginfographicsview();
    ~test_draginfographicsview();
protected:
    CMonthGraphicsview *cDragInfoView = nullptr;
};

#endif // TEST_DRAGINFOGRAPHICSVIEW_H
