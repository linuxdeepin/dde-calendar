// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "reminderalarm.h"

#include <gtest/gtest.h>

TEST(ReminderAlarmTest, KeepsEveryTimedAlarm)
{
    const DSchedule::AlarmType alarmTypes[] {
        DSchedule::Alarm_None,
        DSchedule::Alarm_Begin,
        DSchedule::Alarm_15Min_Front,
        DSchedule::Alarm_30Min_Front,
        DSchedule::Alarm_1Hour_Front,
        DSchedule::Alarm_1Day_Front,
        DSchedule::Alarm_2Day_Front,
        DSchedule::Alarm_1Week_Front,
    };

    for (DSchedule::AlarmType alarmType : alarmTypes) {
        EXPECT_EQ(alarmType,
                  DDECalendar::normalizeReminderAlarm(false, alarmType));
    }
}

TEST(ReminderAlarmTest, KeepsEveryAllDayAlarm)
{
    const DSchedule::AlarmType alarmTypes[] {
        DSchedule::Alarm_AllDay_None,
        DSchedule::Alarm_9Hour_After,
        DSchedule::Alarm_15Hour_Front,
        DSchedule::Alarm_39Hour_Front,
        DSchedule::Alarm_159Hour_Front,
    };

    for (DSchedule::AlarmType alarmType : alarmTypes) {
        EXPECT_EQ(alarmType,
                  DDECalendar::normalizeReminderAlarm(true, alarmType));
    }
}

TEST(ReminderAlarmTest, FallsBackFromAllDayAlarmForTimedEvent)
{
    EXPECT_EQ(DSchedule::Alarm_15Min_Front,
              DDECalendar::normalizeReminderAlarm(false,
                                                  DSchedule::Alarm_15Hour_Front));
}

TEST(ReminderAlarmTest, FallsBackFromTimedAlarmForAllDayEvent)
{
    EXPECT_EQ(DSchedule::Alarm_15Hour_Front,
              DDECalendar::normalizeReminderAlarm(true,
                                                  DSchedule::Alarm_1Hour_Front));
}
