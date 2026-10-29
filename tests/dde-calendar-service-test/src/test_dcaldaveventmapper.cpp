// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldaveventmapper.h"
#include "dschedule.h"

#include <gtest/gtest.h>
#include <QDateTime>

namespace {

const char *kValidIcs =
    "BEGIN:VCALENDAR\r\n"
    "VERSION:2.0\r\n"
    "PRODID:-//Test//Test//EN\r\n"
    "BEGIN:VEVENT\r\n"
    "UID:test-uid-123\r\n"
    "DTSTAMP:20240101T000000Z\r\n"
    "DTSTART:20240101T090000Z\r\n"
    "DTEND:20240101T100000Z\r\n"
    "SUMMARY:Test Event\r\n"
    "END:VEVENT\r\n"
    "END:VCALENDAR\r\n";

DCalDavCalendarQuery::RemoteEvent makeRemoteEvent(const QString &uid, const QString &calendarData)
{
    DCalDavCalendarQuery::RemoteEvent event;
    event.uid = uid;
    event.calendarData = calendarData;
    return event;
}

} // namespace

TEST(DCalDavEventMapperTest, ToSchedule_ValidEvent)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent(
        "test-uid-123", QString::fromLatin1(kValidIcs));
    DSchedule::Ptr schedule;
    QString errorMessage;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, &errorMessage);
    EXPECT_TRUE(result);
    EXPECT_TRUE(errorMessage.isEmpty());
    ASSERT_FALSE(schedule.isNull());
    EXPECT_EQ(schedule->uid(), QStringLiteral("test-uid-123"));
}

TEST(DCalDavEventMapperTest, ToSchedule_ValidEvent_NoErrorMessagePointer)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent(
        "test-uid-123", QString::fromLatin1(kValidIcs));
    DSchedule::Ptr schedule;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, nullptr);
    EXPECT_TRUE(result);
    ASSERT_FALSE(schedule.isNull());
}

TEST(DCalDavEventMapperTest, ToSchedule_EmptyUid)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent(
        "", QString::fromLatin1(kValidIcs));
    DSchedule::Ptr schedule;
    QString errorMessage;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
    EXPECT_TRUE(schedule.isNull());
}

TEST(DCalDavEventMapperTest, ToSchedule_EmptyCalendarData)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent("test-uid-123", "");
    DSchedule::Ptr schedule;
    QString errorMessage;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
    EXPECT_TRUE(schedule.isNull());
}

TEST(DCalDavEventMapperTest, ToSchedule_InvalidIcsData)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent(
        "test-uid-123", "this is not valid ics data");
    DSchedule::Ptr schedule;
    QString errorMessage;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(DCalDavEventMapperTest, ToSchedule_UidMismatch)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent(
        "different-uid", QString::fromLatin1(kValidIcs));
    DSchedule::Ptr schedule;
    QString errorMessage;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, &errorMessage);
    EXPECT_FALSE(result);
    EXPECT_FALSE(errorMessage.isEmpty());
    EXPECT_TRUE(errorMessage.contains("UID"));
}

TEST(DCalDavEventMapperTest, ToSchedule_EmptyUidNoErrorMessagePointer)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent(
        "", QString::fromLatin1(kValidIcs));
    DSchedule::Ptr schedule;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, nullptr);
    EXPECT_FALSE(result);
}

TEST(DCalDavEventMapperTest, ToSchedule_EmptyCalendarDataNoErrorMessagePointer)
{
    DCalDavCalendarQuery::RemoteEvent remote = makeRemoteEvent("test-uid-123", "");
    DSchedule::Ptr schedule;
    bool result = DCalDavEventMapper::toSchedule(remote, schedule, nullptr);
    EXPECT_FALSE(result);
}
