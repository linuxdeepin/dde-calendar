// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QTimeZone>
#include <memory>

#include "calstore.h"
#include "calevent.h"
#include "memcalstore.h"
#include "icscodec.h"
#include "dschedule.h"
#include "dschedulequerypar.h"

#include "golden_util.h"

namespace CK = CalendarCore;

static QString loadFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QTextStream(&f).readAll();
}

static QStringList listFixtureFiles()
{
    QDir dir(FIXTURES_DIR);
    QStringList filters;
    filters << "*.ics";
    return dir.entryList(filters, QDir::Files);
}

static QString formatExpansion(CK::MemCalStore::Ptr cal, const QString &fixtureName)
{
    QString fixturePath = QString(FIXTURES_DIR) + "/" + fixtureName;
    QString icsContent = loadFile(fixturePath);
    if (icsContent.isEmpty()) return QString();

    CK::IcsCodec format;
    if (!format.fromString(cal, icsContent)) return QString();

    const QDateTime from(QDate(2024, 1, 1), QTime(0, 0));
    const QDateTime through(QDate(2028, 12, 31), QTime(23, 59, 59));
    QString result;
    QTextStream stream(&result);

    for (const auto &event : cal->events()) {
        stream << "Event:" << event->uid() << "\n";
        stream << "Summary:" << event->summary() << "\n";

        QList<QDateTime> instances;
        if (event->recurs()) {
            instances = event->recurrence()->timesInInterval(from, through);
        } else if (event->dtStart() >= from && event->dtStart() <= through) {
            instances.append(event->dtStart());
        }
        stream << "Count:" << instances.size() << "\n";
        for (const auto &start : instances) {
            stream << "  " << (event->allDay() ? start.date().toString(Qt::ISODate) : start.toString(Qt::ISODate))
                   << "|" << (event->allDay() ? "allday" : "timed") << "\n";
        }
        stream << "\n";
    }

    stream.flush();
    return result;
}

class RecurrenceExpansionTest : public ::testing::TestWithParam<QString> {};

TEST_P(RecurrenceExpansionTest, FiveYearExpansionTable)
{
    CK::MemCalStore::Ptr cal(new CK::MemCalStore(QTimeZone("Asia/Shanghai")));
    QString result = formatExpansion(cal, GetParam());
    ASSERT_FALSE(result.isEmpty()) << "Failed to process fixture: " << GetParam().toStdString();

    expectMatchesGolden(GetParam() + ".expansion.golden", result);
}

INSTANTIATE_TEST_SUITE_P(Fixtures, RecurrenceExpansionTest,
    ::testing::ValuesIn(listFixtureFiles()));

