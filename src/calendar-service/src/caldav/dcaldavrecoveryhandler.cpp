// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavrecoveryhandler.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavoutboxenqueuer.h"
#include "dcaldavrecoveryitem.h"
#include "ddatabase.h"
#include "dschedule.h"
#include "commondef.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace {

QStringList calendarRecoveryTypeIDs(const DCalDavRecoveryItem &item)
{
    QStringList typeIDs;
    const QJsonDocument document = QJsonDocument::fromJson(item.originalIcs.toUtf8());
    if (document.isArray()) {
        for (const QJsonValue &value : document.array()) {
            const QString typeID = value.toString();
            if (!typeID.isEmpty() && !typeIDs.contains(typeID)) {
                typeIDs.append(typeID);
            }
        }
    }
    if (!item.localScheduleID.isEmpty() && !typeIDs.contains(item.localScheduleID)) {
        typeIDs.prepend(item.localScheduleID);
    }
    return typeIDs;
}

bool recoverCalendarDelete(DAccountDataBase *localDatabase,
                           DAccountManagerDataBase *accountManagerDatabase,
                           const DCalDavRecoveryItem &item)
{
    QStringList typeIDs = calendarRecoveryTypeIDs(item);
    DCalDavCalendarInfo calendar = accountManagerDatabase
        ->getCalDavCalendarByScheduleTypeIDIncludingDisabled(
            item.accountID, item.localScheduleID);
    if (!calendar.calendarId.isEmpty()) {
        const DCalDavCategoryInfo::List categories =
            accountManagerDatabase->getCalDavCategoryMappings(
                item.accountID, calendar.calendarId);
        for (const DCalDavCategoryInfo &category : categories) {
            if (!typeIDs.contains(category.scheduleTypeId)) {
                typeIDs.append(category.scheduleTypeId);
            }
        }
    }

    const bool pendingDelete = accountManagerDatabase->hasPendingCalDavCalendarDelete(
        item.accountID, item.localScheduleID);
    const bool activePrimaryType =
        !localDatabase->getScheduleTypeByID(item.localScheduleID).isNull();
    const bool deletedPrimaryType =
        !localDatabase->getScheduleTypeByID(item.localScheduleID, 1).isNull();
    if (!calendar.calendarId.isEmpty() && calendar.enabled && !pendingDelete
        && activePrimaryType && !deletedPrimaryType) {
        // The coordinated transaction rolled back before deleting local data.
        return true;
    }

    if (calendar.calendarId.isEmpty()) {
        SqlTransactionLocker transaction({localDatabase->getConnectionName()});
        if (!transaction.isValid()) {
            return false;
        }
        for (const QString &typeID : typeIDs) {
            if (!localDatabase->deleteSchedulesByScheduleTypeID(typeID, 1)) {
                transaction.rollback();
                return false;
            }
            if ((!localDatabase->getScheduleTypeByID(typeID).isNull()
                 || !localDatabase->getScheduleTypeByID(typeID, 1).isNull())
                && !localDatabase->deleteScheduleTypeByID(typeID, 1)) {
                transaction.rollback();
                return false;
            }
        }
        return transaction.commit();
    }

    SqlTransactionLocker transaction(
        {localDatabase->getConnectionName(), DDataBase::NameAccountManager});
    if (!transaction.isValid()) {
        return false;
    }
    for (const QString &typeID : typeIDs) {
        if (!localDatabase->deleteSchedulesByScheduleTypeID(typeID, 0)) {
            transaction.rollback();
            return false;
        }
        if (!localDatabase->getScheduleTypeByID(typeID).isNull()
            && !localDatabase->deleteScheduleTypeByID(typeID, 0)) {
            transaction.rollback();
            return false;
        }
    }
    calendar.enabled = false;
    if (!accountManagerDatabase->deleteCalDavCalendarEventOutboxItems(
            item.accountID, calendar.calendarId)
        || !accountManagerDatabase->upsertCalDavCalendar(calendar)
        || !DCalDavOutboxEnqueuer::enqueueCalendarDelete(
            accountManagerDatabase, item.accountID, calendar)) {
        transaction.rollback();
        return false;
    }
    return transaction.commit();
}

