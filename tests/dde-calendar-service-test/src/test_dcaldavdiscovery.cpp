// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavdiscovery.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QList>

TEST(DCalDavDiscoveryTest, DiscoveryCandidates_ValidHttpsUrl)
{
    QUrl serverUrl("https://caldav.example.com/cal");
    QList<QUrl> candidates = DCalDavDiscovery::discoveryCandidates(serverUrl);
    ASSERT_EQ(candidates.size(), 2);
    EXPECT_EQ(candidates[0].path(), QStringLiteral("/.well-known/caldav"));
    EXPECT_EQ(candidates[1].toString(), serverUrl.toString());
}

TEST(DCalDavDiscoveryTest, DiscoveryCandidates_InvalidUrl)
{
    QList<QUrl> candidates = DCalDavDiscovery::discoveryCandidates(QUrl());
    EXPECT_TRUE(candidates.isEmpty());
}

TEST(DCalDavDiscoveryTest, DiscoveryCandidates_AlreadyWellKnown)
{
    QUrl wellKnown("https://caldav.example.com/.well-known/caldav");
    QList<QUrl> candidates = DCalDavDiscovery::discoveryCandidates(wellKnown);
    ASSERT_EQ(candidates.size(), 1);
    EXPECT_EQ(candidates[0].path(), QStringLiteral("/.well-known/caldav"));
}

TEST(DCalDavDiscoveryTest, DiscoveryCandidates_StripsQueryAndFragment)
{
    QUrl serverUrl("https://caldav.example.com/cal?token=abc123#frag");
    QList<QUrl> candidates = DCalDavDiscovery::discoveryCandidates(serverUrl);
    ASSERT_GE(candidates.size(), 1);
    EXPECT_TRUE(candidates[0].query().isEmpty());
    EXPECT_TRUE(candidates[0].fragment().isEmpty());
}

TEST(DCalDavDiscoveryTest, ResolveHref_RelativePath)
{
    QUrl baseUrl("https://caldav.example.com/principals/users/john/");
    QString href = "calendars/cal1/";
    QUrl resolved = DCalDavDiscovery::resolveHref(baseUrl, href);
    EXPECT_TRUE(resolved.isValid());
    EXPECT_EQ(resolved.toString(), QStringLiteral("https://caldav.example.com/principals/users/john/calendars/cal1/"));
}

TEST(DCalDavDiscoveryTest, ResolveHref_AbsoluteSameOrigin)
{
    QUrl baseUrl("https://caldav.example.com/principals/users/john/");
    QString href = "https://caldav.example.com/calendars/cal1/";
    QUrl resolved = DCalDavDiscovery::resolveHref(baseUrl, href);
    EXPECT_TRUE(resolved.isValid());
    EXPECT_EQ(resolved.toString(), QStringLiteral("https://caldav.example.com/calendars/cal1/"));
}

TEST(DCalDavDiscoveryTest, ResolveHref_CrossOrigin)
{
    QUrl baseUrl("https://caldav.example.com/principals/users/john/");
    QString href = "https://other.example.com/calendars/cal1/";
    QUrl resolved = DCalDavDiscovery::resolveHref(baseUrl, href);
    EXPECT_FALSE(resolved.isValid());
}

TEST(DCalDavDiscoveryTest, ResolveHref_InvalidBase)
{
    QUrl resolved = DCalDavDiscovery::resolveHref(QUrl(), "calendars/cal1/");
    EXPECT_FALSE(resolved.isValid());
}

TEST(DCalDavDiscoveryTest, CurrentUserPrincipalRequest_PopulatesFields)
{
    QUrl url("https://caldav.example.com/.well-known/caldav");
    DCalDavTransport::Request request = DCalDavDiscovery::currentUserPrincipalRequest(
        url, "user1", "pass1");
    EXPECT_EQ(request.url, url);
    EXPECT_EQ(request.method, "PROPFIND");
    EXPECT_EQ(request.contentType, "application/xml; charset=utf-8");
    EXPECT_EQ(request.username, QStringLiteral("user1"));
    EXPECT_EQ(request.password, QStringLiteral("pass1"));
    EXPECT_EQ(request.headers.value("Depth"), "0");
    EXPECT_TRUE(request.body.contains("current-user-principal"));
    EXPECT_TRUE(request.body.contains("displayname"));
}

TEST(DCalDavDiscoveryTest, CalendarHomeSetRequest_PopulatesFields)
{
    QUrl url("https://caldav.example.com/principal/user1/");
    DCalDavTransport::Request request = DCalDavDiscovery::calendarHomeSetRequest(
        url, "user2", "pass2");
    EXPECT_EQ(request.url, url);
    EXPECT_EQ(request.method, "PROPFIND");
    EXPECT_EQ(request.contentType, "application/xml; charset=utf-8");
    EXPECT_EQ(request.username, QStringLiteral("user2"));
    EXPECT_EQ(request.password, QStringLiteral("pass2"));
    EXPECT_EQ(request.headers.value("Depth"), "0");
    EXPECT_TRUE(request.body.contains("calendar-home-set"));
    EXPECT_TRUE(request.body.contains("displayname"));
}

TEST(DCalDavDiscoveryTest, CalendarCollectionsRequest_PopulatesFields)
{
    QUrl url("https://caldav.example.com/calendars/user1/");
    DCalDavTransport::Request request = DCalDavDiscovery::calendarCollectionsRequest(
        url, "user3", "pass3");
    EXPECT_EQ(request.url, url);
    EXPECT_EQ(request.method, "PROPFIND");
    EXPECT_EQ(request.contentType, "application/xml; charset=utf-8");
    EXPECT_EQ(request.username, QStringLiteral("user3"));
    EXPECT_EQ(request.password, QStringLiteral("pass3"));
    EXPECT_EQ(request.headers.value("Depth"), "1");
    EXPECT_TRUE(request.body.contains("resourcetype"));
    EXPECT_TRUE(request.body.contains("displayname"));
    EXPECT_TRUE(request.body.contains("calendar-color"));
    EXPECT_TRUE(request.body.contains("current-user-privilege-set"));
}
