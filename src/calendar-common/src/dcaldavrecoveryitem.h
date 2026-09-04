// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVRECOVERYITEM_H
#define DCALDAVRECOVERYITEM_H

#include <QDateTime>
#include <QString>
#include <QVector>

class DCalDavRecoveryItem
{
public:
    enum OperationType {
        CreateOperation = 0,
        ModifyOperation = 1,
        DeleteOperation = 2,
        LocalCreateOperation = 3,
    };

    typedef QVector<DCalDavRecoveryItem> List;

    QString accountID;
    QString localScheduleID;
    OperationType operationType = CreateOperation;
    QString scheduleIcs;
    QString calendarID;
    QString href;
    QString etag;
    QString originalIcs;
    QDateTime createdAt;
};

#endif // DCALDAVRECOVERYITEM_H