TEST(RecurrenceExpansion, PreservesCustomVTimeZoneAcrossDst)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\n"
        "BEGIN:VTIMEZONE\r\nTZID:Custom/Eastern\r\n"
        "BEGIN:STANDARD\r\nDTSTART:19701101T020000\r\n"
        "RRULE:FREQ=YEARLY;BYMONTH=11;BYDAY=1SU\r\n"
        "TZOFFSETFROM:-0400\r\nTZOFFSETTO:-0500\r\nEND:STANDARD\r\n"
        "BEGIN:DAYLIGHT\r\nDTSTART:19700308T020000\r\n"
        "RRULE:FREQ=YEARLY;BYMONTH=3;BYDAY=2SU\r\n"
        "TZOFFSETFROM:-0500\r\nTZOFFSETTO:-0400\r\nEND:DAYLIGHT\r\n"
        "END:VTIMEZONE\r\nBEGIN:VEVENT\r\nUID:custom-zone\r\n"
        "DTSTART;TZID=Custom/Eastern:20240309T090000\r\n"
        "RRULE:FREQ=DAILY;COUNT=3\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n");
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, ics));
    const auto rule = store->events().constFirst()->recurrence()->defaultRRuleConst();
    const auto dates = rule->timesInInterval(QDateTime(QDate(2024, 3, 9), QTime(0, 0), Qt::UTC),
                                             QDateTime(QDate(2024, 3, 12), QTime(0, 0), Qt::UTC));
    ASSERT_EQ(dates.size(), 3);
    EXPECT_EQ(dates.at(0).time(), QTime(9, 0));
    EXPECT_EQ(dates.at(0).offsetFromUtc(), -5 * 3600);
    EXPECT_EQ(dates.at(1).time(), QTime(9, 0));
    EXPECT_EQ(dates.at(1).offsetFromUtc(), -4 * 3600);

    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("TZID:Custom/Eastern")));
    EXPECT_TRUE(output.contains(QStringLiteral("BYDAY=2SU")));
    EXPECT_TRUE(output.contains(QStringLiteral("DTSTART;TZID=Custom/Eastern:20240309T090000")));
    const auto reread = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    ASSERT_TRUE(codec.fromString(reread, output));
    const auto rereadRule = reread->events().constFirst()->recurrence()->defaultRRuleConst();
    const auto rereadDates = rereadRule->timesInInterval(
        QDateTime(QDate(2024, 3, 9), QTime(0, 0), Qt::UTC),
        QDateTime(QDate(2024, 3, 12), QTime(0, 0), Qt::UTC));
    ASSERT_EQ(rereadDates.size(), 3);
    EXPECT_EQ(rereadDates.at(1).time(), QTime(9, 0));
    EXPECT_EQ(rereadDates.at(1).offsetFromUtc(), -4 * 3600);

    const auto fallStore = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    QString fallIcs = ics;
    fallIcs.replace(QStringLiteral("20240309T090000"), QStringLiteral("20241026T090000"));
    fallIcs.remove(QStringLiteral(";COUNT=3"));
    ASSERT_TRUE(codec.fromString(fallStore, fallIcs));
    const auto fallRule = fallStore->events().constFirst()->recurrence()->defaultRRuleConst();
    const QDate fallDay(2024, 11, 4);
    const QList<QDateTime> expected{ QDateTime(fallDay, QTime(14, 0), Qt::UTC) };
    EXPECT_EQ(fallRule->timesInInterval(QDateTime(fallDay, QTime(13, 30), Qt::UTC),
                                       QDateTime(fallDay, QTime(15, 0), Qt::UTC)), expected);
}

TEST(RecurrenceExpansion, AppliesCountExceptionsAndExtraDatesAcrossDst)
{
    const QTimeZone zone("America/New_York");
    ASSERT_TRUE(zone.isValid());
    const auto store = CK::MemCalStore::Ptr::create(zone);
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:dst-count\r\n"
        "DTSTART;TZID=America/New_York:20240309T090000\r\nRRULE:FREQ=DAILY;COUNT=3\r\n"
        "EXDATE;TZID=America/New_York:20240310T090000\r\nRDATE;TZID=America/New_York:20240315T090000\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n");
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, ics));
    ASSERT_EQ(store->events().size(), 1);
    const auto dateTime = [&](int day) { return QDateTime(QDate(2024, 3, day), QTime(9, 0), zone); };
    const QList<QDateTime> expected{ dateTime(9), dateTime(11), dateTime(15) };
    const auto recurrence = store->events().constFirst()->recurrence();
    EXPECT_EQ(recurrence->timesInInterval(dateTime(9), dateTime(18)), expected);
    EXPECT_EQ(recurrence->durationTo(dateTime(18)), 3);

    // The emitted calendar must retain the zone ID and the event's wall-clock time.
    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("BEGIN:VTIMEZONE\r\nTZID:America/New_York\r\n")));
    EXPECT_TRUE(output.contains(QStringLiteral("TZOFFSETTO:-0400")));
    EXPECT_TRUE(output.contains(QStringLiteral("TZOFFSETTO:-0500")));
    const auto reread = CK::MemCalStore::Ptr::create(zone);
    ASSERT_TRUE(codec.fromString(reread, output));
    ASSERT_EQ(reread->events().size(), 1);
    EXPECT_EQ(reread->events().constFirst()->recurrence()->timesInInterval(dateTime(9), dateTime(18)), expected);
}

