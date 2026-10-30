// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CKEYUPDEAL_H
#define TEST_CKEYUPDEAL_H

#include "keypressstub.h"
#include "view/cgraphicsscene.h"
#include "gtest/gtest.h"
#include "keypress/ckeyupdeal.h"

#include <QObject>

class test_CKeyUpDeal : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    test_CKeyUpDeal();
    void SetUp();
    void TearDown();

public:
    KeyPressStub stub;
    QSharedPointer<CGraphicsScene> scene;
    QSharedPointer<CKeyUpDeal> upDeal;
};

#endif // TEST_CKEYUPDEAL_H
