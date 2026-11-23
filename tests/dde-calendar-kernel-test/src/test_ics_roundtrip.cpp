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
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <ctime>

#include "calstore.h"
#include "calevent.h"
#include "dschedule.h"
#include "memcalstore.h"
#include "icscodec.h"

#include "golden_util.h"

namespace CK = CalendarCore;

static QStringList listFixtureFiles()
{
    QDir dir(FIXTURES_DIR);
    QStringList filters;
    filters << "*.ics";
    return dir.entryList(filters, QDir::Files);
}

static QString loadFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QTextStream(&f).readAll();
}

class IcsRoundtripTest : public ::testing::TestWithParam<QString> {};

TEST_P(IcsRoundtripTest, ByteLevelRoundtrip)
{
    QString fixturePath = QString(FIXTURES_DIR) + "/" + GetParam();
    QString icsContent = loadFile(fixturePath);
    ASSERT_FALSE(icsContent.isEmpty()) << "Failed to load fixture: " << GetParam().toStdString();

    CK::MemCalStore::Ptr cal(new CK::MemCalStore(QTimeZone("Asia/Shanghai")));
    CK::IcsCodec format;
    ASSERT_TRUE(format.fromString(cal, icsContent)) << "Parse failed: " << GetParam().toStdString();

    QString output = format.toString(cal);
    ASSERT_FALSE(output.isEmpty()) << "toString returned empty for: " << GetParam().toStdString();

    expectMatchesGolden(GetParam() + ".roundtrip.golden", output);
}

INSTANTIATE_TEST_SUITE_P(Fixtures, IcsRoundtripTest,
    ::testing::ValuesIn(listFixtureFiles()));

TEST(IcsRoundtrip, PreservesEventAndAlarmExtensionsOnEdit)
{
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:extension-case\r\n"
        "DTSTART:20261119T080000Z\r\nSUMMARY:Original\r\n"
        "ATTACH;FMTTYPE=application/pdf:https://example.org/document.pdf\r\n"
        "CONFERENCE;VALUE=URI:https://example.org/room\r\n"
        "BEGIN:VALARM\r\nACTION:AUDIO\r\nTRIGGER:-PT15M\r\n"
        "ATTACH:https://example.org/sound.au\r\nEND:VALARM\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n");
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, ics));
    ASSERT_EQ(store->events().size(), 1);
    store->events().constFirst()->setSummary(QStringLiteral("Edited"));
    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("SUMMARY:Edited")));
    EXPECT_TRUE(output.contains(QStringLiteral("ATTACH;FMTTYPE=application/pdf:https://example.org/document.pdf")));
    EXPECT_TRUE(output.contains(QStringLiteral("CONFERENCE;VALUE=URI:https://example.org/room")));
    EXPECT_TRUE(output.contains(QStringLiteral("ATTACH:https://example.org/sound.au")));
}

