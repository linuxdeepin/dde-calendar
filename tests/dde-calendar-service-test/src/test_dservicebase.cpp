// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicebase.h"

#include <gtest/gtest.h>
#include "../third-party_stub/stub.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusContext>
#include <QVariant>
#include <QString>

namespace {
// Concrete subclass exposing the protected DBus-context helpers so the test
// can exercise them directly.
class TestableServiceBase : public DServiceBase
{
public:
    using DServiceBase::DServiceBase;
    using DServiceBase::getClientName;
    using DServiceBase::clientWhite;
    using DServiceBase::notifyPropertyChanged;
};

// Stub for QDBusContext::message() — returns a static empty message so that
// getClientName() does not dereference an invalid internal pointer when called
// outside a real D-Bus dispatch.
const QDBusMessage &stub_message()
{
    static QDBusMessage msg;
    return msg;
}
} // namespace

class DServiceBaseTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_service = new TestableServiceBase("/test/path", "com.test.Interface");
    }
    void TearDown() override
    {
        delete m_service;
        m_service = nullptr;
    }
    TestableServiceBase *m_service = nullptr;
};

// ---- DServiceBase::DServiceBase (constructor) ----

TEST_F(DServiceBaseTest, ConstructorStoresPathAndInterface)
{
    EXPECT_EQ(m_service->getPath(), QString("/test/path"));
    EXPECT_EQ(m_service->getInterface(), QString("com.test.Interface"));
}

TEST_F(DServiceBaseTest, GetPathReturnsStoredPath)
{
    auto *svc = new TestableServiceBase("/another/path", "another.iface");
    EXPECT_EQ(svc->getPath(), QString("/another/path"));
    delete svc;
}

TEST_F(DServiceBaseTest, GetInterfaceReturnsStoredInterface)
{
    auto *svc = new TestableServiceBase("/p", "org.example.Iface");
    EXPECT_EQ(svc->getInterface(), QString("org.example.Iface"));
    delete svc;
}

TEST_F(DServiceBaseTest, EmptyPathAndInterface)
{
    auto *svc = new TestableServiceBase("", "");
    EXPECT_TRUE(svc->getPath().isEmpty());
    EXPECT_TRUE(svc->getInterface().isEmpty());
    delete svc;
}

// ---- DServiceBase::getClientName ----

TEST_F(DServiceBaseTest, GetClientNameWithoutDBusContextReturnsEmpty)
{
    // Without an incoming D-Bus call, QDBusContext::message() would dereference
    // an invalid internal pointer and crash.  Stub it to return an empty message
    // so getClientName() proceeds with an empty service name, fails to open the
    // /proc/<pid>/status file, and returns an empty string.
    Stub stub;
    stub.set(ADDR(QDBusContext, message), stub_message);
    const QString name = m_service->getClientName();
    EXPECT_TRUE(name.isEmpty());
}

// ---- DServiceBase::clientWhite ----

TEST_F(DServiceBaseTest, ClientWhiteAlwaysAllowsWhenAutoExitDisabled)
{
    // When CALENDAR_SERVICE_AUTO_EXIT is not defined the whitelist check is
    // short-circuited and every caller is admitted.
    EXPECT_TRUE(m_service->clientWhite(0));
    EXPECT_TRUE(m_service->clientWhite(1));
    EXPECT_TRUE(m_service->clientWhite(2));
    EXPECT_TRUE(m_service->clientWhite(-1));
}

// ---- DServiceBase::notifyPropertyChanged ----

TEST_F(DServiceBaseTest, NotifyPropertyChangedDoesNotCrash)
{
    // Sending the PropertiesChanged signal on the session bus must not crash;
    // the property is not registered on this object so the carried value is
    // invalid, which the signal still tolerates.
    m_service->notifyPropertyChanged("com.test.Interface", "objectName");
    SUCCEED();
}
