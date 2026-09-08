// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mock_caldav_server.h"

#include <QHash>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include <QUrl>

namespace {

QByteArray xmlEscape(QByteArray value)
{
    value.replace('&', "&amp;");
    value.replace('<', "&lt;");
    value.replace('>', "&gt;");
    value.replace('"', "&quot;");
    value.replace('\'', "&apos;");
    return value;
}

QByteArray eventIcs()
{
    return "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\n"
           "UID:mock-event-1\r\nDTSTART:20260831T120000Z\r\n"
           "DTEND:20260831T130000Z\r\nSUMMARY:Mock event\r\n"
           "END:VEVENT\r\nEND:VCALENDAR\r\n";
}

QByteArray multistatusFor(const QByteArray &target, const QByteArray &requestBody,
                           bool calendarQueryReturnsCalendarData)
{
    if (target == "/.well-known/caldav") {
        return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
               "<d:response><d:href>/principal/</d:href><d:propstat><d:prop>"
               "<d:current-user-principal><d:href>/principal/</d:href></d:current-user-principal>"
               "<d:displayname>Mock CalDAV</d:displayname>"
               "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>";
    }
    if (target == "/principal/") {
        return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
               "<d:response><d:href>/principal/</d:href><d:propstat><d:prop>"
               "<c:calendar-home-set><d:href>/calendars/</d:href></c:calendar-home-set>"
               "<d:displayname>Mock CalDAV</d:displayname>"
               "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>";
    }
    if (target == "/calendars/") {
        return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
               "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\" "
               "xmlns:ical=\"http://apple.com/ns/ical/\">"
               "<d:response><d:href>/calendars/user/</d:href><d:propstat><d:prop>"
               "<d:resourcetype><c:calendar/></d:resourcetype>"
               "<d:displayname>Mock Calendar</d:displayname>"
               "<ical:calendar-color>#4381D5</ical:calendar-color>"
               "<d:current-user-privilege-set><d:privilege><d:read/></d:privilege>"
               "<d:privilege><d:write-content/></d:privilege></d:current-user-privilege-set>"
               "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>";
    }
    const bool includeCalendarData = requestBody.contains("calendar-multiget")
        || calendarQueryReturnsCalendarData;
    const QByteArray calendarData = includeCalendarData
        ? "<c:calendar-data>" + xmlEscape(eventIcs()) + "</c:calendar-data>"
        : QByteArray();
    return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
           "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
           "<d:response><d:href>/calendars/user/mock-event-1.ics</d:href>"
           "<d:propstat><d:prop><d:getetag>\"mock-etag-1\"</d:getetag>"
           "<d:getcontenttype>text/calendar; component=VEVENT</d:getcontenttype>"
           + calendarData + "</d:prop>"
           "<d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
           "</d:response><d:sync-token>mock-token-1</d:sync-token></d:multistatus>";
}

} // namespace

MockCalDavServer::MockCalDavServer(QObject *parent)
    : QTcpServer(parent)
{
}

bool MockCalDavServer::start(const QByteArray &certificate, const QByteArray &privateKey)
{
    m_certificate = certificate;
    m_privateKey = privateKey;
    return listen(QHostAddress::LocalHost, 0);
}

QUrl MockCalDavServer::url() const
{
    return QUrl(QStringLiteral("https://localhost:%1").arg(serverPort()));
}

const QList<MockCalDavServer::Request> &MockCalDavServer::requests() const
{
    return m_requests;
}

int MockCalDavServer::connectionCount() const
{
    return m_connectionCount;
}

void MockCalDavServer::setResponseStatus(int status)
{
    m_responseStatus = status;
}

void MockCalDavServer::setResponseStatusForMethod(const QByteArray &method, int status)
{
    m_methodResponseStatuses.insert(method.toUpper(), status);
}

void MockCalDavServer::clearResponseStatusForMethod(const QByteArray &method)
{
    m_methodResponseStatuses.remove(method.toUpper());
}

void MockCalDavServer::setResponseStatusForTarget(const QByteArray &target, int status)
{
    m_targetResponseStatuses.insert(target, status);
}

void MockCalDavServer::setResponseBodyForTarget(const QByteArray &target, const QByteArray &body)
{
    m_targetResponseBodies.insert(target, body);
}

void MockCalDavServer::setCalendarMultiGetResponseStatus(int status)
{
    m_calendarMultiGetResponseStatus = status;
}

void MockCalDavServer::setResponseEtag(const QByteArray &etag)
{
    m_responseEtag = etag;
}

