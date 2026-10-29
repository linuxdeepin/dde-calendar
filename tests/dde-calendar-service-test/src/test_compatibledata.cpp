// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "compatibledata.h"
#include "dschedule.h"
#include "units.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDate>

// Known type mapping from mTypeMap in compatibledata.cpp
static const QString TYPE_1 = QStringLiteral("107c369e-b13a-4d45-9ff3-de4eb3c0475b");
static const QString TYPE_2 = QStringLiteral("24cf3ae3-541d-487f-83df-f068416b56b6");
static const QString TYPE_3 = QStringLiteral("403bf009-2005-4679-9c76-e73d9f83a8b4");

// ============ getNewTypeIDByOldTypeID ============

TEST(CompatibleDataTest, GetNewTypeIDByOldTypeID_ValidIDs)
{
    EXPECT_EQ(DDE_Calendar::getNewTypeIDByOldTypeID(1), TYPE_1);
    EXPECT_EQ(DDE_Calendar::getNewTypeIDByOldTypeID(2), TYPE_2);
    EXPECT_EQ(DDE_Calendar::getNewTypeIDByOldTypeID(3), TYPE_3);
}

TEST(CompatibleDataTest, GetNewTypeIDByOldTypeID_InvalidID_ReturnsDefault)
{
    // Invalid old type ID should return the default (mTypeMap.value(3))
    EXPECT_EQ(DDE_Calendar::getNewTypeIDByOldTypeID(99), TYPE_3);
    EXPECT_EQ(DDE_Calendar::getNewTypeIDByOldTypeID(0), TYPE_3);
    EXPECT_EQ(DDE_Calendar::getNewTypeIDByOldTypeID(-1), TYPE_3);
}

// ============ getOldTypeIDByNewTypeID ============

TEST(CompatibleDataTest, GetOldTypeIDByNewTypeID_ValidUUIDs)
{
    EXPECT_EQ(DDE_Calendar::getOldTypeIDByNewTypeID(TYPE_1), 1);
    EXPECT_EQ(DDE_Calendar::getOldTypeIDByNewTypeID(TYPE_2), 2);
    EXPECT_EQ(DDE_Calendar::getOldTypeIDByNewTypeID(TYPE_3), 3);
}

TEST(CompatibleDataTest, GetOldTypeIDByNewTypeID_InvalidUUID_ReturnsDefault)
{
    // Not found → returns 3
    EXPECT_EQ(DDE_Calendar::getOldTypeIDByNewTypeID("invalid-uuid"), 3);
    EXPECT_EQ(DDE_Calendar::getOldTypeIDByNewTypeID(""), 3);
}

// ============ getOldRemindByAlarm ============

TEST(CompatibleDataTest, GetOldRemindByAlarm_AllTypes)
{
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_None), "");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_Begin), "0");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_15Min_Front), "15");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_30Min_Front), "30");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_1Hour_Front), "60");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_1Day_Front), "1440");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_2Day_Front), "2880");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_1Week_Front), "10080");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_AllDay_None), "");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_9Hour_After), "0;09:00");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_15Hour_Front), "1;09:00");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_39Hour_Front), "2;09:00");
    EXPECT_EQ(DDE_Calendar::getOldRemindByAlarm(DSchedule::Alarm_159Hour_Front), "7;09:00");
}

// ============ setAlarmByOldRemind ============

TEST(CompatibleDataTest, SetAlarmByOldRemind_TimedEvent)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setAllDay(false);
    schedule->setSummary("TestEvent");
    DDE_Calendar::setAlarmByOldRemind(schedule, "15");
    // Should have one alarm added
    EXPECT_EQ(schedule->alarms().count(), 1);
}

TEST(CompatibleDataTest, SetAlarmByOldRemind_AllDayEvent)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setAllDay(true);
    schedule->setSummary("AllDayEvent");
    DDE_Calendar::setAlarmByOldRemind(schedule, "1;09:00");
    EXPECT_EQ(schedule->alarms().count(), 1);
}

TEST(CompatibleDataTest, SetAlarmByOldRemind_NegativeRemind_NoAlarm)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setAllDay(false);
    schedule->setSummary("NoAlarm");
    // remindNum < 0 means no alarm
    DDE_Calendar::setAlarmByOldRemind(schedule, "-1");
    EXPECT_EQ(schedule->alarms().count(), 0);
}

// ============ getScheduleByExported ============

TEST(CompatibleDataTest, GetScheduleByExported_ValidJSON)
{
    QJsonObject obj;
    obj.insert("AllDay", false);
    obj.insert("Title", "Meeting");
    obj.insert("Description", "Team sync");
    obj.insert("Type", 1);
    obj.insert("Start", dtToString(QDateTime(QDate(2026, 1, 1), QTime(9, 0))));
    obj.insert("End", dtToString(QDateTime(QDate(2026, 1, 1), QTime(10, 0))));
    obj.insert("Remind", "15");
    QJsonDocument doc;
    doc.setObject(obj);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    DSchedule::Ptr schedule = DDE_Calendar::getScheduleByExported(jsonStr);
    ASSERT_NE(schedule, nullptr);
    EXPECT_EQ(schedule->summary(), "Meeting");
    EXPECT_EQ(schedule->description(), "Team sync");
    EXPECT_FALSE(schedule->allDay());
    EXPECT_EQ(schedule->scheduleTypeID(), TYPE_1);
}

