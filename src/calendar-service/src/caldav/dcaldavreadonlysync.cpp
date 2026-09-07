// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavreadonlysync.h"

#include "dcaldavdiscovery.h"
#include "commondef.h"

namespace {

DCalDavValidationError::Type validationErrorForResponse(
    const DCalDavTransport::Response &response)
{
    if (response.error == DCalDavTransport::CertificateInvalid) {
        return DCalDavValidationError::CertificateInvalid;
    }
    if (response.error == DCalDavTransport::AuthenticationFailed
        || response.httpStatus == 401) {
        return DCalDavValidationError::AuthenticationFailed;
    }
    if (response.error == DCalDavTransport::PermissionDenied
        || response.httpStatus == 403) {
        return DCalDavValidationError::ServerRejected;
    }
    return DCalDavValidationError::NetworkUnavailable;
}

DCalDavValidationError::Type validationErrorForDiscoveryParse(
    const DCalDavTransport::Response &response)
{
    // A resource endpoint returning 404/405/501 indicates that the server does
    // not expose the requested CalDAV discovery resource. A successful HTTP
    // response with malformed XML is a parse failure instead.
    if (response.httpStatus == 404 || response.httpStatus == 405
        || response.httpStatus == 501) {
        return DCalDavValidationError::UnsupportedCalDav;
    }
    return DCalDavValidationError::ParseError;
}

DCalDavErrorCode errorCodeForValidation(
    DCalDavValidationError::Type validationError,
    const DCalDavTransport::Response &response)
{
    switch (validationError) {
    case DCalDavValidationError::AuthenticationFailed:
        return DCalDavErrorCode::AuthenticationFailed;
    case DCalDavValidationError::UnsupportedCalDav:
        return DCalDavErrorCode::UnsupportedCalDav;
    case DCalDavValidationError::CertificateInvalid:
        return DCalDavErrorCode::CertificateInvalid;
    case DCalDavValidationError::ParseError:
        return DCalDavErrorCode::ParseError;
    case DCalDavValidationError::ServerRejected:
        return DCalDavErrorCode::PermissionDenied;
    case DCalDavValidationError::NetworkUnavailable:
        return DCalDavErrorCode::NetworkUnavailable;
    case DCalDavValidationError::Other:
        return DCalDavErrorCode::Unknown;
    case DCalDavValidationError::NoError:
    default:
        break;
    }

    switch (response.error) {
    case DCalDavTransport::CertificateInvalid:
        return DCalDavErrorCode::CertificateInvalid;
    case DCalDavTransport::AuthenticationFailed:
        return DCalDavErrorCode::AuthenticationFailed;
    case DCalDavTransport::PermissionDenied:
        return DCalDavErrorCode::PermissionDenied;
    case DCalDavTransport::NetworkUnavailable:
        return DCalDavErrorCode::NetworkUnavailable;
    case DCalDavTransport::RequestTimedOut:
        return DCalDavErrorCode::RequestTimedOut;
    case DCalDavTransport::RateLimited:
        return DCalDavErrorCode::RateLimited;
    case DCalDavTransport::ServerUnavailable:
        return DCalDavErrorCode::ServerUnavailable;
    case DCalDavTransport::NetworkError:
        return DCalDavErrorCode::NetworkError;
    default:
        return DCalDavErrorCode::Unknown;
    }
}

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

} // namespace

DCalDavReadOnlySync::DCalDavReadOnlySync(QObject *parent)
    : QObject(parent)
    , m_transport()
{
}

void DCalDavReadOnlySync::start(const Request &request, const Callback &callback)
{
    if (m_running) {
        return;
    }

    m_request = request;
    m_callback = callback;
    m_result = Result();
    m_candidates.clear();
    m_candidateIndex = 0;
    m_collectionIndex = 0;
    m_running = true;

    qCDebug(ServiceLogger) << "CalDAV validation started"
                          << "endpoint:" << DCalDavTransport::urlForLog(request.serverUrl)
                          << "usernamePresent:" << !request.username.isEmpty();
    if (!DCalDavTransport::isSecureUrl(request.serverUrl) || request.username.isEmpty()) {
        finish(false, QStringLiteral("Invalid CalDAV server URL or username."),
               DCalDavValidationError::NetworkUnavailable);
        return;
    }

    m_candidates = DCalDavDiscovery::discoveryCandidates(request.serverUrl);
    sendPrincipalRequest();
}

