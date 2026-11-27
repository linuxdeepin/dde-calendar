// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedule.h"

#include <gtest/gtest.h>

#include <QDateTime>
#include <QTimeZone>
#include <QDebug>

namespace {

// Helper: build a CalEvent that represents a detached occurrence at recurId.
CalendarCore::CalEvent makeExceptionEvent(const QDateTime &recurId,
                                           const QString &summary = {},
                                           bool thisAndFuture = false)
{
    CalendarCore::CalEvent event;
    event.setUid(QStringLiteral("test-uid"));
    event.setSummary(summary.isEmpty() ? QStringLiteral("Exception") : summary);
    event.setDtStart(recurId);
    event.setDtEnd(recurId.addSecs(3600));
    event.setRecurrenceId(recurId);
    event.setThisAndFuture(thisAndFuture);
    return event;
}

// Helper: create a daily-recurring schedule starting Jan 1 2025 10:00.
DSchedule::Ptr makeDailySchedule()
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setUid(QStringLiteral("test-uid"));
    schedule->setSummary(QStringLiteral("Daily Standup"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(10, 30, 0)));
    schedule->setRRuleType(DSchedule::RRule_Day);
    return schedule;
}

} // namespace

// ============================================================
// 1. Recurrence exception management
// ============================================================

// ---- setRecurrenceException / hasRecurrenceException / hasRecurrenceExceptions ----

TEST(DScheduleRecurrence, SetRecurrenceExceptionAddsEntry)
{
    DSchedule schedule;
    schedule.setUid(QStringLiteral("uid-1"));
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime recurId(QDate(2025, 1, 3), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(recurId, QStringLiteral("Moved")));

    EXPECT_TRUE(schedule.hasRecurrenceExceptions());
    EXPECT_TRUE(schedule.hasRecurrenceException(recurId));
    EXPECT_FALSE(schedule.hasRecurrenceException(QDateTime(QDate(2025, 1, 4), QTime(10, 0, 0))));
}

TEST(DScheduleRecurrence, SetRecurrenceExceptionWithoutRecurrenceIdIgnored)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));

    CalendarCore::CalEvent event;
    event.setSummary(QStringLiteral("No recur id"));
    // No setRecurrenceId → hasRecurrenceId() is false
    schedule.setRecurrenceException(event);

    EXPECT_FALSE(schedule.hasRecurrenceExceptions());
}

TEST(DScheduleRecurrence, SetRecurrenceExceptionThisAndFutureCreatesRange)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime recurId(QDate(2025, 1, 5), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(recurId, QStringLiteral("Future"), true));

    EXPECT_TRUE(schedule.hasRecurrenceException(recurId));
    EXPECT_TRUE(schedule.hasRecurrenceRangeAt(recurId));
    EXPECT_TRUE(schedule.hasRecurrenceOverride(recurId));
}

// ---- clearRecurrenceExceptions ----

TEST(DScheduleRecurrence, ClearRecurrenceExceptionsRemovesAll)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime id1(QDate(2025, 1, 2), QTime(10, 0, 0));
    const QDateTime id2(QDate(2025, 1, 5), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(id1));
    schedule.setRecurrenceException(makeExceptionEvent(id2));
    ASSERT_TRUE(schedule.hasRecurrenceExceptions());

    schedule.clearRecurrenceExceptions();
    EXPECT_FALSE(schedule.hasRecurrenceExceptions());
    EXPECT_FALSE(schedule.hasRecurrenceException(id1));
    EXPECT_FALSE(schedule.hasRecurrenceException(id2));
}

// ---- hasStoredRecurrenceExceptions / setRecurrenceExceptionsStored ----

TEST(DScheduleRecurrence, SetRecurrenceExceptionSetsStoredFlag)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    EXPECT_FALSE(schedule.hasStoredRecurrenceExceptions());

    const QDateTime recurId(QDate(2025, 1, 3), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(recurId));
    EXPECT_TRUE(schedule.hasStoredRecurrenceExceptions());
}

