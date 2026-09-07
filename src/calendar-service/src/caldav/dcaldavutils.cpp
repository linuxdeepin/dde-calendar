// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavutils.h"

namespace DCalDavUtils {

QString transportErrorText(const DCalDavTransport::Response &response)
{
    switch (response.error) {
    case DCalDavTransport::CertificateInvalid:
        return QStringLiteral("The server certificate is invalid.");
    case DCalDavTransport::AuthenticationFailed:
        return QStringLiteral("Incorrect username or password. Please try again.");
    case DCalDavTransport::NetworkUnavailable:
        return QStringLiteral("Unable to connect to the server. Please check your network connection and server address.");
    case DCalDavTransport::RequestTimedOut:
        return QStringLiteral("The server request timed out.");
    case DCalDavTransport::PermissionDenied:
        return QStringLiteral("The server denied access.");
    default:
        return QStringLiteral("CalDAV request failed (HTTP %1, error %2).")
            .arg(response.httpStatus)
            .arg(static_cast<int>(response.error));
    }
}

QUrl eventUrl(const DCalDavCalendarInfo &calendar, const QString &uid)
{
    QUrl url(calendar.href);
    QString path = url.path();
    if (!path.endsWith(QLatin1Char('/'))) {
        path.append(QLatin1Char('/'));
    }
    path.append(QString::fromLatin1(QUrl::toPercentEncoding(uid)));
    path.append(QStringLiteral(".ics"));
    url.setPath(path);
    return url;
}

} // namespace DCalDavUtils
