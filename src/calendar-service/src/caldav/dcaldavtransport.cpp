// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavtransport.h"

#include "commondef.h"

#include <QAuthenticator>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSharedPointer>
#include <QSslError>
#include <QTimer>

DCalDavTransport::DCalDavTransport(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

QUrl DCalDavTransport::normalizeUrl(const QUrl &url)
{
    const QString input = url.toString(QUrl::FullyEncoded).trimmed();
    if (input.isEmpty()) {
        return QUrl();
    }

    QUrl normalized = url;
    if (normalized.scheme().isEmpty()) {
        normalized = QUrl(input.startsWith(QStringLiteral("//"))
                              ? QStringLiteral("https:") + input
                              : QStringLiteral("https://") + input);
    }

    if (!normalized.isValid() || normalized.host().isEmpty() || !normalized.userName().isEmpty()
        || !normalized.password().isEmpty()) {
        return QUrl();
    }

    normalized.setScheme(normalized.scheme().toLower());
    if (normalized.scheme() != QStringLiteral("https")) {
        return QUrl();
    }
    normalized.setFragment(QString());
    return normalized;
}

bool DCalDavTransport::isSecureUrl(const QUrl &url)
{
    const QUrl normalized = normalizeUrl(url);
    return normalized.isValid()
        && normalized.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0
        && !normalized.host().isEmpty();
}

bool DCalDavTransport::isSameOrigin(const QUrl &first, const QUrl &second)
{
    const QUrl normalizedFirst = normalizeUrl(first);
    const QUrl normalizedSecond = normalizeUrl(second);
    if (!isSecureUrl(normalizedFirst) || !isSecureUrl(normalizedSecond)) {
        return false;
    }

    const auto effectivePort = [](const QUrl &url) { return url.port(443); };
    return normalizedFirst.scheme().compare(normalizedSecond.scheme(), Qt::CaseInsensitive) == 0
        && normalizedFirst.host().compare(normalizedSecond.host(), Qt::CaseInsensitive) == 0
        && effectivePort(normalizedFirst) == effectivePort(normalizedSecond);
}

QString DCalDavTransport::urlForLog(const QUrl &url)
{
    const QUrl normalized = normalizeUrl(url);
    if (!normalized.isValid() || normalized.host().isEmpty()) {
        return QStringLiteral("<invalid-url>");
    }

    const QByteArray hostHash = QCryptographicHash::hash(normalized.host().toUtf8(),
                                                         QCryptographicHash::Sha256)
                                    .toHex()
                                    .left(12);
    return QStringLiteral("caldav-host:%1").arg(QString::fromLatin1(hostHash));
}

DCalDavTransport::Error DCalDavTransport::classifyError(QNetworkReply::NetworkError networkError,
                                                         int httpStatus,
                                                         bool certificateInvalid,
                                                         bool timedOut,
                                                         bool responseTooLarge)
{
    if (responseTooLarge) {
        return ResponseTooLarge;
    }
    if (certificateInvalid) {
        return CertificateInvalid;
    }
    if (timedOut) {
        return RequestTimedOut;
    }
    if (httpStatus == 401) {
        return AuthenticationFailed;
    }
    if (httpStatus == 403) {
        return PermissionDenied;
    }
    if (httpStatus == 429) {
        return RateLimited;
    }
    if (httpStatus == 503) {
        return ServerUnavailable;
    }
    // An HTTP response, including conflict and not-found responses, is not a
    // transport failure. Callers use the status code to apply protocol-specific
    // handling such as ETag conflicts and idempotent DELETEs.
    if (httpStatus > 0 || networkError == QNetworkReply::NoError) {
        return NoError;
    }
    if (networkError == QNetworkReply::HostNotFoundError
        || networkError == QNetworkReply::ConnectionRefusedError
        || networkError == QNetworkReply::TemporaryNetworkFailureError
        || networkError == QNetworkReply::NetworkSessionFailedError) {
        return NetworkUnavailable;
    }
    return NetworkError;
}

void DCalDavTransport::cancel()
{
    const QSet<QNetworkReply *> replies = m_activeReplies;
    m_activeReplies.clear();
    for (const QMetaObject::Connection &connection : m_authenticationConnections) {
        QObject::disconnect(connection);
    }
    m_authenticationConnections.clear();
    for (QNetworkReply *reply : replies) {
        if (reply == nullptr) {
            continue;
        }
        QObject::disconnect(reply, nullptr, nullptr, nullptr);
        reply->abort();
        reply->deleteLater();
    }
}

void DCalDavTransport::send(const Request &request, const Callback &callback)
{
    send(request, callback, 0, normalizeUrl(request.url));
}

void DCalDavTransport::send(const Request &request, const Callback &callback, int redirectCount,
                            const QUrl &redirectOrigin)
{
    if (!callback) {
        return;
    }

    const QUrl normalizedUrl = normalizeUrl(request.url);
    Response invalidResponse;
    invalidResponse.finalUrl = normalizedUrl.isValid() ? normalizedUrl : request.url;
    if (!normalizedUrl.isValid()) {
        invalidResponse.error = InvalidUrl;
        callback(invalidResponse);
        return;
    }
    if (!isSecureUrl(normalizedUrl)) {
        invalidResponse.error = InsecureUrl;
        callback(invalidResponse);
        return;
    }

    const Request effectiveRequest = [&request, &normalizedUrl] {
        Request result = request;
        result.url = normalizedUrl;
        return result;
    }();
    const QUrl effectiveRedirectOrigin = normalizeUrl(redirectOrigin).isValid()
        ? normalizeUrl(redirectOrigin)
        : normalizedUrl;

    QNetworkRequest networkRequest(effectiveRequest.url);
    // CalDAV discovery must retain PROPFIND/REPORT and their XML bodies after a redirect.
    networkRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                                QNetworkRequest::ManualRedirectPolicy);
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("dde-calendar-caldav"));
    if (!effectiveRequest.contentType.isEmpty()) {
        networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, effectiveRequest.contentType);
    }
    networkRequest.setRawHeader("Accept", "application/xml, text/xml, text/calendar");
    for (auto header = effectiveRequest.headers.constBegin(); header != effectiveRequest.headers.constEnd(); ++header) {
        networkRequest.setRawHeader(header.key(), header.value());
    }
    if (!effectiveRequest.username.isEmpty() || !effectiveRequest.password.isEmpty()) {
        const QByteArray authorization = (effectiveRequest.username + QLatin1Char(':') + effectiveRequest.password).toUtf8().toBase64();
        networkRequest.setRawHeader("Authorization", "Basic " + authorization);
    }

    QNetworkReply *reply = m_networkManager->sendCustomRequest(networkRequest, effectiveRequest.method, effectiveRequest.body);
    m_activeReplies.insert(reply);
    reply->setProperty("caldavMaximumResponseBytes", effectiveRequest.maximumResponseBytes);

    // Some CalDAV servers require a challenge-response retry even when the
    // request already carries preemptive Basic authentication.
    const QSharedPointer<QMetaObject::Connection> authenticationConnection =
        QSharedPointer<QMetaObject::Connection>::create();
    *authenticationConnection = connect(
        m_networkManager, &QNetworkAccessManager::authenticationRequired, this,
        [reply, effectiveRequest](QNetworkReply *authenticationReply, QAuthenticator *authenticator) {
            if (authenticationReply != reply) {
                return;
            }
            qCDebug(ServiceLogger) << "CalDAV authentication challenge received"
                                  << "endpoint:" << DCalDavTransport::urlForLog(authenticationReply->url());
            authenticator->setUser(effectiveRequest.username);
            authenticator->setPassword(effectiveRequest.password);
        });
    m_authenticationConnections.append(*authenticationConnection);
    QTimer *timer = new QTimer(reply);
    timer->setSingleShot(true);
    timer->start(effectiveRequest.timeoutMilliseconds);

    QSharedPointer<QByteArray> responseBody(new QByteArray);
    connect(reply, &QNetworkReply::readyRead, reply, [reply, responseBody, effectiveRequest]() {
        responseBody->append(reply->readAll());
        if (responseBody->size() > effectiveRequest.maximumResponseBytes) {
            reply->setProperty("caldavResponseTooLarge", true);
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::sslErrors, reply, [reply](const QList<QSslError> &) {
        reply->setProperty("caldavCertificateInvalid", true);
        reply->abort();
    });
    connect(timer, &QTimer::timeout, reply, [reply]() {
        reply->setProperty("caldavTimedOut", true);
        reply->abort();
    });
    connect(reply, &QNetworkReply::finished, reply,
            [this, reply, timer, responseBody, effectiveRequest, callback, redirectCount,
             effectiveRedirectOrigin, authenticationConnection]() {
        m_activeReplies.remove(reply);
        QObject::disconnect(*authenticationConnection);
        m_authenticationConnections.removeAll(*authenticationConnection);
        timer->stop();
        Response response;
        response.httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        response.finalUrl = reply->url();
        response.retryAfter = reply->rawHeader("Retry-After");
        response.etag = reply->rawHeader("ETag");
        responseBody->append(reply->readAll());
        if (responseBody->size() > reply->property("caldavMaximumResponseBytes").toLongLong()) {
            reply->setProperty("caldavResponseTooLarge", true);
        }
        response.body = *responseBody;
        response.error = classifyError(reply->error(), response.httpStatus,
                                       reply->property("caldavCertificateInvalid").toBool(),
                                       reply->property("caldavTimedOut").toBool(),
                                       reply->property("caldavResponseTooLarge").toBool());

        const QUrl redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        const bool isRedirect = response.httpStatus >= 300 && response.httpStatus < 400;
        if (isRedirect && redirectTarget.isValid()) {
            const QUrl nextUrl = normalizeUrl(reply->url().resolved(redirectTarget));
            reply->deleteLater();
            constexpr int maximumRedirectCount = 5;
            if (!nextUrl.isValid()) {
                response.error = InvalidUrl;
                callback(response);
                return;
            }
            if (!isSecureUrl(nextUrl)) {
                response.error = InsecureUrl;
                callback(response);
                return;
            }
            if (redirectCount >= maximumRedirectCount || !isSameOrigin(effectiveRedirectOrigin, nextUrl)) {
                // Never forward Basic credentials to a different origin.
                response.error = NetworkError;
                callback(response);
                return;
            }

            Request redirectedRequest = effectiveRequest;
            redirectedRequest.url = nextUrl;
            send(redirectedRequest, callback, redirectCount + 1, effectiveRedirectOrigin);
            return;
        }
        if (isRedirect) {
            response.error = NetworkError;
        }
        callback(response);
        reply->deleteLater();
    });
}