TEST(DScheduleRecurrence, ClearRecurrenceExceptionsClearsStoredFlag)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0))));
    ASSERT_TRUE(schedule.hasStoredRecurrenceExceptions());

    schedule.clearRecurrenceExceptions();
    EXPECT_FALSE(schedule.hasStoredRecurrenceExceptions());
}

TEST(DScheduleRecurrence, SetRecurrenceExceptionsStoredManualToggle)
{
    DSchedule schedule;
    EXPECT_FALSE(schedule.hasStoredRecurrenceExceptions());
    schedule.setRecurrenceExceptionsStored(true);
    EXPECT_TRUE(schedule.hasStoredRecurrenceExceptions());
    schedule.setRecurrenceExceptionsStored(false);
    EXPECT_FALSE(schedule.hasStoredRecurrenceExceptions());
}

// ---- recurrenceExceptions() list ----

TEST(DScheduleRecurrence, RecurrenceExceptionsReturnsList)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 2), QTime(10, 0, 0)), QStringLiteral("Ex1")));
    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 5), QTime(10, 0, 0)), QStringLiteral("Ex2")));

    const auto list = schedule.recurrenceExceptions();
    EXPECT_EQ(2, list.size());
}

TEST(DScheduleRecurrence, RecurrenceExceptionsEmptyWhenNoneSet)
{
    DSchedule schedule;
    const auto list = schedule.recurrenceExceptions();
    EXPECT_TRUE(list.isEmpty());
}

// ---- removeRecurrenceExceptionsFrom ----

TEST(DScheduleRecurrence, RemoveRecurrenceExceptionsFromRemovesFromCutoff)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime id1(QDate(2025, 1, 2), QTime(10, 0, 0));
    const QDateTime id2(QDate(2025, 1, 5), QTime(10, 0, 0));
    const QDateTime id3(QDate(2025, 1, 10), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(id1));
    schedule.setRecurrenceException(makeExceptionEvent(id2));
    schedule.setRecurrenceException(makeExceptionEvent(id3));
    ASSERT_EQ(3, schedule.recurrenceExceptions().size());

    // Remove from Jan 5 onwards (lowerBound includes id2)
    schedule.removeRecurrenceExceptionsFrom(id2);
    EXPECT_TRUE(schedule.hasRecurrenceException(id1));
    EXPECT_FALSE(schedule.hasRecurrenceException(id2));
    EXPECT_FALSE(schedule.hasRecurrenceException(id3));
}

TEST(DScheduleRecurrence, RemoveRecurrenceExceptionsFromInvalidDoesNothing)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 2), QTime(10, 0, 0))));
    schedule.removeRecurrenceExceptionsFrom(QDateTime());
    EXPECT_TRUE(schedule.hasRecurrenceExceptions());
}

// ---- pruneRecurrenceExceptionsAfter ----

TEST(DScheduleRecurrence, PruneRecurrenceExceptionsAfterRemovesNonRecurrenceIds)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    // Add an exception at a date that is NOT a valid recurrence start (Jan 3 15:00).
    const QDateTime badId(QDate(2025, 1, 3), QTime(15, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(badId, QStringLiteral("BadTime")));
    ASSERT_TRUE(schedule.hasRecurrenceException(badId));

    // Prune after Jan 4. The badId (Jan 3 15:00) is before the cutoff,
    // but mayIncludeRecurrenceId should return false for a non-aligned time,
    // so it gets pruned... actually pruneRecurrenceExceptionsAfter only
    // removes entries AFTER the cutoff. Let's use a cutoff before the bad entry.
    // Prune after Jan 2 — badId is Jan 3, which is after Jan 2.
    schedule.pruneRecurrenceExceptionsAfter(QDateTime(QDate(2025, 1, 2), QTime(23, 59, 59)));
    // badId is Jan 3 15:00 — daily recurrence starts are at 10:00, so
    // mayIncludeRecurrenceId(Jan 3 15:00) should be false → pruned.
    EXPECT_FALSE(schedule.hasRecurrenceException(badId));
}

