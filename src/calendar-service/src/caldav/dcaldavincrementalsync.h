// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVINCREMENTALSYNC_H
#define DCALDAVINCREMENTALSYNC_H

#include "dcaldavcalendarquery.h"
#include "dcaldaveventmappinginfo.h"
#include "dcaldaveventmapper.h"

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QSet>
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
        DCalDavEventMappingInfo::List existingMappings;
    };

    struct Result {
        bool success = false;
        bool usedFullRangeFallback = false;
        QString errorMessage;
        QString syncToken;
        DCalDavTransport::Response failureResponse;
        DCalDavCalendarQuery::RemoteEventList remoteEvents;
        DSchedule::List schedules;
    };

    typedef std::function<void(const Result &)> Callback;

    explicit DCalDavIncrementalSync(QObject *parent = nullptr);

    /**
     * @brief Fetches and reconciles one remote CalDAV calendar incrementally.
     * @param request Calendar endpoint, credentials, and persisted sync token.
     * @param callback Invoked exactly once with remote changes or a failure.
     */
    void start(const Request &request, const Callback &callback);
    void cancel();

private:
    void sendRequest(bool fullRange);
    void sendResourceListRequest(bool firstSync);
    void fetchNextResource();
    void requestCalendarDataBatch(const DCalDavCalendarQuery::ResourceList &resources);
    void fetchResourceByGet(const DCalDavCalendarQuery::ResourceList &resources, int index);
    bool appendResourceCalendarData(const DCalDavCalendarQuery::Resource &resource,
                                    const QString &calendarData, QString *errorMessage);
    void appendDeletedResources();
    void finish(bool success, const QString &errorMessage = QString());
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
};

#endif // DCALDAVINCREMENTALSYNC_H
