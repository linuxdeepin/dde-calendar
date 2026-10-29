// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicemanager.h"

#include <gtest/gtest.h>

#include <QDBusConnection>

// DServiceManager's constructor registers a D-Bus service name and three
// sub-objects on the session bus.  The service name "com.deepin.dataserver.
// Calendar" is not registered at process start, so the constructor should
// succeed.  Each TEST_F gets a fresh SetUp/TearDown, but the D-Bus service
// name persists for the lifetime of the process, so we unregister it in
// TearDown to allow subsequent tests to construct DServiceManager again.

class DServiceManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_manager = new DServiceManager();
    }
    void TearDown() override
    {
        delete m_manager;
        m_manager = nullptr;
        // Release the service name so the next test can re-register it.
        QDBusConnection::sessionBus().unregisterService(serviceBaseName);
    }
    DServiceManager *m_manager = nullptr;
};

// ---- DServiceManager::DServiceManager (constructor) ----

TEST_F(DServiceManagerTest, ConstructorRegistersServiceSuccessfully)
{
    // If the constructor reached this point without calling exit(), the
    // service and all three sub-objects were registered successfully.
    ASSERT_NE(m_manager, nullptr);

    // The service name should now be owned by this process.
    QDBusReply<QString> owner =
        QDBusConnection::sessionBus().interface()->serviceOwner(serviceBaseName);
    EXPECT_TRUE(owner.isValid());
    EXPECT_FALSE(owner.value().isEmpty());
}

// ---- DServiceManager::updateRemindJob ----

TEST_F(DServiceManagerTest, UpdateRemindJobWithAccountManagerDoesNotCrash)
{
    // After construction m_accountManagerService is non-null, so updateRemindJob
    // delegates to m_accountManagerService->updateRemindJob(false).
    m_manager->updateRemindJob();
    SUCCEED();
}

TEST_F(DServiceManagerTest, UpdateRemindJobWithNullAccountManagerIsNoOp)
{
    // -fno-access-control allows direct write to the private member.
    m_manager->m_accountManagerService = nullptr;
    m_manager->updateRemindJob();
    SUCCEED();
}
