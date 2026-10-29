// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CBUTTONBOX_H
#define TEST_CBUTTONBOX_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/cbuttonbox.h"

DWIDGET_USE_NAMESPACE

class test_cbuttonbox : public QObject, public testing::Test
{
    Q_OBJECT
public:
    explicit test_cbuttonbox(QObject *parent = nullptr)
        : QObject(parent) {}

    void SetUp() override
    {
        m_buttonBox = new CButtonBox();
        m_btn1 = new DButtonBoxButton("Btn1");
        m_btn2 = new DButtonBoxButton("Btn2");
        m_buttonBox->setButtonList(QList<DButtonBoxButton *>() << m_btn1 << m_btn2, true);
        m_buttonBox->setId(m_btn1, 0);
        m_buttonBox->setId(m_btn2, 1);
        m_btn1->setChecked(true);
    }

    void TearDown() override
    {
        delete m_buttonBox;
        m_buttonBox = nullptr;
    }

protected:
    CButtonBox *m_buttonBox = nullptr;
    DButtonBoxButton *m_btn1 = nullptr;
    DButtonBoxButton *m_btn2 = nullptr;
};

#endif // TEST_CBUTTONBOX_H
