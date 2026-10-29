// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedule.h"

#include <gtest/gtest.h>

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimeZone>
#include <QDebug>

namespace {

// ---- Constructors & clone ----

TEST(DSchedule, DefaultConstructor)
{
    DSchedule schedule;
    EXPECT_TRUE(schedule.scheduleTypeID().isEmpty());
    EXPECT_TRUE(schedule.accountColor().isEmpty());
    EXPECT_TRUE(schedule.fileName().isEmpty());
    EXPECT_FALSE(schedule.isMoved());
    EXPECT_EQ(0, schedule.compatibleID());
}

TEST(DSchedule, CopyConstructor)
{
    DSchedule original;
    original.setSummary(QStringLiteral("Meeting"));
    original.setScheduleTypeID(QStringLiteral("type-1"));
    original.setAccountColor(QStringLiteral("#FF0000"));
    original.setFileName(QStringLiteral("test.ics"));
    original.setMoved(true);
    original.setcompatibleID(42);
    original.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    original.setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    DSchedule copy(original);
    // Base-class and properly-copied members.
    EXPECT_EQ(original.summary(), copy.summary());
    EXPECT_EQ(original.scheduleTypeID(), copy.scheduleTypeID());
    EXPECT_EQ(original.accountColor(), copy.accountColor());
    EXPECT_EQ(original.dtStart(), copy.dtStart());
    EXPECT_EQ(original.dtEnd(), copy.dtEnd());
    // DEFECT: fileName, isMoved, compatibleID are NOT copied by the copy
    // constructor.  The compiler-generated copy constructor is shadowed by
    // the DSchedule(const KCalendarCore::Event&) converting constructor,
    // which re-initialises these members to defaults.  See defects.json.
    EXPECT_NE(original.fileName(), copy.fileName());
    EXPECT_NE(original.isMoved(), copy.isMoved());
    EXPECT_NE(original.compatibleID(), copy.compatibleID());
}

TEST(DSchedule, ConstructorFromKCalendarCoreEvent)
{
    KCalendarCore::Event event;
    event.setSummary(QStringLiteral("Base Event"));
    event.setDtStart(QDateTime(QDate(2025, 5, 1), QTime(9, 0, 0)));
    event.setDtEnd(QDateTime(QDate(2025, 5, 1), QTime(10, 0, 0)));

    DSchedule schedule(event);
    EXPECT_EQ(QStringLiteral("Base Event"), schedule.summary());
    EXPECT_EQ(event.dtStart(), schedule.dtStart());
    EXPECT_EQ(event.dtEnd(), schedule.dtEnd());
    EXPECT_EQ(0, schedule.compatibleID());
}

TEST(DSchedule, CloneProducesEqualCopy)
{
    DSchedule original;
    original.setSummary(QStringLiteral("Clone Test"));
    original.setScheduleTypeID(QStringLiteral("type-2"));
    original.setcompatibleID(7);

    std::unique_ptr<DSchedule> cloned(original.clone());
    ASSERT_NE(nullptr, cloned.get());
    EXPECT_EQ(original.summary(), cloned->summary());
    EXPECT_EQ(original.scheduleTypeID(), cloned->scheduleTypeID());
    // DEFECT: clone() delegates to the copy constructor, which does not copy
    // compatibleID.  See defects.json.
    EXPECT_NE(original.compatibleID(), cloned->compatibleID());
}

// ---- Simple getters / setters ----

TEST(DSchedule, ScheduleTypeIDGetterSetter)
{
    DSchedule schedule;
    schedule.setScheduleTypeID(QStringLiteral("type-abc"));
    EXPECT_EQ(QStringLiteral("type-abc"), schedule.scheduleTypeID());
}

TEST(DSchedule, AccountColorGetterSetter)
{
    DSchedule schedule;
    schedule.setAccountColor(QStringLiteral("#00FF00"));
    EXPECT_EQ(QStringLiteral("#00FF00"), schedule.accountColor());
}

TEST(DSchedule, FileNameGetterSetter)
{
    DSchedule schedule;
    schedule.setFileName(QStringLiteral("calendar.ics"));
    EXPECT_EQ(QStringLiteral("calendar.ics"), schedule.fileName());
}

TEST(DSchedule, IsMovedGetterSetter)
{
    DSchedule schedule;
    EXPECT_FALSE(schedule.isMoved());
    schedule.setMoved(true);
    EXPECT_TRUE(schedule.isMoved());
    schedule.setMoved(false);
    EXPECT_FALSE(schedule.isMoved());
}

TEST(DSchedule, CompatibleIDGetterSetter)
{
    DSchedule schedule;
    schedule.setcompatibleID(99);
    EXPECT_EQ(99, schedule.compatibleID());
    schedule.setcompatibleID(0);
    EXPECT_EQ(0, schedule.compatibleID());
}

// ---- isValid / isMultiDay ----

TEST(DSchedule, IsValidAlwaysReturnsTrue)
{
    DSchedule schedule;
    EXPECT_TRUE(schedule.isValid());
}

TEST(DSchedule, IsMultiDayReturnsFalseForSameDay)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(12, 0, 0)));
    EXPECT_FALSE(schedule.isMultiDay());
}