void MockCalDavServer::setCalendarQueryReturnsCalendarData(bool enabled)
{
    m_calendarQueryReturnsCalendarData = enabled;
}

void MockCalDavServer::incomingConnection(qintptr socketDescriptor)
{
    ++m_connectionCount;
    QSslSocket *socket = new QSslSocket(this);
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        socket->deleteLater();
        return;
    }

    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    configuration.setLocalCertificate(QSslCertificate(m_certificate, QSsl::Pem));
    configuration.setPrivateKey(QSslKey(m_privateKey, QSsl::Rsa, QSsl::Pem));
    socket->setSslConfiguration(configuration);
    m_buffers.insert(socket, QByteArray());
    connect(socket, &QSslSocket::readyRead, this, [this, socket]() { processSocket(socket); });
    connect(socket, &QSslSocket::disconnected, this, [this, socket]() {
        m_buffers.remove(socket);
        socket->deleteLater();
    });
    socket->startServerEncryption();
}

void MockCalDavServer::processSocket(QSslSocket *socket)
{
    QByteArray &buffer = m_buffers[socket];
    buffer.append(socket->readAll());
    const int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0) {
        return;
    }

    const QByteArray headers = buffer.left(headerEnd);
    const QList<QByteArray> lines = headers.split('\n');
    if (lines.isEmpty()) {
        socket->disconnectFromHost();
        return;
    }
    const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');
    if (requestLine.size() < 2) {
        socket->disconnectFromHost();
        return;
    }

    qint64 contentLength = 0;
    QMap<QByteArray, QByteArray> requestHeaders;
    for (int i = 1; i < lines.size(); ++i) {
        const QByteArray line = lines.at(i).trimmed();
        const int separator = line.indexOf(':');
        if (separator <= 0) {
            continue;
        }
        const QByteArray name = line.left(separator).trimmed();
        const QByteArray value = line.mid(separator + 1).trimmed();
        requestHeaders.insert(name, value);
        if (name.compare("Content-Length", Qt::CaseInsensitive) == 0) {
            contentLength = value.toLongLong();
        }
    }
    const qint64 totalLength = headerEnd + 4 + contentLength;
    if (buffer.size() < totalLength) {
        return;
    }

    const QByteArray method = requestLine.at(0);
    const QByteArray target = requestLine.at(1);
    const QByteArray body = buffer.mid(headerEnd + 4, contentLength);
    m_requests.append({method, target, body, requestHeaders});
    buffer.remove(0, totalLength);
    sendResponse(socket, method, target, body);
}

void MockCalDavServer::sendResponse(QSslSocket *socket, const QByteArray &method,
                                    const QByteArray &target, const QByteArray &requestBody)
{
    const bool isCalendarMultiGet = method == "REPORT"
        && requestBody.contains("calendar-multiget");
    const int status = isCalendarMultiGet && m_calendarMultiGetResponseStatus != 0
        ? m_calendarMultiGetResponseStatus
        : m_targetResponseStatuses.value(
            target, m_methodResponseStatuses.value(method.toUpper(), m_responseStatus));
    const QByteArray reason = status == 200 ? "OK"
        : status == 204 ? "No Content"
        : status == 401 ? "Unauthorized"
        : status == 403 ? "Forbidden"
        : status == 409 ? "Conflict"
        : status == 412 ? "Precondition Failed"
        : status == 429 ? "Too Many Requests"
        : status == 503 ? "Service Unavailable"
        : "Multi-Status";
    const QByteArray body = m_targetResponseBodies.contains(target)
        ? m_targetResponseBodies.value(target)
        : status == 207 && (method == "PROPFIND" || method == "REPORT")
        ? multistatusFor(target, requestBody, m_calendarQueryReturnsCalendarData)
        : status == 200 && method == "GET" && target == "/calendars/user/mock-event-1.ics"
        ? eventIcs() : QByteArray();
    QByteArray response = "HTTP/1.1 " + QByteArray::number(status) + " " + reason
        + "\r\nContent-Type: application/xml\r\nContent-Length: "
        + QByteArray::number(body.size()) + "\r\n";
    if (!m_responseEtag.isEmpty()) {
        response += "ETag: " + m_responseEtag + "\r\n";
    }
    if (status == 429) {
        response += "Retry-After: 1\r\n";
    }
    response += "Connection: close\r\n\r\n" + body;
    socket->write(response);
    socket->disconnectFromHost();
}
