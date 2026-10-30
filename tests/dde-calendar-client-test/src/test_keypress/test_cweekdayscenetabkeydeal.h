// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CWEEKDAYSCENETABKEYDEAL_H
#define TEST_CWEEKDAYSCENETABKEYDEAL_H

#include "keypressstub.h"
#include "view/cgraphicsscene.h"
#include "gtest/gtest.h"
#include "keypress/cweekdayscenetabkeydeal.h"

#include <QObject>

class test_CWeekDaySceneTabKeyDeal : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    test_CWeekDaySceneTabKeyDeal();
    void SetUp();
    void TearDown();

public:
    KeyPressStub stub;
    QSharedPointer<CGraphicsScene> scene;
    QSharedPointer<QGraphicsView> view;
    QSharedPointer<CWeekDaySceneTabKeyDeal> weekTabDeal;
};

#endif // TEST_CWEEKDAYSCENETABKEYDEAL_H