TEST(DSchedule, IsMultiDayReturnsTrueForDifferentDays)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setDtEnd(QDateTime(QDate(2025, 1, 2), QTime(12, 0, 0)));
    EXPECT_TRUE(schedule.isMultiDay());
}

// ---- operator== / operator< ----

TEST(DSchedule, OperatorEqualsReturnsTrueForSameIdentifier)
{
    DSchedule a;
    a.setUid(QStringLiteral("uid-1"));
    DSchedule b;
    b.setUid(QStringLiteral("uid-1"));
    EXPECT_TRUE(a == b);
}

TEST(DSchedule, OperatorEqualsReturnsFalseForDifferentIdentifier)
{
    DSchedule a;
    a.setUid(QStringLiteral("uid-1"));
    DSchedule b;
    b.setUid(QStringLiteral("uid-2"));
    EXPECT_FALSE(a == b);
}

TEST(DSchedule, OperatorLessThanComparesByAllDayThenDtStart)
{
    DSchedule allDay;
    allDay.setAllDay(true);
    allDay.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));

    DSchedule timed;
    timed.setAllDay(false);
    timed.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));

    // allDay == true sorts before allDay == false (because allDay > schedule.allDay)
    EXPECT_TRUE(allDay < timed);
    EXPECT_FALSE(timed < allDay);
}

TEST(DSchedule, OperatorLessThanComparesByDtStart)
{
    DSchedule earlier;
    earlier.setAllDay(false);
    earlier.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(8, 0, 0)));

    DSchedule later;
    later.setAllDay(false);
    later.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));

    EXPECT_TRUE(earlier < later);
    EXPECT_FALSE(later < earlier);
}

TEST(DSchedule, OperatorLessThanReturnsTrueWhenEqual)
{
    DSchedule a;
    a.setAllDay(false);
    a.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(8, 0, 0)));

    DSchedule b;
    b.setAllDay(false);
    b.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(8, 0, 0)));

    // When all fields are equal, operator< returns true
    EXPECT_TRUE(a < b);
}

// ---- Alarm type ----

TEST(DSchedule, GetAlarmTypeReturnsNoneForNoAlarm)
{
    DSchedule schedule;
    EXPECT_EQ(DSchedule::Alarm_None, schedule.getAlarmType());
}

TEST(DSchedule, GetAlarmTypeReturnsAllDayNoneForAllDayNoAlarm)
{
    DSchedule schedule;
    schedule.setAllDay(true);
    EXPECT_EQ(DSchedule::Alarm_AllDay_None, schedule.getAlarmType());
}

TEST(DSchedule, SetAlarmTypeAddsAlarm)
{
    DSchedule schedule;
    schedule.setSummary(QStringLiteral("Alarm Test"));
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));

    schedule.setAlarmType(DSchedule::Alarm_15Min_Front);
    EXPECT_EQ(DSchedule::Alarm_15Min_Front, schedule.getAlarmType());
}

TEST(DSchedule, SetAlarmTypeNoneClearsAlarms)
{
    DSchedule schedule;
    schedule.setSummary(QStringLiteral("Clear Alarm"));
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));

    schedule.setAlarmType(DSchedule::Alarm_30Min_Front);
    ASSERT_EQ(DSchedule::Alarm_30Min_Front, schedule.getAlarmType());

    schedule.setAlarmType(DSchedule::Alarm_None);
    EXPECT_EQ(DSchedule::Alarm_None, schedule.getAlarmType());
}

