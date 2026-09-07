// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavsyncjobmanager.h"

#include "commondef.h"

#include <QTimer>

DCalDavSyncJobManager::DCalDavSyncJobManager(QObject *parent)
    : QObject(parent)
    , m_stateMachine()
{
    connect(&m_stateMachine, &DCalDavSyncStateMachine::syncRequested,
            this, &DCalDavSyncJobManager::processNext);
    connect(&m_stateMachine, &DCalDavSyncStateMachine::stateChanged,
            this, &DCalDavSyncJobManager::accountSyncStateChanged);
}

bool DCalDavSyncJobManager::registerAccount(const DCalDavAccountSync::Request &request)
{
    if (request.accountID.isEmpty()) {
        return false;
    }

    if (m_stateMachine.containsAccount(request.accountID)) {
        if (m_stateMachine.stateFor(request.accountID) == DCalDavSyncStateMachine::Running) {
            return false;
        }
        m_requests.insert(request.accountID, request);
        return true;
    }

    if (!m_stateMachine.registerAccount(request.accountID)) {
        return false;
    }
    m_requests.insert(request.accountID, request);
    m_jobs.insert(request.accountID, new DCalDavAccountSync(this));
    return true;
}

bool DCalDavSyncJobManager::unregisterAccount(const QString &accountID)
{
    return cancelAccount(accountID);
}

bool DCalDavSyncJobManager::cancelAccount(const QString &accountID)
{
    if (!m_stateMachine.containsAccount(accountID)) {
        return true;
    }
    if (DCalDavAccountSync *job = m_jobs.take(accountID)) {
        job->cancel(false);
        job->deleteLater();
    }
    if (!m_stateMachine.unregisterAccount(accountID)) {
        return false;
    }
    m_requests.remove(accountID);
    return true;
}

bool DCalDavSyncJobManager::requestSync(const QString &accountID, DCalDavSyncStateMachine::Trigger trigger)
{
    if (trigger == DCalDavSyncStateMachine::ManualTrigger
        && m_requests.contains(accountID)) {
        m_requests[accountID].forceOutboxRetry = true;
    }
    return m_stateMachine.requestSync(accountID, trigger);
}

int DCalDavSyncJobManager::requestSyncForAll(DCalDavSyncStateMachine::Trigger trigger)
{
    return m_stateMachine.requestSyncForAll(trigger);
}

DCalDavSyncStateMachine::State DCalDavSyncJobManager::stateFor(const QString &accountID) const
{
    return m_stateMachine.stateFor(accountID);
}

bool DCalDavSyncJobManager::containsAccount(const QString &accountID) const
{
    return m_stateMachine.containsAccount(accountID);
}

void DCalDavSyncJobManager::processNext(const QString &requestedAccountID)
{
    const QString accountID = requestedAccountID.isEmpty()
        ? m_stateMachine.takeNextRunnableAccount()
        : m_stateMachine.takeRunnableAccount(requestedAccountID);
    if (accountID.isEmpty()) {
        return;
    }

    qCDebug(ServiceLogger) << "CalDAV sync job selected account:" << accountID
                              << "requested account:" << requestedAccountID;
    if (!m_requests.contains(accountID) || !m_jobs.contains(accountID)) {
        m_stateMachine.complete(accountID, false);
        return;
    }

    DCalDavAccountSync *job = m_jobs.value(accountID);
    const DCalDavAccountSync::Request request = m_requests.value(accountID);
    m_requests[accountID].forceOutboxRetry = false;
    job->start(request, [this, accountID](const DCalDavAccountSync::Result &result) {
        m_stateMachine.complete(accountID, result.success);
        if (result.success
            && (result.createdCount > 0 || result.updatedCount > 0 || result.deletedCount > 0)) {
            emit accountSyncDataChanged(accountID);
        }
        if (result.createFailure != DCalDavScheduleCreateError::NoError) {
            emit accountScheduleCreateFailed(accountID, static_cast<int>(result.createFailure));
        }
        emit accountSyncFinished(accountID, result.success, result.errorMessage,
                                  result.failureResponse);
        QTimer::singleShot(0, this, [this, accountID]() {
            processNext(accountID);
        });
    });
}
