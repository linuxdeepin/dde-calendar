// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavrecoveryhandler.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavoutboxenqueuer.h"
#include "dcaldavrecoveryitem.h"
#include "dschedule.h"
#include "commondef.h"

namespace {

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
            if (deleted && mappingRestored) {
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
        }
        if (recovered) {
            localDatabase->deleteCalDavRecoveryItem(item.accountID, item.localScheduleID);
        }
    }
}
