// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcalendarquery.h"
#include <QRegularExpression>

#include <QXmlStreamReader>
namespace {
constexpr int kMaximumXmlDepth = 64;

constexpr qint64 kMaximumCalDavXmlBytes = 4 * 1024 * 1024;
constexpr int kMaximumCalDavResources = 4096;
constexpr int kMaximumCalDavPropertyLength = 4096;
}
static bool hasAcceptableXmlDepth(const QByteArray &xml)
{
    QXmlStreamReader reader(xml);

    int depth = 0;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            if (++depth > kMaximumXmlDepth) {
                return false;
            }
        } else if (reader.isEndElement()) {
            --depth;
        }
    }
    return !reader.hasError() && depth == 0;
}
namespace {
QString formatUtcDateTime(const QDateTime &dateTime)
{
    return dateTime.toUTC().toString(QStringLiteral("yyyyMMddTHHmmssZ"));

}
QString xmlEscape(const QString &value)

{
    QString escaped = value;
    escaped.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    escaped.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    escaped.replace(QLatin1Char('>'), QStringLiteral("&gt;"));

    escaped.replace(QLatin1Char('\"'), QStringLiteral("&quot;"));
    escaped.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    return escaped;
}
QString uidFromCalendarData(const QString &calendarData)
{
    QStringList unfoldedLines;
    const QStringList lines = calendarData.split(QRegularExpression(QStringLiteral("\\r?\\n")));
    for (const QString &line : lines) {
        if (!unfoldedLines.isEmpty() && (line.startsWith(QLatin1Char(' ')) || line.startsWith(QLatin1Char('\t')))) {
            unfoldedLines.last().append(line.mid(1));

        } else {
            unfoldedLines.append(line);
        }
    }
    for (const QString &line : unfoldedLines) {
        const int separator = line.indexOf(QLatin1Char(':'));
        if (separator < 1) {
            continue;
        }
        const QString propertyName = line.left(separator);
        if (propertyName.compare(QStringLiteral("UID"), Qt::CaseInsensitive) == 0
            || propertyName.startsWith(QStringLiteral("UID;"), Qt::CaseInsensitive)) {

            return line.mid(separator + 1);
        }
    }
    return QString();
}
int httpStatusCode(const QString &status)
{
    const QStringList parts = status.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.size() < 2) {
        return -1;
    }
    bool ok = false;
    const int code = parts.at(1).toInt(&ok);
    return ok ? code : -1;

}
void readProperties(QXmlStreamReader &reader, DCalDavCalendarQuery::RemoteEvent &properties)
{
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("getetag")) {
            properties.etag = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QStringLiteral("calendar-data")) {
            properties.calendarData = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QStringLiteral("getcontenttype")) {
            properties.contentType = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else {

            reader.skipCurrentElement();
        }
    }
}
void mergeProperties(const DCalDavCalendarQuery::RemoteEvent &source,
                     DCalDavCalendarQuery::RemoteEvent &target)
{
    if (!source.etag.isEmpty()) {
        target.etag = source.etag;
    }
    if (!source.calendarData.isEmpty()) {
        target.calendarData = source.calendarData;
    }
    if (!source.contentType.isEmpty()) {
        target.contentType = source.contentType;

    }
}
void readPropertyStatus(QXmlStreamReader &reader, DCalDavCalendarQuery::RemoteEvent &event)
{
    DCalDavCalendarQuery::RemoteEvent properties;
    int statusCode = -1;
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("prop")) {
            readProperties(reader, properties);
        } else if (reader.name() == QStringLiteral("status")) {
            statusCode = httpStatusCode(reader.readElementText(QXmlStreamReader::SkipChildElements));
        } else {
            reader.skipCurrentElement();
        }

    }
    if (statusCode >= 200 && statusCode < 300) {
        mergeProperties(properties, event);
        event.hasSuccessfulStatus = true;
        event.deleted = false;
    } else if (statusCode == 404 && !event.hasSuccessfulStatus) {
        event.deleted = true;
    }
}

