// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVCATEGORYINFO_H
#define DCALDAVCATEGORYINFO_H

#include <QString>
#include <QVector>

class DCalDavCategoryInfo
{
public:
    typedef QVector<DCalDavCategoryInfo> List;

    QString accountId;
    QString calendarId;
    QString categoryKey;
    QString scheduleTypeId;
};

#endif // DCALDAVCATEGORYINFO_H
