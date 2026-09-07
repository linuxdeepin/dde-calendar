// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavoutboxprocessor.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavprofile.h"
#include "dcaldavretrypolicy.h"
#include "dcaldavutils.h"
#include "dcaldavxmlreader.h"
#include "dschedule.h"


#include <QDateTime>

#include <algorithm>
#include <QUrl>
#include <QXmlStreamReader>
#include <QRegularExpression>

namespace {

bool isSuccessful(int httpStatus)
{
    return httpStatus >= 200 && httpStatus < 300;
}

QString responseError(const DCalDavOutboxItem &item, const DCalDavTransport::Response &response)
{
    QString operation;
    switch (item.operationType) {
    case DCalDavOutboxItem::CreateOperation:
        operation = QStringLiteral("create");
        break;
    case DCalDavOutboxItem::ModifyOperation:
        operation = QStringLiteral("modify");
        break;
    case DCalDavOutboxItem::DeleteOperation:
        operation = QStringLiteral("delete");
        break;
    }
    return QStringLiteral("CalDAV %1 failed (HTTP %2, error %3).")
        .arg(operation)
        .arg(response.httpStatus)
        .arg(static_cast<int>(response.error));
}

DCalDavCalendarInfo calendarFor(const DCalDavOutboxItem &item, const DSchedule::Ptr &schedule,
                                DAccountManagerDataBase *database)
{
    const DCalDavEventMappingInfo mapping = database->getCalDavEventMappingByLocalScheduleID(
        item.accountID, item.localScheduleID);
    if (!mapping.calendarID.isEmpty()) {
        const DCalDavCalendarInfo::List calendars = database->getCalDavCalendarList(item.accountID);
        for (const DCalDavCalendarInfo &calendar : calendars) {
            if (calendar.enabled && calendar.calendarId == mapping.calendarID) {
                return calendar;
            }
        }
        // A persisted event mapping must never silently move to a different
        // collection just because its original collection is unavailable.
        return DCalDavCalendarInfo();
    }
    return schedule.isNull() ? DCalDavCalendarInfo()
                             : database->getCalDavCalendarByScheduleTypeID(
                                   item.accountID, schedule->scheduleTypeID());
}


QString parseEtag(const QByteArray &xml)
{
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == QStringLiteral("getetag")) {
            return reader.readElementText(QXmlStreamReader::SkipChildElements);
        }
    }
    return QString();
}

QString icsForRemoteWrite(const DSchedule::Ptr &schedule,
                          const DCalDavEventMappingInfo &mapping)
{
    QString localIcs = DSchedule::toIcsString(schedule);
    if (mapping.originalIcs.isEmpty()) {
        return localIcs;
    }

    const QRegularExpression eventExpression(
        QStringLiteral("BEGIN:VEVENT\\r?\\n.*?END:VEVENT"),
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression uidExpression(
        QStringLiteral("(?:^|\\r?\\n)UID:([^\\r\\n]+)"),
        QRegularExpression::CaseInsensitiveOption);
    QStringList detachedEvents;
    auto match = eventExpression.globalMatch(mapping.originalIcs);
    while (match.hasNext()) {
        const QString event = match.next().captured(0);
        if (!event.contains(QStringLiteral("RECURRENCE-ID:"), Qt::CaseInsensitive)) {
            continue;
        }
        const QRegularExpressionMatch uidMatch = uidExpression.match(event);
        if (uidMatch.hasMatch() && uidMatch.captured(1).trimmed() == schedule->uid()) {
            detachedEvents.append(event);
        }
    }
    if (detachedEvents.isEmpty()) {
        return localIcs;
    }

    const int calendarEnd = localIcs.lastIndexOf(QStringLiteral("END:VCALENDAR"));
    if (calendarEnd < 0) {
        return localIcs;
    }
    localIcs.insert(calendarEnd, detachedEvents.join(QStringLiteral("\r\n"))
                    + QStringLiteral("\r\n"));
    return localIcs;
}

} // namespace

