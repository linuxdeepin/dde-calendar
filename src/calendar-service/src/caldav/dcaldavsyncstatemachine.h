// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVSYNCSTATEMACHINE_H
#define DCALDAVSYNCSTATEMACHINE_H

#include <QObject>
#include <QHash>
#include <QMetaType>
#include <QQueue>
#include <QString>

class DCalDavSyncStateMachine : public QObject
{
    Q_OBJECT
public:
    enum State {
        Idle,
        Queued,
        Running,
        Succeeded,
        Failed,
    };
    Q_ENUM(State)

    enum Trigger {
        NoTrigger = 0x0,
        StartupTrigger = 0x1,
        ForegroundTrigger = 0x2,
        DailyTrigger = 0x4,
        ManualTrigger = 0x8,
        NetworkRestoredTrigger = 0x10,
        RetryTrigger = 0x20,
        LocalChangeTrigger = 0x40,
    };
    Q_DECLARE_FLAGS(Triggers, Trigger)

    explicit DCalDavSyncStateMachine(QObject *parent = nullptr);

    bool registerAccount(const QString &accountId);
    bool unregisterAccount(const QString &accountId);
    bool requestSync(const QString &accountId, Trigger trigger);
    int requestSyncForAll(Trigger trigger);

    QString takeNextRunnableAccount();
    QString takeRunnableAccount(const QString &accountId);
    bool complete(const QString &accountId, bool success);

    State stateFor(const QString &accountId) const;
    Triggers pendingTriggersFor(const QString &accountId) const;
    bool containsAccount(const QString &accountId) const;

signals:
    void stateChanged(const QString &accountId, DCalDavSyncStateMachine::State state);
    void syncRequested(const QString &accountId);

private:
    struct AccountState {
        State state = Idle;
        Triggers pendingTriggers = NoTrigger;
    };

    void emitStateChanged(const QString &accountId, State state);

    QHash<QString, AccountState> m_accounts;
    QQueue<QString> m_queue;
};

Q_DECLARE_METATYPE(DCalDavSyncStateMachine::State)
Q_DECLARE_OPERATORS_FOR_FLAGS(DCalDavSyncStateMachine::Triggers)

#endif // DCALDAVSYNCSTATEMACHINE_H