TEST(IcsRoundtrip, PreservesIndividualTimeZonesOfEventProperties)
{
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\n"
        "BEGIN:VTIMEZONE\r\nTZID:Custom/Eastern\r\n"
        "BEGIN:STANDARD\r\nDTSTART:19700101T000000\r\n"
        "TZOFFSETFROM:-0400\r\nTZOFFSETTO:-0400\r\nEND:STANDARD\r\nEND:VTIMEZONE\r\n"
        "BEGIN:VTIMEZONE\r\nTZID:Custom/Pacific\r\n"
        "BEGIN:STANDARD\r\nDTSTART:19700101T000000\r\n"
        "TZOFFSETFROM:-0700\r\nTZOFFSETTO:-0700\r\nEND:STANDARD\r\nEND:VTIMEZONE\r\n"
        "BEGIN:VEVENT\r\nUID:mixed-zones\r\n"
        "DTSTART;TZID=Custom/Eastern:20240311T090000\r\n"
        "DTEND;TZID=Europe/Berlin:20240311T150000\r\n"
        "RECURRENCE-ID;TZID=Custom/Pacific:20240311T060000\r\n"
        "RRULE:FREQ=DAILY\r\nEXDATE;TZID=Custom/Pacific:20240312T060000\r\n"
        "RDATE;TZID=Europe/Berlin:20240313T090000\r\nRDATE:20240314T150000Z\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n");
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, ics));
    ASSERT_EQ(store->events().size(), 1);
    const auto original = store->events().constFirst();
    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("TZID:Custom/Pacific\r\n")));
    EXPECT_TRUE(output.contains(QStringLiteral("DTSTART;TZID=Custom/Eastern:20240311T090000")));
    EXPECT_TRUE(output.contains(QStringLiteral("DTEND;TZID=Europe/Berlin:20240311T150000")));
    EXPECT_TRUE(output.contains(QStringLiteral("RECURRENCE-ID;TZID=Custom/Pacific:20240311T060000")));
    EXPECT_TRUE(output.contains(QStringLiteral("EXDATE;TZID=Custom/Pacific:20240312T060000")));
    EXPECT_TRUE(output.contains(QStringLiteral("RDATE;TZID=Europe/Berlin:20240313T090000")));
    EXPECT_TRUE(output.contains(QStringLiteral("RDATE:20240314T150000Z")));

    const auto reread = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    ASSERT_TRUE(codec.fromString(reread, output));
    ASSERT_EQ(reread->events().size(), 1);
    const auto restored = reread->events().constFirst();
    EXPECT_EQ(restored->dtStart(), original->dtStart());
    EXPECT_EQ(restored->dtEnd(), original->dtEnd());
    EXPECT_EQ(restored->recurrenceId(), original->recurrenceId());
    EXPECT_EQ(restored->recurrence()->rDateTimes(), original->recurrence()->rDateTimes());
    EXPECT_EQ(restored->recurrence()->exDateTimes(), original->recurrence()->exDateTimes());
}

TEST(IcsRoundtrip, BindsUnzonedTimesAndKeepsLargeDuration)
{
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:floating\r\n"
        "DTSTART:20240311T090000\r\nDURATION:P100000DT1H\r\nRRULE:FREQ=DAILY;COUNT=3\r\n"
        "RDATE:20240315T090000\r\nEXDATE:20240312T090000\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n");
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, ics));
    ASSERT_EQ(store->events().size(), 1);
    const auto event = store->events().constFirst();
    EXPECT_EQ(event->dtStart(), QDateTime(QDate(2024, 3, 11), QTime(9, 0), Qt::UTC));
    EXPECT_EQ(event->duration().asSeconds(), 8640003600LL);
    EXPECT_EQ(event->dtStart().secsTo(event->dtEnd()), 8640003600LL);
    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("DTSTART:20240311T090000Z")));
    EXPECT_TRUE(output.contains(QStringLiteral("RDATE:20240315T090000Z")));
    EXPECT_TRUE(output.contains(QStringLiteral("EXDATE:20240312T090000Z")));
    EXPECT_FALSE(output.contains(QStringLiteral("VTIMEZONE")));
    const auto reread = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    ASSERT_TRUE(codec.fromString(reread, output));
    ASSERT_EQ(reread->events().size(), 1);
    EXPECT_EQ(reread->events().constFirst()->dtEnd(), event->dtEnd());
}

TEST(IcsRoundtrip, RejectsEmptyFilesAndReportsParseWarnings)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    QString diagnostic;
    EXPECT_FALSE(codec.load(store, file.fileName(), &diagnostic));
    EXPECT_FALSE(diagnostic.isEmpty());
    ASSERT_EQ(file.write(" \r\n\t"), 4);
    ASSERT_TRUE(file.flush());
    EXPECT_FALSE(codec.load(store, file.fileName(), &diagnostic));
    EXPECT_FALSE(diagnostic.isEmpty());

    ASSERT_TRUE(codec.fromString(store, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:diagnostic\r\n"
        "DTSTART:20240311T090000Z\r\nDTEND:INVALID\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n"), false, &diagnostic));
    EXPECT_TRUE(diagnostic.contains(QStringLiteral("malformed")));
}