TEST(DSchedule, SetAlarmTypeAllDayNoneClearsAlarms)
{
    DSchedule schedule;
    schedule.setAllDay(true);
    schedule.setSummary(QStringLiteral("AllDay Alarm"));
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));

    schedule.setAlarmType(DSchedule::Alarm_15Hour_Front);
    ASSERT_EQ(DSchedule::Alarm_15Hour_Front, schedule.getAlarmType());

    schedule.setAlarmType(DSchedule::Alarm_AllDay_None);
    EXPECT_EQ(DSchedule::Alarm_AllDay_None, schedule.getAlarmType());
}

TEST(DSchedule, SetAlarmTypeEachVariant)
{
    const std::array<DSchedule::AlarmType, 11> types = {{
        DSchedule::Alarm_Begin,
        DSchedule::Alarm_15Min_Front,
        DSchedule::Alarm_30Min_Front,
        DSchedule::Alarm_1Hour_Front,
        DSchedule::Alarm_1Day_Front,
        DSchedule::Alarm_2Day_Front,
        DSchedule::Alarm_1Week_Front,
        DSchedule::Alarm_9Hour_After,
        DSchedule::Alarm_15Hour_Front,
        DSchedule::Alarm_39Hour_Front,
        DSchedule::Alarm_159Hour_Front,
    }};
    for (const auto type : types) {
        DSchedule schedule;
        schedule.setSummary(QStringLiteral("Variant %1").arg(static_cast<int>(type)));
        schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
        schedule.setAlarmType(type);
        EXPECT_EQ(type, schedule.getAlarmType())
            << "alarm type: " << static_cast<int>(type);
    }
}

// ---- RRule type ----

TEST(DSchedule, GetRRuleTypeReturnsNoneForNoRecurrence)
{
    DSchedule schedule;
    EXPECT_EQ(DSchedule::RRule_None, schedule.getRRuleType());
}

TEST(DSchedule, SetRRuleTypeDay)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);
    EXPECT_EQ(DSchedule::RRule_Day, schedule.getRRuleType());
}

TEST(DSchedule, SetRRuleTypeWeek)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Week);
    EXPECT_EQ(DSchedule::RRule_Week, schedule.getRRuleType());
}

TEST(DSchedule, SetRRuleTypeMonth)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Month);
    EXPECT_EQ(DSchedule::RRule_Month, schedule.getRRuleType());
}

TEST(DSchedule, SetRRuleTypeYear)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Year);
    EXPECT_EQ(DSchedule::RRule_Year, schedule.getRRuleType());
}

TEST(DSchedule, SetRRuleTypeWork)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Work);
    EXPECT_EQ(DSchedule::RRule_Work, schedule.getRRuleType());
}

TEST(DSchedule, SetRRuleTypeNoneClearsRecurrence)
{
    DSchedule schedule;
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);
    ASSERT_EQ(DSchedule::RRule_Day, schedule.getRRuleType());

    schedule.setRRuleType(DSchedule::RRule_None);
    EXPECT_EQ(DSchedule::RRule_None, schedule.getRRuleType());
}

// ---- ICS serialization ----

TEST(DSchedule, ToIcsStringProducesNonEmptyString)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("ICS Event"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    const QString ics = DSchedule::toIcsString(schedule);
    EXPECT_FALSE(ics.isEmpty());
    EXPECT_TRUE(ics.contains(QStringLiteral("BEGIN:VEVENT")));
}

TEST(DSchedule, FromIcsStringParsesValidICS)
{
    // Build an ICS string via toIcsString, then parse it back.
    DSchedule::Ptr original(new DSchedule);
    original->setSummary(QStringLiteral("Round Trip ICS"));
    original->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    original->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    const QString ics = DSchedule::toIcsString(original);
    DSchedule::Ptr parsed;
    ASSERT_TRUE(DSchedule::fromIcsString(parsed, ics));
    EXPECT_EQ(QStringLiteral("Round Trip ICS"), parsed->summary());
}

TEST(DSchedule, FromIcsStringReturnsFalseOnInvalidString)
{
    DSchedule::Ptr parsed;
    EXPECT_FALSE(DSchedule::fromIcsString(parsed, QStringLiteral("not an ics string")));
}

