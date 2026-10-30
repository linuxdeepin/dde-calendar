// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_MONTHSCHEDULEVIEW_H
#define TEST_MONTHSCHEDULEVIEW_H

#include <QObject>
#include "gtest/gtest.h"
#include "monthWidget/monthscheduleview.h"

class test_monthscheduleview : public QObject, public ::testing::Test
{
public:
    test_monthscheduleview();
    ~test_monthscheduleview() override;

protected:
    CMonthScheduleView *mMonthScheduleView = nullptr;
    CWeekScheduleView *mWeekScheduleView = nullptr;
    QGraphicsScene *m_scene = nullptr;
};

#endif // TEST_MONTHSCHEDULEVIEW_H
