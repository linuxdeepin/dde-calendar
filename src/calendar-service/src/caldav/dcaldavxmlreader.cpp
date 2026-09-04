// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavxmlreader.h"

#include <QXmlStreamReader>

namespace {
constexpr int kMaximumXmlDepth = 64;
constexpr qint64 kMaximumDiscoveryXmlBytes = 4 * 1024 * 1024;
constexpr int kMaximumCalendarCollections = 256;
constexpr int kMaximumPropertyLength = 4096;
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

struct DiscoveryProperties
{
    QString currentUserPrincipalHref;
    QString calendarHomeSetHref;
    QString displayName;
    QString color;
    int privileges = DCalDavXmlReader::NoPrivilege;
    bool privilegesKnown = false;
    bool isCalendar = false;
};

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

void readPrivilegeSet(QXmlStreamReader &reader, int &privileges)
{
    while (reader.readNextStartElement()) {
        if (reader.name() != QStringLiteral("privilege")) {
            reader.skipCurrentElement();
            continue;
        }
        while (reader.readNextStartElement()) {
            const auto privilegeName = reader.name();
            if (privilegeName == QStringLiteral("read")) {
                privileges |= DCalDavXmlReader::ReadPrivilege;
            } else if (privilegeName == QStringLiteral("write")
                       || privilegeName == QStringLiteral("write-content")
                       || privilegeName == QStringLiteral("write-properties")) {
                // CalDAV servers commonly advertise write permission using
                // the DAV write-content/write-properties privileges instead
                // of the aggregate DAV write privilege.
                privileges |= DCalDavXmlReader::WritePrivilege;
            }
            reader.skipCurrentElement();
        }
    }
}

bool readResourceType(QXmlStreamReader &reader)
{
    bool isCalendar = false;
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("calendar")) {
            isCalendar = true;
        }
        reader.skipCurrentElement();
    }
    return isCalendar;
}

void readProperty(QXmlStreamReader &reader, DiscoveryProperties &properties)
{
    while (reader.readNextStartElement()) {
        const auto name = reader.name();
        if (name == QStringLiteral("current-user-principal")) {
            while (reader.readNextStartElement()) {
                if (reader.name() == QStringLiteral("href")) {
                    properties.currentUserPrincipalHref =
                        reader.readElementText(QXmlStreamReader::SkipChildElements);
                } else {
                    reader.skipCurrentElement();
                }
            }
        } else if (name == QStringLiteral("calendar-home-set")) {
            while (reader.readNextStartElement()) {
                if (reader.name() == QStringLiteral("href")) {
                    properties.calendarHomeSetHref =
                        reader.readElementText(QXmlStreamReader::SkipChildElements);
                } else {
                    reader.skipCurrentElement();
                }
            }
        } else if (name == QStringLiteral("displayname")) {
            properties.displayName = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (name == QStringLiteral("calendar-color")) {
            properties.color = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (name == QStringLiteral("resourcetype")) {
            properties.isCalendar = readResourceType(reader);
        } else if (name == QStringLiteral("current-user-privilege-set")) {
            properties.privilegesKnown = true;
            readPrivilegeSet(reader, properties.privileges);
        } else {
            reader.skipCurrentElement();
        }
    }
}

void mergeProperties(const DiscoveryProperties &source,
                     DCalDavXmlReader::DiscoveryResult &result,
                     DCalDavXmlReader::CalendarCollection &collection,
                     bool &isCalendar)
{
    if (!source.currentUserPrincipalHref.isEmpty()) {
        result.currentUserPrincipalHref = source.currentUserPrincipalHref;
    }
    if (!source.calendarHomeSetHref.isEmpty()) {
        result.calendarHomeSetHref = source.calendarHomeSetHref;
    }
    if (!source.displayName.isEmpty()) {
        collection.displayName = source.displayName;
    }
    if (!source.color.isEmpty()) {
        collection.color = source.color;
    }
    if (source.isCalendar) {
        isCalendar = true;
    }
    if (source.privilegesKnown) {
        collection.privilegesKnown = true;
        collection.privileges |= source.privileges;
    }
}

void readPropertyStatus(QXmlStreamReader &reader, DCalDavXmlReader::DiscoveryResult &result,
                        DCalDavXmlReader::CalendarCollection &collection, bool &isCalendar)
{
    DiscoveryProperties properties;
    int statusCode = -1;
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("prop")) {
            readProperty(reader, properties);
        } else if (reader.name() == QStringLiteral("status")) {
            statusCode = httpStatusCode(reader.readElementText(QXmlStreamReader::SkipChildElements));
        } else {
            reader.skipCurrentElement();
        }
    }
    if (statusCode >= 200 && statusCode < 300) {
        mergeProperties(properties, result, collection, isCalendar);
    }
}

void readResponse(QXmlStreamReader &reader, DCalDavXmlReader::DiscoveryResult &result)
{
    DCalDavXmlReader::CalendarCollection collection;
    bool isCalendar = false;
    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("href")) {
            collection.href = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QStringLiteral("propstat")) {
            readPropertyStatus(reader, result, collection, isCalendar);
        } else {
            reader.skipCurrentElement();
        }
    }

    if (isCalendar && !collection.href.isEmpty()
        && collection.href.size() <= kMaximumPropertyLength) {
        result.calendarCollections.append(collection);
    } else if (!collection.href.isEmpty() && result.principalDisplayName.isEmpty()) {
        result.principalDisplayName = collection.displayName;
    }
}

} // namespace

bool DCalDavXmlReader::parseDiscovery(const QByteArray &xml, DiscoveryResult &result, QString *errorMessage)
{
    if (!hasAcceptableXmlDepth(xml)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV discovery response is too deeply nested.");
        }
        return false;
    }
    if (xml.size() > kMaximumDiscoveryXmlBytes) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV discovery response is too large.");
        }
        return false;
    }
    QXmlStreamReader reader(xml);
    DiscoveryResult parsed;
    if (!reader.readNextStartElement() || reader.name() != QStringLiteral("multistatus")) {
        if (errorMessage != nullptr) {
            *errorMessage = reader.hasError() ? reader.errorString() : QStringLiteral("Invalid DAV multistatus response.");
        }
        return false;
    }

    while (reader.readNextStartElement()) {
        if (reader.name() == QStringLiteral("response")) {
            readResponse(reader, parsed);
            if (parsed.calendarCollections.size() > kMaximumCalendarCollections) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("DAV discovery response contains too many calendar collections.");
                }
                return false;
            }
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

    if (parsed.currentUserPrincipalHref.size() > kMaximumPropertyLength
        || parsed.calendarHomeSetHref.size() > kMaximumPropertyLength
        || parsed.principalDisplayName.size() > kMaximumPropertyLength
        || parsed.calendarCollections.size() > kMaximumCalendarCollections) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV discovery response contains oversized or excessive properties.");
        }
        return false;
    }
    for (const CalendarCollection &collection : parsed.calendarCollections) {
        if (collection.href.size() > kMaximumPropertyLength
            || collection.displayName.size() > kMaximumPropertyLength
            || collection.color.size() > kMaximumPropertyLength) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("DAV calendar collection contains oversized properties.");
            }
            return false;
        }
    }
    result = parsed;
    return true;
}
