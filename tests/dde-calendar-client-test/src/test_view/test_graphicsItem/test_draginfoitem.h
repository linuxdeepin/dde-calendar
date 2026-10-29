// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_DRAGINFOITEM_H
#define TEST_DRAGINFOITEM_H

#include "graphicsItem/draginfoitem.h"
#include <gtest/gtest.h>
#include <QObject>
#include <QGraphicsScene>

class TestDragInfoItem : public DragInfoItem
{
    Q_OBJECT
public:
    explicit TestDragInfoItem(QRectF rect, QGraphicsItem *parent = nullptr)
        : DragInfoItem(rect, parent)
    {
    }
protected:
    void paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap = false) override
    {
        Q_UNUSED(painter)
        Q_UNUSED(rect)
        Q_UNUSED(isPixMap)
    }
};

class test_DragInfoItem : public testing::Test
{
public:
    test_DragInfoItem();
    void SetUp() override;
    void TearDown() override;

protected:
    TestDragInfoItem *m_item = nullptr;
    QGraphicsScene *m_scene = nullptr;
};

#endif // TEST_DRAGINFOITEM_H
