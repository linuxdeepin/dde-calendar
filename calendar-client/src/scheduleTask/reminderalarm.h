// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REMINDERALARM_H
#define REMINDERALARM_H

#include "dschedule.h"

namespace DDECalendar {
inline DSchedule::AlarmType normalizeReminderAlarm(bool allDay,
                                                    DSchedule::AlarmType alarmType)
{
    const bool isTimedAlarm = alarmType >= DSchedule::Alarm_None
                              && alarmType <= DSchedule::Alarm_1Week_Front;
    const bool isAllDayAlarm = alarmType >= DSchedule::Alarm_AllDay_None
                               && alarmType <= DSchedule::Alarm_159Hour_Front;

    if ((allDay && isAllDayAlarm) || (!allDay && isTimedAlarm)) {
        return alarmType;
    }

    return allDay ? DSchedule::Alarm_15Hour_Front
                  : DSchedule::Alarm_15Min_Front;
}
} // namespace DDECalendar

#endif // REMINDERALARM_H
