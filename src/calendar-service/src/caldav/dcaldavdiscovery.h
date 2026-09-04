// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVDISCOVERY_H
#define DCALDAVDISCOVERY_H

#include "dcaldavtransport.h"

#include <QList>

class DCalDavDiscovery
{
public:
    static QList<QUrl> discoveryCandidates(const QUrl &serverUrl);
    static QUrl resolveHref(const QUrl &baseUrl, const QString &href);
    static DCalDavTransport::Request currentUserPrincipalRequest(const QUrl &url,
                                                                   const QString &username,
                                                                   const QString &password);
    static DCalDavTransport::Request calendarHomeSetRequest(const QUrl &url, const QString &username,
                                                            const QString &password);
    static DCalDavTransport::Request calendarCollectionsRequest(const QUrl &url, const QString &username,
                                                                const QString &password);
};

#endif // DCALDAVDISCOVERY_H