TEST(CompatibleDataTest, GetScheduleByExported_AllDayEvent)
{
    QJsonObject obj;
    obj.insert("AllDay", true);
    obj.insert("Title", "Holiday");
    obj.insert("Remind", "0;09:00");
    QJsonDocument doc;
    doc.setObject(obj);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    DSchedule::Ptr schedule = DDE_Calendar::getScheduleByExported(jsonStr);
    ASSERT_NE(schedule, nullptr);
    EXPECT_TRUE(schedule->allDay());
    EXPECT_EQ(schedule->summary(), "Holiday");
}

TEST(CompatibleDataTest, GetScheduleByExported_InvalidJSON_ReturnsNull)
{
    DSchedule::Ptr schedule = DDE_Calendar::getScheduleByExported("not valid json");
    EXPECT_EQ(schedule, nullptr);
}

TEST(CompatibleDataTest, GetScheduleByExported_EmptyJSON)
{
    DSchedule::Ptr schedule = DDE_Calendar::getScheduleByExported("{}");
    ASSERT_NE(schedule, nullptr);
    // Default values
    EXPECT_FALSE(schedule->allDay());
}

// ============ getExternalSchedule ============

TEST(CompatibleDataTest, GetExternalSchedule_ValidMap)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary("TestSchedule");
    schedule->setAllDay(false);
    schedule->setDtStart(QDateTime(QDate(2026, 1, 1), QTime(9, 0)));
    schedule->setDtEnd(QDateTime(QDate(2026, 1, 1), QTime(10, 0)));
    schedule->setScheduleTypeID(TYPE_1);
    schedule->recurrence()->setDaily(1);

    DSchedule::List list;
    list.append(schedule);

    DSchedule::Map map;
    map.insert(QDate(2026, 1, 1), list);

    QString result = DDE_Calendar::getExternalSchedule(map);
    ASSERT_FALSE(result.isEmpty());

    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    ASSERT_TRUE(doc.isArray());
    QJsonArray arr = doc.array();
    ASSERT_EQ(arr.count(), 1);
    QJsonObject dayObj = arr.at(0).toObject();
    EXPECT_EQ(dayObj.value("Date").toString(), "2026-01-01");
    QJsonArray jobs = dayObj.value("Jobs").toArray();
    ASSERT_EQ(jobs.count(), 1);
    QJsonObject job = jobs.at(0).toObject();
    EXPECT_EQ(job.value("Title").toString(), "TestSchedule");
}

TEST(CompatibleDataTest, GetExternalSchedule_EmptyMap)
{
    DSchedule::Map map;
    QString result = DDE_Calendar::getExternalSchedule(map);
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    EXPECT_TRUE(doc.isArray());
    EXPECT_EQ(doc.array().count(), 0);
}

// ============ setRRuleByOldRRule ============

TEST(CompatibleDataTest, SetRRuleByOldRRule_ValidRRule)
{
    DSchedule::Ptr schedule(new DSchedule);
    // A valid ICal recurrence rule string
    QString rrule = "FREQ=DAILY;INTERVAL=1";
    DDE_Calendar::setRRuleByOldRRule(schedule, rrule);
    // The recurrence should have at least one rrule
    EXPECT_GE(schedule->recurrence()->rRules().count(), 1);
}

TEST(CompatibleDataTest, SetRRuleByOldRRule_InvalidRRule)
{
    DSchedule::Ptr schedule(new DSchedule);
    DDE_Calendar::setRRuleByOldRRule(schedule, "invalid-rrule");
    // Invalid rule should not add anything
    EXPECT_EQ(schedule->recurrence()->rRules().count(), 0);
}

// ============ setExDate ============

TEST(CompatibleDataTest, SetExDate_AllDayEvent)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setAllDay(true);
    QJsonArray ignore;
    ignore.append(dtToString(QDateTime(QDate(2026, 1, 2), QTime(0, 0))));
    DDE_Calendar::setExDate(schedule, ignore);
    EXPECT_GE(schedule->recurrence()->exDates().count(), 1);
}

TEST(CompatibleDataTest, SetExDate_TimedEvent)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setAllDay(false);
    QJsonArray ignore;
    ignore.append(dtToString(QDateTime(QDate(2026, 1, 2), QTime(9, 0))));
    DDE_Calendar::setExDate(schedule, ignore);
    EXPECT_GE(schedule->recurrence()->exDateTimes().count(), 1);
}

TEST(CompatibleDataTest, SetExDate_EmptyArray)
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setAllDay(true);
    QJsonArray ignore;
    DDE_Calendar::setExDate(schedule, ignore);
    EXPECT_EQ(schedule->recurrence()->exDates().count(), 0);
}