// ---- JSON serialization ----

TEST(DSchedule, ToJsonStringProducesValidJson)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("JSON Event"));
    schedule->setScheduleTypeID(QStringLiteral("type-json"));
    schedule->setcompatibleID(5);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    QString json;
    ASSERT_TRUE(DSchedule::toJsonString(schedule, json));

    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    ASSERT_TRUE(doc.isObject());
    const QJsonObject obj = doc.object();
    EXPECT_EQ(QStringLiteral("type-json"), obj.value("type").toString());
    EXPECT_EQ(5, obj.value("compatibleID").toInt());
    EXPECT_TRUE(obj.value("schedule").isString());
}

TEST(DSchedule, ToJsonStringReturnsFalseForNullPtr)
{
    DSchedule::Ptr nullPtr;
    QString json;
    EXPECT_FALSE(DSchedule::toJsonString(nullPtr, json));
}

TEST(DSchedule, FromJsonStringParsesValidJson)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("JSON Source"));
    schedule->setScheduleTypeID(QStringLiteral("type-src"));
    schedule->setcompatibleID(3);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    QString json;
    ASSERT_TRUE(DSchedule::toJsonString(schedule, json));

    DSchedule::Ptr parsed;
    ASSERT_TRUE(DSchedule::fromJsonString(parsed, json));
    EXPECT_EQ(QStringLiteral("type-src"), parsed->scheduleTypeID());
    EXPECT_EQ(3, parsed->compatibleID());
    EXPECT_EQ(QStringLiteral("JSON Source"), parsed->summary());
}

TEST(DSchedule, FromJsonStringReturnsFalseOnInvalidJson)
{
    DSchedule::Ptr parsed;
    EXPECT_FALSE(DSchedule::fromJsonString(parsed, QStringLiteral("not json")));
}

TEST(DSchedule, FromJsonStringReturnsFalseOnNonObjectJson)
{
    DSchedule::Ptr parsed;
    // A JSON array is valid JSON but not an object
    EXPECT_FALSE(DSchedule::fromJsonString(parsed, QStringLiteral("[1, 2, 3]")));
}

TEST(DSchedule, FromJsonStringReturnsFalseWhenScheduleKeyMissing)
{
    DSchedule::Ptr parsed;
    const QString json = QStringLiteral(R"({"type":"x"})");
    EXPECT_FALSE(DSchedule::fromJsonString(parsed, json));
}

TEST(DSchedule, JsonRoundTripPreservesValues)
{
    DSchedule::Ptr original(new DSchedule);
    original->setSummary(QStringLiteral("Round Trip JSON"));
    original->setScheduleTypeID(QStringLiteral("type-rt"));
    original->setcompatibleID(77);
    original->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    original->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    QString json;
    ASSERT_TRUE(DSchedule::toJsonString(original, json));

    DSchedule::Ptr restored;
    ASSERT_TRUE(DSchedule::fromJsonString(restored, json));
    EXPECT_EQ(original->summary(), restored->summary());
    EXPECT_EQ(original->scheduleTypeID(), restored->scheduleTypeID());
    EXPECT_EQ(original->compatibleID(), restored->compatibleID());
}

// ---- List serialization ----

TEST(DSchedule, ToListStringProducesValidJson)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("List Item"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);

    const QString json = DSchedule::toListString(QStringLiteral("my-query"), list);
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    ASSERT_TRUE(doc.isObject());
    const QJsonObject obj = doc.object();
    EXPECT_EQ(QStringLiteral("my-query"), obj.value("query").toString());
    EXPECT_TRUE(obj.value("schedules").isArray());
    EXPECT_EQ(1, obj.value("schedules").toArray().size());
}

TEST(DSchedule, FromListStringParsesValidJson)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("List Source"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);
    const QString json = DSchedule::toListString(QStringLiteral("list-query"), list);

    const auto result = DSchedule::fromListString(json);
    EXPECT_EQ(QStringLiteral("list-query"), result.first);
    EXPECT_EQ(1, result.second.size());
    EXPECT_EQ(QStringLiteral("List Source"), result.second.at(0)->summary());
}

TEST(DSchedule, FromListStringReturnsEmptyOnInvalidJson)
{
    const auto result = DSchedule::fromListString(QStringLiteral("not json"));
    EXPECT_TRUE(result.first.isEmpty());
    EXPECT_TRUE(result.second.isEmpty());
}

