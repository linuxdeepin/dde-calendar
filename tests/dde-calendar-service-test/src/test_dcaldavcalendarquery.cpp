// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcalendarquery.h"

#include <gtest/gtest.h>
#include <QDateTime>
#include <QString>
#include <QStringList>

namespace {

const char *kValidIcs =
    "BEGIN:VCALENDAR\r\n"
    "BEGIN:VEVENT\r\n"
    "UID:event-001\r\n"
    "DTSTAMP:20240101T000000Z\r\n"
    "DTSTART:20240101T090000Z\r\n"
    "DTEND:20240101T100000Z\r\n"
    "SUMMARY:Test Event\r\n"
    "END:VEVENT\r\n"
    "END:VCALENDAR\r\n";

} // namespace

// ---- Request builder tests ----

TEST(DCalDavCalendarQueryTest, FirstSyncRequest_PopulatesFields)
{
    QUrl calendarUrl("https://caldav.example.com/cal");
    QDateTime referenceTime = QDateTime::fromString(
        QStringLiteral("2024-06-15T10:00:00"), Qt::ISODate);
    DCalDavTransport::Request request = DCalDavCalendarQuery::firstSyncRequest(
        calendarUrl, "user1", "pass1", referenceTime);
    EXPECT_EQ(request.url, calendarUrl);
    EXPECT_EQ(request.method, "REPORT");
    EXPECT_EQ(request.contentType, "application/xml; charset=utf-8");
    EXPECT_EQ(request.username, QStringLiteral("user1"));
    EXPECT_EQ(request.password, QStringLiteral("pass1"));
    EXPECT_EQ(request.headers.value("Depth"), "1");
    EXPECT_TRUE(request.body.contains("calendar-query"));
    EXPECT_TRUE(request.body.contains("time-range"));
    EXPECT_TRUE(request.body.contains("VEVENT"));
}

TEST(DCalDavCalendarQueryTest, IncrementalSyncRequest_PopulatesFields)
{
    QUrl calendarUrl("https://caldav.example.com/cal");
    QString syncToken = QStringLiteral("sync-token-123");
    DCalDavTransport::Request request = DCalDavCalendarQuery::incrementalSyncRequest(
        calendarUrl, "user1", "pass1", syncToken);
    EXPECT_EQ(request.url, calendarUrl);
    EXPECT_EQ(request.method, "REPORT");
    EXPECT_EQ(request.contentType, "application/xml; charset=utf-8");
    EXPECT_EQ(request.username, QStringLiteral("user1"));
    EXPECT_EQ(request.password, QStringLiteral("pass1"));
    EXPECT_EQ(request.headers.value("Depth"), "1");
    EXPECT_TRUE(request.body.contains("sync-collection"));
    EXPECT_TRUE(request.body.contains("sync-token"));
    EXPECT_TRUE(request.body.contains(syncToken.toUtf8()));
}

TEST(DCalDavCalendarQueryTest, ResourceListRequest_PopulatesFields)
{
    QUrl calendarUrl("https://caldav.example.com/cal");
    DCalDavTransport::Request request = DCalDavCalendarQuery::resourceListRequest(
        calendarUrl, "user1", "pass1");
    EXPECT_EQ(request.url, calendarUrl);
    EXPECT_EQ(request.method, "REPORT");
    EXPECT_EQ(request.contentType, "application/xml; charset=utf-8");
    EXPECT_EQ(request.username, QStringLiteral("user1"));
    EXPECT_EQ(request.password, QStringLiteral("pass1"));
    EXPECT_EQ(request.headers.value("Depth"), "1");
    EXPECT_TRUE(request.body.contains("calendar-query"));
    EXPECT_TRUE(request.body.contains("VEVENT"));
}

