// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavoutboxenqueuer.h"

#include "daccountmanagerdatabase.h"
#include "dcaldavprofile.h"
#include "dcaldavxmlreader.h"
#include "ddatabase.h"
#include "dschedule.h"

namespace {

bool supportsWrite(DAccountManagerDataBase *database, const QString &accountID,
                   const QString &scheduleTypeID, DCalDavEventMappingInfo &mapping)
{
    DCalDavAccountInfo accountInfo;
    if (!database->getCalDavAccountInfo(accountID, accountInfo)
        || !DCalDavProviderProfile::forProvider(
               static_cast<DCalDavProviderProfile::ProviderType>(accountInfo.providerType)).supportsWrite) {
        return false;
    }

    mapping = database->getCalDavEventMappingByLocalScheduleID(accountID, mapping.localScheduleID);
    DCalDavCalendarInfo calendar;
    if (!mapping.calendarID.isEmpty()) {
        const DCalDavCalendarInfo::List calendars = database->getCalDavCalendarList(accountID);
        for (const DCalDavCalendarInfo &candidate : calendars) {
            if (candidate.enabled && candidate.calendarId == mapping.calendarID) {
                calendar = candidate;
                break;
            }
        }
    } else {
        calendar = database->getCalDavCalendarByScheduleTypeID(accountID, scheduleTypeID);
        if (calendar.calendarId.isEmpty()) {
            const DCalDavCategoryInfo category =
                database->getCalDavCategoryMappingByScheduleTypeID(accountID, scheduleTypeID);
            if (!category.calendarId.isEmpty()) {
                const DCalDavCalendarInfo::List calendars = database->getCalDavCalendarList(accountID);
                for (const DCalDavCalendarInfo &candidate : calendars) {
                    if (candidate.enabled && candidate.calendarId == category.calendarId) {
                        calendar = candidate;
                        break;
                    }
                }
            }
        }
    }
    return !calendar.calendarId.isEmpty()
        && (calendar.privileges & DCalDavXmlReader::WritePrivilege);
}

DCalDavOutboxItem::OperationType operationFor(DCalDavOutboxItem::OperationType existing,
                                              DCalDavOutboxEnqueuer::ChangeType change,
                                              bool hasRemoteMapping)
{
    if (existing == DCalDavOutboxItem::CreateOperation) {
        // Once a remote mapping exists, a local edit must update the resource
        // instead of replaying an If-None-Match create.
        if (change == DCalDavOutboxEnqueuer::ModifyChange && hasRemoteMapping) {
            return DCalDavOutboxItem::ModifyOperation;
        }
        return DCalDavOutboxItem::CreateOperation;
    }
    if (change == DCalDavOutboxEnqueuer::DeleteChange) {
        return DCalDavOutboxItem::DeleteOperation;
    }
    if (existing == DCalDavOutboxItem::DeleteOperation) {
        return hasRemoteMapping ? DCalDavOutboxItem::ModifyOperation
                                : DCalDavOutboxItem::CreateOperation;
    }
    return DCalDavOutboxItem::ModifyOperation;
}

} // namespace

bool DCalDavOutboxEnqueuer::enqueue(DAccountManagerDataBase *database, const QString &accountID,
                                    const DSchedule::Ptr &schedule, ChangeType change)
{
    if (database == nullptr || schedule.isNull() || accountID.isEmpty() || schedule->uid().isEmpty()) {
        return false;
    }

    DCalDavEventMappingInfo mapping;
    mapping.localScheduleID = schedule->uid();
    // A delete must remain queued even when discovery temporarily marks the
    // collection disabled or no longer exposes its current permissions. The
    // persisted mapping is enough to address the remote resource; the
    // processor will retry the DELETE until the server confirms it.
    if (change == DeleteChange) {
        DCalDavAccountInfo accountInfo;
        if (!database->getCalDavAccountInfo(accountID, accountInfo)
            || !DCalDavProviderProfile::forProvider(
                   static_cast<DCalDavProviderProfile::ProviderType>(accountInfo.providerType)).supportsWrite) {
            return false;
        }
        mapping = database->getCalDavEventMappingByLocalScheduleID(accountID, schedule->uid());
    } else if (!supportsWrite(database, accountID, schedule->scheduleTypeID(), mapping)) {
        return false;
    }

    DCalDavOutboxItem current = database->getCalDavOutboxItem(accountID, schedule->uid());
    if (change == DeleteChange && mapping.href.isEmpty()
        && (current.operationID.isEmpty() || current.operationType != DCalDavOutboxItem::CreateOperation)) {
        // Do not persist a DELETE that cannot be addressed remotely. A caller
        // can roll back its local deletion and report the inconsistent mapping.
        return false;
    }
    if (!current.operationID.isEmpty() && current.operationType == DCalDavOutboxItem::CreateOperation
        && change == DeleteChange) {
        return database->deleteCalDavOutboxItem(accountID, schedule->uid());
    }

    DCalDavOutboxItem item = current;
    if (item.operationID.isEmpty()) {
        item.operationID = DDataBase::createUuid();
        item.accountID = accountID;
        item.localScheduleID = schedule->uid();
        item.baseEtag = mapping.etag;
        item.operationType = change == DeleteChange ? DCalDavOutboxItem::DeleteOperation
            : (change == CreateChange || mapping.href.isEmpty()
               ? DCalDavOutboxItem::CreateOperation
               : DCalDavOutboxItem::ModifyOperation);
    } else {
        item.operationType = operationFor(item.operationType, change, !mapping.href.isEmpty());
        if (item.baseEtag.isEmpty()) {
            item.baseEtag = mapping.etag;
        }
        item.retryCount = 0;
        item.nextRetryAt = QDateTime();
        item.failureType = DCalDavOutboxItem::NoFailure;
    }
    return database->upsertCalDavOutboxItem(item);
}
