// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CSCENETABKEYDEAL_H
#define TEST_CSCENETABKEYDEAL_H

#include "keypressstub.h"
#include "view/cgraphicsscene.h"
#include "gtest/gtest.h"
#include "keypress/cscenetabkeydeal.h"

#include <QObject>

class test_CSceneTabKeyDeal : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    test_CSceneTabKeyDeal();
    void SetUp();
    void TearDown();

public:
    KeyPressStub stub;
    QSharedPointer<CGraphicsScene> scene;
    QSharedPointer<QGraphicsView> view;
    QSharedPointer<CSceneTabKeyDeal> tabDeal;
};

#endif // TEST_CSCENETABKEYDEAL_H