TEST(DScheduleRecurrence, PruneRecurrenceExceptionsAfterInvalidDoesNothing)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0))));
    schedule.pruneRecurrenceExceptionsAfter(QDateTime());
    EXPECT_TRUE(schedule.hasRecurrenceExceptions());
}

// ---- inheritRecurrenceExceptions ----

TEST(DScheduleRecurrence, InheritRecurrenceExceptionsFromSameAnchor)
{
    DSchedule source;
    source.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    source.setRRuleType(DSchedule::RRule_Day);
    source.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0)), QStringLiteral("Inherited")));

    DSchedule target;
    target.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    target.setRRuleType(DSchedule::RRule_Day);

    EXPECT_TRUE(target.inheritRecurrenceExceptions(source));
    EXPECT_TRUE(target.hasRecurrenceException(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0))));
}

TEST(DScheduleRecurrence, InheritRecurrenceExceptionsClearsExistingFirst)
{
    DSchedule source;
    source.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    source.setRRuleType(DSchedule::RRule_Day);
    source.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 5), QTime(10, 0, 0)), QStringLiteral("Src")));

    DSchedule target;
    target.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    target.setRRuleType(DSchedule::RRule_Day);
    target.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 2), QTime(10, 0, 0)), QStringLiteral("Old")));
    ASSERT_TRUE(target.hasRecurrenceException(QDateTime(QDate(2025, 1, 2), QTime(10, 0, 0))));

    EXPECT_TRUE(target.inheritRecurrenceExceptions(source));
    // Old exception should be gone (cleared then inherited from source)
    EXPECT_FALSE(target.hasRecurrenceException(QDateTime(QDate(2025, 1, 2), QTime(10, 0, 0))));
    EXPECT_TRUE(target.hasRecurrenceException(QDateTime(QDate(2025, 1, 5), QTime(10, 0, 0))));
}

// ---- updateRecurrenceException ----

TEST(DScheduleRecurrence, UpdateRecurrenceExceptionReplacesEntry)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime recurId(QDate(2025, 1, 3), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(recurId, QStringLiteral("Original")));
    ASSERT_TRUE(schedule.hasRecurrenceException(recurId));

    // Update with a new event at the same recurrenceId
    schedule.updateRecurrenceException(makeExceptionEvent(recurId, QStringLiteral("Updated")));
    EXPECT_TRUE(schedule.hasRecurrenceException(recurId));
    // The list should still have exactly one entry at this id
    EXPECT_EQ(1, schedule.recurrenceExceptions().size());
}

TEST(DScheduleRecurrence, UpdateRecurrenceExceptionWithThisAndFutureThenNormal)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime recurId(QDate(2025, 1, 5), QTime(10, 0, 0));
    // First set a thisAndFuture range
    schedule.setRecurrenceException(makeExceptionEvent(recurId, QStringLiteral("Range"), true));
    ASSERT_TRUE(schedule.hasRecurrenceRangeAt(recurId));

    // Then update with a non-thisAndFuture event at the same id.
    // This should trigger rangeContinuation to create a new exception.
    schedule.updateRecurrenceException(makeExceptionEvent(recurId, QStringLiteral("Single"), false));
    EXPECT_TRUE(schedule.hasRecurrenceException(recurId));
}

// ============================================================
// 2. Occurrence/range handling (public API)
// ============================================================

// ---- hasRecurrenceOverride ----

TEST(DScheduleRecurrence, HasRecurrenceOverrideForException)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime recurId(QDate(2025, 1, 3), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(recurId));
    EXPECT_TRUE(schedule.hasRecurrenceOverride(recurId));
    EXPECT_FALSE(schedule.hasRecurrenceOverride(QDateTime(QDate(2025, 1, 4), QTime(10, 0, 0))));
}

TEST(DScheduleRecurrence, HasRecurrenceOverrideForRange)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime rangeId(QDate(2025, 1, 5), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(rangeId, QStringLiteral("Range"), true));
    // hasRecurrenceOverride = hasRecurrenceException || hasRecurrenceRangeAt
    EXPECT_TRUE(schedule.hasRecurrenceOverride(rangeId));
}

