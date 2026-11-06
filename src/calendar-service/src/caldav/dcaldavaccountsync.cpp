// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavaccountsync.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavprofile.h"
#include "dcaldaveventreconciler.h"
#include "dcaldavretrypolicy.h"
#include "dcaldavsyncstatusmapper.h"
#include "commondef.h"
#include "ddatabase.h"

#include <QHash>
#include <QList>
#include <QSet>
#include <QStringList>

#include <algorithm>
#include <climits>

namespace {

struct IcsComponent {
    QString name;
    QStringList properties;
    QList<IcsComponent> children;
};

QString canonicalIcsComponent(const IcsComponent &component)
{
    QStringList parts = component.properties;
    parts.sort();
    QStringList children;
    for (const IcsComponent &child : component.children) {
        children.append(canonicalIcsComponent(child));
    }
    children.sort();
    parts.append(children);
    return component.name + QLatin1Char('{') + parts.join(QLatin1Char('\n')) + QLatin1Char('}');
}

QString remoteEventSignature(const QString &calendarData)
{
    QString normalized = calendarData;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    QStringList unfoldedLines;
    for (const QString &line : normalized.split(QLatin1Char('\n'))) {
        if ((line.startsWith(QLatin1Char(' ')) || line.startsWith(QLatin1Char('\t')))
            && !unfoldedLines.isEmpty()) {
            unfoldedLines.last().append(line.mid(1));
        } else if (!line.isEmpty()) {
            unfoldedLines.append(line);
        }
    }

    IcsComponent root;
    QList<IcsComponent *> stack {&root};
    for (const QString &line : unfoldedLines) {
        const QString upperLine = line.toUpper();
        if (upperLine.startsWith(QStringLiteral("BEGIN:"))) {
            IcsComponent child;
            child.name = line.mid(6).trimmed().toUpper();
            stack.constLast()->children.append(child);
            stack.append(&stack.constLast()->children.last());
            continue;
        }
        if (upperLine.startsWith(QStringLiteral("END:"))) {
            const QString name = line.mid(4).trimmed().toUpper();
            if (stack.size() <= 1 || stack.constLast()->name != name) {
                return QString();
            }
            stack.removeLast();
            continue;
        }
        if (stack.size() <= 1 || stack.constLast() == nullptr) {
            continue;
        }

        const qsizetype colon = line.indexOf(QLatin1Char(':'));
        if (colon <= 0) {
            return QString();
        }
        const QString propertyName = line.left(colon).section(QLatin1Char(';'), 0, 0).toUpper();
        const bool inEvent = std::any_of(stack.cbegin(), stack.cend(), [](const IcsComponent *component) {
            return component != nullptr && component->name == QStringLiteral("VEVENT");
        });
        if (inEvent && (propertyName == QStringLiteral("DTSTAMP")
                        || propertyName == QStringLiteral("CREATED")
                        || propertyName == QStringLiteral("LAST-MODIFIED"))) {
            continue;
        }
        if (inEvent && propertyName == QStringLiteral("TRANSP")
            && line.compare(QStringLiteral("TRANSP:OPAQUE"), Qt::CaseInsensitive) == 0) {
            continue;
        }
        if (inEvent && propertyName == QStringLiteral("X-KDE-KCALCORE-ENABLED")
            && line.compare(QStringLiteral("X-KDE-KCALCORE-ENABLED:TRUE"), Qt::CaseInsensitive) == 0) {
            continue;
        }
        const QString propertyHead = line.left(colon);
        const QString normalizedHead = propertyName + propertyHead.mid(propertyName.size());
        stack.constLast()->properties.append(normalizedHead + line.mid(colon));
    }
    if (stack.size() != 1) {
        return QString();
    }

    QStringList events;
    QStringList timeZones;
    for (const IcsComponent &calendar : root.children) {
        for (const IcsComponent &child : calendar.children) {
            if (child.name == QStringLiteral("VEVENT")) {
                events.append(canonicalIcsComponent(child));
            } else if (child.name == QStringLiteral("VTIMEZONE")) {
                timeZones.append(canonicalIcsComponent(child));
            }
        }
    }
    if (events.isEmpty()) {
        return QString();
    }
    events.sort();
    timeZones.sort();
    return timeZones.join(QLatin1Char('\n')) + QLatin1Char('\n')
        + events.join(QLatin1Char('\n'));
}

DCalDavEventMappingInfo::List accountMappings(
    const QString &accountID, const QVector<DCalDavAccountSync::CalendarRequest> &calendars,
    DAccountManagerDataBase *database)
{
    DCalDavEventMappingInfo::List mappings;
    QSet<QString> includedCalendarIDs;
    for (const DCalDavAccountSync::CalendarRequest &calendar : calendars) {
        const DCalDavEventMappingInfo::List calendarMappings = database->getCalDavEventMappingList(
            accountID, calendar.calendar.calendarId);
        mappings.append(calendarMappings);
        includedCalendarIDs.insert(calendar.calendar.calendarId);
    }

    DCalDavAccountInfo accountInfo;
    if (database->getCalDavAccountInfo(accountID, accountInfo)) {
        for (const DCalDavCalendarInfo &calendar : database->getCalDavCalendarList(accountID)) {
            const bool isQQAggregate = DCalDavProviderProfile::isQQMailAggregateCalendar(
                DCalDavProviderProfile::Provider_QQMail, accountInfo.username,
                QUrl(calendar.href));
            if (includedCalendarIDs.contains(calendar.calendarId)
                || !isQQAggregate) {
                continue;
            }
            mappings.append(database->getCalDavEventMappingList(accountID, calendar.calendarId));
        }
    }
    return mappings;
}

bool hasEquivalentMappingInOtherCalendar(
    const DCalDavCalendarQuery::RemoteEvent &remoteEvent, const QString &calendarID,
    const DCalDavEventMappingInfo::List &mappings)
{
    if (remoteEvent.deleted || remoteEvent.uid.isEmpty()) {
        return false;
    }
    const QString signature = remoteEventSignature(remoteEvent.calendarData);
    if (signature.isEmpty()) {
        return false;
    }
    for (const DCalDavEventMappingInfo &mapping : mappings) {
        if (mapping.calendarID != calendarID && mapping.uid == remoteEvent.uid
            && remoteEventSignature(mapping.originalIcs) == signature) {
            return true;
        }
    }
    return false;
}

bool removeDuplicateMappings(const QString &accountID,
                             const QVector<DCalDavAccountSync::CalendarRequest> &calendars,
                             DAccountDataBase *localDatabase,
                             DAccountManagerDataBase *accountManagerDatabase,
                             int *removedCount)
{
    const DCalDavEventMappingInfo::List mappings = accountMappings(
        accountID, calendars, accountManagerDatabase);
    QHash<QString, DCalDavEventMappingInfo> canonicalBySignature;
    QHash<QString, int> calendarOrder;
    for (int index = 0; index < calendars.size(); ++index) {
        calendarOrder.insert(calendars.at(index).calendar.calendarId, index);
    }

    for (const DCalDavEventMappingInfo &mapping : mappings) {
        if (mapping.uid.isEmpty()) {
            continue;
        }
        const QString signature = remoteEventSignature(mapping.originalIcs);
        if (signature.isEmpty()) {
            continue;
        }
        const QString key = mapping.uid + QLatin1Char('\n') + signature;
        const auto existing = canonicalBySignature.constFind(key);
        if (existing == canonicalBySignature.cend()) {
            canonicalBySignature.insert(key, mapping);
            continue;
        }
        if (existing->calendarID == mapping.calendarID) {
            continue;
        }

        const bool existingIsTombstone = localDatabase->isScheduleDeletedByScheduleID(
            existing->localScheduleID);
        const bool mappingIsTombstone = localDatabase->isScheduleDeletedByScheduleID(
            mapping.localScheduleID);
        const int existingOrder = calendarOrder.value(existing->calendarID, INT_MAX);
        const int mappingOrder = calendarOrder.value(mapping.calendarID, INT_MAX);
        const bool mappingIsCanonical = existingIsTombstone != mappingIsTombstone
            ? existingIsTombstone
            : mappingOrder < existingOrder;
        const DCalDavEventMappingInfo canonical = mappingIsCanonical ? mapping : existing.value();
        const DCalDavEventMappingInfo duplicate = mappingIsCanonical ? existing.value() : mapping;
        if (!accountManagerDatabase->getCalDavOutboxItem(accountID, duplicate.localScheduleID)
                 .operationID.isEmpty()) {
            continue;
        }
        if (localDatabase->isScheduleDeletedByScheduleID(duplicate.localScheduleID)) {
            canonicalBySignature.insert(key, canonical);
            continue;
        }
        const bool sharesLocalSchedule = duplicate.localScheduleID == existing->localScheduleID;
        SqlTransactionLocker transaction(
            {localDatabase->getConnectionName(), DDataBase::NameAccountManager});
        if (!transaction.isValid()) {
            return false;
        }
        const bool localDeleted = sharesLocalSchedule
            || localDatabase->deleteScheduleByScheduleID(duplicate.localScheduleID);
        const bool mappingRemoved = sharesLocalSchedule;
        const bool mappingUpdated = localDeleted
            && (!mappingRemoved
                || accountManagerDatabase->deleteCalDavEventMapping(accountID, duplicate.href));
        if (!localDeleted || !mappingUpdated) {
            transaction.rollback();
            return false;
        }
        if (!transaction.commit()) {
            bool repaired = false;
            if (transaction.hasPartialCommit()
                && transaction.committedConnectionNames().contains(localDatabase->getConnectionName())
                && mappingRemoved) {
                SqlTransactionLocker repairTransaction({DDataBase::NameAccountManager});
                repaired = repairTransaction.isValid()
                    && accountManagerDatabase->deleteCalDavEventMapping(accountID, duplicate.href)
                    && repairTransaction.commit();
            } else if (transaction.hasPartialCommit()
                       && transaction.committedConnectionNames().contains(localDatabase->getConnectionName())) {
                repaired = true;
            }
            if (!repaired) {
                return false;
            }
        }
        ++(*removedCount);
        canonicalBySignature.insert(key, canonical);
    }
    return true;
}

} // namespace