void readResourceProperties(QXmlStreamReader &reader, DCalDavCalendarQuery::Resource &properties)
{
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("getetag")) {

            properties.etag = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QStringLiteral("getcontenttype")) {
            properties.contentType = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QStringLiteral("calendar-data")) {
            properties.calendarData = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else {
            reader.skipCurrentElement();
        }
    }
}

void mergeResourceProperties(const DCalDavCalendarQuery::Resource &source,
                             DCalDavCalendarQuery::Resource &target)
{
    if (!source.etag.isEmpty()) {
        target.etag = source.etag;
    }
    if (!source.contentType.isEmpty()) {
        target.contentType = source.contentType;
    }
    if (!source.calendarData.isEmpty()) {
        target.calendarData = source.calendarData;
    }
}

void applyResourceStatus(int statusCode, DCalDavCalendarQuery::Resource &resource)
{
    if (statusCode >= 200 && statusCode < 300) {
        resource.hasSuccessfulStatus = true;
        resource.deleted = false;
    } else if (statusCode == 404 && !resource.hasSuccessfulStatus) {
        resource.deleted = true;
    }
}

void readResourcePropertyStatus(QXmlStreamReader &reader,
                                DCalDavCalendarQuery::Resource &resource)
{
    DCalDavCalendarQuery::Resource properties;
    int statusCode = -1;
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("prop")) {
            readResourceProperties(reader, properties);
        } else if (reader.name() == QStringLiteral("status")) {
            statusCode = httpStatusCode(reader.readElementText(QXmlStreamReader::SkipChildElements));
        } else {
            reader.skipCurrentElement();
        }
    }
    if (statusCode >= 200 && statusCode < 300) {
        mergeResourceProperties(properties, resource);
    }
    applyResourceStatus(statusCode, resource);
}

void readResponse(QXmlStreamReader &reader, DCalDavCalendarQuery::RemoteEventList &events)
{
    DCalDavCalendarQuery::RemoteEvent event;
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("href")) {
            event.href = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QStringLiteral("propstat")) {
            readPropertyStatus(reader, event);
        } else {
            reader.skipCurrentElement();
        }
    }

    if (!event.href.isEmpty() && (event.deleted || !event.calendarData.isEmpty()
                                  || !event.etag.isEmpty())) {
        if (!event.deleted && !event.calendarData.isEmpty()) {
            event.uid = uidFromCalendarData(event.calendarData);
        }
        events.append(event);
    }
}

} // namespace

DCalDavTransport::Request DCalDavCalendarQuery::firstSyncRequest(const QUrl &calendarUrl,
                                                                  const QString &username,
                                                                  const QString &password,
                                                                  const QDateTime &referenceTime)
{
    const QDateTime rangeStart = referenceTime.addMonths(-1);
    const QDateTime rangeEnd = referenceTime.addMonths(6);

    DCalDavTransport::Request request;
    request.url = calendarUrl;
    request.method = "REPORT";
    request.contentType = "application/xml; charset=utf-8";
    request.username = username;
    request.password = password;
    request.maximumResponseBytes = kMaximumCalDavXmlBytes;
    request.headers.insert("Depth", "1");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<c:calendar-query xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
                   "<d:prop><d:getetag/><d:getcontenttype/><c:calendar-data/></d:prop>"
                   "<c:filter><c:comp-filter name=\"VCALENDAR\">"
                   "<c:comp-filter name=\"VEVENT\">"
                   "<c:time-range start=\"" + formatUtcDateTime(rangeStart).toUtf8()
        + "\" end=\"" + formatUtcDateTime(rangeEnd).toUtf8()
        + "\"/></c:comp-filter>"
          "</c:comp-filter></c:filter>"
          "</c:calendar-query>";
    return request;
}

DCalDavTransport::Request DCalDavCalendarQuery::incrementalSyncRequest(const QUrl &calendarUrl,
                                                                       const QString &username,
                                                                       const QString &password,
                                                                       const QString &syncToken)
{
    DCalDavTransport::Request request;
    request.url = calendarUrl;
    request.method = "REPORT";
    request.contentType = "application/xml; charset=utf-8";
    request.username = username;
    request.password = password;
    request.maximumResponseBytes = kMaximumCalDavXmlBytes;
    request.headers.insert("Depth", "1");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<d:sync-collection xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
                   "<d:sync-token>" + xmlEscape(syncToken).toUtf8()
        + "</d:sync-token><d:sync-level>1</d:sync-level>"
          "<d:prop><d:getetag/><d:getcontenttype/><c:calendar-data/></d:prop>"
          "</d:sync-collection>";
    return request;
}