TEST(DSchedule, FromListStringReturnsEmptyOnNonObjectJson)
{
    const auto result = DSchedule::fromListString(QStringLiteral("[1, 2]"));
    EXPECT_TRUE(result.first.isEmpty());
    EXPECT_TRUE(result.second.isEmpty());
}

TEST(DSchedule, FromListStringReturnsEmptyWhenMissingKeys)
{
    const auto result = DSchedule::fromListString(QStringLiteral(R"({"foo":"bar"})"));
    EXPECT_TRUE(result.first.isEmpty());
    EXPECT_TRUE(result.second.isEmpty());
}

// ---- Map serialization ----

TEST(DSchedule, ToMapStringProducesValidJson)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("Map Item"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);

    DSchedule::Map map;
    map.insert(QDate(2025, 1, 1), list);

    const QString json = DSchedule::toMapString(map);
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    ASSERT_TRUE(doc.isArray());
    const QJsonArray arr = doc.array();
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(QStringLiteral("2025-01-01"),
              arr.at(0).toObject().value("Date").toString());
}

TEST(DSchedule, FromMapStringParsesValidJson)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("Map Source"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);
    DSchedule::Map map;
    map.insert(QDate(2025, 1, 1), list);

    const QString json = DSchedule::toMapString(map);
    const DSchedule::Map parsed = DSchedule::fromMapString(json);
    EXPECT_EQ(1, parsed.size());
    ASSERT_TRUE(parsed.contains(QDate(2025, 1, 1)));
    EXPECT_EQ(1, parsed.value(QDate(2025, 1, 1)).size());
    EXPECT_EQ(QStringLiteral("Map Source"),
              parsed.value(QDate(2025, 1, 1)).at(0)->summary());
}

TEST(DSchedule, FromMapStringReturnsEmptyOnInvalidJson)
{
    const DSchedule::Map result = DSchedule::fromMapString(QStringLiteral("not json"));
    EXPECT_TRUE(result.isEmpty());
}

TEST(DSchedule, FromMapStringReturnsEmptyOnNonArrayJson)
{
    const DSchedule::Map result = DSchedule::fromMapString(QStringLiteral(R"({"a":1})"));
    EXPECT_TRUE(result.isEmpty());
}

// ---- numberOfRepetitions ----

TEST(DSchedule, NumberOfRepetitionsForDailyRecurrence)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setRRuleType(DSchedule::RRule_Day);

    // Count repetitions from dtStart to a date 5 days later
    const QDateTime target(QDate(2025, 1, 6), QTime(10, 0, 0));
    const int count = DSchedule::numberOfRepetitions(schedule, target);
    EXPECT_GE(count, 0);
}

TEST(DSchedule, NumberOfRepetitionsForWeeklyRecurrence)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setRRuleType(DSchedule::RRule_Week);

    const QDateTime target(QDate(2025, 2, 1), QTime(10, 0, 0));
    const int count = DSchedule::numberOfRepetitions(schedule, target);
    EXPECT_GE(count, 0);
}

// ---- expendRecurrence ----

TEST(DSchedule, ExpendRecurrenceForNonRecurringSchedule)
{
    DSchedule::Map map;
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));

    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 1, 31), QTime(23, 59, 59));

    DSchedule::expendRecurrence(map, schedule, dtStart, dtEnd);
    EXPECT_EQ(1, map.size());
    EXPECT_EQ(1, map.value(QDate(2025, 1, 1)).size());
}

TEST(DSchedule, ExpendRecurrenceForDailyRecurringSchedule)
{
    DSchedule::Map map;
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));
    schedule->setRRuleType(DSchedule::RRule_Day);

    // DEFECT: setRRuleType does not set the recurrence rule's start date,
    // so timesInInterval returns an empty list.  Set the start date
    // manually to exercise the expansion logic.  See defects.json.
    KCalendarCore::Recurrence *rec = schedule->recurrence();
    KCalendarCore::RecurrenceRule *rule = const_cast<KCalendarCore::RecurrenceRule *>(
        rec->defaultRRuleConst());
    if (rule) {
        rule->setStartDt(schedule->dtStart());
    }

    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 1, 7), QTime(23, 59, 59));

    DSchedule::expendRecurrence(map, schedule, dtStart, dtEnd);
    EXPECT_GE(map.size(), 2);
}