DCalDavAccountSync::DCalDavAccountSync(QObject *parent)
    : QObject(parent)
    , m_calendarSync()
    , m_outboxProcessor()
{
}

DCalDavAccountSync::~DCalDavAccountSync()
{
    cancel(false);
}

void DCalDavAccountSync::cancel(bool notifyCallback)
{
    if (!m_running) {
        return;
    }
    m_outboxProcessor.cancel(false);
    m_calendarSync.cancel(false);
    m_request.password.clear();
    Callback callback = m_callback;
    m_callback = Callback();
    m_running = false;
    if (notifyCallback && callback) {
        m_result.success = false;
        m_result.errorMessage = QStringLiteral("CalDAV synchronization cancelled.");
        callback(m_result);
    }
}

void DCalDavAccountSync::start(const Request &request, const Callback &callback)
{
    if (m_running) {
        return;
    }

    m_request = request;
    m_callback = callback;
    m_result = Result();
    m_calendarIndex = 0;
    m_providerType = -1;
    m_successfulCalendarCount = 0;
    m_permissionDeniedCalendarCount = 0;
    m_lastPermissionDeniedError.clear();
    m_running = true;

    if (request.accountID.isEmpty() || request.username.isEmpty() || request.localDatabase == nullptr
        || request.accountManagerDatabase == nullptr) {
        finish(false, QStringLiteral("CalDAV account sync request is incomplete."));
        return;
    }

    DCalDavAccountInfo accountInfo;
    if (request.accountManagerDatabase->getCalDavAccountInfo(request.accountID, accountInfo)) {
        m_request.retryCount = accountInfo.retryCount;
        m_providerType = accountInfo.providerType;
    }

    int duplicateCount = 0;
    if (m_providerType == DCalDavProviderProfile::Provider_QQMail
        && !removeDuplicateMappings(m_request.accountID, m_request.calendars,
                                    m_request.localDatabase, m_request.accountManagerDatabase,
                                    &duplicateCount)) {
        finish(false, QStringLiteral("Failed to remove duplicate QQ Mail calendar entries."));
        return;
    }
    m_result.deletedCount += duplicateCount;

    if (!request.accountManagerDatabase->updateCalDavSyncStatus(
            request.accountID, DCalDavSyncStatus::Running, QDateTime(), QString())) {
        finish(false, QStringLiteral("Failed to mark CalDAV account as syncing."));
        return;
    }
    qCDebug(ServiceLogger) << "CalDAV account sync started"
                             << "account:" << request.accountID
                             << "calendarCount:" << request.calendars.size();
    flushOutbox();
}

