// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CFOCUSITEM_H
#define TEST_CFOCUSITEM_H

#include "graphicsItem/cfocusitem.h"
#include "gtest/gtest.h"
#include <QObject>
#include <QGraphicsScene>

class test_cfocusitem : public QObject, public ::testing::Test
{
    Q_OBJECT
public:
    test_cfocusitem() = default;
    void SetUp() override
    {
        m_item = new CFocusItem();
        m_scene = new QGraphicsScene();
        m_scene->addItem(m_item);
    }
    void TearDown() override
    {
        delete m_scene;
        m_item = nullptr;
    }

protected:
    CFocusItem *m_item = nullptr;
    QGraphicsScene *m_scene = nullptr;
};

#endif // TEST_CFOCUSITEM_H
