// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavsyncstatemachine.h"

DCalDavSyncStateMachine::DCalDavSyncStateMachine(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DCalDavSyncStateMachine::State>("DCalDavSyncStateMachine::State");
}

bool DCalDavSyncStateMachine::registerAccount(const QString &accountId)
{
    if (accountId.isEmpty() || m_accounts.contains(accountId)) {
        return false;
    }

    m_accounts.insert(accountId, AccountState());
    return true;
}

bool DCalDavSyncStateMachine::unregisterAccount(const QString &accountId)
{
    if (!m_accounts.contains(accountId)) {
        return false;
    }

    m_accounts.remove(accountId);
    return true;
}

bool DCalDavSyncStateMachine::requestSync(const QString &accountId, Trigger trigger)
{
    auto account = m_accounts.find(accountId);
    if (account == m_accounts.end() || trigger == NoTrigger) {
        return false;
    }

    if (account->state == Running) {
        return false;
    }

    account->pendingTriggers |= trigger;
    if (account->state != Queued) {
        account->state = Queued;
        m_queue.enqueue(accountId);
        emitStateChanged(accountId, Queued);
        emit syncRequested(accountId);
    }
    return true;
}

int DCalDavSyncStateMachine::requestSyncForAll(Trigger trigger)
{
    int count = 0;
    const QStringList accountIds = m_accounts.keys();
    for (const QString &accountId : accountIds) {
        if (requestSync(accountId, trigger)) {
            ++count;
        }
    }
    return count;
}

QString DCalDavSyncStateMachine::takeNextRunnableAccount()
{
    while (!m_queue.isEmpty()) {
        const QString accountId = m_queue.dequeue();
        auto account = m_accounts.find(accountId);
        if (account == m_accounts.end() || account->state != Queued) {
            continue;
        }

        account->state = Running;
        account->pendingTriggers = NoTrigger;
        emitStateChanged(accountId, Running);
        return accountId;
    }
    return QString();
}

QString DCalDavSyncStateMachine::takeRunnableAccount(const QString &accountId)
{
    if (accountId.isEmpty()) {
        return QString();
    }

    auto account = m_accounts.find(accountId);
    if (account == m_accounts.end() || account->state != Queued) {
        return QString();
    }

    m_queue.removeAll(accountId);
    account->state = Running;
    account->pendingTriggers = NoTrigger;
    emitStateChanged(accountId, Running);
    return accountId;
}

bool DCalDavSyncStateMachine::complete(const QString &accountId, bool success)
{
    auto account = m_accounts.find(accountId);
    if (account == m_accounts.end() || account->state != Running) {
        return false;
    }

    if (account->pendingTriggers != NoTrigger) {
        account->state = Queued;
        m_queue.enqueue(accountId);
        emitStateChanged(accountId, Queued);
        emit syncRequested(accountId);
        return true;
    }

    account->state = success ? Succeeded : Failed;
    emitStateChanged(accountId, account->state);
    return true;
}

DCalDavSyncStateMachine::State DCalDavSyncStateMachine::stateFor(const QString &accountId) const
{
    const auto account = m_accounts.constFind(accountId);
    return account == m_accounts.constEnd() ? Idle : account->state;
}

DCalDavSyncStateMachine::Triggers DCalDavSyncStateMachine::pendingTriggersFor(const QString &accountId) const
{
    const auto account = m_accounts.constFind(accountId);
    return account == m_accounts.constEnd() ? NoTrigger : account->pendingTriggers;
}

bool DCalDavSyncStateMachine::containsAccount(const QString &accountId) const
{
    return m_accounts.contains(accountId);
}

void DCalDavSyncStateMachine::emitStateChanged(const QString &accountId, State state)
{
    emit stateChanged(accountId, state);
}
