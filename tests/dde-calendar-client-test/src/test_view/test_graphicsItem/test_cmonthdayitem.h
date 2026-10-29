// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CMONTHDAYITEM_H
#define TEST_CMONTHDAYITEM_H

#include "graphicsItem/cmonthdayitem.h"
#include <QObject>
#include <QGraphicsScene>
#include <gtest/gtest.h>

class test_cmonthdayitem : public ::testing::Test
{
public:
    test_cmonthdayitem() = default;

    virtual void SetUp()
    {
        m_scene = new QGraphicsScene();
        mItem = new CMonthDayItem();
        m_scene->addItem(mItem);
        mItem->setRect(QRectF(0, 0, 100, 100));
    }

    virtual void TearDown()
    {
        delete m_scene;
        mItem = nullptr;
    }

protected:
    CMonthDayItem *mItem = nullptr;
    QGraphicsScene *m_scene = nullptr;
};

#endif // TEST_CMONTHDAYITEM_H
