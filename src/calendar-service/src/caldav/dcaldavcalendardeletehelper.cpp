// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcalendardeletehelper.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavoutboxenqueuer.h"
#include "dscheduletype.h"

namespace DCalDavCalendarDeleteHelper {

QStringList collectTypeIDs(const QString &primaryTypeID,
                           const DCalDavCategoryInfo::List &categories)
{
    QStringList typeIDs;
    const auto appendTypeID = [&typeIDs](const QString &typeID) {
        if (!typeID.isEmpty() && !typeIDs.contains(typeID)) {
            typeIDs.append(typeID);
        }
    };

    appendTypeID(primaryTypeID);
    for (const DCalDavCategoryInfo &category : categories) {
        appendTypeID(category.scheduleTypeId);
    }
    return typeIDs;
}

QStringList collectScheduleIDs(DAccountDataBase *localDatabase,
                               const QStringList &typeIDs)
{
    QStringList scheduleIDs;
    if (localDatabase == nullptr) {
        return scheduleIDs;
    }

    for (const QString &typeID : typeIDs) {
        for (const QString &scheduleID : localDatabase->getScheduleIDListByTypeID(typeID)) {
            if (!scheduleIDs.contains(scheduleID)) {
                scheduleIDs.append(scheduleID);
            }
        }
    }
    return scheduleIDs;
}

bool deleteScheduleTypes(DAccountDataBase *localDatabase,
                         const QStringList &typeIDs,
                         bool hardDelete,
                         bool deleteColors)
{
    if (localDatabase == nullptr) {
        return false;
    }

    const int deleteMode = hardDelete ? 1 : 0;
    for (const QString &typeID : typeIDs) {
        const DScheduleType::Ptr type = localDatabase->getScheduleTypeByID(typeID);
        if (type.isNull()) {
            continue;
        }
        if (!localDatabase->deleteSchedulesByScheduleTypeID(typeID, deleteMode)
            || !localDatabase->deleteScheduleTypeByID(typeID, deleteMode)) {
            return false;
        }
        if (deleteColors && type->typeColor().privilege() != DTypeColor::PriSystem) {
            localDatabase->deleteTypeColor(type->typeColor().colorID());
        }
    }
    return true;
}

bool deleteOutboxItems(DAccountManagerDataBase *accountManagerDatabase,
                       const QString &accountID,
                       const QStringList &scheduleIDs)
{
    if (accountManagerDatabase == nullptr || accountID.isEmpty()) {
        return false;
    }

    for (const QString &scheduleID : scheduleIDs) {
        if (!accountManagerDatabase->deleteCalDavOutboxItem(accountID, scheduleID)) {
            return false;
        }
    }
    return true;
}

bool deleteRemoteManagerData(DAccountManagerDataBase *accountManagerDatabase,
                             const QString &accountID,
                             const QString &calendarID,
                             const QStringList &scheduleIDs)
{
    return deleteOutboxItems(accountManagerDatabase, accountID, scheduleIDs)
        && accountManagerDatabase->deleteCalDavCalendarData(
            accountID, calendarID, false);
}

bool persistLocalDeleteState(DAccountManagerDataBase *accountManagerDatabase,
                             const QString &accountID,
                             const DCalDavCalendarInfo &calendar,
                             const QStringList &scheduleIDs)
{
    if (!deleteOutboxItems(accountManagerDatabase, accountID, scheduleIDs)
        || !accountManagerDatabase->deleteCalDavCalendarEventOutboxItems(
            accountID, calendar.calendarId)) {
        return false;
    }

    DCalDavCalendarInfo disabledCalendar = calendar;
    disabledCalendar.enabled = false;
    return accountManagerDatabase->upsertCalDavCalendar(disabledCalendar)
        && DCalDavOutboxEnqueuer::enqueueCalendarDelete(
            accountManagerDatabase, accountID, calendar);
}

} // namespace DCalDavCalendarDeleteHelper
