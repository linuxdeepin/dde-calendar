// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVCALENDARQUERY_H
#define DCALDAVCALENDARQUERY_H

#include "dcaldavtransport.h"

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

class DCalDavCalendarQuery
{
public:
    struct RemoteEvent {
        QString href;
        QString etag;
        QString uid;
        QString calendarData;
        QString contentType;
        bool deleted = false;
        bool hasSuccessfulStatus = false;
    };

    struct Resource {
        QString href;
        QString etag;
        QString contentType;
        QString calendarData;
        bool deleted = false;
        bool hasSuccessfulStatus = false;
    };

    typedef QVector<RemoteEvent> RemoteEventList;
    typedef QVector<Resource> ResourceList;

    static DCalDavTransport::Request firstSyncRequest(const QUrl &calendarUrl, const QString &username,
                                                       const QString &password,
                                                       const QDateTime &referenceTime);
    static DCalDavTransport::Request incrementalSyncRequest(const QUrl &calendarUrl, const QString &username,
                                                            const QString &password, const QString &syncToken);
    static DCalDavTransport::Request resourceListRequest(const QUrl &calendarUrl, const QString &username,
                                                         const QString &password);
    static DCalDavTransport::Request resourceGetRequest(const QUrl &resourceUrl, const QString &username,
                                                        const QString &password);
    static DCalDavTransport::Request resourceMultiGetRequest(const QUrl &calendarUrl,
                                                             const QString &username,
                                                             const QString &password,
                                                             const QStringList &resourceHrefs);
    static bool parseResponse(const QByteArray &xml, RemoteEventList &events,
                              QString *errorMessage = nullptr);
    static bool parseResponseWithSyncToken(const QByteArray &xml, RemoteEventList &events,
                                           QString *syncToken, QString *errorMessage = nullptr);
    static bool parseResourceList(const QByteArray &xml, ResourceList &resources,
                                  QString *errorMessage = nullptr);
};

#endif // DCALDAVCALENDARQUERY_H