DCalDavTransport::Request DCalDavCalendarQuery::resourceListRequest(
    const QUrl &calendarUrl, const QString &username, const QString &password)
{
    DCalDavTransport::Request request;
    request.url = calendarUrl;
    request.method = "REPORT";
    request.contentType = "application/xml; charset=utf-8";
    request.username = username;
    request.password = password;
    request.maximumResponseBytes = kMaximumCalDavXmlBytes;
    request.headers.insert("Depth", "1");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<c:calendar-query xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
                   "<d:prop><d:getetag/><d:getcontenttype/><c:calendar-data/></d:prop>"
                   "<c:filter><c:comp-filter name=\"VCALENDAR\">"
                   "<c:comp-filter name=\"VEVENT\"/>"
                   "</c:comp-filter></c:filter>"
                   "</c:calendar-query>";
    return request;
}

DCalDavTransport::Request DCalDavCalendarQuery::resourceGetRequest(
    const QUrl &resourceUrl, const QString &username, const QString &password)
{
    DCalDavTransport::Request request;
    request.url = resourceUrl;
    request.method = "GET";
    request.username = username;
    request.password = password;
    request.maximumResponseBytes = kMaximumCalDavXmlBytes;
    return request;
}

DCalDavTransport::Request DCalDavCalendarQuery::resourceMultiGetRequest(
    const QUrl &calendarUrl, const QString &username, const QString &password,
    const QStringList &resourceHrefs)
{
    DCalDavTransport::Request request;
    request.url = calendarUrl;
    request.method = "REPORT";
    request.contentType = "application/xml; charset=utf-8";
    request.username = username;
    request.password = password;
    request.maximumResponseBytes = kMaximumCalDavXmlBytes;
    request.headers.insert("Depth", "1");

    QString hrefs;
    for (const QString &resourceHref : resourceHrefs) {
        QUrl hrefUrl(resourceHref);
        QString href = resourceHref;
        if (hrefUrl.isValid() && !hrefUrl.scheme().isEmpty()) {
            href = hrefUrl.path(QUrl::FullyEncoded);
            if (hrefUrl.hasQuery()) {
                href.append(QLatin1Char('?'));
                href.append(hrefUrl.query(QUrl::FullyEncoded));
            }
        }
        if (!href.isEmpty()) {
            hrefs.append(QStringLiteral("<d:href>%1</d:href>").arg(xmlEscape(href)));
        }
    }

    request.body = QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                  "<c:calendar-multiget xmlns:d=\"DAV:\" "
                                  "xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
                                  "<d:prop><d:getetag/><d:getcontenttype/><c:calendar-data/></d:prop>%1"
                                  "</c:calendar-multiget>")
                       .arg(hrefs)
                       .toUtf8();
    return request;
}