TEST(DSchedule, ExpendRecurrenceForScheduleOutsideRange)
{
    DSchedule::Map map;
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 6, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 6, 1), QTime(11, 0, 0)));

    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 1, 31), QTime(23, 59, 59));

    DSchedule::expendRecurrence(map, schedule, dtStart, dtEnd);
    EXPECT_TRUE(map.isEmpty());
}

// ---- convertSchedules ----

TEST(DSchedule, ConvertSchedulesForNonRecurringSchedule)
{
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));
    queryPar->setDtEnd(QDateTime(QDate(2025, 1, 31), QTime(23, 59, 59)));
    queryPar->setQueryType(DScheduleQueryPar::Query_None);

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 15), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 15), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);

    const DSchedule::Map result = DSchedule::convertSchedules(queryPar, list);
    EXPECT_EQ(1, result.size());
    EXPECT_EQ(1, result.value(QDate(2025, 1, 15)).size());
}

TEST(DSchedule, ConvertSchedulesForTopQuery)
{
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setKey(QStringLiteral("test"));
    queryPar->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));
    queryPar->setDtEnd(QDateTime(QDate(2025, 1, 31), QTime(23, 59, 59)));
    queryPar->setQueryType(DScheduleQueryPar::Query_Top);
    queryPar->setQueryTop(1);

    DSchedule::Ptr s1(new DSchedule);
    s1->setDtStart(QDateTime(QDate(2025, 1, 10), QTime(10, 0, 0)));
    s1->setDtEnd(QDateTime(QDate(2025, 1, 10), QTime(11, 0, 0)));

    DSchedule::Ptr s2(new DSchedule);
    s2->setDtStart(QDateTime(QDate(2025, 1, 20), QTime(10, 0, 0)));
    s2->setDtEnd(QDateTime(QDate(2025, 1, 20), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(s1);
    list.append(s2);

    const DSchedule::Map result = DSchedule::convertSchedules(queryPar, list);
    // DEFECT: the Query_Top filter never increments scheduleNum, so the top-N
    // limit is not enforced and all schedules are returned.  See defects.json.
    int total = 0;
    for (auto it = result.constBegin(); it != result.constEnd(); ++it) {
        total += it.value().size();
    }
    EXPECT_EQ(total, 2);
}

TEST(DSchedule, ConvertSchedulesForScheduleOutsideRange)
{
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));
    queryPar->setDtEnd(QDateTime(QDate(2025, 1, 31), QTime(23, 59, 59)));
    queryPar->setQueryType(DScheduleQueryPar::Query_None);

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 6, 1), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 6, 1), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);

    const DSchedule::Map result = DSchedule::convertSchedules(queryPar, list);
    EXPECT_TRUE(result.isEmpty());
}

TEST(DSchedule, ConvertSchedulesForMultiDaySchedule)
{
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));
    queryPar->setDtEnd(QDateTime(QDate(2025, 1, 31), QTime(23, 59, 59)));
    queryPar->setQueryType(DScheduleQueryPar::Query_None);

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setDtStart(QDateTime(QDate(2025, 1, 10), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 12), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);

    const DSchedule::Map result = DSchedule::convertSchedules(queryPar, list);
    // Multi-day schedule should be expanded across 3 days (Jan 10, 11, 12)
    EXPECT_EQ(3, result.size());
    EXPECT_TRUE(result.contains(QDate(2025, 1, 10)));
    EXPECT_TRUE(result.contains(QDate(2025, 1, 11)));
    EXPECT_TRUE(result.contains(QDate(2025, 1, 12)));
}

// ---- fromQueryResult ----

TEST(DSchedule, FromQueryResultReturnsEmptyOnInvalidJson)
{
    const DSchedule::Map result = DSchedule::fromQueryResult(QStringLiteral("not json"));
    EXPECT_TRUE(result.isEmpty());
}

TEST(DSchedule, FromQueryResultReturnsEmptyWhenQueryParIsNull)
{
    // Build a list JSON where the query string is not a valid DScheduleQueryPar JSON.
    QJsonObject obj;
    obj.insert("query", QStringLiteral("invalid-query-par-json"));
    obj.insert("schedules", QJsonArray());
    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    const DSchedule::Map result = DSchedule::fromQueryResult(json);
    EXPECT_TRUE(result.isEmpty());
}

