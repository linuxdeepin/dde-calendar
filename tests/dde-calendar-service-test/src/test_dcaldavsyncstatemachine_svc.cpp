// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavsyncstatemachine.h"

#include <gtest/gtest.h>
#include <QSignalSpy>

class DCalDavSyncStateMachineTest : public ::testing::Test
{
protected:
    DCalDavSyncStateMachine machine;
};

TEST_F(DCalDavSyncStateMachineTest, RegisterAccountRejectsEmptyId)
{
    EXPECT_FALSE(machine.registerAccount(QString()));
}

TEST_F(DCalDavSyncStateMachineTest, RegisterAccountRejectsDuplicate)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    EXPECT_FALSE(machine.registerAccount(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, RegisterAccountSucceedsForNewId)
{
    EXPECT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    EXPECT_TRUE(machine.containsAccount(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, UnregisterAccountReturnsFalseForUnknown)
{
    EXPECT_FALSE(machine.unregisterAccount(QStringLiteral("nope")));
}

TEST_F(DCalDavSyncStateMachineTest, UnregisterAccountRemovesKnownAccount)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    EXPECT_TRUE(machine.unregisterAccount(QStringLiteral("a1")));
    EXPECT_FALSE(machine.containsAccount(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, RequestSyncFailsForUnknownAccount)
{
    EXPECT_FALSE(machine.requestSync(QStringLiteral("nope"),
                                     DCalDavSyncStateMachine::ManualTrigger));
}

TEST_F(DCalDavSyncStateMachineTest, RequestSyncFailsForNoTrigger)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    EXPECT_FALSE(machine.requestSync(QStringLiteral("a1"),
                                     DCalDavSyncStateMachine::NoTrigger));
}

TEST_F(DCalDavSyncStateMachineTest, RequestSyncQueuesIdleAccountAndEmitsSignals)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    QSignalSpy syncSpy(&machine, &DCalDavSyncStateMachine::syncRequested);
    QSignalSpy stateSpy(&machine, &DCalDavSyncStateMachine::stateChanged);

    EXPECT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a1")));
    ASSERT_EQ(1, syncSpy.count());
    EXPECT_EQ(QStringLiteral("a1"), syncSpy.takeFirst().at(0).toString());
    ASSERT_EQ(1, stateSpy.count());
    EXPECT_EQ(static_cast<int>(DCalDavSyncStateMachine::Queued),
              stateSpy.takeFirst().at(1).toInt());
}

TEST_F(DCalDavSyncStateMachineTest, RequestSyncDoesNotRequeueAlreadyQueuedAccount)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    QSignalSpy syncSpy(&machine, &DCalDavSyncStateMachine::syncRequested);

    EXPECT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(0, syncSpy.count());
    EXPECT_EQ(DCalDavSyncStateMachine::DailyTrigger | DCalDavSyncStateMachine::ManualTrigger,
              machine.pendingTriggersFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, RequestSyncAccumulatesTriggersWhileRunning)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_FALSE(machine.takeNextRunnableAccount().isEmpty());
    ASSERT_EQ(DCalDavSyncStateMachine::Running,
              machine.stateFor(QStringLiteral("a1")));
    QSignalSpy syncSpy(&machine, &DCalDavSyncStateMachine::syncRequested);

    EXPECT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Running,
              machine.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(0, syncSpy.count());
    EXPECT_EQ(DCalDavSyncStateMachine::DailyTrigger,
              machine.pendingTriggersFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, RequestSyncForAllQueuesAllAccounts)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a2")));
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a3")));

    EXPECT_EQ(3, machine.requestSyncForAll(DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a2")));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a3")));
}

TEST_F(DCalDavSyncStateMachineTest, RequestSyncForAllAccumulatesTriggersForRunningAccount)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a2")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_FALSE(machine.takeNextRunnableAccount().isEmpty());

    // Running accounts return true (trigger accumulated), so both are counted.
    EXPECT_EQ(2, machine.requestSyncForAll(DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Running,
              machine.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(DCalDavSyncStateMachine::DailyTrigger,
              machine.pendingTriggersFor(QStringLiteral("a1")));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a2")));
}

TEST_F(DCalDavSyncStateMachineTest, TakeNextRunnableAccountReturnsEmptyWhenQueueExhausted)
{
    EXPECT_TRUE(machine.takeNextRunnableAccount().isEmpty());
}

TEST_F(DCalDavSyncStateMachineTest, TakeNextRunnableAccountTransitionsToRunning)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    QSignalSpy stateSpy(&machine, &DCalDavSyncStateMachine::stateChanged);

    EXPECT_EQ(QStringLiteral("a1"), machine.takeNextRunnableAccount());
    EXPECT_EQ(DCalDavSyncStateMachine::Running,
              machine.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(DCalDavSyncStateMachine::NoTrigger,
              machine.pendingTriggersFor(QStringLiteral("a1")));
    ASSERT_EQ(1, stateSpy.count());
    EXPECT_EQ(static_cast<int>(DCalDavSyncStateMachine::Running),
              stateSpy.takeFirst().at(1).toInt());
}

TEST_F(DCalDavSyncStateMachineTest, TakeNextRunnableAccountSkipsNonQueuedEntries)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a2")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a2"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_TRUE(machine.unregisterAccount(QStringLiteral("a1")));

    EXPECT_EQ(QStringLiteral("a2"), machine.takeNextRunnableAccount());
}