void DCalDavAccountSync::flushOutbox()
{
    DCalDavOutboxProcessor::Request outboxRequest;
    outboxRequest.accountID = m_request.accountID;
    outboxRequest.username = m_request.username;
    outboxRequest.password = m_request.password;
    outboxRequest.localDatabase = m_request.localDatabase;
    outboxRequest.accountManagerDatabase = m_request.accountManagerDatabase;
    outboxRequest.forceRetry = m_request.forceOutboxRetry;
    m_outboxProcessor.start(outboxRequest, [this](const DCalDavOutboxProcessor::Result &result) {
        m_result.updatedCount += result.serverVersionAppliedCount;
        m_result.failureResponse = result.failureResponse;
        m_result.failureCode = result.failureCode != DCalDavErrorCode::NoError
            ? result.failureCode
            : DCalDavSyncStatusMapper::errorCodeForFailure(m_result.failureResponse);
        if (!result.success && m_result.failureCode == DCalDavErrorCode::NoError) {
            m_result.failureCode = DCalDavErrorCode::Unknown;
        }
        m_result.createFailure = result.createFailure;
        m_result.retryDeferred = result.retryScheduledCount > 0
            && result.permanentFailureCount == 0
            && !result.requestAttempted;
        if (!result.success) {
            if (result.retryScheduledCount > 0 && result.permanentFailureCount == 0) {
                m_request.accountManagerDatabase->updateCalDavSyncStatus(
                    m_request.accountID, DCalDavSyncStatus::RetryScheduled, QDateTime(),
                    result.errorMessage, m_result.failureCode);
                finish(false, result.errorMessage);
            } else {
                fail(result.errorMessage);
            }
            return;
        }
        syncNextCalendar();
    });
}