TEST(DCalDavCalendarQueryTest, ResourceGetRequest_PopulatesFields)
{
    QUrl resourceUrl("https://caldav.example.com/cal/event1.ics");
    DCalDavTransport::Request request = DCalDavCalendarQuery::resourceGetRequest(
        resourceUrl, "user1", "pass1");
    EXPECT_EQ(request.url, resourceUrl);
    EXPECT_EQ(request.method, "GET");
    EXPECT_EQ(request.username, QStringLiteral("user1"));
    EXPECT_EQ(request.password, QStringLiteral("pass1"));
    EXPECT_TRUE(request.body.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ResourceMultiGetRequest_RelativeHrefs)
{
    QUrl calendarUrl("https://caldav.example.com/cal");
    QStringList hrefs = {"event1.ics", "event2.ics"};
    DCalDavTransport::Request request = DCalDavCalendarQuery::resourceMultiGetRequest(
        calendarUrl, "user1", "pass1", hrefs);
    EXPECT_EQ(request.url, calendarUrl);
    EXPECT_EQ(request.method, "REPORT");
    EXPECT_EQ(request.contentType, "application/xml; charset=utf-8");
    EXPECT_EQ(request.headers.value("Depth"), "1");
    EXPECT_TRUE(request.body.contains("calendar-multiget"));
    EXPECT_TRUE(request.body.contains("event1.ics"));
    EXPECT_TRUE(request.body.contains("event2.ics"));
}

TEST(DCalDavCalendarQueryTest, ResourceMultiGetRequest_AbsoluteUrlHrefs)
{
    QUrl calendarUrl("https://caldav.example.com/cal");
    QStringList hrefs = {"https://caldav.example.com/cal/event1.ics?param=1"};
    DCalDavTransport::Request request = DCalDavCalendarQuery::resourceMultiGetRequest(
        calendarUrl, "user1", "pass1", hrefs);
    EXPECT_TRUE(request.body.contains("/cal/event1.ics"));
    EXPECT_TRUE(request.body.contains("?param=1"));
}

TEST(DCalDavCalendarQueryTest, ResourceMultiGetRequest_EmptyHrefs)
{
    QUrl calendarUrl("https://caldav.example.com/cal");
    QStringList hrefs;
    DCalDavTransport::Request request = DCalDavCalendarQuery::resourceMultiGetRequest(
        calendarUrl, "user1", "pass1", hrefs);
    EXPECT_EQ(request.method, "REPORT");
    EXPECT_TRUE(request.body.contains("calendar-multiget"));
    EXPECT_FALSE(request.body.contains("<d:href>"));
}

TEST(DCalDavCalendarQueryTest, ResourceMultiGetRequest_SpecialCharsEscaped)
{
    QUrl calendarUrl("https://caldav.example.com/cal");
    QStringList hrefs = {"event<&>.ics"};
    DCalDavTransport::Request request = DCalDavCalendarQuery::resourceMultiGetRequest(
        calendarUrl, "user1", "pass1", hrefs);
    EXPECT_TRUE(request.body.contains("&amp;"));
    EXPECT_TRUE(request.body.contains("&lt;"));
    EXPECT_TRUE(request.body.contains("&gt;"));
}

// ---- parseResponse tests ----

TEST(DCalDavCalendarQueryTest, ParseResponse_EmptyMultistatus)
{
    QByteArray xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                     "<d:multistatus xmlns:d=\"DAV:\"/>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage);
    EXPECT_TRUE(result);
    EXPECT_TRUE(events.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResponse_InvalidXml)
{
    QByteArray xml = "not xml at all";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResponse_RootNotMultistatus)
{
    QByteArray xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                     "<d:foo xmlns:d=\"DAV:\"/>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResponse_SingleEventWithEtagAndData)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event1.ics</d:href>"
        "<d:propstat>"
        "<d:prop>"
        "<d:getetag>\"etag1\"</d:getetag>"
        "<d:getcontenttype>text/calendar; component=vevent</d:getcontenttype>"
        "<c:calendar-data>BEGIN:VCALENDAR\r\nBEGIN:VEVENT\r\nUID:event-001\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n</c:calendar-data>"
        "</d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
        "</d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].href, QStringLiteral("/cal/event1.ics"));
    EXPECT_EQ(events[0].etag, QStringLiteral("\"etag1\""));
    EXPECT_EQ(events[0].uid, QStringLiteral("event-001"));
    EXPECT_TRUE(events[0].hasSuccessfulStatus);
    EXPECT_FALSE(events[0].deleted);
}

TEST(DCalDavCalendarQueryTest, ParseResponse_DeletedEvent)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event2.ics</d:href>"
        "<d:propstat>"
        "<d:prop/>"
        "<d:status>HTTP/1.1 404 Not Found</d:status>"
        "</d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(events.size(), 1);
    EXPECT_TRUE(events[0].deleted);
    EXPECT_FALSE(events[0].hasSuccessfulStatus);
}

TEST(DCalDavCalendarQueryTest, ParseResponse_ResponseWithOnlyHref)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event3.ics</d:href>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage);
    EXPECT_TRUE(result);
    EXPECT_TRUE(events.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResponse_NullErrorMessagePointer)
{
    QByteArray xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                     "<d:multistatus xmlns:d=\"DAV:\"/>";
    DCalDavCalendarQuery::RemoteEventList events;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, nullptr);
    EXPECT_TRUE(result);
}