TEST(IcsRoundtrip, SavesAtomicallyWhenTakingBackup)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("calendar.ics"));
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.save(store, path));
    const auto readBytes = [](const QString &name) {
        QFile file(name);
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    };
    const auto previous = readBytes(path);
    ASSERT_FALSE(previous.isEmpty());
    const auto backup = path + QLatin1Char('~');
    ASSERT_TRUE(QDir().mkdir(backup));
    store->setNonKDECustomProperty("X-WR-CALNAME", QStringLiteral("Updated"));
    EXPECT_FALSE(codec.save(store, path));
    EXPECT_EQ(readBytes(path), previous);
    EXPECT_TRUE(QFileInfo(backup).isDir());

    ASSERT_TRUE(QDir().rmdir(backup));
    ASSERT_TRUE(codec.save(store, path));
    EXPECT_EQ(readBytes(backup), previous);
    EXPECT_TRUE(readBytes(path).contains("X-WR-CALNAME:Updated"));
}

TEST(IcsRoundtrip, PreservesAttendeeLinesAndSerializesNewAttendees)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:attendees\r\n"
        "DTSTART:20240311T090000Z\r\nATTENDEE;CN=Alice:mailto:alice@example.test\r\n"
        "ATTENDEE;CN=Alice-Alternate:mailto:alice@example.test\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n")));
    ASSERT_EQ(store->events().size(), 1);
    const auto event = store->events().constFirst();
    ASSERT_EQ(event->attendees().size(), 1);
    event->addAttendee(CK::Participant(QStringLiteral("Bob"), QStringLiteral("bob@example.test")));
    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("CN=Alice:mailto:alice@example.test")));
    EXPECT_TRUE(output.contains(QStringLiteral("CN=Alice-Alternate:mailto:alice@example.test")));
    EXPECT_TRUE(output.contains(QStringLiteral("MAILTO:bob@example.test")));
}

TEST(CalStore, IndexesRecurrenceIdsAndRevisions)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    const auto master = CK::CalEvent::Ptr::create();
    auto detached = CK::CalEvent::Ptr::create(*master);
    detached->setRecurrenceId(QDateTime(QDate(2024, 3, 11), QTime(9, 0), Qt::UTC));
    ASSERT_TRUE(store->addEvent(master));
    ASSERT_TRUE(store->addEvent(detached));
    auto replacement = CK::CalEvent::Ptr::create(*detached);
    EXPECT_FALSE(store->addEvent(replacement));
    replacement->setRevision(1);
    ASSERT_TRUE(store->addEvent(replacement));
    ASSERT_EQ(store->events().size(), 2);
    EXPECT_EQ(store->events().at(1), replacement);
    store->removeEvent(master->uid(), {});
    replacement = CK::CalEvent::Ptr::create(*replacement);
    replacement->setRevision(2);
    ASSERT_TRUE(store->addEvent(replacement));
    ASSERT_EQ(store->events().size(), 1);
    EXPECT_EQ(store->events().constFirst(), replacement);
}

TEST(IcsRoundtrip, BindsFloatingUntilWithTheStartTime)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:floating-until\r\n"
        "DTSTART:20240309T090000\r\nRRULE:FREQ=DAILY;UNTIL=20240311T090000\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n")));
    ASSERT_EQ(store->events().size(), 1);
    const QDateTime start(QDate(2024, 3, 9), QTime(9, 0), Qt::UTC);
    const QDateTime end(QDate(2024, 3, 11), QTime(9, 0), Qt::UTC);
    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("RRULE:FREQ=DAILY;UNTIL=20240311T090000Z\r\n")));
    const auto reread = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    ASSERT_TRUE(codec.fromString(reread, output));
    ASSERT_EQ(reread->events().size(), 1);
    const auto recurrence = reread->events().constFirst()->recurrence();
    const QList<QDateTime> expected{ start, start.addDays(1), end };
    EXPECT_EQ(recurrence->timesInInterval(start, end.addDays(1)), expected);

    recurrence->setEndDateTime(end.addDays(1));
    EXPECT_TRUE(codec.toString(reread).contains(QStringLiteral("RRULE:FREQ=DAILY;UNTIL=20240312T090000Z\r\n")));
}

