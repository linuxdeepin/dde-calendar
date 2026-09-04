// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavdiscovery.h"

QList<QUrl> DCalDavDiscovery::discoveryCandidates(const QUrl &serverUrl)
{
    QList<QUrl> candidates;
    const QUrl normalizedServerUrl = DCalDavTransport::normalizeUrl(serverUrl);
    if (!normalizedServerUrl.isValid()) {
        return candidates;
    }

    QUrl wellKnown(normalizedServerUrl);
    wellKnown.setPath(QStringLiteral("/.well-known/caldav"));
    wellKnown.setQuery(QString());
    wellKnown.setFragment(QString());
    candidates.append(wellKnown);
    if (normalizedServerUrl != wellKnown) {
        candidates.append(normalizedServerUrl);
    }
    return candidates;
}

QUrl DCalDavDiscovery::resolveHref(const QUrl &baseUrl, const QString &href)
{
    const QUrl normalizedBaseUrl = DCalDavTransport::normalizeUrl(baseUrl);
    if (!normalizedBaseUrl.isValid()) {
        return QUrl();
    }
    const QUrl resolved = DCalDavTransport::normalizeUrl(normalizedBaseUrl.resolved(QUrl(href)));
    if (!resolved.isValid() || !DCalDavTransport::isSameOrigin(normalizedBaseUrl, resolved)) {
        return QUrl();
    }
    return resolved;
}

DCalDavTransport::Request DCalDavDiscovery::currentUserPrincipalRequest(const QUrl &url,
                                                                          const QString &username,
                                                                          const QString &password)
{
    DCalDavTransport::Request request;
    request.url = url;
    request.method = "PROPFIND";
    request.contentType = "application/xml; charset=utf-8";
    request.username = username;
    request.password = password;
    request.headers.insert("Depth", "0");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<d:propfind xmlns:d=\"DAV:\">"
                   "<d:prop><d:current-user-principal/><d:displayname/></d:prop>"
                   "</d:propfind>";
    return request;
}

DCalDavTransport::Request DCalDavDiscovery::calendarHomeSetRequest(const QUrl &url,
                                                                    const QString &username,
                                                                    const QString &password)
{
    DCalDavTransport::Request request;
    request.url = url;
    request.method = "PROPFIND";
    request.contentType = "application/xml; charset=utf-8";
    request.username = username;
    request.password = password;
    request.headers.insert("Depth", "0");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<d:propfind xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
                   "<d:prop><c:calendar-home-set/><d:displayname/></d:prop>"
                   "</d:propfind>";
    return request;
}

DCalDavTransport::Request DCalDavDiscovery::calendarCollectionsRequest(const QUrl &url,
                                                                        const QString &username,
                                                                        const QString &password)
{
    DCalDavTransport::Request request;
    request.url = url;
    request.method = "PROPFIND";
    request.contentType = "application/xml; charset=utf-8";
    request.username = username;
    request.password = password;
    request.headers.insert("Depth", "1");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<d:propfind xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\" "
                   "xmlns:ical=\"http://apple.com/ns/ical/\">"
                   "<d:prop><d:resourcetype/><d:displayname/><ical:calendar-color/>"
                   "<d:current-user-privilege-set/></d:prop>"
                   "</d:propfind>";
    return request;
}
