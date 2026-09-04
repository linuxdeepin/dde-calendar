// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVOUTBOXITEM_H
#define DCALDAVOUTBOXITEM_H

#include <QDateTime>
#include <QString>
#include <QVector>

class DCalDavOutboxItem
{
public:
    enum OperationType {
        CreateOperation = 0,
        ModifyOperation = 1,
        DeleteOperation = 2,
    };

    enum FailureType {
        NoFailure = 0,
        NetworkFailure = 1,
        AuthenticationFailure = 2,
        PermissionFailure = 3,
        ConflictFailure = 4,
        PermanentFailure = 5,
    };

    typedef QVector<DCalDavOutboxItem> List;

    QString operationID;
    QString accountID;
    QString localScheduleID;
    OperationType operationType = CreateOperation;
    QString baseEtag;
    QString conflictIcs;
    QString serverIcs;
    int retryCount = 0;
    QDateTime nextRetryAt;
    FailureType failureType = NoFailure;
};

#endif // DCALDAVOUTBOXITEM_H