TEST(IcsRoundtrip, ExportsTheRulesUsedToReadAKnownTimeZone)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\n"
        "BEGIN:VTIMEZONE\r\nTZID:Europe/Berlin\r\n"
        "BEGIN:STANDARD\r\nDTSTART:19700101T000000\r\n"
        "TZOFFSETFROM:+0345\r\nTZOFFSETTO:+0345\r\n"
        "END:STANDARD\r\nEND:VTIMEZONE\r\n"
        "BEGIN:VEVENT\r\nUID:system-time-zone\r\nDTSTART;TZID=Europe/Berlin:20240309T090000\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n")));
    ASSERT_EQ(store->events().size(), 1);
    EXPECT_EQ(store->events().constFirst()->dtStart().offsetFromUtc(), 3600);
    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("BEGIN:VTIMEZONE\r\nTZID:Europe/Berlin\r\n")));
    EXPECT_TRUE(output.contains(QStringLiteral("TZOFFSETTO:+0100\r\n")));
    EXPECT_FALSE(output.contains(QStringLiteral("TZOFFSETTO:+0345\r\n")));
    EXPECT_TRUE(output.contains(QStringLiteral("DTSTART;TZID=Europe/Berlin:20240309T090000\r\n")));
}

TEST(IcsRoundtrip, RejectsInvalidRecurrenceWithEventDiagnostic)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    const CK::IcsCodec codec;
    const auto source = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:valid-event\r\n"
        "DTSTART:20250301T000000Z\r\nEND:VEVENT\r\nBEGIN:VEVENT\r\nUID:bad-rule\r\n"
        "DTSTART:20250301T000000Z\r\n%1\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n");
    QString diagnostic;
    for (const auto &rule : {QStringLiteral("RRULE:FREQ=DAILY;UNTIL=20250230T000000Z"),
                             QStringLiteral("EXRULE:FREQ=INVALID")}) {
        SCOPED_TRACE(rule.toStdString());
        EXPECT_FALSE(codec.fromString(store, source.arg(rule), false, &diagnostic));
        EXPECT_TRUE(diagnostic.contains(rule.section(QLatin1Char(':'), 0, 0)));
        EXPECT_TRUE(diagnostic.contains(QStringLiteral("bad-rule")));
        EXPECT_TRUE(store->events().isEmpty());
    }
}

TEST(IcsRoundtrip, RejectsInvalidUtf8WithoutChangingStore)
{
    QTemporaryFile file;
    ASSERT_TRUE(file.open());
    const QByteArray content = "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:text\r\n"
                               "DTSTART:20250301T000000Z\r\nSUMMARY:\xD6\xD0\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n";
    ASSERT_EQ(file.write(content), content.size());
    ASSERT_TRUE(file.flush());
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    QString diagnostic;
    EXPECT_FALSE(CK::IcsCodec().load(store, file.fileName(), &diagnostic));
    EXPECT_TRUE(diagnostic.contains(QStringLiteral("UTF-8")));
    EXPECT_TRUE(store->events().isEmpty());
}

