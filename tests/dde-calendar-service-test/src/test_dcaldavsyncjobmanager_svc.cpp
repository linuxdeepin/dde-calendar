// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavsyncjobmanager.h"
#include "dcaldavaccountsync.h"

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTest>

namespace {

DCalDavAccountSync::Request makeRequest(const QString &accountID)
{
    DCalDavAccountSync::Request request;
    request.accountID = accountID;
    return request;
}

} // namespace

class DCalDavSyncJobManagerTest : public ::testing::Test
{
protected:
    DCalDavSyncJobManager manager;

    // Disconnects syncRequested→processNext so requestSync can be inspected
    // without the real DCalDavAccountSync job firing synchronously.
    void disconnectProcessNext()
    {
        manager.disconnect(&manager.m_stateMachine,
                           &DCalDavSyncStateMachine::syncRequested,
                           &manager, &DCalDavSyncJobManager::processNext);
    }
};

TEST_F(DCalDavSyncJobManagerTest, ConstructorWiresStateChangedSignal)
{
    disconnectProcessNext();
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    QSignalSpy stateSpy(&manager, &DCalDavSyncJobManager::accountSyncStateChanged);
    ASSERT_TRUE(stateSpy.isValid());

    ASSERT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_EQ(1, stateSpy.count());
    const auto args = stateSpy.takeFirst();
    EXPECT_EQ(QStringLiteral("a1"), args.at(0).toString());
    EXPECT_EQ(static_cast<int>(DCalDavSyncStateMachine::Queued), args.at(1).toInt());
}

TEST_F(DCalDavSyncJobManagerTest, RegisterAccountRejectsEmptyId)
{
    EXPECT_FALSE(manager.registerAccount(makeRequest(QString())));
}

