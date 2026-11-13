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
        // 外部真实 dde-calendar-service 进程可能已注册该名字（桌面会话常驻），
        // 此时 DServiceManager 构造内 registerService 失败会直接 exit(1)，
        // 导致整个测试进程退出且 gtest XML/gcov 数据丢失；
        // 名字被占时跳过（用例前提“进程启动时该名字未被注册”不成立）
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceBaseName))
            GTEST_SKIP() << "com.deepin.dataserver.Calendar already owned by another process";
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