TEST(DScheduleRecurrence, HasRecurrenceOverrideFalseForInvalidId)
{
    DSchedule schedule;
    EXPECT_FALSE(schedule.hasRecurrenceOverride(QDateTime()));
}

// ---- hasRecurrenceRangeAt ----

TEST(DScheduleRecurrence, HasRecurrenceRangeAtFalseWhenNoRanges)
{
    DSchedule schedule;
    EXPECT_FALSE(schedule.hasRecurrenceRangeAt(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0))));
}

TEST(DScheduleRecurrence, HasRecurrenceRangeAtFalseForInvalidId)
{
    DSchedule schedule;
    EXPECT_FALSE(schedule.hasRecurrenceRangeAt(QDateTime()));
}

TEST(DScheduleRecurrence, HasRecurrenceRangeAtTrueAfterRangeStart)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime rangeId(QDate(2025, 1, 10), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(rangeId, QStringLiteral("Range"), true));

    // id >= firstKey of ranges → true
    EXPECT_TRUE(schedule.hasRecurrenceRangeAt(rangeId));
    // A later date is also >= firstKey → true
    EXPECT_TRUE(schedule.hasRecurrenceRangeAt(QDateTime(QDate(2025, 2, 1), QTime(10, 0, 0))));
    // A date before the range start → false
    EXPECT_FALSE(schedule.hasRecurrenceRangeAt(QDateTime(QDate(2025, 1, 5), QTime(10, 0, 0))));
}

// ---- recurrenceException(id) ----

TEST(DScheduleRecurrence, RecurrenceExceptionReturnsExceptionForExistingId)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const QDateTime recurId(QDate(2025, 1, 3), QTime(10, 0, 0));
    schedule.setRecurrenceException(makeExceptionEvent(recurId, QStringLiteral("Detached")));

    auto result = schedule.recurrenceException(recurId);
    ASSERT_NE(nullptr, result.data());
    EXPECT_EQ(QStringLiteral("Detached"), result->summary());
}

TEST(DScheduleRecurrence, RecurrenceExceptionReturnsNullForNonExistentId)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    auto result = schedule.recurrenceException(QDateTime(QDate(2025, 1, 20), QTime(10, 0, 0)));
    EXPECT_EQ(nullptr, result.data());
}

TEST(DScheduleRecurrence, RecurrenceExceptionReturnsNullForCancelledException)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    CalendarCore::CalEvent cancelledEvent = makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0)));
    cancelledEvent.setCancelled(true);
    schedule.setRecurrenceException(cancelledEvent);

    auto result = schedule.recurrenceException(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0)));
    EXPECT_EQ(nullptr, result.data());
}

// ---- recurrenceExceptionsInInterval ----

TEST(DScheduleRecurrence, RecurrenceExceptionsInIntervalReturnsMatchingExceptions)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0)), QStringLiteral("Ex3")));
    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 15), QTime(10, 0, 0)), QStringLiteral("Ex15")));

    // Query Jan 1 – Jan 10
    const auto result = schedule.recurrenceExceptionsInInterval(
        QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)),
        QDateTime(QDate(2025, 1, 10), QTime(23, 59, 59)));
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(QStringLiteral("Ex3"), result.at(0)->summary());
}

TEST(DScheduleRecurrence, RecurrenceExceptionsInIntervalEmptyWhenNoneOverlap)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    schedule.setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 6, 1), QTime(10, 0, 0))));

    const auto result = schedule.recurrenceExceptionsInInterval(
        QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)),
        QDateTime(QDate(2025, 1, 31), QTime(23, 59, 59)));
    EXPECT_TRUE(result.isEmpty());
}

TEST(DScheduleRecurrence, RecurrenceExceptionsInIntervalEmptyWhenNoExceptions)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    const auto result = schedule.recurrenceExceptionsInInterval(
        QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)),
        QDateTime(QDate(2025, 1, 31), QTime(23, 59, 59)));
    EXPECT_TRUE(result.isEmpty());
}

