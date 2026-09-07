// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVEVENTSCHEDULEAPPLIER_H
#define DCALDAVEVENTSCHEDULEAPPLIER_H

#include "dcaldaveventreconciler.h"

#include <QDateTime>
#include <QString>

class DAccountDataBase;
class DAccountManagerDataBase;

class DCalDavEventScheduleApplier
{
public:
    struct Request {
        QString accountID;
        QString calendarID;
        QString scheduleTypeID;
        int calendarPrivileges = 0;
        DCalDavEventReconciler::ActionList actions;
        DAccountDataBase *localDatabase = nullptr;
        DAccountManagerDataBase *accountManagerDatabase = nullptr;
    };

    struct Result {
        bool success = false;
        int createdCount = 0;
        int updatedCount = 0;
        int deletedCount = 0;
        QString errorMessage;
    };

    /**
     * @brief Applies reconciled remote-event actions to the local databases.
     * @param request Databases, calendar metadata, privileges, and actions to apply.
     * @return Counts of applied operations and an error when an action fails.
     */
    static Result apply(const Request &request);

private:
    static bool applyCreate(const Request &request, const DCalDavEventReconciler::Action &action,
                            Result &result);
    static bool applyUpdate(const Request &request, const DCalDavEventReconciler::Action &action,
                            Result &result);
    static bool applyDelete(const Request &request, const DCalDavEventReconciler::Action &action,
                            Result &result);
    static bool mapSchedule(const Request &request, const DCalDavEventReconciler::Action &action,
                            const QString &localScheduleID, Result &result);
};

#endif // DCALDAVEVENTSCHEDULEAPPLIER_H
