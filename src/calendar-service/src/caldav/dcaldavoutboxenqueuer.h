// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVOUTBOXENQUEUER_H
#define DCALDAVOUTBOXENQUEUER_H

#include "dschedule.h"

#include <QString>

class DAccountManagerDataBase;

class DCalDavOutboxEnqueuer
{
public:
    enum ChangeType {
        CreateChange,
        ModifyChange,
        DeleteChange,
    };

    static bool enqueue(DAccountManagerDataBase *database, const QString &accountID,
                        const DSchedule::Ptr &schedule, ChangeType change);
};

#endif // DCALDAVOUTBOXENQUEUER_H