// ============================================================
// 3. ICS parsing — fromStoredIcsString / parseIcsString existingData
// ============================================================

TEST(DScheduleRecurrence, FromStoredIcsStringParsesValidIcs)
{
    DSchedule::Ptr original = makeDailySchedule();
    original->setScheduleTypeID(QStringLiteral("stored-type"));
    const QString ics = DSchedule::toIcsString(original);
    ASSERT_FALSE(ics.isEmpty());

    DSchedule::Ptr parsed;
    EXPECT_TRUE(DSchedule::fromStoredIcsString(parsed, ics));
    ASSERT_NE(nullptr, parsed.data());
    EXPECT_EQ(original->summary(), parsed->summary());
    EXPECT_EQ(original->dtStart(), parsed->dtStart());
}

TEST(DScheduleRecurrence, FromStoredIcsStringReturnsFalseOnInvalidIcs)
{
    DSchedule::Ptr parsed;
    EXPECT_FALSE(DSchedule::fromStoredIcsString(parsed, QStringLiteral("not an ics string")));
}

TEST(DScheduleRecurrence, FromStoredIcsStringParsesWithExceptions)
{
    DSchedule::Ptr original = makeDailySchedule();
    original->setRecurrenceException(makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0)), QStringLiteral("Detached")));
    const QString ics = DSchedule::toIcsString(original);
    ASSERT_FALSE(ics.isEmpty());

    DSchedule::Ptr parsed;
    ASSERT_TRUE(DSchedule::fromStoredIcsString(parsed, ics));
    ASSERT_NE(nullptr, parsed.data());
    // The detached exception should be parsed back as a recurrence exception.
    EXPECT_TRUE(parsed->hasRecurrenceExceptions());
}

TEST(DScheduleRecurrence, FromIcsStringVsFromStoredIcsStringBothParse)
{
    DSchedule::Ptr original = makeDailySchedule();
    const QString ics = DSchedule::toIcsString(original);

    DSchedule::Ptr parsedStrict;
    EXPECT_TRUE(DSchedule::fromIcsString(parsedStrict, ics));

    DSchedule::Ptr parsedStored;
    EXPECT_TRUE(DSchedule::fromStoredIcsString(parsedStored, ics));

    EXPECT_EQ(parsedStrict->summary(), parsedStored->summary());
    EXPECT_EQ(parsedStrict->dtStart(), parsedStored->dtStart());
}

// ============================================================
// 4. Pinyin search — titleMatches
// ============================================================

TEST(DScheduleRecurrence, TitleMatchesEmptyKeyReturnsTrue)
{
    DSchedule schedule;
    schedule.setSummary(QStringLiteral("Some Event"));
    EXPECT_TRUE(schedule.titleMatches(QString()));
    EXPECT_TRUE(schedule.titleMatches(QStringLiteral("")));
}

TEST(DScheduleRecurrence, TitleMatchesExactSubstringCaseInsensitive)
{
    DSchedule schedule;
    schedule.setSummary(QStringLiteral("Project Meeting"));
    EXPECT_TRUE(schedule.titleMatches(QStringLiteral("meeting")));
    EXPECT_TRUE(schedule.titleMatches(QStringLiteral("Project")));
    EXPECT_TRUE(schedule.titleMatches(QStringLiteral("PROJECT MEETING")));
}

TEST(DScheduleRecurrence, TitleMatchesFalseForNonMatchingKey)
{
    DSchedule schedule;
    schedule.setSummary(QStringLiteral("Project Meeting"));
    EXPECT_FALSE(schedule.titleMatches(QStringLiteral("zzzznotfound")));
}

TEST(DScheduleRecurrence, TitleMatchesChineseSummary)
{
    DSchedule schedule;
    schedule.setSummary(QStringLiteral("项目会议"));
    EXPECT_TRUE(schedule.titleMatches(QStringLiteral("项目")));
    EXPECT_TRUE(schedule.titleMatches(QStringLiteral("会议")));
    EXPECT_FALSE(schedule.titleMatches(QStringLiteral("不存在")));
}

