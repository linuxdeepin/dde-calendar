// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldaveventscheduleapplier.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldaveventmapper.h"
#include "dcaldavcategoryinfo.h"
#include "dcaldavcolorallocator.h"
#include "dcaldavxmlreader.h"
#include "ddatabase.h"
#include "dscheduletype.h"
#include "units.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>


namespace {

QStringList normalizedCategories(const DSchedule &schedule)
{
    QStringList categories;
    categories.reserve(schedule.categories().size());
    for (const QString &category : schedule.categories()) {
        const QString normalized = category.trimmed();
        if (!normalized.isEmpty()) {
            categories.append(normalized);
        }
    }
    return categories;
}

QString categoryKey(const QStringList &categories)
{
    QJsonArray array;
    for (const QString &category : categories) {
        array.append(category);
    }
    return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QString categoryDisplayName(const QStringList &categories)
{
    return categories.join(QStringLiteral(", "));
}


QString scheduleTypeForRemoteEvent(const DCalDavEventScheduleApplier::Request &request,
                                    const DSchedule::Ptr &schedule,
                                    QString *errorMessage)
{
    const QStringList categories = normalizedCategories(*schedule);
    const DScheduleType::Privileges privileges = request.calendarPrivileges
            & DCalDavXmlReader::WritePrivilege
        ? DScheduleType::Privileges(DScheduleType::Read | DScheduleType::Write | DScheduleType::Delete)
        : DScheduleType::Read;

    // An event without CATEGORIES belongs to the remote calendar itself. Use
    // the local type created from that calendar's display name instead of
    // creating a synthetic "Other" type for every calendar.
    if (categories.isEmpty() && !request.scheduleTypeID.isEmpty()) {
        const DScheduleType::Ptr calendarType = request.localDatabase->getScheduleTypeByID(
            request.scheduleTypeID);
        if (calendarType.isNull()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Failed to load local CalDAV calendar type.");
            }
            return QString();
        }
        if (calendarType->privilege() != privileges) {
            calendarType->setPrivilege(privileges);
            if (!request.localDatabase->updateScheduleType(calendarType)) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("Failed to update CalDAV calendar permissions.");
                }
                return QString();
            }
        }
        return request.scheduleTypeID;
    }

    const QString key = categoryKey(categories);
    const DCalDavCategoryInfo existing = request.accountManagerDatabase->getCalDavCategoryMapping(
        request.accountID, request.calendarID, key);
    if (!existing.scheduleTypeId.isEmpty()) {
        const DScheduleType::Ptr type = request.localDatabase->getScheduleTypeByID(existing.scheduleTypeId);
        if (type.isNull()) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Failed to load local CalDAV category type.");
            }
            return QString();
        }
        if (type->privilege() != privileges) {
            type->setPrivilege(privileges);
            if (!request.localDatabase->updateScheduleType(type)) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("Failed to update CalDAV category permissions.");
                }
                return QString();
            }
        }
        return existing.scheduleTypeId;
    }

    DScheduleType::Ptr type(new DScheduleType(request.accountID));
    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(DCalDavColorAllocator::nextColor(request.localDatabase));
    color.setPrivilege(DTypeColor::PriSystem);
    color.setDtCreate(QDateTime::currentDateTime());
    type->setTypeName(categoryDisplayName(categories));
    type->setDisplayName(categoryDisplayName(categories));
    type->setTypePath(request.calendarID);
    type->setTypeColor(color);
    type->setDescription(QStringLiteral("CalDAV category"));
    type->setPrivilege(privileges);
    type->setShowState(DScheduleType::Show);
    type->setDtCreate(QDateTime::currentDateTime());
    type->setDeleted(0);
    if (!request.localDatabase->addTypeColor(color)
        || request.localDatabase->createScheduleType(type).isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to create local CalDAV category type.");
        }
        return QString();
    }

    DCalDavCategoryInfo mapping;
    mapping.accountId = request.accountID;
    mapping.calendarId = request.calendarID;
    mapping.categoryKey = key;
    mapping.scheduleTypeId = type->typeID();
    if (!request.accountManagerDatabase->upsertCalDavCategoryMapping(mapping)) {
        request.localDatabase->deleteScheduleTypeByID(type->typeID(), 1);
        request.localDatabase->deleteTypeColor(type->typeColor().colorID());
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to persist local CalDAV category mapping.");
        }
        return QString();
    }
    return type->typeID();
}

bool hasRequiredDatabases(const DCalDavEventScheduleApplier::Request &request)
{
    return !request.accountID.isEmpty() && !request.calendarID.isEmpty() && request.localDatabase != nullptr
        && request.accountManagerDatabase != nullptr;
}

void setError(DCalDavEventScheduleApplier::Result &result, const QString &errorMessage)
{
    if (result.errorMessage.isEmpty()) {
        result.errorMessage = errorMessage;
    }
}

} // namespace

DCalDavEventScheduleApplier::Result DCalDavEventScheduleApplier::apply(const Request &request)
{
    Result result;
    if (!hasRequiredDatabases(request)) {
        result.errorMessage = QStringLiteral("CalDAV schedule apply request is incomplete.");
        return result;
    }
    for (const DCalDavEventReconciler::Action &action : request.actions) {
        bool applied = false;
        switch (action.type) {
        case DCalDavEventReconciler::CreateAction:
            applied = applyCreate(request, action, result);
            break;
        case DCalDavEventReconciler::UpdateAction:
            applied = applyUpdate(request, action, result);
            break;
        case DCalDavEventReconciler::DeleteAction:
            applied = applyDelete(request, action, result);
            break;
        }
        if (!applied) {
            return result;
        }
    }

    result.success = true;
    return result;
}

