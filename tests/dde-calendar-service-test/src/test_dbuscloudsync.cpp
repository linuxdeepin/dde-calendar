// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbuscloudsync.h"

#include <gtest/gtest.h>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

class DbuscloudsyncTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_svc = new Dbuscloudsync();
    }
    void TearDown() override
    {
        delete m_svc;
        m_svc = nullptr;
    }
    Dbuscloudsync *m_svc = nullptr;
};

// ---- Dbuscloudsync::Dbuscloudsync (constructor) ----

TEST_F(DbuscloudsyncTest, ConstructorCreatesObject)
{
    EXPECT_NE(m_svc, nullptr);
}

TEST_F(DbuscloudsyncTest, ConstructorDoesNotCrashWithParent)
{
    QObject parent;
    auto *svc = new Dbuscloudsync(&parent);
    EXPECT_NE(svc, nullptr);
    delete svc;
}

// ---- Dbuscloudsync::MsgCallBack ----

TEST_F(DbuscloudsyncTest, MsgCallBackWithValidJson)
{
    QJsonObject obj;
    obj["key"] = QStringLiteral("value");
    QByteArray msg = QJsonDocument(obj).toJson();
    m_svc->MsgCallBack(msg);
    SUCCEED();
}

TEST_F(DbuscloudsyncTest, MsgCallBackWithEmptyByteArray)
{
    m_svc->MsgCallBack(QByteArray());
    SUCCEED();
}

TEST_F(DbuscloudsyncTest, MsgCallBackWithInvalidJson)
{
    m_svc->MsgCallBack(QByteArray("not a json string"));
    SUCCEED();
}
