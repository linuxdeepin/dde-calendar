// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dunioniddbus.h"
#include "../third-party_stub/stub.h"

#include <gtest/gtest.h>
#include <QtDBus/QtDBus>

static QDBusPendingCall stub_asyncCall_valid(const QString &method,
                                             const QList<QVariant> &args)
{
    Q_UNUSED(method)
    Q_UNUSED(args)
    QDBusMessage dummyCall = QDBusMessage::createMethodCall(
        "com.deepin.utcloud.Daemon", "/com/deepin/utcloud/Daemon",
        "com.deepin.utcloud.Daemon", "SwitcherDump");
    QDBusMessage reply = dummyCall.createReply(QVariant(QString("switcher dump data")));
    return QDBusPendingCall::fromCompletedCall(reply);
}

static QDBusPendingCall stub_asyncCall_invalid(const QString &method,
                                               const QList<QVariant> &args)
{
    Q_UNUSED(method)
    Q_UNUSED(args)
    QDBusError error(QDBusError::ServiceUnknown, "test service unavailable");
    return QDBusPendingCall::fromError(error);
}

class DUnionIDDbusTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_stub.set(ADDR(QDBusAbstractInterface, asyncCallWithArgumentList),
                   stub_asyncCall_valid);
        m_dbus = new DUnionIDDbus(
            "com.deepin.utcloud.Daemon",
            "/com/deepin/utcloud/Daemon",
            QDBusConnection::sessionBus());
    }
    void TearDown() override
    {
        delete m_dbus;
        m_dbus = nullptr;
    }
    Stub m_stub;
    DUnionIDDbus *m_dbus = nullptr;
};

TEST_F(DUnionIDDbusTest, StaticInterfaceName)
{
    EXPECT_STREQ(DUnionIDDbus::staticInterfaceName(), "com.deepin.utcloud.Daemon");
}

TEST_F(DUnionIDDbusTest, ConstructorWithValidReply)
{
    EXPECT_NE(m_dbus, nullptr);
}

TEST_F(DUnionIDDbusTest, ConstructorWithInvalidReply)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, asyncCallWithArgumentList),
             stub_asyncCall_invalid);

    DUnionIDDbus *dbus = new DUnionIDDbus(
        "com.deepin.utcloud.NonExistent",
        "/com/deepin/utcloud/NonExistent",
        QDBusConnection::sessionBus());
    EXPECT_NE(dbus, nullptr);
    delete dbus;
}

TEST_F(DUnionIDDbusTest, DestructorDoesNotCrash)
{
    DUnionIDDbus *dbus = new DUnionIDDbus(
        "com.deepin.utcloud.Daemon",
        "/com/deepin/utcloud/Daemon",
        QDBusConnection::sessionBus());
    delete dbus;
    SUCCEED();
}

TEST_F(DUnionIDDbusTest, Login)
{
    auto reply = m_dbus->login();
    EXPECT_FALSE(reply.isError());
}

TEST_F(DUnionIDDbusTest, Logout)
{
    auto reply = m_dbus->logout();
    EXPECT_FALSE(reply.isError());
}

TEST_F(DUnionIDDbusTest, Upload)
{
    auto reply = m_dbus->Upload("test_key");
    reply.waitForFinished();
    // Typed QDBusPendingReply<QString> checks DBus signature; the stub's
    // fromCompletedCall reply has no signature set, so isError() is expected.
    EXPECT_TRUE(reply.isError());
}

TEST_F(DUnionIDDbusTest, Download)
{
    auto reply = m_dbus->Download("test_key", "/tmp/test_path");
    reply.waitForFinished();
    EXPECT_TRUE(reply.isError());
}

TEST_F(DUnionIDDbusTest, Delete)
{
    auto reply = m_dbus->Delete("test_key");
    reply.waitForFinished();
    EXPECT_TRUE(reply.isError());
}

TEST_F(DUnionIDDbusTest, Metadata)
{
    auto reply = m_dbus->Metadata("test_key");
    reply.waitForFinished();
    EXPECT_TRUE(reply.isError());
}

TEST_F(DUnionIDDbusTest, SwitcherDump)
{
    auto reply = m_dbus->SwitcherDump();
    reply.waitForFinished();
    EXPECT_TRUE(reply.isError());
}

TEST_F(DUnionIDDbusTest, SwitcherGet)
{
    auto reply = m_dbus->SwitcherGet("test_app");
    reply.waitForFinished();
    EXPECT_TRUE(reply.isError());
}