bool recoverRemoteCalendarDelete(DAccountDataBase *localDatabase,
                                 DAccountManagerDataBase *accountManagerDatabase,
                                 const DCalDavRecoveryItem &item)
{
    QStringList typeIDs = calendarRecoveryTypeIDs(item);
    const DCalDavCalendarInfo calendar = accountManagerDatabase
        ->getCalDavCalendarByScheduleTypeIDIncludingDisabled(
            item.accountID, item.localScheduleID);
    if (!calendar.calendarId.isEmpty()) {
        const DCalDavCategoryInfo::List categories =
            accountManagerDatabase->getCalDavCategoryMappings(
                item.accountID, calendar.calendarId);
        for (const DCalDavCategoryInfo &category : categories) {
            if (!typeIDs.contains(category.scheduleTypeId)) {
                typeIDs.append(category.scheduleTypeId);
            }
        }
    }

    const QStringList connectionNames = calendar.calendarId.isEmpty()
        ? QStringList {localDatabase->getConnectionName()}
        : QStringList {localDatabase->getConnectionName(), DDataBase::NameAccountManager};
    SqlTransactionLocker transaction(connectionNames);
    if (!transaction.isValid()) {
        return false;
    }
    for (const QString &typeID : typeIDs) {
        DScheduleType::Ptr type = localDatabase->getScheduleTypeByID(typeID);
        if (type.isNull()) {
            type = localDatabase->getScheduleTypeByID(typeID, 1);
        }
        if (!localDatabase->deleteSchedulesByScheduleTypeID(typeID, 1)
            || !localDatabase->deleteScheduleTypeByID(typeID, 1)) {
            transaction.rollback();
            return false;
        }
        if (!type.isNull() && type->typeColor().privilege() != DTypeColor::PriSystem) {
            localDatabase->deleteTypeColor(type->typeColor().colorID());
        }
    }
    if (!calendar.calendarId.isEmpty()
        && (!accountManagerDatabase->deleteCalDavOutboxItem(
                item.accountID, item.localScheduleID)
            || !accountManagerDatabase->deleteCalDavCalendarData(
                item.accountID, calendar.calendarId, false))) {
        transaction.rollback();
        return false;
    }
    return transaction.commit();
}

bool restoreMapping(DAccountManagerDataBase *database,
                    const DCalDavRecoveryItem &item,
                    const DSchedule::Ptr &schedule)
{
    if (database == nullptr || schedule.isNull() || item.accountID.isEmpty()
        || item.localScheduleID.isEmpty() || item.calendarID.isEmpty() || item.href.isEmpty()) {
        return false;
    }
    DCalDavEventMappingInfo mapping = database->getCalDavEventMappingByLocalScheduleID(
        item.accountID, item.localScheduleID);
    if (!mapping.href.isEmpty()) {
        return true;
    }
    mapping.accountID = item.accountID;
    mapping.localScheduleID = item.localScheduleID;
    mapping.calendarID = item.calendarID;
    mapping.uid = schedule->uid();
    mapping.href = item.href;
    mapping.etag = item.etag;
    mapping.originalIcs = item.originalIcs;
    return database->upsertCalDavEventMapping(mapping);
}

} // namespace

