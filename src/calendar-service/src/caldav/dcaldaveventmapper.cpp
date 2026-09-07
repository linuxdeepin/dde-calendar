// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldaveventmapper.h"

bool DCalDavEventMapper::toSchedule(const DCalDavCalendarQuery::RemoteEvent &remoteEvent,
                                    DSchedule::Ptr &schedule, QString *errorMessage)
{
    if (remoteEvent.uid.isEmpty() || remoteEvent.calendarData.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Remote event is missing UID or calendar data.");
        }
        return false;
    }

    DSchedule::Ptr parsed;
    if (!DSchedule::fromIcsString(parsed, remoteEvent.calendarData)) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to parse remote calendar data.");
        }
        return false;
    }

    if (parsed->uid() != remoteEvent.uid) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Remote event UID does not match calendar data.");
        }
        return false;
    }

    schedule = parsed;
    return true;
}
