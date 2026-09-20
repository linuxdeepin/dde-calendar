// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavxmlreader.h"

#include <QXmlStreamReader>

DCalDavXmlStreamReader::DCalDavXmlStreamReader(const QByteArray &xml)
    : m_reader(xml)
{
}

QXmlStreamReader::TokenType DCalDavXmlStreamReader::readNext()
{
    const QXmlStreamReader::TokenType token = m_reader.readNext();
    if (token == QXmlStreamReader::StartElement) {
        ++m_depth;
        if (m_depth > MaximumDepth) {
            m_depthExceeded = true;
        }
    } else if (token == QXmlStreamReader::EndElement) {
        --m_depth;
    }
    return token;
}

bool DCalDavXmlStreamReader::readNextStartElement()
{
    while (!m_reader.atEnd()) {
        const QXmlStreamReader::TokenType token = readNext();
        if (token == QXmlStreamReader::StartElement) {
            return true;
        }
        if (token == QXmlStreamReader::EndElement
            || token == QXmlStreamReader::Invalid) {
            return false;
        }
    }
    return false;
}

void DCalDavXmlStreamReader::skipCurrentElement()
{
    const int parentDepth = m_depth - 1;
    while (!m_reader.atEnd() && m_depth > parentDepth) {
        readNext();
    }
}

QString DCalDavXmlStreamReader::readElementText(
    QXmlStreamReader::ReadElementTextBehaviour behaviour)
{
    const int parentDepth = m_depth - 1;
    QString text;
    while (!m_reader.atEnd()) {
        const QXmlStreamReader::TokenType token = readNext();
        if (token == QXmlStreamReader::Characters
            || token == QXmlStreamReader::EntityReference) {
            text += m_reader.text().toString();
        } else if (token == QXmlStreamReader::StartElement) {
            if (behaviour == QXmlStreamReader::SkipChildElements) {
                skipCurrentElement();
            } else if (behaviour == QXmlStreamReader::ErrorOnUnexpectedElement) {
                m_reader.raiseError(QStringLiteral("Unexpected child element."));
                return text;
            }
        } else if (token == QXmlStreamReader::EndElement && m_depth == parentDepth) {
            return text;
        } else if (token == QXmlStreamReader::Invalid) {
            return text;
        }
    }
    return text;
}

QString DCalDavXmlStreamReader::name() const
{
    return m_reader.name().toString();
}

bool DCalDavXmlStreamReader::atEnd() const
{
    return m_reader.atEnd();
}

bool DCalDavXmlStreamReader::hasError() const
{
    return m_reader.hasError();
}

QString DCalDavXmlStreamReader::errorString() const
{
    return m_reader.errorString();
}

bool DCalDavXmlStreamReader::depthExceeded() const
{
    return m_depthExceeded;
}

namespace {
constexpr qint64 kMaximumDiscoveryXmlBytes = 4 * 1024 * 1024;
constexpr int kMaximumCalendarCollections = 256;
constexpr int kMaximumPropertyLength = 4096;
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

void readPrivilegeSet(DCalDavXmlStreamReader &reader, int &privileges)
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

bool readResourceType(DCalDavXmlStreamReader &reader)
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

void readProperty(DCalDavXmlStreamReader &reader, DiscoveryProperties &properties)
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

void readPropertyStatus(DCalDavXmlStreamReader &reader, DCalDavXmlReader::DiscoveryResult &result,
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

void readResponse(DCalDavXmlStreamReader &reader, DCalDavXmlReader::DiscoveryResult &result)
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
    if (xml.size() > kMaximumDiscoveryXmlBytes) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV discovery response is too large.");
        }
        return false;
    }
    DCalDavXmlStreamReader reader(xml);
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

    if (reader.depthExceeded()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("DAV discovery response is too deeply nested.");
        }
        return false;
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