void DCalDavReadOnlySync::sendPrincipalRequest()
{
    if (m_candidateIndex >= m_candidates.size()) {
        if (m_result.failureResponse.error != DCalDavTransport::NoError) {
            finish(false, transportErrorText(m_result.failureResponse));
        } else {
            finish(false, QStringLiteral("This server does not support CalDAV."));
        }
        return;
    }

    const QUrl candidate = m_candidates.at(m_candidateIndex);
    const DCalDavTransport::Request request = DCalDavDiscovery::currentUserPrincipalRequest(
        candidate, m_request.username, m_request.password);
    m_transport.send(request, [this, candidate](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            qCWarning(ServiceLogger) << "CalDAV principal discovery request failed"
                                     << "endpoint:" << DCalDavTransport::urlForLog(candidate)
                                     << "httpStatus:" << response.httpStatus
                                     << "error:" << static_cast<int>(response.error);
            m_result.failureResponse = response;
            // Authentication and certificate failures are terminal validation errors.
            // Do not probe another discovery candidate after a TLS failure: the
            // requirement is to terminate the connection and report the certificate
            // problem directly to the user.
            if (response.error == DCalDavTransport::AuthenticationFailed
                || response.error == DCalDavTransport::PermissionDenied
                || response.error == DCalDavTransport::CertificateInvalid) {
                finish(false, transportErrorText(response));
                return;
            }
            tryNextPrincipalCandidate(transportErrorText(response));
            return;
        }

        DCalDavXmlReader::DiscoveryResult discovery;
        QString errorMessage;
        if (!DCalDavXmlReader::parseDiscovery(response.body, discovery, &errorMessage)) {
            tryNextPrincipalCandidate(
                errorMessage.isEmpty() ? QStringLiteral("Unable to parse the DAV discovery response.") : errorMessage,
                validationErrorForDiscoveryParse(response));
            return;
        }
        if (discovery.currentUserPrincipalHref.isEmpty()) {
            tryNextPrincipalCandidate(QStringLiteral("Principal is missing."),
                                      DCalDavValidationError::UnsupportedCalDav);
            return;
        }

        qCDebug(ServiceLogger) << "CalDAV principal discovery succeeded"
                                 << "httpStatus:" << response.httpStatus;
        m_result.discovery.currentUserPrincipalHref = discovery.currentUserPrincipalHref;
        if (!discovery.principalDisplayName.isEmpty()) {
            m_result.discovery.principalDisplayName = discovery.principalDisplayName;
        }
        const QUrl baseUrl = response.finalUrl.isValid() ? response.finalUrl : candidate;
        sendCalendarHomeRequest(DCalDavDiscovery::resolveHref(baseUrl, discovery.currentUserPrincipalHref));
    });
}

void DCalDavReadOnlySync::tryNextPrincipalCandidate(
    const QString &errorMessage, DCalDavValidationError::Type validationError)
{
    ++m_candidateIndex;
    if (m_candidateIndex < m_candidates.size()) {
        sendPrincipalRequest();
        return;
    }
    finish(false, errorMessage, validationError);
}

void DCalDavReadOnlySync::sendCalendarHomeRequest(const QUrl &principalUrl)
{
    const DCalDavTransport::Request request = DCalDavDiscovery::calendarHomeSetRequest(
        principalUrl, m_request.username, m_request.password);
    m_transport.send(request, [this, principalUrl](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            qCWarning(ServiceLogger) << "CalDAV calendar home request failed"
                                     << "endpoint:" << DCalDavTransport::urlForLog(principalUrl)
                                     << "httpStatus:" << response.httpStatus
                                     << "error:" << static_cast<int>(response.error);
            m_result.failureResponse = response;
            finish(false, transportErrorText(response));
            return;
        }

        qCDebug(ServiceLogger) << "CalDAV calendar home request succeeded"
                                 << "httpStatus:" << response.httpStatus;
        DCalDavXmlReader::DiscoveryResult discovery;
        QString errorMessage;
        if (!DCalDavXmlReader::parseDiscovery(response.body, discovery, &errorMessage)) {
            finish(false,
                   errorMessage.isEmpty() ? QStringLiteral("Unable to parse the DAV discovery response.") : errorMessage,
                   validationErrorForDiscoveryParse(response));
            return;
        }
        if (discovery.calendarHomeSetHref.isEmpty()) {
            finish(false, QStringLiteral("Calendar home is missing."),
                   DCalDavValidationError::UnsupportedCalDav);
            return;
        }

        if (m_result.discovery.principalDisplayName.isEmpty()) {
            m_result.discovery.principalDisplayName = discovery.principalDisplayName;
        }
        const QUrl baseUrl = response.finalUrl.isValid() ? response.finalUrl : principalUrl;
        sendCollectionsRequest(DCalDavDiscovery::resolveHref(baseUrl, discovery.calendarHomeSetHref));
    });
}

