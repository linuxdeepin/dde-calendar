// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CWEEKDAYBACKGROUNDITEM_H
#define TEST_CWEEKDAYBACKGROUNDITEM_H

#include "graphicsItem/cweekdaybackgrounditem.h"
#include "gtest/gtest.h"

#include <QObject>
#include <QGraphicsScene>

class test_CWeekDayBackgroundItem : public QObject
    , public testing::Test
{
    Q_OBJECT
public:
    test_CWeekDayBackgroundItem();
    void SetUp() override;
    void TearDown() override;

public:
    CWeekDayBackgroundItem *m_weekItem = nullptr;
    QGraphicsScene *m_scene = nullptr;
};

#endif // TEST_CWEEKDAYBACKGROUNDITEM_H
