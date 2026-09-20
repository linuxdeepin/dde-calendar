// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVACCOUNTREGISTRAR_H
#define DCALDAVACCOUNTREGISTRAR_H

#include "dcaldavaccountinfo.h"
#include "dcaldaverrorcode.h"
#include "dcaldavaccountsync.h"
#include "dcaldavreadonlysync.h"

#include <QObject>
#include <QStringList>

#include <functional>

class DAccountDataBase;
class DAccountManagerDataBase;
class DCalDavSyncJobManager;

class DCalDavAccountRegistrar : public QObject
{
    Q_OBJECT
public:
    using CredentialReader = std::function<bool(const QString &credentialRef,
                                                 QString &password,
                                                 QString *errorMessage)>;

    struct Request {
        DCalDavAccountInfo account;
        DAccountDataBase *localDatabase = nullptr;
        DAccountManagerDataBase *accountManagerDatabase = nullptr;
        DCalDavSyncJobManager *jobManager = nullptr;
        CredentialReader credentialReader;
    };

    struct Result {
        bool success = false;
        QString errorMessage;
        DCalDavTransport::Response failureResponse;
        DCalDavErrorCode failureCode = DCalDavErrorCode::NoError;
        int calendarCount = 0;
    };

    typedef std::function<void(const Result &)> Callback;

    explicit DCalDavAccountRegistrar(QObject *parent = nullptr);
    ~DCalDavAccountRegistrar() override;

    void start(const Request &request, const Callback &callback);
    /**
     * @brief Cancels account discovery and registration.
     * @param notifyCallback Whether to complete the registration callback with a cancellation result.
     */
    void cancel(bool notifyCallback = true);

private:
    QString findScheduleTypeID(const QString &calendarID,
                               const DCalDavCalendarInfo::List &existing);
    /**
     * @brief Persists discovered collections and prepares their initial sync requests.
     *
     * Reuses existing calendar identities and sync tokens, creates or updates
     * local schedule types and permissions, removes missing collections, then
     * appends every readable collection to the account synchronization request.
     */
    bool persistCalendars(const DCalDavXmlReader::DiscoveryResult &discovery,
                          DCalDavAccountSync::Request &syncRequest,
                          QString *errorMessage);
    bool handleRemoteCalendarDeletion(const DCalDavCalendarInfo &calendar,
                                      QString *errorMessage);
    bool persistRemoteCalendarDeletionRecovery(const DCalDavCalendarInfo &calendar,
                                               const QStringList &typeIDs,
                                               QString *errorMessage);
    bool commitRemoteCalendarDeletion(const DCalDavCalendarInfo &calendar,
                                      const QStringList &typeIDs,
                                      const QStringList &scheduleIDs,
                                      QString *errorMessage);
    void finish(bool success, const QString &errorMessage = QString(),
                const DCalDavTransport::Response &failureResponse = DCalDavTransport::Response(),
                DCalDavErrorCode failureCode = DCalDavErrorCode::Unknown);

    DCalDavReadOnlySync m_discovery;
    Request m_request;
    Callback m_callback;
    QString m_password;
    bool m_running = false;
};

#endif // DCALDAVACCOUNTREGISTRAR_H