void DCalDavReadOnlySync::sendCollectionsRequest(const QUrl &homeUrl)
{
    const DCalDavTransport::Request request = DCalDavDiscovery::calendarCollectionsRequest(
        homeUrl, m_request.username, m_request.password);
    m_transport.send(request, [this, homeUrl](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            qCWarning(ServiceLogger) << "CalDAV calendar collections request failed"
                                     << "endpoint:" << DCalDavTransport::urlForLog(homeUrl)
                                     << "httpStatus:" << response.httpStatus
                                     << "error:" << static_cast<int>(response.error);
            m_result.failureResponse = response;
            finish(false, transportErrorText(response));
            return;
        }

        qCDebug(ServiceLogger) << "CalDAV calendar collections request succeeded"
                                 << "httpStatus:" << response.httpStatus;
        DCalDavXmlReader::DiscoveryResult discovery;
        QString errorMessage;
        if (!DCalDavXmlReader::parseDiscovery(response.body, discovery, &errorMessage)) {
            finish(false,
                   QStringLiteral("Unable to parse the data returned by the server. Please verify the server address or try again later."),
                   validationErrorForDiscoveryParse(response));
            return;
        }

        qCDebug(ServiceLogger) << "CalDAV calendar collections discovered"
                                 << "count:" << discovery.calendarCollections.size();
        m_result.discovery.calendarHomeSetHref = homeUrl.toString();
        const QUrl baseUrl = response.finalUrl.isValid() ? response.finalUrl : homeUrl;
        for (DCalDavXmlReader::CalendarCollection &collection : discovery.calendarCollections) {
            collection.href = DCalDavDiscovery::resolveHref(baseUrl, collection.href).toString();
            if (collection.href.isEmpty()) {
                finish(false, QStringLiteral("CalDAV discovery returned an invalid calendar URL."),
                       DCalDavValidationError::ParseError);
                return;
            }
            if (collection.privilegesKnown
                && !(collection.privileges & DCalDavXmlReader::ReadPrivilege)) {
                continue;
            }
            m_result.discovery.calendarCollections.append(collection);
        }
        if (m_result.discovery.calendarCollections.isEmpty()) {
            finish(false, QStringLiteral("No readable CalDAV calendar was found."),
                   DCalDavValidationError::UnsupportedCalDav);
            return;
        }
        m_collectionIndex = 0;
        sendCalendarQuery();
    });
}

void DCalDavReadOnlySync::sendCalendarQuery()
{
    if (m_collectionIndex >= m_result.discovery.calendarCollections.size()) {
        finish(true);
        return;
    }

    const DCalDavXmlReader::CalendarCollection &collection =
        m_result.discovery.calendarCollections.at(m_collectionIndex);
    const QUrl collectionUrl(collection.href);
    const DCalDavTransport::Request request = DCalDavCalendarQuery::firstSyncRequest(
        collectionUrl, m_request.username, m_request.password, QDateTime::currentDateTimeUtc());
    m_transport.send(request, [this, collectionUrl](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            qCWarning(ServiceLogger) << "CalDAV calendar query request failed"
                                     << "endpoint:" << DCalDavTransport::urlForLog(collectionUrl)
                                     << "httpStatus:" << response.httpStatus
                                     << "error:" << static_cast<int>(response.error);
            m_result.failureResponse = response;
            finish(false, transportErrorText(response));
            return;
        }

        qCDebug(ServiceLogger) << "CalDAV calendar query request succeeded"
                              << "endpoint:" << DCalDavTransport::urlForLog(collectionUrl)
                              << "httpStatus:" << response.httpStatus;
        DCalDavCalendarQuery::RemoteEventList events;
        QString errorMessage;
        if (!DCalDavCalendarQuery::parseResponse(response.body, events, &errorMessage)) {
            finish(false,
                   QStringLiteral("Unable to parse the data returned by the server. Please verify the server address or try again later."),
                   DCalDavValidationError::ParseError);
            return;
        }

        for (DCalDavCalendarQuery::RemoteEvent event : events) {
            event.href = DCalDavDiscovery::resolveHref(
                response.finalUrl.isValid() ? response.finalUrl : collectionUrl, event.href).toString();
            if (event.deleted) {
                qCWarning(ServiceLogger) << "Skipping deleted CalDAV resource during validation";
                continue;
            }

            if (event.calendarData.isEmpty() || event.uid.isEmpty()) {
                qCWarning(ServiceLogger) << "Skipping CalDAV validation resource without calendar data.";
                continue;
            }

            DSchedule::Ptr schedule;
            if (!DCalDavEventMapper::toSchedule(event, schedule, &errorMessage)) {
                qCWarning(ServiceLogger) << "Skipping unparsable CalDAV validation resource.";
                continue;
            }
            m_result.remoteEvents.append(event);
            m_result.schedules.append(schedule);
        }

        ++m_collectionIndex;
        sendCalendarQuery();
    });
}

void DCalDavReadOnlySync::finish(bool success, const QString &errorMessage,
                                  DCalDavValidationError::Type validationError)
{
    if (!m_running) {
        return;
    }

    m_result.success = success;
    m_result.validationError = success ? DCalDavValidationError::NoError
        : (validationError != DCalDavValidationError::NoError
               ? validationError
               : (m_result.failureResponse.error != DCalDavTransport::NoError
                      ? validationErrorForResponse(m_result.failureResponse)
                      : DCalDavValidationError::UnsupportedCalDav));
    m_result.failureCode = success
        ? DCalDavErrorCode::NoError
        : errorCodeForValidation(m_result.validationError, m_result.failureResponse);
    m_result.errorMessage = errorMessage;
    qCDebug(ServiceLogger) << "CalDAV validation finished"
                          << "success:" << success
                          << "validationError:" << static_cast<int>(m_result.validationError)
                          << "errorPresent:" << !errorMessage.isEmpty();
    m_request.password.clear();
    m_running = false;
    if (m_callback) {
        const Callback callback = m_callback;
        m_callback = Callback();
        callback(m_result);
    }
}