TEST(RecurrenceExpansion, CapsLargeCountAndFindsLaterOccurrences)
{
    const QDateTime start(QDate(2024, 3, 9), QTime(9, 0), Qt::UTC);
    for (const auto &rrule : { QStringLiteral("FREQ=MINUTELY;COUNT=20000"),
                              QStringLiteral("FREQ=MINUTELY;COUNT=20000;BYSECOND=0") }) {
        SCOPED_TRACE(rrule.toStdString());
        CK::RRule rule;
        ASSERT_TRUE(rule.parse(rrule));
        rule.setStartDt(start);
        const auto dates = rule.timesInInterval(start, start.addDays(14));
        ASSERT_EQ(dates.size(), 10000);
        EXPECT_EQ(dates.last(), start.addSecs(9999 * 60));

        const auto later = rule.timesInInterval(start.addSecs(19995 * 60 + 1), start.addDays(14));
        ASSERT_EQ(later.size(), 4);
        EXPECT_EQ(later.first(), start.addSecs(19996 * 60));
        EXPECT_EQ(later.last(), start.addSecs(19999 * 60));
        EXPECT_EQ(rule.durationTo(start.addSecs(15000 * 60)), 15001);
        EXPECT_EQ(rule.endDt(), later.last());
    }
}

TEST(RecurrenceExpansion, ReportsUnknownCountAndInvalidatesCachedEnd)
{
    const QDateTime start(QDate(2024, 3, 9), QTime(9, 0), Qt::UTC);
    CK::RRule rule;
    ASSERT_TRUE(rule.parse(QStringLiteral("FREQ=MINUTELY;COUNT=100001;BYSECOND=0")));
    rule.setStartDt(start);
    EXPECT_EQ(rule.durationTo(start.addSecs(100000 * 60)), -1);
    EXPECT_EQ(rule.duration(), 100001);
    EXPECT_FALSE(rule.endDt().isValid());
    rule.setDuration(3);
    EXPECT_EQ(rule.endDt(), start.addSecs(120));
    rule.setStartDt(start.addDays(1));
    EXPECT_EQ(rule.endDt(), start.addDays(1).addSecs(120));
}

TEST(RecurrenceExpansion, SeeksLargeCountInNamedTimeZone)
{
    const QTimeZone zone("America/New_York");
    ASSERT_TRUE(zone.isValid());
    const QDateTime start(QDate(2024, 1, 1), QTime(9, 0), zone);
    const QDateTime from(QDate(2024, 4, 30), QTime(9, 0), zone);
    CK::RRule rule;
    ASSERT_TRUE(rule.parse(QStringLiteral("FREQ=MINUTELY;COUNT=200000")));
    rule.setStartDt(start);

    const auto dates = rule.timesInInterval(from, from.addSecs(300));
    ASSERT_EQ(dates.size(), 6);
    EXPECT_EQ(dates.first(), from);
    EXPECT_EQ(dates.last(), from.addSecs(300));
    EXPECT_EQ(rule.durationTo(from), start.secsTo(from) / 60 + 1);
    EXPECT_EQ(rule.endDt(), start.addSecs(199999 * 60));
}

TEST(RecurrenceExpansion, SkipsNonexistentWallTimeWithoutConsumingCount)
{
    const QTimeZone zone("America/New_York");
    ASSERT_TRUE(zone.isValid());
    const QDateTime start(QDate(2024, 3, 9), QTime(2, 30), zone);
    const QDateTime next(QDate(2024, 3, 11), QTime(2, 30), zone);
    const QDateTime last(QDate(2024, 3, 12), QTime(2, 30), zone);
    CK::RRule rule;
    ASSERT_TRUE(rule.parse(QStringLiteral("FREQ=DAILY;COUNT=3")));
    rule.setStartDt(start);

    const QList<QDateTime> expected{ start, next, last };
    EXPECT_EQ(rule.timesInInterval(start, last.addDays(1)), expected);
    EXPECT_EQ(rule.durationTo(next), 2);
    EXPECT_EQ(rule.endDt(), last);
}

