// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVCALENDARINFO_H
#define DCALDAVCALENDARINFO_H

#include <QString>
#include <QVector>

class DCalDavCalendarInfo
{
public:
    typedef QVector<DCalDavCalendarInfo> List;

    QString calendarId;
    QString accountId;
    QString href;
    QString displayName;
    QString color;
    QString scheduleTypeID;
    QString syncToken;
    bool initialSyncCompleted = false;
    int privileges = 0;
    bool privilegesKnown = false;
    bool enabled = true;
};

#endif // DCALDAVCALENDARINFO_H
