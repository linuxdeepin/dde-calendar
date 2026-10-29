// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CMONTHSCHEDULEITEM_H
#define TEST_CMONTHSCHEDULEITEM_H

#include "graphicsItem/cmonthscheduleitem.h"
#include <QObject>
#include <gtest/gtest.h>

class test_cmonthscheduleitem: public::testing::Test
{
public:
    virtual void SetUp()
    {
        QRect rect(0, 0, 200, 30);
        mItem = new CMonthScheduleItem(rect);
        mSchedule = DSchedule::Ptr(new DSchedule());
        mSchedule->setSummary("Test Schedule");
        mSchedule->setScheduleTypeID("1");
        mItem->setData(mSchedule);
    }

    virtual void TearDown()
    {
        delete mItem;
        mItem = nullptr;
    }
protected:
    CMonthScheduleItem *mItem = nullptr;
    DSchedule::Ptr mSchedule;
};

#endif // TEST_CMONTHSCHEDULEITEM_H