TEST(RecurrenceExpansion, ExpandsExplicitDatesOnLunarScheduleWithoutRule)
{
    DSchedule::Ptr schedule;
    ASSERT_TRUE(DSchedule::fromIcsString(schedule, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:lunar-rdate-only\r\n"
        "DTSTART;VALUE=DATE:20240210\r\nRDATE;VALUE=DATE:20240211\r\n"
        "X-DDE-ICAL-LUNNAR:TRUE\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n")));
    ASSERT_FALSE(schedule->recurrence()->defaultRRuleConst());
    ASSERT_TRUE(schedule->lunar());
    const auto query = DScheduleQueryPar::Ptr::create();
    query->setDtStart(QDateTime(QDate(2024, 2, 10), QTime(0, 0)));
    query->setDtEnd(QDateTime(QDate(2024, 2, 11), QTime(23, 59, 59)));
    const auto instances = DSchedule::convertSchedules(query, {schedule});
    EXPECT_EQ(instances.value(QDate(2024, 2, 10)).size(), 1);
    EXPECT_EQ(instances.value(QDate(2024, 2, 11)).size(), 1);
}

TEST(RecurrenceExpansion, UsesElapsedTimeForHourlyRulesAcrossDst)
{
    const QTimeZone zone("America/New_York");
    ASSERT_TRUE(zone.isValid());
    const QDate date(2024, 3, 10);
    const QDateTime start(date, QTime(0, 30), zone);
    const QDateTime through(date, QTime(6, 0), zone);
    const QList<QDateTime> expected{
        start,
        QDateTime(date, QTime(3, 30), zone),
        QDateTime(date, QTime(5, 30), zone)
    };
    for (const auto &rrule : {QStringLiteral("FREQ=HOURLY;INTERVAL=2"),
                              QStringLiteral("FREQ=HOURLY;INTERVAL=2;COUNT=3"),
                              QStringLiteral("FREQ=HOURLY;INTERVAL=2;UNTIL=20240310T093000Z")}) {
        SCOPED_TRACE(rrule.toStdString());
        CK::RRule rule;
        ASSERT_TRUE(rule.parse(rrule));
        rule.setStartDt(start);
        EXPECT_EQ(rule.timesInInterval(start, through), expected);
        EXPECT_EQ(rule.durationTo(through), expected.size());
        EXPECT_EQ(rule.timesInInterval(expected.at(1), through), expected.mid(1));
    }
}

TEST(RecurrenceExpansion, CountsUniqueSlotsInLegacyStoredByHourRule)
{
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nPRODID:-//K Desktop Environment//NONSGML libkcal 4.3//EN\r\n"
        "VERSION:2.0\r\nX-KDE-ICAL-IMPLEMENTATION-VERSION:1.0\r\n"
        "BEGIN:VEVENT\r\nUID:legacy-byhour\r\nDTSTART:20240101T090000Z\r\n"
        "RRULE:FREQ=DAILY;COUNT=5;BYHOUR=8,8,17,17\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n");
    DSchedule::Ptr schedule;
    ASSERT_TRUE(DSchedule::fromStoredIcsString(schedule, ics));
    const auto start = schedule->dtStart();
    const auto recurrence = schedule->recurrence();
    const auto dates = recurrence->timesInInterval(start, start.addDays(7));
    ASSERT_EQ(dates.size(), 5);
    EXPECT_EQ(dates.last(), QDateTime(QDate(2024, 1, 3), QTime(17, 0), Qt::UTC));
    EXPECT_EQ(recurrence->endDateTime(), dates.last());
    EXPECT_TRUE(DSchedule::toIcsString(schedule).contains(QStringLiteral("BYHOUR=8,17\r\n")));
}

TEST(RecurrenceExpansion, PreservesLegacyDateOnlyUntilOnTimedEvents)
{
    CK::RRule rule;
    ASSERT_TRUE(rule.parse(QStringLiteral("FREQ=DAILY;UNTIL=20240103")));
    const QDateTime start(QDate(2024, 1, 1), QTime(9, 0));
    rule.setStartDt(start);
    const auto end = start.addDays(2);
    const QList<QDateTime> expected{start, start.addDays(1)};
    EXPECT_EQ(rule.timesInInterval(start, end), expected);
    EXPECT_EQ(rule.endDt(), QDateTime(end.date(), QTime(0, 0)));
}