TEST(IcsRoundtrip, ResolvesAlarmTriggersAgainstEachOccurrence)
{
    const auto store = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(store, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:triggers\r\n"
        "DTSTART:20261120T100000Z\r\nDTEND:20261120T110000Z\r\nRRULE:FREQ=DAILY;COUNT=2\r\n"
        "BEGIN:VALARM\r\nACTION:DISPLAY\r\nTRIGGER:-PT15M\r\nDESCRIPTION:Start\r\nEND:VALARM\r\n"
        "BEGIN:VALARM\r\nACTION:DISPLAY\r\nTRIGGER;RELATED=END:-PT10M\r\nDESCRIPTION:End\r\nEND:VALARM\r\n"
        "BEGIN:VALARM\r\nACTION:DISPLAY\r\nTRIGGER;VALUE=DATE-TIME:20261120T090000Z\r\n"
        "DESCRIPTION:Absolute\r\nEND:VALARM\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n")));
    ASSERT_EQ(store->events().size(), 1);
    const auto event = store->events().constFirst();
    const auto alarms = event->alarms();
    ASSERT_EQ(alarms.size(), 3);
    const auto start = event->dtStart();
    const auto end = event->dtEnd();
    const auto absolute = start.addSecs(-3600);
    EXPECT_EQ(alarms.at(0)->triggerTime(start, end), start.addSecs(-15 * 60));
    EXPECT_EQ(alarms.at(1)->triggerTime(start, end), end.addSecs(-10 * 60));
    EXPECT_EQ(alarms.at(2)->triggerTime(start, end), absolute);

    const auto occurrences = event->recurrence()->timesInInterval(start, end.addDays(1));
    ASSERT_EQ(occurrences.size(), 2);
    const auto nextStart = occurrences.last();
    const auto nextEnd = nextStart.addSecs(start.secsTo(end));
    EXPECT_EQ(alarms.at(0)->triggerTime(nextStart, nextEnd), nextStart.addSecs(-15 * 60));
    EXPECT_EQ(alarms.at(1)->triggerTime(nextStart, nextEnd), nextEnd.addSecs(-10 * 60));
    EXPECT_EQ(alarms.at(2)->triggerTime(nextStart, nextEnd), absolute);

    const auto output = codec.toString(store);
    EXPECT_TRUE(output.contains(QStringLiteral("TRIGGER:-PT15M\r\n")));
    EXPECT_TRUE(output.contains(QStringLiteral("TRIGGER;RELATED=END:-PT10M\r\n")));
    EXPECT_TRUE(output.contains(QStringLiteral("TRIGGER;VALUE=DATE-TIME:20261120T090000Z\r\n")));
}

TEST(IcsRoundtrip, KeepsLegacyAllDayEndOnOrAfterStart)
{
    const auto calendar = CK::MemCalStore::Ptr::create(QTimeZone::utc());
    const CK::IcsCodec codec;
    ASSERT_TRUE(codec.fromString(calendar, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:legacy-all-day\r\n"
        "DTSTART;VALUE=DATE:20240309\r\nDTEND;VALUE=DATE:20240309\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n")));
    ASSERT_EQ(calendar->events().size(), 1);
    const auto event = calendar->events().constFirst();
    EXPECT_TRUE(event->allDay());
    EXPECT_EQ(event->dtStart().date(), QDate(2024, 3, 9));
    EXPECT_EQ(event->dtEnd().date(), event->dtStart().date());
    EXPECT_TRUE(codec.toString(calendar).contains(QStringLiteral("DTEND;VALUE=DATE:20240310\r\n")));
}

TEST(IcsRoundtrip, RecoversCreationTimeFromLegacyKdeDtstamp)
{
    const auto source = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//K Desktop Environment//NONSGML libkcal 4.3//EN\r\n"
        "%1BEGIN:VEVENT\r\nUID:legacy-created\r\nDTSTART:20240309T090000Z\r\n"
        "DTSTAMP:20240101T000000Z\r\n%2END:VEVENT\r\nEND:VCALENDAR\r\n");
    const CK::IcsCodec codec;
    const auto parse = [&](const QString &version, const QString &created) {
        const auto calendar = CK::MemCalStore::Ptr::create(QTimeZone::utc());
        EXPECT_TRUE(codec.fromString(calendar, source.arg(version, created)));
        return calendar;
    };

    const auto legacy = parse({}, {});
    ASSERT_EQ(legacy->events().size(), 1);
    const QDateTime stamp(QDate(2024, 1, 1), QTime(0, 0), Qt::UTC);
    EXPECT_EQ(legacy->events().constFirst()->created(), stamp);
    EXPECT_TRUE(codec.toString(legacy).contains(QStringLiteral("CREATED:20240101T000000Z\r\n")));

    const auto explicitCreated = parse({}, QStringLiteral("CREATED:20231231T000000Z\r\n"));
    ASSERT_EQ(explicitCreated->events().size(), 1);
    EXPECT_EQ(explicitCreated->events().constFirst()->created(), stamp.addDays(-1));

    const auto versioned = parse(QStringLiteral("X-KDE-ICAL-IMPLEMENTATION-VERSION:1.0\r\n"), {});
    ASSERT_EQ(versioned->events().size(), 1);
    EXPECT_NE(versioned->events().constFirst()->created(), stamp);
}

