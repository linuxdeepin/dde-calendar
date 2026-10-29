// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CALLDAYSCHEDULEITEM_H
#define TEST_CALLDAYSCHEDULEITEM_H

#include "gtest/gtest.h"
#include "graphicsItem/calldayscheduleitem.h"
#include "dschedule.h"

#include <QObject>
#include <QGraphicsScene>

class test_CAlldayscheduleitem : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    test_CAlldayscheduleitem() = default;
    void SetUp() override
    {
        m_rectF.setRect(0, 0, 200, 50);
        m_allItem = new CAllDayScheduleItem(m_rectF);
        m_scene = new QGraphicsScene();
        m_scene->addItem(m_allItem);
    }
    void TearDown() override
    {
        delete m_scene;
        m_allItem = nullptr;
    }
signals:

public slots:
public:
    CAllDayScheduleItem *m_allItem = nullptr;
    QGraphicsScene *m_scene = nullptr;
    QRectF m_rectF;

    DSchedule::Ptr createTestSchedule();
};

#endif // TEST_CALLDAYSCHEDULEITEM_H
