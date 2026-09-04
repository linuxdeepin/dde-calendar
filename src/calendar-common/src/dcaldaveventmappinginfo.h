// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVEVENTMAPPINGINFO_H
#define DCALDAVEVENTMAPPINGINFO_H

#include <QString>
#include <QVector>

class DCalDavEventMappingInfo
{
public:
    typedef QVector<DCalDavEventMappingInfo> List;

    QString localScheduleID;
    QString accountID;
    QString calendarID;
    QString uid;
    QString href;
    QString etag;
    QString originalIcs;
};

#endif // DCALDAVEVENTMAPPINGINFO_H