TEST(DCalDavCalendarQueryTest, ParseResponse_MultipleEvents)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/a.ics</d:href>"
        "<d:propstat><d:prop><d:getetag>\"e1\"</d:getetag></d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "<d:response>"
        "<d:href>/cal/b.ics</d:href>"
        "<d:propstat><d:prop><d:getetag>\"e2\"</d:getetag></d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(events.size(), 2);
    EXPECT_EQ(events[0].href, QStringLiteral("/cal/a.ics"));
    EXPECT_EQ(events[1].href, QStringLiteral("/cal/b.ics"));
}

// ---- parseResponseWithSyncToken tests ----

TEST(DCalDavCalendarQueryTest, ParseResponseWithSyncToken_ExtractsToken)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event1.ics</d:href>"
        "<d:propstat><d:prop><d:getetag>\"etag1\"</d:getetag></d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response>"
        "<d:sync-token>http://example.com/sync/123</d:sync-token>"
        "</d:multistatus>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString syncToken;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponseWithSyncToken(
        xml, events, &syncToken, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(events.size(), 1);
    EXPECT_EQ(syncToken, QStringLiteral("http://example.com/sync/123"));
}

TEST(DCalDavCalendarQueryTest, ParseResponseWithSyncToken_NullSyncTokenPointer)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\"/>"
        "<d:sync-token>token123</d:sync-token>";
    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponseWithSyncToken(
        xml, events, nullptr, &errorMessage);
    EXPECT_TRUE(result);
}

TEST(DCalDavCalendarQueryTest, ParseResponseWithSyncToken_TooLargeXml)
{
    QByteArray xml(5 * 1024 * 1024, 'x');
    DCalDavCalendarQuery::RemoteEventList events;
    QString syncToken;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResponseWithSyncToken(
        xml, events, &syncToken, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
}

// ---- parseResourceList tests ----

TEST(DCalDavCalendarQueryTest, ParseResourceList_ValidResources)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event1.ics</d:href>"
        "<d:propstat>"
        "<d:prop>"
        "<d:getetag>\"etag1\"</d:getetag>"
        "<d:getcontenttype>text/calendar; component=vevent</d:getcontenttype>"
        "</d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
        "</d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(resources.size(), 1);
    EXPECT_EQ(resources[0].href, QStringLiteral("/cal/event1.ics"));
    EXPECT_EQ(resources[0].etag, QStringLiteral("\"etag1\""));
    EXPECT_TRUE(resources[0].hasSuccessfulStatus);
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_DeletedResource)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event2.ics</d:href>"
        "<d:propstat><d:prop/>"
        "<d:status>HTTP/1.1 404 Not Found</d:status></d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_TRUE(result);
    EXPECT_TRUE(resources.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_ResourceWithStatus)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event3.ics</d:href>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(resources.size(), 1);
    EXPECT_TRUE(resources[0].hasSuccessfulStatus);
    EXPECT_FALSE(resources[0].deleted);
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_InvalidXml)
{
    QByteArray xml = "invalid";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_EmptyMultistatus)
{
    QByteArray xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                     "<d:multistatus xmlns:d=\"DAV:\"/>";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_TRUE(result);
    EXPECT_TRUE(resources.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_NullErrorMessagePointer)
{
    QByteArray xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                     "<d:multistatus xmlns:d=\"DAV:\"/>";
    DCalDavCalendarQuery::ResourceList resources;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, nullptr);
    EXPECT_TRUE(result);
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_TooLargeXml)
{
    QByteArray xml(5 * 1024 * 1024, 'x');
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_ContentTypeVEvent)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/event4.ics</d:href>"
        "<d:propstat>"
        "<d:prop>"
        "<d:getcontenttype>text/calendar; component=vevent</d:getcontenttype>"
        "</d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
        "</d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(resources.size(), 1);
    EXPECT_EQ(resources[0].href, QStringLiteral("/cal/event4.ics"));
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_HrefEndsWithIcs)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response>"
        "<d:href>/cal/data.ics</d:href>"
        "<d:propstat>"
        "<d:prop>"
        "<c:calendar-data>data</c:calendar-data>"
        "</d:prop>"
        "<d:status>HTTP/1.1 200 OK</d:status>"
        "</d:propstat>"
        "</d:response>"
        "</d:multistatus>";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_TRUE(result) << errorMessage.toStdString();
    ASSERT_EQ(resources.size(), 1);
}

TEST(DCalDavCalendarQueryTest, ParseResourceList_UnknownRootElementSkipped)
{
    QByteArray xml =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:unknown>data</d:unknown>"
        "</d:multistatus>";
    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    bool result = DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage);
    EXPECT_TRUE(result);
    EXPECT_TRUE(resources.isEmpty());
}
