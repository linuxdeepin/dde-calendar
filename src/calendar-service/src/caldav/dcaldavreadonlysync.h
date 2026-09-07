// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVREADONLYSYNC_H
#define DCALDAVREADONLYSYNC_H

#include "dcaldavcalendarquery.h"
#include "dcaldaverrorcode.h"
#include "dcaldaveventmapper.h"
#include "dcaldavxmlreader.h"
#include "dcaldavvalidationerror.h"

#include <QObject>
#include <QUrl>

#include <functional>

class DCalDavReadOnlySync : public QObject
{
public:
    struct Request {
        QUrl serverUrl;
        QString username;
        QString password;
    };

    struct Result {
        bool success = false;
        DCalDavValidationError::Type validationError = DCalDavValidationError::NoError;
        DCalDavErrorCode failureCode = DCalDavErrorCode::NoError;
        QString errorMessage;
        DCalDavTransport::Response failureResponse;
        DCalDavXmlReader::DiscoveryResult discovery;
        DCalDavCalendarQuery::RemoteEventList remoteEvents;
        DSchedule::List schedules;
    };

    typedef std::function<void(const Result &)> Callback;

    explicit DCalDavReadOnlySync(QObject *parent = nullptr);

    void start(const Request &request, const Callback &callback);

private:
    void sendPrincipalRequest();
    void sendCalendarHomeRequest(const QUrl &principalUrl);
    void sendCollectionsRequest(const QUrl &homeUrl);
    void sendCalendarQuery();
    void finish(bool success, const QString &errorMessage = QString(),
                DCalDavValidationError::Type validationError = DCalDavValidationError::NoError);
    void tryNextPrincipalCandidate(
        const QString &errorMessage,
        DCalDavValidationError::Type validationError = DCalDavValidationError::NoError);

    DCalDavTransport m_transport;
    Request m_request;
    Callback m_callback;
    Result m_result;
    QList<QUrl> m_candidates;
    int m_candidateIndex = 0;
    int m_collectionIndex = 0;
    bool m_running = false;
};

#endif // DCALDAVREADONLYSYNC_H