bool DCalDavEventScheduleApplier::applyCreate(const Request &request,
                                              const DCalDavEventReconciler::Action &action,
                                              Result &result)
{
    DSchedule::Ptr schedule;
    if (!DCalDavEventMapper::toSchedule(action.remoteEvent, schedule, &result.errorMessage)) {
        return false;
    }
    const QString scheduleTypeID = scheduleTypeForRemoteEvent(request, schedule, &result.errorMessage);
    if (scheduleTypeID.isEmpty()) {
        return false;
    }
    schedule->setScheduleTypeID(scheduleTypeID);
    schedule->setCreated(QDateTime::currentDateTime());

    const QString localScheduleID = request.localDatabase->createSchedule(schedule);
    if (localScheduleID.isEmpty()) {
        setError(result, QStringLiteral("Failed to create local CalDAV schedule."));
        return false;
    }
    if (!mapSchedule(request, action, localScheduleID, result)) {
        request.localDatabase->deleteScheduleByScheduleID(localScheduleID, 1);
        return false;
    }

    ++result.createdCount;
    return true;
}

bool DCalDavEventScheduleApplier::applyUpdate(const Request &request,
                                              const DCalDavEventReconciler::Action &action,
                                              Result &result)
{
    if (action.existingMapping.localScheduleID.isEmpty()) {
        setError(result, QStringLiteral("CalDAV update mapping has no local schedule ID."));
        return false;
    }

    DSchedule::Ptr localSchedule = request.localDatabase->getScheduleByScheduleID(
        action.existingMapping.localScheduleID);
    if (localSchedule.isNull() || localSchedule->uid().isEmpty()) {
        setError(result, QStringLiteral("Local schedule for CalDAV update was not found."));
        return false;
    }

    DSchedule::Ptr remoteSchedule;
    if (!DCalDavEventMapper::toSchedule(action.remoteEvent, remoteSchedule, &result.errorMessage)) {
        return false;
    }
    remoteSchedule->setUid(action.existingMapping.localScheduleID);
    const QString scheduleTypeID = scheduleTypeForRemoteEvent(request, remoteSchedule, &result.errorMessage);
    if (scheduleTypeID.isEmpty()) {
        return false;
    }
    remoteSchedule->setScheduleTypeID(scheduleTypeID);
    remoteSchedule->setCreated(localSchedule->created());
    if (!request.localDatabase->updateSchedule(remoteSchedule)) {
        setError(result, QStringLiteral("Failed to update local CalDAV schedule."));
        return false;
    }
    if (!mapSchedule(request, action, action.existingMapping.localScheduleID, result)) {
        if (!request.localDatabase->updateSchedule(localSchedule)) {
            result.errorMessage.append(QStringLiteral(" Failed to restore the local schedule."));
        }
        return false;
    }

    ++result.updatedCount;
    return true;
}

bool DCalDavEventScheduleApplier::applyDelete(const Request &request,
                                              const DCalDavEventReconciler::Action &action,
                                              Result &result)
{
    const QString localScheduleID = action.existingMapping.localScheduleID;
    if (localScheduleID.isEmpty()) {
        setError(result, QStringLiteral("CalDAV delete mapping has no local schedule ID."));
        return false;
    }
    const QString mappingHref = action.existingMapping.href.isEmpty()
        ? action.remoteEvent.href : action.existingMapping.href;
    if (!request.accountManagerDatabase->deleteCalDavEventMapping(request.accountID, mappingHref)) {
        setError(result, QStringLiteral("Failed to delete CalDAV event mapping."));
        return false;
    }
    if (!request.localDatabase->deleteScheduleByScheduleID(localScheduleID)) {
        if (!request.accountManagerDatabase->upsertCalDavEventMapping(action.existingMapping)) {
            result.errorMessage = QStringLiteral("Failed to delete local CalDAV schedule and restore its mapping.");
        } else {
            setError(result, QStringLiteral("Failed to delete local CalDAV schedule."));
        }
        return false;
    }

    ++result.deletedCount;
    return true;
}

bool DCalDavEventScheduleApplier::mapSchedule(const Request &request,
                                              const DCalDavEventReconciler::Action &action,
                                              const QString &localScheduleID, Result &result)
{
    DCalDavEventMappingInfo mapping;
    mapping.localScheduleID = localScheduleID;
    mapping.accountID = request.accountID;
    mapping.calendarID = request.calendarID;
    mapping.uid = action.remoteEvent.uid;
    mapping.href = action.remoteEvent.href;
    mapping.etag = action.remoteEvent.etag;
    mapping.originalIcs = action.remoteEvent.calendarData;
    if (!action.existingMapping.href.isEmpty()
        && action.existingMapping.href != mapping.href
        && !request.accountManagerDatabase->deleteCalDavEventMapping(
            request.accountID, action.existingMapping.href)) {
        setError(result, QStringLiteral("Failed to remove stale CalDAV event mapping."));
        return false;
    }
    if (!request.accountManagerDatabase->upsertCalDavEventMapping(mapping)) {
        setError(result, QStringLiteral("Failed to save CalDAV event mapping."));
        return false;
    }
    return true;
}