// ============================================================
// 5. Alarm detection — hasEnabledAlarms
// ============================================================

TEST(DScheduleRecurrence, HasEnabledAlarmsFalseWhenNoAlarms)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    EXPECT_FALSE(schedule.hasEnabledAlarms());
}

TEST(DScheduleRecurrence, HasEnabledAlarmsTrueWhenAlarmSet)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_15Min_Front);
    EXPECT_TRUE(schedule.hasEnabledAlarms());
}

TEST(DScheduleRecurrence, HasEnabledAlarmsFalseWhenAlarmCleared)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_15Min_Front);
    ASSERT_TRUE(schedule.hasEnabledAlarms());

    schedule.setAlarmType(DSchedule::Alarm_None);
    EXPECT_FALSE(schedule.hasEnabledAlarms());
}

TEST(DScheduleRecurrence, HasEnabledAlarmsTrueWhenExceptionHasAlarm)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);
    // Master has no alarm
    ASSERT_FALSE(schedule.hasEnabledAlarms());

    // Add an exception with an alarm
    CalendarCore::CalEvent exEvent = makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0)), QStringLiteral("With Alarm"));
    auto alarm = CalendarCore::Remind::Ptr::create();
    alarm->setEnabled(true);
    alarm->setType(CalendarCore::Remind::Display);
    alarm->setDisplayAlarm(QStringLiteral("Reminder"));
    alarm->setStartOffset(CalendarCore::TimeSpan(-15 * 60));
    exEvent.addAlarm(alarm);
    schedule.setRecurrenceException(exEvent);

    EXPECT_TRUE(schedule.hasEnabledAlarms());
}

TEST(DScheduleRecurrence, HasEnabledAlarmsFalseWhenExceptionCancelled)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    // Cancelled exception with alarm should NOT trigger hasEnabledAlarms
    CalendarCore::CalEvent exEvent = makeExceptionEvent(QDateTime(QDate(2025, 1, 3), QTime(10, 0, 0)), QStringLiteral("Cancelled"));
    exEvent.setCancelled(true);
    auto alarm = CalendarCore::Remind::Ptr::create();
    alarm->setEnabled(true);
    alarm->setType(CalendarCore::Remind::Display);
    alarm->setDisplayAlarm(QStringLiteral("Reminder"));
    alarm->setStartOffset(CalendarCore::TimeSpan(-15 * 60));
    exEvent.addAlarm(alarm);
    schedule.setRecurrenceException(exEvent);

    EXPECT_FALSE(schedule.hasEnabledAlarms());
}

// ---- Alarm duration mapping (getAlarmType round-trip) ----

TEST(DScheduleRecurrence, AlarmTypeRoundTrip15Min)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_15Min_Front);
    EXPECT_EQ(DSchedule::Alarm_15Min_Front, schedule.getAlarmType());
}

TEST(DScheduleRecurrence, AlarmTypeRoundTrip1Hour)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_1Hour_Front);
    EXPECT_EQ(DSchedule::Alarm_1Hour_Front, schedule.getAlarmType());
}

TEST(DScheduleRecurrence, AlarmTypeRoundTrip1Day)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_1Day_Front);
    EXPECT_EQ(DSchedule::Alarm_1Day_Front, schedule.getAlarmType());
}

TEST(DScheduleRecurrence, AlarmTypeRoundTrip1Week)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_1Week_Front);
    EXPECT_EQ(DSchedule::Alarm_1Week_Front, schedule.getAlarmType());
}

TEST(DScheduleRecurrence, AlarmTypeRoundTripNone)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_None);
    EXPECT_EQ(DSchedule::Alarm_None, schedule.getAlarmType());
}

TEST(DScheduleRecurrence, AlarmTypeRoundTripBegin)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setAlarmType(DSchedule::Alarm_Begin);
    EXPECT_EQ(DSchedule::Alarm_Begin, schedule.getAlarmType());
}