TEST(RecurrenceExpansion, AppliesThisAndFutureToStartsAndDurationsWithIndividualOverrides)
{
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:range\r\n"
        "DTSTART:20240101T090000Z\r\nDTEND:20240101T100000Z\r\nRRULE:FREQ=DAILY;COUNT=7\r\n"
        "SUMMARY:Original\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:range\r\n"
        "RECURRENCE-ID;RANGE=THISANDFUTURE:20240102T090000Z\r\nDTSTART:20240103T110000Z\r\n"
        "DTEND:20240103T130000Z\r\nSUMMARY:Shifted\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:range\r\n"
        "RECURRENCE-ID:20240104T090000Z\r\nDTSTART:20240104T080000Z\r\n"
        "DTEND:20240104T083000Z\r\nSUMMARY:Individual\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:range\r\n"
        "RECURRENCE-ID;RANGE=THISANDFUTURE:20240106T090000Z\r\nSTATUS:CANCELLED\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n");
    DSchedule::Ptr schedule;
    ASSERT_TRUE(DSchedule::fromIcsString(schedule, ics));
    const auto utc = [](int day, int hour) { return QDateTime(QDate(2024, 1, day), QTime(hour, 0), Qt::UTC); };
    auto query = DScheduleQueryPar::Ptr::create();
    query->setDtStart(utc(4, 10));
    query->setDtEnd(utc(8, 17));
    const auto occurrences = DSchedule::convertSchedules(query, {schedule});
    ASSERT_EQ(occurrences.size(), 2);
    const auto shifted = occurrences.value(utc(4, 11).toLocalTime().date());
    ASSERT_EQ(shifted.size(), 1);
    EXPECT_EQ(shifted.first()->recurrenceId(), utc(3, 9));
    EXPECT_EQ(shifted.first()->dtStart(), utc(4, 11));
    EXPECT_EQ(shifted.first()->dtEnd(), utc(4, 13));
    EXPECT_FALSE(shifted.first()->thisAndFuture());
    const auto individual = schedule->recurrenceException(utc(4, 9));
    ASSERT_TRUE(individual);
    EXPECT_EQ(individual->summary(), QStringLiteral("Individual"));
    EXPECT_FALSE(schedule->recurrenceException(utc(7, 9)));
    EXPECT_EQ(DSchedule::toIcsString(schedule).count(QStringLiteral("RANGE=THISANDFUTURE")), 2);
}

TEST(RecurrenceExpansion, TransfersMovedAndCancelledExceptionsToNewSeries)
{
    const auto oldStart = QDateTime(QDate(2024, 1, 1), QTime(9, 0), Qt::UTC);
    const auto previous = DSchedule::Ptr::create();
    previous->setDtStart(oldStart);
    CK::CalEvent exception;
    exception.setRecurrenceId(oldStart);
    exception.setCancelled(true);
    previous->setRecurrenceException(exception);
    exception.setRecurrenceId(oldStart.addDays(2));
    exception.setDtStart(oldStart.addDays(4));
    exception.setDtEnd(exception.dtStart().addSecs(3600));
    exception.setCancelled(false);
    previous->setRecurrenceException(exception);
    exception.setRecurrenceId(oldStart.addDays(3));
    exception.setCancelled(true);
    previous->setRecurrenceException(exception);

    const auto cutoff = oldStart.addDays(1);
    const auto newStart = cutoff.addSecs(3600);
    const auto next = DSchedule::Ptr::create();
    next->setDtStart(newStart);
    ASSERT_TRUE(next->inheritRecurrenceExceptions(*previous, cutoff));
    previous->removeRecurrenceExceptionsFrom(cutoff);

    ASSERT_EQ(previous->recurrenceExceptions().size(), 1);
    EXPECT_TRUE(previous->hasRecurrenceException(oldStart));
    ASSERT_EQ(next->recurrenceExceptions().size(), 2);
    const auto moved = next->recurrenceException(newStart.addDays(1));
    ASSERT_TRUE(moved);
    EXPECT_EQ(moved->dtStart(), oldStart.addDays(4));
    EXPECT_TRUE(next->hasRecurrenceException(newStart.addDays(2)));
    EXPECT_FALSE(next->recurrenceException(newStart.addDays(2)));
}