TEST_F(DCalDavSyncStateMachineTest, TakeRunnableAccountReturnsEmptyForEmptyId)
{
    EXPECT_TRUE(machine.takeRunnableAccount(QString()).isEmpty());
}

TEST_F(DCalDavSyncStateMachineTest, TakeRunnableAccountReturnsEmptyForUnknownAccount)
{
    EXPECT_TRUE(machine.takeRunnableAccount(QStringLiteral("nope")).isEmpty());
}

TEST_F(DCalDavSyncStateMachineTest, TakeRunnableAccountReturnsEmptyForNonQueuedAccount)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    EXPECT_TRUE(machine.takeRunnableAccount(QStringLiteral("a1")).isEmpty());
}

TEST_F(DCalDavSyncStateMachineTest, TakeRunnableAccountTransitionsToRunningAndDequeues)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a2")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a2"),
                                    DCalDavSyncStateMachine::ManualTrigger));

    EXPECT_EQ(QStringLiteral("a2"), machine.takeRunnableAccount(QStringLiteral("a2")));
    EXPECT_EQ(DCalDavSyncStateMachine::Running,
              machine.stateFor(QStringLiteral("a2")));
    EXPECT_EQ(QStringLiteral("a1"), machine.takeNextRunnableAccount());
}

TEST_F(DCalDavSyncStateMachineTest, CompleteReturnsFalseForUnknownAccount)
{
    EXPECT_FALSE(machine.complete(QStringLiteral("nope"), true));
}

TEST_F(DCalDavSyncStateMachineTest, CompleteReturnsFalseForNonRunningAccount)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    EXPECT_FALSE(machine.complete(QStringLiteral("a1"), true));
}

TEST_F(DCalDavSyncStateMachineTest, CompleteRequeuesWhenPendingTriggersExist)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_FALSE(machine.takeNextRunnableAccount().isEmpty());
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::DailyTrigger));
    QSignalSpy syncSpy(&machine, &DCalDavSyncStateMachine::syncRequested);
    QSignalSpy stateSpy(&machine, &DCalDavSyncStateMachine::stateChanged);

    EXPECT_TRUE(machine.complete(QStringLiteral("a1"), true));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a1")));
    EXPECT_EQ(1, syncSpy.count());
    ASSERT_EQ(1, stateSpy.count());
    EXPECT_EQ(static_cast<int>(DCalDavSyncStateMachine::Queued),
              stateSpy.takeFirst().at(1).toInt());
}

TEST_F(DCalDavSyncStateMachineTest, CompleteSucceedsWhenNoPendingTriggers)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_FALSE(machine.takeNextRunnableAccount().isEmpty());
    QSignalSpy stateSpy(&machine, &DCalDavSyncStateMachine::stateChanged);

    EXPECT_TRUE(machine.complete(QStringLiteral("a1"), true));
    EXPECT_EQ(DCalDavSyncStateMachine::Succeeded,
              machine.stateFor(QStringLiteral("a1")));
    ASSERT_EQ(1, stateSpy.count());
    EXPECT_EQ(static_cast<int>(DCalDavSyncStateMachine::Succeeded),
              stateSpy.takeFirst().at(1).toInt());
}

TEST_F(DCalDavSyncStateMachineTest, CompleteFailsWhenNoPendingTriggers)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_FALSE(machine.takeNextRunnableAccount().isEmpty());

    EXPECT_TRUE(machine.complete(QStringLiteral("a1"), false));
    EXPECT_EQ(DCalDavSyncStateMachine::Failed,
              machine.stateFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, StateForReturnsIdleForUnknownAccount)
{
    EXPECT_EQ(DCalDavSyncStateMachine::Idle,
              machine.stateFor(QStringLiteral("nope")));
}

TEST_F(DCalDavSyncStateMachineTest, StateForReturnsCurrentStateForKnownAccount)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    EXPECT_EQ(DCalDavSyncStateMachine::Idle,
              machine.stateFor(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              machine.stateFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, PendingTriggersForReturnsNoTriggerForUnknownAccount)
{
    EXPECT_EQ(DCalDavSyncStateMachine::NoTrigger,
              machine.pendingTriggersFor(QStringLiteral("nope")));
}

TEST_F(DCalDavSyncStateMachineTest, PendingTriggersForReturnsAccumulatedTriggers)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::ManualTrigger));
    ASSERT_TRUE(machine.requestSync(QStringLiteral("a1"),
                                    DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::ManualTrigger | DCalDavSyncStateMachine::DailyTrigger,
              machine.pendingTriggersFor(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, ContainsAccountReportsPresence)
{
    EXPECT_FALSE(machine.containsAccount(QStringLiteral("nope")));
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    EXPECT_TRUE(machine.containsAccount(QStringLiteral("a1")));
}

TEST_F(DCalDavSyncStateMachineTest, EmitStateChangedEmitsStateChangedSignal)
{
    ASSERT_TRUE(machine.registerAccount(QStringLiteral("a1")));
    QSignalSpy stateSpy(&machine, &DCalDavSyncStateMachine::stateChanged);
    machine.emitStateChanged(QStringLiteral("a1"), DCalDavSyncStateMachine::Failed);
    ASSERT_EQ(1, stateSpy.count());
    const auto args = stateSpy.takeFirst();
    EXPECT_EQ(QStringLiteral("a1"), args.at(0).toString());
    EXPECT_EQ(static_cast<int>(DCalDavSyncStateMachine::Failed), args.at(1).toInt());
}