TEST(IcsRoundtrip, KeepsImportedTimesAfterChangingLocalTimeZone)
{
    const struct RestoreTimeZone {
        QByteArray previous = qgetenv("TZ");
        ~RestoreTimeZone()
        {
            if (previous.isNull()) {
                qunsetenv("TZ");
            } else {
                qputenv("TZ", previous);
            }
            tzset();
        }
    } restoreTimeZone;
    qputenv("TZ", "Asia/Shanghai");
    tzset();
    const CK::IcsCodec codec;
    const auto calendar = CK::MemCalStore::Ptr::create(QTimeZone::systemTimeZone());
    ASSERT_TRUE(codec.fromString(calendar, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\n"
        "BEGIN:VEVENT\r\nUID:floating\r\nDTSTART:20240309T090000\r\n"
        "RRULE:FREQ=DAILY;UNTIL=20240310T090000\r\nEND:VEVENT\r\n"
        "BEGIN:VEVENT\r\nUID:zoned\r\nDTSTART;TZID=Asia/Shanghai:20240309T090000\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n")));
    const auto inShanghai = calendar->events();
    ASSERT_EQ(inShanghai.size(), 2);
    const QDateTime originalInstant(QDate(2024, 3, 9), QTime(1, 0), Qt::UTC);
    EXPECT_EQ(inShanghai.at(0)->dtStart().toUTC(), originalInstant);
    EXPECT_EQ(inShanghai.at(1)->dtStart().toUTC(), originalInstant);
    const auto output = codec.toString(calendar);

    qputenv("TZ", "UTC");
    tzset();
    const auto reread = CK::MemCalStore::Ptr::create(QTimeZone::systemTimeZone());
    ASSERT_TRUE(codec.fromString(reread, output));
    const auto inUtc = reread->events();
    ASSERT_EQ(inUtc.size(), 2);
    EXPECT_EQ(inUtc.at(0)->dtStart().timeZone().id(), QByteArray("Asia/Shanghai"));
    EXPECT_EQ(inUtc.at(0)->dtStart().toUTC(), originalInstant);
    EXPECT_EQ(inUtc.at(1)->dtStart().toUTC(), originalInstant);
    const QList<QDateTime> expected{originalInstant, originalInstant.addDays(1)};
    EXPECT_EQ(inUtc.at(0)->recurrence()->timesInInterval(originalInstant, originalInstant.addDays(2)), expected);
}

TEST(IcsRoundtrip, SelectsTheFirstEnabledReminderWithoutDroppingOtherAlarms)
{
    DSchedule::Ptr schedule;
    ASSERT_TRUE(DSchedule::fromIcsString(schedule, QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\nUID:enabled-reminder\r\n"
        "DTSTART:20240101T090000Z\r\nBEGIN:VALARM\r\nACTION:DISPLAY\r\n"
        "TRIGGER:-PT5M\r\nX-KDE-KCALCORE-ENABLED:FALSE\r\nEND:VALARM\r\n"
        "BEGIN:VALARM\r\nACTION:DISPLAY\r\nTRIGGER:-PT15M\r\n"
        "X-KDE-KCALCORE-ENABLED:TRUE\r\nEND:VALARM\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n")));
    ASSERT_EQ(schedule->alarms().size(), 2);
    ASSERT_TRUE(schedule->firstEnabledAlarm());
    EXPECT_EQ(schedule->firstEnabledAlarm()->startOffset().asSeconds(), -900);
    EXPECT_EQ(schedule->getAlarmType(), DSchedule::Alarm_15Min_Front);
    EXPECT_TRUE(DSchedule::toIcsString(schedule).contains(QStringLiteral("X-KDE-KCALCORE-ENABLED:FALSE")));
}