void DCalDavAccountSync::syncNextCalendar()
{
    if (m_calendarIndex >= m_request.calendars.size()) {
        if (m_successfulCalendarCount == 0 && m_permissionDeniedCalendarCount > 0) {
            fail(m_lastPermissionDeniedError);
            return;
        }
        if (!m_request.accountManagerDatabase->clearCalDavRetryState(m_request.accountID)
            || !m_request.accountManagerDatabase->updateCalDavSyncStatus(
                m_request.accountID, DCalDavSyncStatus::Succeeded, QDateTime::currentDateTimeUtc(), QString())) {
            finish(false, QStringLiteral("CalDAV sync succeeded but status update failed."));
            return;
        }
        m_result.failureResponse = DCalDavTransport::Response();
        finish(true);
        return;
    }

    const CalendarRequest &cachedCalendar = m_request.calendars.at(m_calendarIndex);
    const DCalDavCalendarInfo currentCalendar = m_request.accountManagerDatabase
        ->getCalDavCalendarByIDIncludingDisabled(
            m_request.accountID, cachedCalendar.calendar.calendarId);
    if (currentCalendar.calendarId.isEmpty() || !currentCalendar.enabled
        || m_request.accountManagerDatabase->hasPendingCalDavCalendarDelete(
               m_request.accountID, cachedCalendar.scheduleTypeID)) {
        ++m_calendarIndex;
        syncNextCalendar();
        return;
    }
    CalendarRequest calendar = cachedCalendar;
    calendar.calendar = currentCalendar;
    calendar.scheduleTypeID = currentCalendar.scheduleTypeID;
    qCDebug(ServiceLogger) << "CalDAV calendar sync started"
                             << "account:" << m_request.accountID
                             << "calendar:" << calendar.calendar.displayName
                             << "endpoint:" << DCalDavTransport::urlForLog(QUrl(calendar.calendar.href));
    DCalDavIncrementalSync::Request syncRequest;
    syncRequest.calendarUrl = QUrl(calendar.calendar.href);
    syncRequest.username = m_request.username;
    syncRequest.password = m_request.password;
    syncRequest.syncToken = calendar.calendar.syncToken;
    syncRequest.initialSyncCompleted = calendar.calendar.initialSyncCompleted;
    syncRequest.calendarId = calendar.calendar.calendarId;
    const DCalDavEventMappingInfo::List existingMappings =
        m_request.accountManagerDatabase->getCalDavEventMappingList(
            m_request.accountID, calendar.calendar.calendarId);
    syncRequest.existingMappings = existingMappings;
    if (!m_request.accountManagerDatabase->getCalDavSkippedResources(
            m_request.accountID, calendar.calendar.calendarId,
            syncRequest.existingSkippedResources)) {
        fail(QStringLiteral("Failed to load malformed CalDAV resource records."));
        return;
    }
    m_calendarSync.start(syncRequest, [this, calendar, existingMappings](
                                        const DCalDavIncrementalSync::Result &syncResult) {
        if (!syncResult.success) {
            qCWarning(ServiceLogger) << "CalDAV calendar sync failed"
                                     << "account:" << m_request.accountID
                                     << "calendar:" << calendar.calendar.displayName
                                     << "endpoint:" << DCalDavTransport::urlForLog(QUrl(calendar.calendar.href))
                                     << "httpStatus:" << syncResult.failureResponse.httpStatus
                                     << "transportError:" << static_cast<int>(syncResult.failureResponse.error)
                                     << "errorPresent:" << !syncResult.errorMessage.isEmpty();
            m_result.failureResponse = syncResult.failureResponse;
            m_result.failureCode = syncResult.failureCode != DCalDavErrorCode::NoError
                ? syncResult.failureCode
                : DCalDavSyncStatusMapper::errorCodeForFailure(m_result.failureResponse);
            if (!syncResult.success && m_result.failureCode == DCalDavErrorCode::NoError) {
                m_result.failureCode = DCalDavErrorCode::Unknown;
            }
            const bool permissionDenied = syncResult.failureResponse.httpStatus == 403
                || syncResult.failureResponse.error == DCalDavTransport::PermissionDenied;
            if (permissionDenied) {
                DCalDavCalendarInfo inaccessibleCalendar = calendar.calendar;
                inaccessibleCalendar.enabled = false;
                if (!m_request.accountManagerDatabase->upsertCalDavCalendar(inaccessibleCalendar)) {
                    fail(QStringLiteral("Failed to disable inaccessible CalDAV calendar."));
                    return;
                }
                ++m_permissionDeniedCalendarCount;
                m_lastPermissionDeniedError = syncResult.errorMessage;
                qCWarning(ServiceLogger) << "Skipping inaccessible CalDAV calendar"
                                         << "account:" << m_request.accountID
                                         << "calendar:" << calendar.calendar.displayName;
                ++m_calendarIndex;
                syncNextCalendar();
                return;
            }
            fail(syncResult.errorMessage);
            return;
        }

        qCDebug(ServiceLogger) << "CalDAV calendar sync succeeded"
                                 << "account:" << m_request.accountID
                                 << "calendar:" << calendar.calendar.displayName
                                 << "endpoint:" << DCalDavTransport::urlForLog(QUrl(calendar.calendar.href))
                                 << "remoteEventCount:" << syncResult.remoteEvents.size();

        DCalDavCalendarQuery::RemoteEventList remoteEvents = syncResult.remoteEvents;
        if (m_providerType == DCalDavProviderProfile::Provider_QQMail) {
            const DCalDavEventMappingInfo::List allMappings = accountMappings(
                m_request.accountID, m_request.calendars, m_request.accountManagerDatabase);
            DAccountDataBase *localDatabase = m_request.localDatabase;
            remoteEvents.erase(
                std::remove_if(remoteEvents.begin(), remoteEvents.end(),
                               [&calendar, &allMappings, localDatabase](
                                   const DCalDavCalendarQuery::RemoteEvent &event) {
                    if (event.href.isEmpty() || event.deleted) {
                        return false;
                    }
                    const bool hasDeletedUidInCalendar = !event.uid.isEmpty()
                        && std::any_of(allMappings.cbegin(), allMappings.cend(),
                                       [&event, &calendar, localDatabase](
                            const DCalDavEventMappingInfo &mapping) {
                            return mapping.calendarID == calendar.calendar.calendarId
                                && mapping.uid == event.uid
                                && localDatabase->isScheduleDeletedByScheduleID(
                                    mapping.localScheduleID);
                        });
                    if (hasDeletedUidInCalendar) {
                        return true;
                    }
                    const auto mappingHere = std::find_if(
                        allMappings.cbegin(), allMappings.cend(), [&event, &calendar](
                            const DCalDavEventMappingInfo &mapping) {
                            return mapping.calendarID == calendar.calendar.calendarId
                                && mapping.href == event.href;
                        });
                    if (mappingHere != allMappings.cend()) {
                        return localDatabase->isScheduleDeletedByScheduleID(
                            mappingHere->localScheduleID);
                    }
                    return hasEquivalentMappingInOtherCalendar(event,
                                                               calendar.calendar.calendarId,
                                                               allMappings);
                }), remoteEvents.end());
        }

        DCalDavEventReconciler::ActionList actions;
        QString errorMessage;
        if (!DCalDavEventReconciler::buildActions(
                remoteEvents, existingMappings, actions, &errorMessage)) {
            fail(errorMessage);
            return;
        }

        DCalDavEventScheduleApplier::Request applyRequest;
        applyRequest.accountID = m_request.accountID;
        applyRequest.calendarID = calendar.calendar.calendarId;
        applyRequest.scheduleTypeID = calendar.scheduleTypeID;
        applyRequest.calendarPrivileges = calendar.calendar.privileges;
        applyRequest.actions = actions;
        applyRequest.localDatabase = m_request.localDatabase;
        applyRequest.accountManagerDatabase = m_request.accountManagerDatabase;
        const DCalDavEventScheduleApplier::Result applyResult = DCalDavEventScheduleApplier::apply(applyRequest);
        if (!applyResult.success) {
            fail(applyResult.errorMessage);
            return;
        }

        m_result.createdCount += applyResult.createdCount;
        m_result.updatedCount += applyResult.updatedCount;
        m_result.deletedCount += applyResult.deletedCount;
        if (syncResult.usedFullRangeFallback) {
            ++m_result.fullRangeFallbackCount;
        }

        for (const DCalDavSkippedResource &resource : syncResult.skippedResources) {
            if (!m_request.accountManagerDatabase->upsertCalDavSkippedResource(
                    m_request.accountID, resource)) {
                fail(QStringLiteral("Failed to save malformed CalDAV resource record."));
                return;
            }
        }
        for (const QString &href : syncResult.clearedSkippedResourceHrefs) {
            if (!m_request.accountManagerDatabase->deleteCalDavSkippedResource(
                    m_request.accountID, calendar.calendar.calendarId, href)) {
                fail(QStringLiteral("Failed to clear malformed CalDAV resource record."));
                return;
            }
        }

        if (syncResult.usedFullRangeFallback || !syncResult.syncToken.isEmpty()) {
            const QString nextToken = syncResult.usedFullRangeFallback ? QString() : syncResult.syncToken;
            if (!m_request.accountManagerDatabase->updateCalDavCalendarSyncToken(
                    calendar.calendar.calendarId, nextToken)) {
                fail(QStringLiteral("Failed to save CalDAV calendar sync token."));
                return;
            }
        }
        if (!calendar.calendar.initialSyncCompleted
            && !m_request.accountManagerDatabase->updateCalDavCalendarInitialSyncCompleted(
                calendar.calendar.calendarId, true)) {
            fail(QStringLiteral("Failed to save CalDAV initial sync state."));
            return;
        }

        ++m_successfulCalendarCount;
        ++m_calendarIndex;
        syncNextCalendar();
    });
}

