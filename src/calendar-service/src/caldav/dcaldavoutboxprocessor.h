// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVOUTBOXPROCESSOR_H
#define DCALDAVOUTBOXPROCESSOR_H

#include "dcaldaveventmappinginfo.h"
#include "dcaldavoutboxitem.h"
#include "dcaldavtransport.h"
#include "dcaldavvalidationerror.h"

#include <QObject>

#include <functional>

class DAccountDataBase;
class DAccountManagerDataBase;

class DCalDavOutboxProcessor : public QObject
{
    Q_OBJECT
public:
    struct Request {
        QString accountID;
        QString username;
        QString password;
        DAccountDataBase *localDatabase = nullptr;
        DAccountManagerDataBase *accountManagerDatabase = nullptr;
        bool forceRetry = false;
    };

    struct Result {
        bool success = false;
        QString errorMessage;
        DCalDavTransport::Response failureResponse;
        int processedCount = 0;
        int retryScheduledCount = 0;
        int conflictDiscardedCount = 0;
        DCalDavScheduleCreateError::Type createFailure = DCalDavScheduleCreateError::NoError;
        int permanentFailureCount = 0;
    };

    typedef std::function<void(const Result &)> Callback;

    explicit DCalDavOutboxProcessor(QObject *parent = nullptr);

    /**
     * @brief Sends due durable Outbox operations to the CalDAV server.
     * @param request Account credentials, databases, and retry policy.
     * @param callback Invoked exactly once after processing or cancellation.
     */
    void start(const Request &request, const Callback &callback);
    void cancel();

private:
    void processNext();
    void processMissingSchedule(const DCalDavOutboxItem &item,
                                const DCalDavEventMappingInfo &mapping);
    void repairCreateOperation(const DCalDavOutboxItem &item,
                               const DCalDavEventMappingInfo &mapping);
    void sendDeleteRequest(const DCalDavOutboxItem &item,
                           const DCalDavEventMappingInfo &mapping);
    void fetchWriteEtag(const DCalDavOutboxItem &item, const QUrl &resourceUrl);
    /**
     * @brief Processes one pending local-to-remote operation.
     * @param item Outbox item containing the operation and retry metadata.
     *
     * Resolves the target calendar and resource URL, performs the required
     * CalDAV request, and advances or records failure through the processor.
     */
    void processItem(const DCalDavOutboxItem &item);
    void sendWriteRequest(const DCalDavOutboxItem &item, const QUrl &resourceUrl,
                          const QByteArray &etag);
    void handleWriteResponse(const DCalDavOutboxItem &item, const QUrl &resourceUrl,
                             const DCalDavTransport::Response &response);
    void fetchEtag(const DCalDavOutboxItem &item, const QUrl &resourceUrl);
    void fetchConflictSnapshot(const DCalDavOutboxItem &item, const QUrl &resourceUrl);
    void completeSuccess(const DCalDavOutboxItem &item, const QUrl &resourceUrl,
                         const QByteArray &etag);
    void recordFailure(const DCalDavOutboxItem &item, const DCalDavTransport::Response &response);
    void finish(bool success, const QString &errorMessage = QString());
    bool isCurrentItem(const DCalDavOutboxItem &item) const;
    bool processCurrentItemOrAdvance(const DCalDavOutboxItem &item);

    DCalDavTransport m_transport;
    Request m_request;
    Callback m_callback;
    DCalDavOutboxItem::List m_items;
    Result m_result;
    int m_itemIndex = 0;
    bool m_running = false;
};

#endif // DCALDAVOUTBOXPROCESSOR_H