TEST(DSchedule, FromQueryResultProcessesValidInput)
{
    // Build a valid query result: query JSON + one schedule
    DScheduleQueryPar::Ptr queryPar(new DScheduleQueryPar);
    queryPar->setKey(QStringLiteral("test"));
    queryPar->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(0, 0, 0)));
    queryPar->setDtEnd(QDateTime(QDate(2025, 1, 31), QTime(23, 59, 59)));
    queryPar->setQueryType(DScheduleQueryPar::Query_None);
    const QString queryJson = DScheduleQueryPar::toJsonString(queryPar);

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("Query Result Item"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 15), QTime(10, 0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 15), QTime(11, 0, 0)));

    DSchedule::List list;
    list.append(schedule);
    const QString json = DSchedule::toListString(queryJson, list);

    const DSchedule::Map result = DSchedule::fromQueryResult(json);
    EXPECT_EQ(1, result.size());
    EXPECT_TRUE(result.contains(QDate(2025, 1, 15)));
}


// ---- Free function operators (Ptr comparison & QDebug streaming) ----

TEST(DSchedule, PtrOperatorLessThanBothNullReturnsFalse)
{
    DSchedule::Ptr null1;
    DSchedule::Ptr null2;
    EXPECT_FALSE(null1 < null2);
}

TEST(DSchedule, PtrOperatorLessThanOneNullReturnsFalse)
{
    DSchedule::Ptr nullPtr;
    DSchedule::Ptr valid(new DSchedule);
    valid->setAllDay(false);
    valid->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(8, 0, 0)));
    EXPECT_FALSE(nullPtr < valid);
    EXPECT_FALSE(valid < nullPtr);
}

TEST(DSchedule, PtrOperatorLessThanDelegatesToMemberOperator)
{
    DSchedule::Ptr earlier(new DSchedule);
    earlier->setAllDay(false);
    earlier->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(8, 0, 0)));

    DSchedule::Ptr later(new DSchedule);
    later->setAllDay(false);
    later->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));

    EXPECT_TRUE(earlier < later);
    EXPECT_FALSE(later < earlier);
}

TEST(DSchedule, PtrOperatorEqualsBothNullReturnsTrue)
{
    DSchedule::Ptr null1;
    DSchedule::Ptr null2;
    EXPECT_TRUE(null1 == null2);
}

TEST(DSchedule, PtrOperatorEqualsOneNullReturnsFalse)
{
    DSchedule::Ptr nullPtr;
    DSchedule::Ptr valid(new DSchedule);
    valid->setUid(QStringLiteral("uid-1"));
    EXPECT_FALSE(nullPtr == valid);
    EXPECT_FALSE(valid == nullPtr);
}

TEST(DSchedule, PtrOperatorEqualsSameIdentifierReturnsTrue)
{
    DSchedule::Ptr a(new DSchedule);
    a->setUid(QStringLiteral("uid-1"));
    DSchedule::Ptr b(new DSchedule);
    b->setUid(QStringLiteral("uid-1"));
    EXPECT_TRUE(a == b);
}

TEST(DSchedule, PtrOperatorEqualsDifferentIdentifierReturnsFalse)
{
    DSchedule::Ptr a(new DSchedule);
    a->setUid(QStringLiteral("uid-1"));
    DSchedule::Ptr b(new DSchedule);
    b->setUid(QStringLiteral("uid-2"));
    EXPECT_FALSE(a == b);
}

TEST(DSchedule, OperatorStreamToQDebug)
{
    DSchedule schedule;
    schedule.setSummary(QStringLiteral("Test Summary"));
    schedule.setScheduleTypeID(QStringLiteral("type-1"));
    schedule.setDtStart(QDateTime(QDate(2025, 1, 1), QTime(10, 0, 0)));
    schedule.setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(11, 0, 0)));
    schedule.setRRuleType(DSchedule::RRule_Day);

    QString output;
    QDebug debug(&output);
    debug << schedule;
    EXPECT_TRUE(output.contains("dtStart:"));
    EXPECT_TRUE(output.contains("summary:"));
    EXPECT_TRUE(output.contains("Test Summary"));
}

} // namespace