TEST_F(DCalDavSyncJobManagerTest, RegisterAccountAddsNewAccount)
{
    EXPECT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    EXPECT_TRUE(manager.containsAccount(QStringLiteral("a1")));
    EXPECT_EQ(DCalDavSyncStateMachine::Idle, manager.stateFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncJobManagerTest, RegisterAccountUpdatesRequestForExistingNonRunningAccount)
{
    auto request = makeRequest(QStringLiteral("a1"));
    request.username = QStringLiteral("user1");
    ASSERT_TRUE(manager.registerAccount(request));

    auto updated = makeRequest(QStringLiteral("a1"));
    updated.username = QStringLiteral("user2");
    EXPECT_TRUE(manager.registerAccount(updated));
    EXPECT_EQ(QStringLiteral("user2"),
              manager.m_requests.value(QStringLiteral("a1")).username);
}

TEST_F(DCalDavSyncJobManagerTest, RegisterAccountRejectsExistingRunningAccount)
{
    disconnectProcessNext();
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    ASSERT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_FALSE(manager.m_stateMachine.takeNextRunnableAccount().isEmpty());
    ASSERT_EQ(DCalDavSyncStateMachine::Running, manager.stateFor(QStringLiteral("a1")));

    EXPECT_FALSE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
}

TEST_F(DCalDavSyncJobManagerTest, UnregisterAccountCancelsAndRemovesAccount)
{
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    EXPECT_TRUE(manager.unregisterAccount(QStringLiteral("a1")));
    EXPECT_FALSE(manager.containsAccount(QStringLiteral("a1")));
    EXPECT_FALSE(manager.m_requests.contains(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncJobManagerTest, UnregisterAccountReturnsTrueForUnknownAccount)
{
    EXPECT_TRUE(manager.unregisterAccount(QStringLiteral("nope")));
}

TEST_F(DCalDavSyncJobManagerTest, CancelAccountReturnsTrueForUnknownAccount)
{
    EXPECT_TRUE(manager.cancelAccount(QStringLiteral("nope")));
}

TEST_F(DCalDavSyncJobManagerTest, CancelAccountCancelsJobAndRemovesRequest)
{
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    ASSERT_TRUE(manager.m_jobs.contains(QStringLiteral("a1")));

    EXPECT_TRUE(manager.cancelAccount(QStringLiteral("a1")));
    EXPECT_FALSE(manager.containsAccount(QStringLiteral("a1")));
    EXPECT_FALSE(manager.m_requests.contains(QStringLiteral("a1")));
    EXPECT_FALSE(manager.m_jobs.contains(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncJobManagerTest, RequestSyncSetsForceOutboxRetryForManualTrigger)
{
    disconnectProcessNext();
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    ASSERT_FALSE(manager.m_requests.value(QStringLiteral("a1")).forceOutboxRetry);

    EXPECT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    EXPECT_TRUE(manager.m_requests.value(QStringLiteral("a1")).forceOutboxRetry);
    EXPECT_EQ(DCalDavSyncStateMachine::Queued, manager.stateFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncJobManagerTest, RequestSyncDoesNotSetForceOutboxRetryForNonManualTrigger)
{
    disconnectProcessNext();
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));

    EXPECT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_FALSE(manager.m_requests.value(QStringLiteral("a1")).forceOutboxRetry);
    EXPECT_EQ(DCalDavSyncStateMachine::Queued, manager.stateFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncJobManagerTest, RequestSyncFailsForUnknownAccount)
{
    EXPECT_FALSE(manager.requestSync(QStringLiteral("nope"),
                                     DCalDavSyncStateMachine::ManualTrigger));
}

TEST_F(DCalDavSyncJobManagerTest, RequestSyncForAllQueuesAllAccounts)
{
    disconnectProcessNext();
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a2"))));

    EXPECT_EQ(2, manager.requestSyncForAll(DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued, manager.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued, manager.stateFor(QStringLiteral("a2")));
}

TEST_F(DCalDavSyncJobManagerTest, StateForReturnsIdleForUnknownAccount)
{
    EXPECT_EQ(DCalDavSyncStateMachine::Idle, manager.stateFor(QStringLiteral("nope")));
}

TEST_F(DCalDavSyncJobManagerTest, StateForReturnsCurrentStateForKnownAccount)
{
    disconnectProcessNext();
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    EXPECT_EQ(DCalDavSyncStateMachine::Idle, manager.stateFor(QStringLiteral("a1")));
    ASSERT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued, manager.stateFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncJobManagerTest, ContainsAccountReportsPresence)
{
    EXPECT_FALSE(manager.containsAccount(QStringLiteral("nope")));
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    EXPECT_TRUE(manager.containsAccount(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncJobManagerTest, ProcessNextReturnsEarlyWhenNoRunnableAccount)
{
    manager.processNext(QString());
    SUCCEED();
}

TEST_F(DCalDavSyncJobManagerTest, ProcessNextCompletesFailureWhenRequestMissing)
{
    disconnectProcessNext();
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    ASSERT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_EQ(DCalDavSyncStateMachine::Queued, manager.stateFor(QStringLiteral("a1")));
    manager.m_requests.remove(QStringLiteral("a1"));

    QSignalSpy finishedSpy(&manager, &DCalDavSyncJobManager::accountSyncFinished);
    manager.processNext(QStringLiteral("a1"));
    QCoreApplication::processEvents();

    EXPECT_EQ(DCalDavSyncStateMachine::Failed,
              manager.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(0, finishedSpy.count());
}

TEST_F(DCalDavSyncJobManagerTest, ProcessNextTriggeredBySyncRequestedSignal)
{
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    manager.m_jobs.remove(QStringLiteral("a1"));

    QSignalSpy finishedSpy(&manager, &DCalDavSyncJobManager::accountSyncFinished);
    ASSERT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    QCoreApplication::processEvents();

    EXPECT_EQ(DCalDavSyncStateMachine::Failed,
              manager.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(0, finishedSpy.count());
}

TEST_F(DCalDavSyncJobManagerTest, ProcessNextInvokesCallbackWhenJobStartFails)
{
    // With an incomplete request (empty username, null DB pointers),
    // DCalDavAccountSync::start() calls finish(false, ...) synchronously,
    // which invokes the callback lambda registered by processNext.
    // This exercises the result-callback lambda and the nested
    // QTimer::singleShot re-queue lambda.
    ASSERT_TRUE(manager.registerAccount(makeRequest(QStringLiteral("a1"))));
    ASSERT_TRUE(manager.m_jobs.contains(QStringLiteral("a1")));

    QSignalSpy finishedSpy(&manager, &DCalDavSyncJobManager::accountSyncFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    // requestSync emits syncRequested -> processNext -> job->start(incomplete)
    // -> finish(false) -> callback lambda runs synchronously.
    ASSERT_TRUE(manager.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));

    // Flush the QTimer::singleShot(0) queued by the callback lambda.
    QCoreApplication::processEvents();

    EXPECT_EQ(DCalDavSyncStateMachine::Failed,
              manager.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(1, finishedSpy.count());
    const auto args = finishedSpy.takeFirst();
    EXPECT_EQ(QStringLiteral("a1"), args.at(0).toString());
    EXPECT_FALSE(args.at(1).toBool());
}