DCalDavOutboxProcessor::DCalDavOutboxProcessor(QObject *parent)
    : QObject(parent)
    , m_transport(this)
{
}

void DCalDavOutboxProcessor::cancel()
{
    if (!m_running) {
        return;
    }
    m_transport.cancel();
    m_request.password.clear();
    m_callback = Callback();
    m_running = false;
}

void DCalDavOutboxProcessor::start(const Request &request, const Callback &callback)
{
    if (m_running) {
        return;
    }

    m_request = request;
    m_callback = callback;
    m_result = Result();
    m_itemIndex = 0;
    m_running = true;
    if (request.accountID.isEmpty() || request.username.isEmpty() || request.localDatabase == nullptr
        || request.accountManagerDatabase == nullptr) {
        finish(false, QStringLiteral("CalDAV Outbox request is incomplete."));
        return;
    }

    DCalDavAccountInfo accountInfo;
    if (!request.accountManagerDatabase->getCalDavAccountInfo(request.accountID, accountInfo)
        || !DCalDavProviderProfile::forProvider(
               static_cast<DCalDavProviderProfile::ProviderType>(accountInfo.providerType)).supportsWrite) {
        finish(true);
        return;
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    const DCalDavOutboxItem::List blockedItems =
        request.accountManagerDatabase->getCalDavBlockedOutboxItems(request.accountID);
    const bool hasConflict = std::any_of(
        blockedItems.cbegin(), blockedItems.cend(), [](const DCalDavOutboxItem &item) {
            return item.failureType == DCalDavOutboxItem::ConflictFailure;
        });
    if (hasConflict || (!request.forceRetry && !blockedItems.isEmpty())) {
        m_result.permanentFailureCount = blockedItems.size();
        for (const DCalDavOutboxItem &item : blockedItems) {
            switch (item.failureType) {
            case DCalDavOutboxItem::AuthenticationFailure:
                m_result.errorMessage = QStringLiteral(
                    "CalDAV synchronization failed (HTTP 401, error 0).");
                break;
            case DCalDavOutboxItem::PermissionFailure:
                m_result.errorMessage = QStringLiteral(
                    "CalDAV synchronization failed (HTTP 403, error 0).");
                break;
            case DCalDavOutboxItem::ConflictFailure:
                m_result.errorMessage = QStringLiteral(
                    "CalDAV synchronization conflict requires resolution.");
                break;
            default:
                m_result.errorMessage = QStringLiteral(
                    "CalDAV synchronization failure requires manual retry.");
                break;
            }
            if (item.failureType == DCalDavOutboxItem::ConflictFailure) {
                break;
            }
        }
        finish(false, m_result.errorMessage);
        return;
    }

    const DCalDavOutboxItem::List retryScheduledItems =
        request.accountManagerDatabase->getCalDavRetryScheduledOutboxItems(
            request.accountID, now);
    if (!request.forceRetry && !retryScheduledItems.isEmpty()) {
        m_result.retryScheduledCount = retryScheduledItems.size();
        m_result.errorMessage = QStringLiteral(
            "CalDAV synchronization retry is scheduled.");
        finish(false, m_result.errorMessage);
        return;
    }

    m_items = request.accountManagerDatabase->getDueCalDavOutboxItems(
        request.accountID, now, request.forceRetry);
    processNext();
}

void DCalDavOutboxProcessor::processNext()
{
    if (m_itemIndex >= m_items.size()) {
        finish(m_result.permanentFailureCount == 0 && m_result.retryScheduledCount == 0,
               m_result.errorMessage);
        return;
    }
    processItem(m_items.at(m_itemIndex));
}

void DCalDavOutboxProcessor::processMissingSchedule(
    const DCalDavOutboxItem &item, const DCalDavEventMappingInfo &mapping)
{
    // A local row can disappear while its durable outbox item is still
    // pending (for example after a partial database commit or a cleanup on
    // restart). If the mapping is known, turn the item into an idempotent
    // remote DELETE instead of replaying a create/update with no payload.
    if (!mapping.href.isEmpty()) {
        DCalDavOutboxItem cleanup = item;
        cleanup.operationType = DCalDavOutboxItem::DeleteOperation;
        cleanup.baseEtag = mapping.etag;
        cleanup.conflictIcs.clear();
        cleanup.serverIcs.clear();
        cleanup.retryCount = 0;
        cleanup.nextRetryAt = QDateTime();
        cleanup.failureType = DCalDavOutboxItem::NoFailure;
        if (m_request.accountManagerDatabase->upsertCalDavOutboxItem(cleanup)) {
            processItem(cleanup);
        } else {
            recordFailure(item, DCalDavTransport::Response());
        }
        return;
    }

    // Without a local payload or a persisted remote href there is no safe URL
    // to call. Keep the item visible as a permanent failure rather than
    // silently losing a potentially remote event.
    DCalDavTransport::Response response;
    response.httpStatus = 500;
    recordFailure(item, response);
}

void DCalDavOutboxProcessor::repairCreateOperation(
    const DCalDavOutboxItem &item, const DCalDavEventMappingInfo &mapping)
{
    DCalDavOutboxItem repaired = item;
    repaired.operationType = DCalDavOutboxItem::ModifyOperation;
    repaired.baseEtag = mapping.etag;
    if (m_request.accountManagerDatabase->upsertCalDavOutboxItem(repaired)) {
        processItem(repaired);
    } else {
        recordFailure(item, DCalDavTransport::Response());
    }
}

void DCalDavOutboxProcessor::sendDeleteRequest(const DCalDavOutboxItem &item,
                                                const DCalDavEventMappingInfo &mapping)
{
    const QUrl resourceUrl(mapping.href);
    if (mapping.href.isEmpty() || !resourceUrl.isValid() || resourceUrl.scheme().isEmpty()
        || resourceUrl.host().isEmpty()) {
        DCalDavTransport::Response response;
        response.httpStatus = 412;
        recordFailure(item, response);
        return;
    }

    DCalDavTransport::Request request;
    request.url = resourceUrl;
    request.method = "DELETE";
    request.username = m_request.username;
    request.password = m_request.password;
    if (!item.baseEtag.isEmpty()) {
        request.headers.insert("If-Match", item.baseEtag.toUtf8());
    }
    m_transport.send(request, [this, item, resourceUrl](const DCalDavTransport::Response &response) {
        handleWriteResponse(item, resourceUrl, response);
    });
}

void DCalDavOutboxProcessor::processItem(const DCalDavOutboxItem &item)
{
    const DSchedule::Ptr schedule = m_request.localDatabase->getScheduleByScheduleID(item.localScheduleID);
    const DCalDavEventMappingInfo mapping = m_request.accountManagerDatabase
        ->getCalDavEventMappingByLocalScheduleID(item.accountID, item.localScheduleID);
    if (schedule.isNull() && item.operationType != DCalDavOutboxItem::DeleteOperation) {
        processMissingSchedule(item, mapping);
        return;
    }

    if (!schedule.isNull() && !mapping.href.isEmpty()
        && item.operationType == DCalDavOutboxItem::CreateOperation) {
        repairCreateOperation(item, mapping);
        return;
    }

    if (item.operationType == DCalDavOutboxItem::DeleteOperation) {
        sendDeleteRequest(item, mapping);
        return;
    }

    const DCalDavCalendarInfo calendar = calendarFor(item, schedule, m_request.accountManagerDatabase);
    if (calendar.calendarId.isEmpty() || !(calendar.privileges & DCalDavXmlReader::WritePrivilege)) {
        DCalDavTransport::Response response;
        response.httpStatus = 403;
        response.error = DCalDavTransport::PermissionDenied;
        recordFailure(item, response);
        return;
    }

    if (schedule.isNull() || schedule->uid().isEmpty()) {
        DCalDavTransport::Response response;
        response.httpStatus = 412;
        recordFailure(item, response);
        return;
    }
    const QUrl resourceUrl = item.operationType == DCalDavOutboxItem::CreateOperation
        ? DCalDavUtils::eventUrl(calendar, schedule->uid())
        : (mapping.href.isEmpty() ? DCalDavUtils::eventUrl(calendar, schedule->uid()) : QUrl(mapping.href));
    if (!resourceUrl.isValid()) {
        DCalDavTransport::Response response;
        response.httpStatus = 412;
        recordFailure(item, response);
        return;
    }

    if (item.operationType == DCalDavOutboxItem::ModifyOperation && item.baseEtag.isEmpty()) {
        fetchWriteEtag(item, resourceUrl);
        return;
    }

    sendWriteRequest(item, resourceUrl, item.baseEtag.toUtf8());
}

void DCalDavOutboxProcessor::fetchWriteEtag(const DCalDavOutboxItem &item, const QUrl &resourceUrl)
{
    DCalDavTransport::Request request;
    request.url = resourceUrl;
    request.method = "PROPFIND";
    request.contentType = "application/xml; charset=utf-8";
    request.username = m_request.username;
    request.password = m_request.password;
    request.headers.insert("Depth", "0");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<d:propfind xmlns:d=\"DAV:\"><d:prop><d:getetag/>"
                   "</d:prop></d:propfind>";
    m_transport.send(request, [this, item, resourceUrl](const DCalDavTransport::Response &response) {
        if (!isCurrentItem(item)) {
            processCurrentItemOrAdvance(item);
            return;
        }
        const QByteArray etag = isSuccessful(response.httpStatus)
            ? parseEtag(response.body).trimmed().toUtf8() : QByteArray();
        if (etag.isEmpty()) {
            DCalDavTransport::Response failure = response;
            if (failure.httpStatus == 0) {
                failure.httpStatus = 412;
            }
            recordFailure(item, failure);
            return;
        }
        sendWriteRequest(item, resourceUrl, etag);
    });
}

void DCalDavOutboxProcessor::sendWriteRequest(const DCalDavOutboxItem &item,
                                               const QUrl &resourceUrl,
                                               const QByteArray &etag)
{
    if (!isCurrentItem(item)) {
        processCurrentItemOrAdvance(item);
        return;
    }

    const DSchedule::Ptr schedule = m_request.localDatabase->getScheduleByScheduleID(item.localScheduleID);
    const DCalDavEventMappingInfo mapping = m_request.accountManagerDatabase
        ->getCalDavEventMappingByLocalScheduleID(item.accountID, item.localScheduleID);
    if (schedule.isNull() || schedule->uid().isEmpty()) {
        DCalDavTransport::Response response;
        response.httpStatus = 412;
        recordFailure(item, response);
        return;
    }
    if (item.operationType == DCalDavOutboxItem::ModifyOperation && etag.isEmpty()) {
        DCalDavTransport::Response response;
        response.httpStatus = 412;
        recordFailure(item, response);
        return;
    }

    DSchedule::Ptr remoteSchedule(new DSchedule(*schedule));
    if (!mapping.uid.isEmpty()) {
        remoteSchedule->setUid(mapping.uid);
    }
    DCalDavTransport::Request request;
    request.url = resourceUrl;
    request.method = "PUT";
    request.contentType = "text/calendar; charset=utf-8";
    request.username = m_request.username;
    request.password = m_request.password;
    request.body = icsForRemoteWrite(remoteSchedule, mapping).toUtf8();
    if (item.operationType == DCalDavOutboxItem::CreateOperation) {
        request.headers.insert("If-None-Match", "*");
    } else {
        request.headers.insert("If-Match", etag);
    }

    m_transport.send(request, [this, item, resourceUrl](const DCalDavTransport::Response &response) {
        handleWriteResponse(item, resourceUrl, response);
    });
}

void DCalDavOutboxProcessor::handleWriteResponse(const DCalDavOutboxItem &item, const QUrl &resourceUrl,
                                                  const DCalDavTransport::Response &response)
{
    if (!isCurrentItem(item)) {
        processCurrentItemOrAdvance(item);
        return;
    }
    if (isSuccessful(response.httpStatus)
        || (item.operationType == DCalDavOutboxItem::DeleteOperation && response.httpStatus == 404)) {
        if (item.operationType == DCalDavOutboxItem::DeleteOperation || !response.etag.isEmpty()) {
            completeSuccess(item, resourceUrl, response.etag);
        } else {
            fetchEtag(item, resourceUrl);
        }
        return;
    }
    if (response.httpStatus == 409 || response.httpStatus == 412) {
        fetchConflictSnapshot(item, resourceUrl);
        return;
    }
    recordFailure(item, response);
}

void DCalDavOutboxProcessor::fetchEtag(const DCalDavOutboxItem &item, const QUrl &resourceUrl)
{
    DCalDavTransport::Request request;
    request.url = resourceUrl;
    request.method = "PROPFIND";
    request.contentType = "application/xml; charset=utf-8";
    request.username = m_request.username;
    request.password = m_request.password;
    request.headers.insert("Depth", "0");
    request.body = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                   "<d:propfind xmlns:d=\"DAV:\"><d:prop><d:getetag/>"
                   "</d:prop></d:propfind>";
    m_transport.send(request, [this, item, resourceUrl](const DCalDavTransport::Response &response) {
        if (isSuccessful(response.httpStatus)) {
            completeSuccess(item, resourceUrl, parseEtag(response.body).toUtf8());
        } else {
            // The remote write already succeeded. Keep the mapping with an empty
            // ETag rather than replaying a potentially duplicate PUT.
            completeSuccess(item, resourceUrl, QByteArray());
        }
    });
}

void DCalDavOutboxProcessor::fetchConflictSnapshot(const DCalDavOutboxItem &item,
                                                   const QUrl &resourceUrl)
{
    DCalDavTransport::Request request;
    request.url = resourceUrl;
    request.method = "GET";
    request.username = m_request.username;
    request.password = m_request.password;
    m_transport.send(request, [this, item](const DCalDavTransport::Response &response) {
        if (!isCurrentItem(item)) {
            processCurrentItemOrAdvance(item);
            return;
        }

        // The snapshot request is part of the same synchronization attempt.
        // A transient failure here must use the normal retry policy; turning
        // it into a conflict with an empty server snapshot would permanently
        // block the item and leave the user without a server-version choice.
        if (!isSuccessful(response.httpStatus)) {
            recordFailure(item, response);
            return;
        }

        DCalDavOutboxItem updated = item;
        updated.failureType = DCalDavOutboxItem::ConflictFailure;
        updated.retryCount = 0;
        updated.nextRetryAt = QDateTime();
        const DSchedule::Ptr localSchedule = m_request.localDatabase->getScheduleByScheduleID(
            item.localScheduleID);
        updated.conflictIcs = localSchedule.isNull() ? QString() : DSchedule::toIcsString(localSchedule);
        updated.serverIcs = QString::fromUtf8(response.body).trimmed();
        DSchedule::Ptr serverSchedule;
        if (!updated.serverIcs.isEmpty()
            && !DSchedule::fromIcsString(serverSchedule, updated.serverIcs)) {
            updated.serverIcs.clear();
        }
        const bool hasServerSnapshot = !updated.serverIcs.isEmpty();
        if (!hasServerSnapshot) {
            // Keep this as a resolvable conflict so the user can still choose
            // the local version when the server returned no usable object.
            // The server-version action remains unavailable without a snapshot.
            updated.failureType = DCalDavOutboxItem::ConflictFailure;
        }
        if (!m_request.accountManagerDatabase->upsertCalDavOutboxItem(updated)) {
            ++m_result.permanentFailureCount;
            if (m_result.errorMessage.isEmpty()) {
                m_result.errorMessage = QStringLiteral(
                    "Failed to record a CalDAV synchronization conflict.");
            }
        } else {
            ++m_result.conflictDiscardedCount;
            ++m_result.permanentFailureCount;
            if (m_result.errorMessage.isEmpty()) {
                m_result.errorMessage = hasServerSnapshot
                    ? QStringLiteral("CalDAV synchronization conflict requires resolution.")
                    : QStringLiteral("CalDAV conflict snapshot is empty; keep the local version or retry.");
            }
        }
        ++m_itemIndex;
        processNext();
    });
}

void DCalDavOutboxProcessor::completeSuccess(const DCalDavOutboxItem &item, const QUrl &resourceUrl,
                                              const QByteArray &etag)
{
    if (!isCurrentItem(item)) {
        processCurrentItemOrAdvance(item);
        return;
    }
    if (item.operationType == DCalDavOutboxItem::DeleteOperation) {
        const DCalDavEventMappingInfo mapping = m_request.accountManagerDatabase
            ->getCalDavEventMappingByLocalScheduleID(item.accountID, item.localScheduleID);
        if (m_request.localDatabase->scheduleExistsByScheduleID(item.localScheduleID)
            && !m_request.localDatabase->deleteScheduleByScheduleID(item.localScheduleID, 1)) {
            finish(false, QStringLiteral("Failed to remove completed local schedule."));
            return;
        }
        if (!mapping.href.isEmpty()
            && !m_request.accountManagerDatabase->deleteCalDavEventMapping(item.accountID, mapping.href)) {
            finish(false, QStringLiteral("Failed to remove completed CalDAV event mapping."));
            return;
        }
    } else {
        const DSchedule::Ptr schedule = m_request.localDatabase->getScheduleByScheduleID(item.localScheduleID);
        if (schedule.isNull()) {
            ++m_itemIndex;
            processNext();
            return;
        }
        const DCalDavCalendarInfo calendar = calendarFor(item, schedule, m_request.accountManagerDatabase);
        DCalDavEventMappingInfo mapping = m_request.accountManagerDatabase
            ->getCalDavEventMappingByLocalScheduleID(item.accountID, item.localScheduleID);
        mapping.localScheduleID = item.localScheduleID;
        mapping.accountID = item.accountID;
        mapping.calendarID = calendar.calendarId;
        if (mapping.uid.isEmpty()) {
            mapping.uid = schedule->uid();
        }
        DSchedule::Ptr remoteSchedule(new DSchedule(*schedule));
        remoteSchedule->setUid(mapping.uid);
        mapping.href = resourceUrl.toString();
        mapping.etag = QString::fromUtf8(etag);
        mapping.originalIcs = icsForRemoteWrite(remoteSchedule, mapping);
        if (!m_request.accountManagerDatabase->upsertCalDavEventMapping(mapping)) {
            finish(false, QStringLiteral("Failed to save CalDAV write mapping."));
            return;
        }
    }

    if (!m_request.accountManagerDatabase->deleteCalDavOutboxItemIfCurrent(item)) {
        if (!isCurrentItem(item)) {
            processCurrentItemOrAdvance(item);
            return;
        }
        finish(false, QStringLiteral("Failed to remove completed CalDAV Outbox item."));
        return;
    }
    ++m_result.processedCount;
    ++m_itemIndex;
    processNext();
}

void DCalDavOutboxProcessor::recordFailure(const DCalDavOutboxItem &item,
                                           const DCalDavTransport::Response &response)
{
    if (!isCurrentItem(item)) {
        processCurrentItemOrAdvance(item);
        return;
    }
    DCalDavOutboxItem updated = item;
    m_result.failureResponse = response;
    const DCalDavRetryPolicy::Decision retry = DCalDavRetryPolicy::decide(response, item.retryCount);
    if (retry.retry) {
        updated.retryCount = item.retryCount + 1;
        updated.nextRetryAt = QDateTime::currentDateTimeUtc().addSecs(retry.delaySeconds);
        updated.failureType = DCalDavOutboxItem::NetworkFailure;
        ++m_result.retryScheduledCount;
    } else if (response.httpStatus == 401) {
        updated.failureType = DCalDavOutboxItem::AuthenticationFailure;
        ++m_result.permanentFailureCount;
    } else if (response.httpStatus == 403) {
        updated.failureType = DCalDavOutboxItem::PermissionFailure;
        ++m_result.permanentFailureCount;
    } else if (response.httpStatus == 409 || response.httpStatus == 412) {
        updated.failureType = DCalDavOutboxItem::ConflictFailure;
        updated.retryCount = 0;
        updated.nextRetryAt = QDateTime();
        if (updated.conflictIcs.isEmpty()) {
            const DSchedule::Ptr localSchedule = m_request.localDatabase->getScheduleByScheduleID(
                item.localScheduleID);
            updated.conflictIcs = localSchedule.isNull() ? QString() : DSchedule::toIcsString(localSchedule);
        }
        ++m_result.permanentFailureCount;
    } else {
        updated.failureType = DCalDavOutboxItem::PermanentFailure;
        ++m_result.permanentFailureCount;
    }
    const bool notifyCreateFailure = item.operationType == DCalDavOutboxItem::CreateOperation
        && (item.retryCount == 0 || m_request.forceRetry);
    if (notifyCreateFailure && response.httpStatus == 403) {
        m_result.createFailure = DCalDavScheduleCreateError::PermissionDenied;
    } else if (notifyCreateFailure
               && (response.error == DCalDavTransport::NetworkUnavailable
                   || response.error == DCalDavTransport::RequestTimedOut
                   || response.error == DCalDavTransport::NetworkError)) {
        m_result.createFailure = DCalDavScheduleCreateError::NetworkUnavailable;
    }
    if (!m_request.accountManagerDatabase->upsertCalDavOutboxItem(updated)
        && m_result.errorMessage.isEmpty()) {
        m_result.errorMessage = QStringLiteral("Failed to record CalDAV synchronization failure.");
    }
    if (m_result.errorMessage.isEmpty()) {
        m_result.errorMessage = responseError(item, response);
    }
    ++m_itemIndex;
    processNext();
}

bool DCalDavOutboxProcessor::isCurrentItem(const DCalDavOutboxItem &item) const
{
    const DCalDavOutboxItem current = m_request.accountManagerDatabase->getCalDavOutboxItem(
        item.accountID, item.localScheduleID);
    return current.operationID == item.operationID
        && current.operationType == item.operationType
        && current.baseEtag == item.baseEtag
        && current.conflictIcs == item.conflictIcs
        && current.serverIcs == item.serverIcs
        && current.retryCount == item.retryCount
        && current.nextRetryAt == item.nextRetryAt
        && current.failureType == item.failureType;
}

bool DCalDavOutboxProcessor::processCurrentItemOrAdvance(const DCalDavOutboxItem &item)
{
    const DCalDavOutboxItem current = m_request.accountManagerDatabase->getCalDavOutboxItem(
        item.accountID, item.localScheduleID);
    if (current.operationID.isEmpty()) {
        ++m_itemIndex;
        processNext();
        return false;
    }
    processItem(current);
    return true;
}

void DCalDavOutboxProcessor::finish(bool success, const QString &errorMessage)
{
    if (!m_running) {
        return;
    }
    m_result.success = success;
    if (!errorMessage.isEmpty()) {
        m_result.errorMessage = errorMessage;
    }
    m_request.password.clear();
    m_running = false;
    if (m_callback) {
        const Callback callback = m_callback;
        m_callback = Callback();
        callback(m_result);
    }
}
