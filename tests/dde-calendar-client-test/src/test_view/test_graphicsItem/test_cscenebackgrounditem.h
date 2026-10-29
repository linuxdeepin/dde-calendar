// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CSCENEBACKGROUNDITEM_H
#define TEST_CSCENEBACKGROUNDITEM_H

#include "graphicsItem/cscenebackgrounditem.h"
#include <gtest/gtest.h>
#include <QObject>
#include <QGraphicsScene>

class test_cscenebackgrounditem: public::testing::Test
{
public:
    virtual void SetUp()
    {
        mScene = new QGraphicsScene();
        mItem = new CSceneBackgroundItem(CSceneBackgroundItem::OnMonthView);
        mScene->addItem(mItem);
        mItem->setRect(QRectF(0, 0, 100, 100));
    }

    virtual void TearDown()
    {
        delete mScene;
        mScene = nullptr;
        mItem = nullptr;
    }
protected:
    CSceneBackgroundItem *mItem = nullptr;
    QGraphicsScene *mScene = nullptr;
};

#endif // TEST_CSCENEBACKGROUNDITEM_H
