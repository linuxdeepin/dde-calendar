// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVEVENTMAPPER_H
#define DCALDAVEVENTMAPPER_H

#include "dcaldavcalendarquery.h"

#include <dschedule.h>

class DCalDavEventMapper
{
public:
    static bool toSchedule(const DCalDavCalendarQuery::RemoteEvent &remoteEvent,
                           DSchedule::Ptr &schedule, QString *errorMessage = nullptr);
};

#endif // DCALDAVEVENTMAPPER_H