TEST(RecurrenceExpansion, PreservesLegacyFloatingUntilAcrossTimeZones)
{
    const QDateTime systemEnd(QDate(2024, 1, 3), QTime(9, 0), QTimeZone::systemTimeZone());
    const QString eventZone = systemEnd.offsetFromUtc() > -5 * 3600
        ? QStringLiteral("America/New_York") : QStringLiteral("Pacific/Kiritimati");
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nPRODID:-//K Desktop Environment//NONSGML libkcal 4.3//EN\r\n"
        "VERSION:2.0\r\nX-KDE-ICAL-IMPLEMENTATION-VERSION:1.0\r\n"
        "BEGIN:VEVENT\r\nUID:legacy-floating-until\r\nDTSTART;TZID=%1:20240101T090000\r\n"
        "RRULE:FREQ=DAILY;UNTIL=20240103T090000\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n").arg(eventZone);
    DSchedule::Ptr schedule;
    ASSERT_TRUE(DSchedule::fromStoredIcsString(schedule, ics));
    const auto start = schedule->dtStart();
    const auto through = start.addDays(5);
    QList<QDateTime> expected;
    for (auto time = start; time <= systemEnd; time = time.addDays(1)) {
        expected.append(time);
    }
    EXPECT_NE(expected.size(), 3);
    EXPECT_EQ(schedule->recurrence()->endDateTime(), systemEnd);
    EXPECT_EQ(schedule->recurrence()->timesInInterval(start, through), expected);

    CK::RRule legacyRule;
    ASSERT_TRUE(CK::IcsCodec().fromString(&legacyRule, QStringLiteral("FREQ=DAILY;UNTIL=20240103T090000")));
    legacyRule.setStartDt(start);
    EXPECT_EQ(legacyRule.timesInInterval(start, through), expected);

    DSchedule::Ptr reread;
    ASSERT_TRUE(DSchedule::fromIcsString(reread, DSchedule::toIcsString(schedule)));
    EXPECT_EQ(reread->recurrence()->timesInInterval(start, through), expected);

    QString otherCalendar = ics;
    otherCalendar.replace(QStringLiteral("-//K Desktop Environment//NONSGML libkcal 4.3//EN"), QStringLiteral("-//Test//EN"));
    ASSERT_TRUE(DSchedule::fromIcsString(reread, otherCalendar));
    EXPECT_EQ(reread->recurrence()->timesInInterval(start, through).size(), 3);
}

TEST(RecurrenceExpansion, PrunesOnlyDetachedSlotsOutsideAllInclusionRules)
{
    const QDateTime start(QDate(2024, 1, 1), QTime(9, 0), Qt::UTC);
    const auto schedule = DSchedule::Ptr::create();
    schedule->setDtStart(start);
    schedule->setRRuleType(DSchedule::RRule_Day);
    auto recurrence = schedule->recurrence();
    recurrence->setDuration(2);
    auto weekly = std::make_unique<CK::RRule>();
    ASSERT_TRUE(weekly->parse(QStringLiteral("FREQ=WEEKLY;COUNT=2")));
    recurrence->addRRule(weekly.release());
    const auto weeklyId = start.addDays(7);
    const auto extraId = start.addDays(9);
    const auto removedId = start.addDays(3);
    recurrence->addRDateTime(extraId);
    recurrence->addExDateTime(extraId);
    for (const auto &id : {weeklyId, extraId, removedId}) {
        CK::CalEvent exception;
        exception.setDtStart(id.addSecs(3600));
        exception.setRecurrenceId(id);
        schedule->setRecurrenceException(exception);
    }
    schedule->pruneRecurrenceExceptionsAfter(recurrence->defaultRRuleConst()->endDt());
    EXPECT_TRUE(schedule->hasRecurrenceException(weeklyId));
    EXPECT_TRUE(schedule->hasRecurrenceException(extraId));
    EXPECT_FALSE(schedule->hasRecurrenceException(removedId));
}

