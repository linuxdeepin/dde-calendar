// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CSCHEDULEITEM_H
#define TEST_CSCHEDULEITEM_H

#include "graphicsItem/scheduleitem.h"
#include "dschedule.h"

#include <gtest/gtest.h>
#include <QObject>
#include <QGraphicsScene>

class test_CScheduleItem : public testing::Test
{
public:
    test_CScheduleItem();
    void SetUp() override;
    void TearDown() override;

protected:
    CScheduleItem *m_item = nullptr;
    QGraphicsScene *m_scene = nullptr;
};

#endif // TEST_CSCHEDULEITEM_H