void DCalDavAccountSync::fail(const QString &errorMessage)
{
    if (m_result.failureResponse.error != DCalDavTransport::NoError) {
        const DCalDavRetryPolicy::Decision retry = DCalDavRetryPolicy::decide(
            m_result.failureResponse, m_request.retryCount);
        if (retry.retry) {
            m_request.accountManagerDatabase->updateCalDavRetryState(
                m_request.accountID, m_request.retryCount + 1,
                QDateTime::currentDateTimeUtc().addSecs(retry.delaySeconds), errorMessage,
                m_result.failureCode);
        } else {
            m_request.accountManagerDatabase->updateCalDavSyncStatus(
                m_request.accountID, DCalDavSyncStatus::fromErrorCode(m_result.failureCode), QDateTime(), errorMessage,
                    m_result.failureCode);
        }
    } else {
        m_request.accountManagerDatabase->updateCalDavSyncStatus(
            m_request.accountID, DCalDavSyncStatus::fromErrorCode(m_result.failureCode), QDateTime(), errorMessage,
                m_result.failureCode);
    }
    finish(false, errorMessage);
}

void DCalDavAccountSync::finish(bool success, const QString &errorMessage)
{
    if (!m_running) {
        return;
    }

    m_result.success = success;
    m_result.errorMessage = errorMessage;
    m_request.password.clear();
    m_running = false;
    if (m_callback) {
        const Callback callback = m_callback;
        m_callback = Callback();
        callback(m_result);
    }
}
