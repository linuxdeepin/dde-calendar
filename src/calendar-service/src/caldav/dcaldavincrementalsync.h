// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVINCREMENTALSYNC_H
#define DCALDAVINCREMENTALSYNC_H

#include "dcaldavcalendarquery.h"
#include "dcaldaverrorcode.h"
#include "dcaldaveventmappinginfo.h"
#include "dcaldaveventmapper.h"

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QSet>
#include <QStringList>
#include <QUrl>

#include <functional>

class DCalDavIncrementalSync : public QObject
{
    Q_OBJECT
public:
    struct Request {
        QUrl calendarUrl;
        QString username;
        QString password;
        QString syncToken;
        QDateTime referenceTime;
        bool initialSyncCompleted = false;
        QString calendarId;
        DCalDavEventMappingInfo::List existingMappings;
        DCalDavSkippedResource::List existingSkippedResources;
    };

    struct Result {
        bool success = false;
        bool usedFullRangeFallback = false;
        QString errorMessage;
        QString syncToken;
        DCalDavTransport::Response failureResponse;
        DCalDavErrorCode failureCode = DCalDavErrorCode::NoError;
        DCalDavCalendarQuery::RemoteEventList remoteEvents;
        DSchedule::List schedules;
        DCalDavSkippedResource::List skippedResources;
        QStringList clearedSkippedResourceHrefs;
    };

    typedef std::function<void(const Result &)> Callback;

    explicit DCalDavIncrementalSync(QObject *parent = nullptr);

    /**
     * @brief Fetches and reconciles one remote CalDAV calendar incrementally.
     * @param request Calendar endpoint, credentials, and persisted sync token.
     * @param callback Invoked exactly once with remote changes or a failure.
     */
    void start(const Request &request, const Callback &callback);
    /**
     * @brief Cancels the current request.
     * @param notifyCallback Whether to complete the request callback with a cancellation result.
     */
    void cancel(bool notifyCallback = true);

private:
    void sendRequest(bool fullRange);
    void sendResourceListRequest(bool firstSync, bool inventoryOnly = false);
    void fetchNextResource();
    void requestCalendarDataBatch(const DCalDavCalendarQuery::ResourceList &resources);
    void fetchResourceByGet(const DCalDavCalendarQuery::ResourceList &resources, int index);
    void appendResourceCalendarData(const DCalDavCalendarQuery::Resource &resource,
                                    const QString &calendarData);
    void appendSkippedResource(const DCalDavCalendarQuery::RemoteEvent &event,
                               const QString &reason);
    void appendRemoteEvent(DCalDavCalendarQuery::RemoteEvent event);
    void appendDeletedResource(const DCalDavCalendarQuery::Resource &resource);
    void appendDeletedResources();
    void finish(bool success, const QString &errorMessage = QString(),
                DCalDavErrorCode failureCode = DCalDavErrorCode::NoError);
    bool shouldFallbackToFullRange(const DCalDavTransport::Response &response) const;

    DCalDavTransport m_transport;
    Request m_request;
    Callback m_callback;
    Result m_result;
    DCalDavCalendarQuery::ResourceList m_pendingResources;
    QHash<QString, DCalDavEventMappingInfo> m_mappingByHref;
    QSet<QString> m_remoteHrefs;
    int m_resourceIndex = 0;
    bool m_running = false;
    bool m_fallbackAttempted = false;
    bool m_resourceListHasEventFilter = false;
    bool m_syncCollectionMode = false;
    bool m_resourceInventoryOnly = false;
    bool m_hasCompleteRemoteResourceList = false;
};

#endif // DCALDAVINCREMENTALSYNC_H