TEST(RecurrenceExpansion, ContinuesActiveRangesAcrossSeriesSplit)
{
    const QDateTime start(QDate(2024, 1, 1), QTime(9, 0), Qt::UTC);
    const auto cutoff = start.addDays(2);
    const auto newStart = cutoff.addSecs(3600);
    for (bool cancelledRange : {false, true}) {
        SCOPED_TRACE(cancelledRange);
        const auto previous = DSchedule::Ptr::create();
        previous->setDtStart(start);
        previous->setRRuleType(DSchedule::RRule_Day);
        previous->recurrence()->setDuration(7);
        CK::CalEvent range;
        range.setThisAndFuture(true);
        range.setRecurrenceId(start.addDays(1));
        range.setCancelled(cancelledRange);
        if (!cancelledRange) {
            range.setDtStart(range.recurrenceId().addSecs(7200));
            range.setDtEnd(range.dtStart().addSecs(1800));
        }
        previous->setRecurrenceException(range);

        CK::CalEvent exception;
        exception.setRecurrenceId(cutoff);
        exception.setCancelled(true);
        previous->setRecurrenceException(exception);

        const auto next = DSchedule::Ptr::create();
        next->setDtStart(newStart);
        next->setRRuleType(DSchedule::RRule_Day);
        next->recurrence()->setDuration(5);
        ASSERT_TRUE(next->inheritRecurrenceExceptions(*previous, cutoff));
        previous->removeRecurrenceExceptionsFrom(cutoff);
        ASSERT_EQ(previous->recurrenceExceptions().size(), 1);
        const auto overrides = next->recurrenceExceptions();
        ASSERT_EQ(overrides.size(), 2);
        EXPECT_TRUE(overrides.first()->cancelled());
        EXPECT_FALSE(overrides.first()->thisAndFuture());
        const auto continuation = overrides.last();
        EXPECT_TRUE(continuation->thisAndFuture());
        EXPECT_EQ(continuation->recurrenceId(), newStart.addDays(1));
        EXPECT_EQ(continuation->cancelled(), cancelledRange);
        if (cancelledRange) {
            EXPECT_FALSE(continuation->dtStart().isValid());
            EXPECT_FALSE(next->recurrenceException(newStart.addDays(2)));
        } else {
            const auto shifted = next->recurrenceException(newStart.addDays(2));
            ASSERT_TRUE(shifted);
            EXPECT_EQ(shifted->dtStart(), newStart.addDays(2).addSecs(7200));
            EXPECT_EQ(shifted->dtEnd(), shifted->dtStart().addSecs(1800));
        }
    }
}

TEST(RecurrenceExpansion, MovesPrunedRangeToTheFirstSurvivingOccurrence)
{
    const QDateTime start(QDate(2024, 1, 1), QTime(9, 0), Qt::UTC);
    const auto removedId = start.addDays(2);
    const auto survivingId = start.addDays(5);
    const auto laterId = start.addDays(6);
    for (bool cancelledRange : {false, true}) {
        SCOPED_TRACE(cancelledRange);
        const auto schedule = DSchedule::Ptr::create();
        schedule->setDtStart(start);
        schedule->setRRuleType(DSchedule::RRule_Day);
        const auto recurrence = schedule->recurrence();
        recurrence->setDuration(2);
        recurrence->addRDateTime(survivingId);
        recurrence->addRDateTime(laterId);
        CK::CalEvent range;
        range.setRecurrenceId(removedId);
        range.setThisAndFuture(true);
        range.setCancelled(cancelledRange);
        if (!cancelledRange) {
            range.setDtStart(removedId.addSecs(7200));
            range.setDtEnd(removedId.addSecs(9000));
        }
        schedule->setRecurrenceException(range);
        schedule->pruneRecurrenceExceptionsAfter(start.addDays(1));
        const auto exceptions = schedule->recurrenceExceptions();
        ASSERT_EQ(exceptions.size(), 1);
        EXPECT_EQ(exceptions.first()->recurrenceId(), survivingId);
        EXPECT_TRUE(exceptions.first()->thisAndFuture());
        EXPECT_EQ(exceptions.first()->cancelled(), cancelledRange);
        const auto later = schedule->recurrenceException(laterId);
        if (cancelledRange) {
            EXPECT_FALSE(later);
        } else {
            ASSERT_TRUE(later);
            EXPECT_EQ(later->dtStart(), laterId.addSecs(7200));
            EXPECT_EQ(later->dtEnd(), laterId.addSecs(9000));
        }
    }
}