void DCalDavRecoveryHandler::recover(DAccountDataBase *localDatabase,
                                     DAccountManagerDataBase *accountManagerDatabase,
                                     const QString &accountID)
{
    if (localDatabase == nullptr || accountManagerDatabase == nullptr || accountID.isEmpty()) {
        return;
    }

    const DCalDavRecoveryItem::List items = localDatabase->getCalDavRecoveryItems(accountID);
    for (const DCalDavRecoveryItem &item : items) {
        if (item.operationType == DCalDavRecoveryItem::DeleteCalendarOperation
            || item.operationType == DCalDavRecoveryItem::RemoteDeleteCalendarOperation) {
            const bool recovered = item.operationType == DCalDavRecoveryItem::DeleteCalendarOperation
                ? recoverCalendarDelete(localDatabase, accountManagerDatabase, item)
                : recoverRemoteCalendarDelete(localDatabase, accountManagerDatabase, item);
            if (recovered) {
                localDatabase->deleteCalDavRecoveryItem(item.accountID, item.localScheduleID);
            }
            continue;
        }
        DSchedule::Ptr recoveredSchedule;
        if (!DSchedule::fromIcsString(recoveredSchedule, item.scheduleIcs)
            || recoveredSchedule.isNull()) {
            // CREATE and DELETE recovery only need the persisted resource
            // address. Do not discard a compensating DELETE merely because an
            // older or damaged ICS payload cannot be parsed.
            if (item.operationType == DCalDavRecoveryItem::ModifyOperation) {
                qCWarning(ServiceLogger) << "Keeping invalid CalDAV modify recovery record for schedule:"
                                         << item.localScheduleID;
                continue;
            }
            recoveredSchedule.reset(new DSchedule);
        }
        recoveredSchedule->setUid(item.localScheduleID);

        const bool exists = localDatabase->scheduleExistsByScheduleID(item.localScheduleID);
        const bool deleted = exists && localDatabase->isScheduleDeletedByScheduleID(item.localScheduleID);
        const bool mappingRestored = item.href.isEmpty()
            || restoreMapping(accountManagerDatabase, item, recoveredSchedule);
        bool recovered = false;
        switch (item.operationType) {
        case DCalDavRecoveryItem::CreateOperation:
            if (exists && !deleted) {
                recovered = mappingRestored;
            } else if (mappingRestored) {
                recovered = DCalDavOutboxEnqueuer::enqueue(
                    accountManagerDatabase, item.accountID, recoveredSchedule,
                    DCalDavOutboxEnqueuer::DeleteChange);
            }
            break;
        case DCalDavRecoveryItem::LocalCreateOperation:
            if (exists && !deleted) {
                const DSchedule::Ptr current =
                    localDatabase->getScheduleByScheduleID(item.localScheduleID);
                recovered = !current.isNull()
                    && DCalDavOutboxEnqueuer::enqueue(
                        accountManagerDatabase, item.accountID, current,
                        DCalDavOutboxEnqueuer::CreateChange);
            } else {
                // The local creation never committed, so there is no remote
                // resource to compensate.
                recovered = true;
            }
            break;
        case DCalDavRecoveryItem::ModifyOperation:
            if (exists && !deleted && mappingRestored) {
                const DSchedule::Ptr current =
                    localDatabase->getScheduleByScheduleID(item.localScheduleID);
                recovered = !current.isNull()
                    && DSchedule::toIcsString(current) == item.scheduleIcs
                    && DCalDavOutboxEnqueuer::enqueue(
                        accountManagerDatabase, item.accountID, current,
                        DCalDavOutboxEnqueuer::ModifyChange);
                if (!recovered && !current.isNull()
                    && DSchedule::toIcsString(current) != item.scheduleIcs) {
                    recovered = true;
                }
            }
            break;
        case DCalDavRecoveryItem::DeleteOperation:
            if (item.href.isEmpty()) {
                // A missing href identifies a local-only/orphaned schedule. If
                // its hard delete committed, finish by cancelling any stale
                // local Outbox item. If the local transaction rolled back,
                // keep the restored schedule and discard the recovery marker.
                if (!exists) {
                    recovered = accountManagerDatabase->deleteCalDavOutboxItem(
                        item.accountID, item.localScheduleID);
                } else if (deleted) {
                    recovered = localDatabase->deleteScheduleByScheduleID(
                                    item.localScheduleID, 1)
                        && accountManagerDatabase->deleteCalDavOutboxItem(
                            item.accountID, item.localScheduleID);
                } else {
                    recovered = true;
                }
            } else if (deleted && mappingRestored) {
                recovered = DCalDavOutboxEnqueuer::enqueue(
                    accountManagerDatabase, item.accountID, recoveredSchedule,
                    DCalDavOutboxEnqueuer::DeleteChange);
            } else if (exists && !deleted) {
                const DSchedule::Ptr current =
                    localDatabase->getScheduleByScheduleID(item.localScheduleID);
                recovered = !current.isNull()
                    && DCalDavOutboxEnqueuer::enqueue(
                        accountManagerDatabase, item.accountID, current,
                        DCalDavOutboxEnqueuer::ModifyChange);
            }
            break;
        case DCalDavRecoveryItem::DeleteCalendarOperation:
        case DCalDavRecoveryItem::RemoteDeleteCalendarOperation:
            break;
        }
        if (recovered) {
            localDatabase->deleteCalDavRecoveryItem(item.accountID, item.localScheduleID);
        }
    }
}