bool DCalDavCalendarQuery::parseResourceList(const QByteArray &xml, ResourceList &resources,
                                             QString *errorMessage)
{
    if (!hasAcceptableXmlDepth(xml)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV response is too deeply nested.");
        }
        return false;
    }
    if (xml.size() > kMaximumCalDavXmlBytes) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV resource response is too large.");
        }
        return false;
    }
    QXmlStreamReader reader(xml);
    ResourceList parsed;
    if (!reader.readNextStartElement() || reader.name() != QStringLiteral("multistatus")) {
        if (errorMessage != nullptr) {
            *errorMessage = reader.hasError() ? reader.errorString()
                                              : QStringLiteral("Invalid DAV multistatus response.");
        }
        return false;
    }

    while (reader.readNextStartElement()) {
        if (reader.name() != QStringLiteral("response")) {
            reader.skipCurrentElement();
            continue;
        }
        if (parsed.size() >= kMaximumCalDavResources) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("DAV resource response contains too many resources.");
            }
            return false;
        }
        Resource resource;
        while (reader.readNextStartElement()) {
            if (reader.name() == QStringLiteral("href")) {
                resource.href = reader.readElementText(QXmlStreamReader::SkipChildElements);
            } else if (reader.name() == QStringLiteral("propstat")) {
                readResourcePropertyStatus(reader, resource);
            } else if (reader.name() == QStringLiteral("status")) {
                applyResourceStatus(
                    httpStatusCode(reader.readElementText(QXmlStreamReader::SkipChildElements)), resource);
            } else {
                reader.skipCurrentElement();
            }
        }
        const bool isEvent = resource.contentType.contains(QStringLiteral("vevent"), Qt::CaseInsensitive)
            || (resource.contentType.isEmpty()
                && resource.href.endsWith(QStringLiteral(".ics"), Qt::CaseInsensitive));
        if (!resource.deleted && !resource.href.isEmpty()
            && (!resource.etag.isEmpty() || isEvent || !resource.calendarData.isEmpty())) {
            parsed.append(resource);
        }
    }

    if (reader.hasError()) {
        if (errorMessage != nullptr) {
            *errorMessage = reader.errorString();
        }
        return false;
    }
    if (parsed.size() > kMaximumCalDavResources) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV resource response contains too many resources.");
        }
        return false;
    }
    for (const Resource &resource : parsed) {
        if (resource.href.size() > kMaximumCalDavPropertyLength
            || resource.etag.size() > kMaximumCalDavPropertyLength
            || resource.contentType.size() > kMaximumCalDavPropertyLength
            || resource.calendarData.size() > kMaximumCalDavXmlBytes) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("DAV resource response contains oversized properties.");
            }
            return false;
        }
    }
    resources = parsed;
    return true;
}

bool DCalDavCalendarQuery::parseResponse(const QByteArray &xml, RemoteEventList &events,
                                         QString *errorMessage)
{
    return parseResponseWithSyncToken(xml, events, nullptr, errorMessage);
}

bool DCalDavCalendarQuery::parseResponseWithSyncToken(const QByteArray &xml, RemoteEventList &events,
                                                        QString *syncToken, QString *errorMessage)
{
    if (!hasAcceptableXmlDepth(xml)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV response is too deeply nested.");
        }
        return false;
    }
    if (xml.size() > kMaximumCalDavXmlBytes) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV event response is too large.");
        }
        return false;
    }
    QXmlStreamReader reader(xml);
    RemoteEventList parsed;
    QString parsedSyncToken;
    if (!reader.readNextStartElement() || reader.name() != QStringLiteral("multistatus")) {
        if (errorMessage != nullptr) {
            *errorMessage = reader.hasError() ? reader.errorString() : QStringLiteral("Invalid DAV multistatus response.");
        }
        return false;
    }

    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("response")) {
            if (parsed.size() >= kMaximumCalDavResources) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("DAV event response contains too many resources.");
                }
                return false;
            }
            readResponse(reader, parsed);
        } else if (reader.name() == QStringLiteral("sync-token")) {
            parsedSyncToken = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else {
            reader.skipCurrentElement();
        }
    }

    if (reader.hasError()) {
        if (errorMessage != nullptr) {
            *errorMessage = reader.errorString();
        }
        return false;
    }

    if (parsed.size() > kMaximumCalDavResources
        || parsedSyncToken.size() > kMaximumCalDavPropertyLength) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV event response contains too many or oversized resources.");
        }
        return false;
    }
    for (const RemoteEvent &event : parsed) {
        if (event.href.size() > kMaximumCalDavPropertyLength
            || event.etag.size() > kMaximumCalDavPropertyLength
            || event.contentType.size() > kMaximumCalDavPropertyLength
            || event.calendarData.size() > kMaximumCalDavXmlBytes) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("DAV event response contains oversized properties.");
            }
            return false;
        }
        if (!event.deleted && !event.calendarData.isEmpty() && event.uid.isEmpty()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Calendar data is missing UID.");
            }
            return false;
        }
    }

    events = parsed;
    if (syncToken != nullptr) {
        *syncToken = parsedSyncToken;
    }
    return true;
}
