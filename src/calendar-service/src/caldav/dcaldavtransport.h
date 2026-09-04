// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVTRANSPORT_H
#define DCALDAVTRANSPORT_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QSet>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include <functional>

class QNetworkAccessManager;

class DCalDavTransport : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        InvalidUrl,
        InsecureUrl,
        CertificateInvalid,
        NetworkUnavailable,
        RequestTimedOut,
        AuthenticationFailed,
        PermissionDenied,
        RateLimited,
        ServerUnavailable,
        ResponseTooLarge,
        NetworkError,
    };

    struct Request {
        QUrl url;
        QByteArray method;
        QByteArray body;
        QByteArray contentType;
        QString username;
        QString password;
        int timeoutMilliseconds = 15000;
        qint64 maximumResponseBytes = 1024 * 1024;
        QMap<QByteArray, QByteArray> headers;
    };

    struct Response {
        Error error = NoError;
        int httpStatus = 0;
        QUrl finalUrl;
        QByteArray body;
        QByteArray retryAfter;
        QByteArray etag;
    };

    typedef std::function<void(const Response &)> Callback;

    explicit DCalDavTransport(QObject *parent = nullptr);

    void send(const Request &request, const Callback &callback);
    void cancel();

    static QUrl normalizeUrl(const QUrl &url);
    static bool isSecureUrl(const QUrl &url);
    static bool isSameOrigin(const QUrl &first, const QUrl &second);
    static QString urlForLog(const QUrl &url);
    static Error classifyError(QNetworkReply::NetworkError networkError, int httpStatus,
                               bool certificateInvalid, bool timedOut, bool responseTooLarge);

private:
    void send(const Request &request, const Callback &callback, int redirectCount,
              const QUrl &redirectOrigin);

    QNetworkAccessManager *m_networkManager = nullptr;
    QSet<QNetworkReply *> m_activeReplies;
    QList<QMetaObject::Connection> m_authenticationConnections;
};

#endif // DCALDAVTRANSPORT_H
