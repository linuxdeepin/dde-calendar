// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVSYNCJOBMANAGER_H
#define DCALDAVSYNCJOBMANAGER_H

#include "dcaldavaccountsync.h"
#include "dcaldavsyncstatemachine.h"

#include <QHash>
#include <QObject>

class DCalDavSyncJobManager : public QObject
{
    Q_OBJECT
public:
    explicit DCalDavSyncJobManager(QObject *parent = nullptr);

    bool registerAccount(const DCalDavAccountSync::Request &request);
    bool unregisterAccount(const QString &accountID);
    bool cancelAccount(const QString &accountID);
    bool requestSync(const QString &accountID, DCalDavSyncStateMachine::Trigger trigger);
    int requestSyncForAll(DCalDavSyncStateMachine::Trigger trigger);

    DCalDavSyncStateMachine::State stateFor(const QString &accountID) const;
    bool containsAccount(const QString &accountID) const;

signals:
    void accountSyncStateChanged(const QString &accountID, DCalDavSyncStateMachine::State state);
    void accountSyncFinished(const QString &accountID, bool success, const QString &errorMessage,
                             const DCalDavTransport::Response &failureResponse);
    void accountSyncDataChanged(const QString &accountID);
    void accountScheduleCreateFailed(const QString &accountID, int createFailure);

private:
    void processNext(const QString &requestedAccountID);

    DCalDavSyncStateMachine m_stateMachine;
    QHash<QString, DCalDavAccountSync::Request> m_requests;
    QHash<QString, DCalDavAccountSync *> m_jobs;
};

#endif // DCALDAVSYNCJOBMANAGER_H
