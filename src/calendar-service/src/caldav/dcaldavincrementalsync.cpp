// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavincrementalsync.h"

#include "dcaldavdiscovery.h"
#include "dcaldavutils.h"
#include "commondef.h"


namespace {

const QString kNoSyncTokenMarker = QStringLiteral("dde-calendar:no-sync-token");


QString uidFromCalendarData(const QString &calendarData)
{
    QString logicalLine;
    const auto uidFromLogicalLine = [](const QString &line) {
        const int separator = line.indexOf(QLatin1Char(':'));
        if (separator < 1) {
            return QString();
        }
        const QString propertyName = line.left(separator);
        if (propertyName.compare(QStringLiteral("UID"), Qt::CaseInsensitive) == 0
            || propertyName.startsWith(QStringLiteral("UID;"), Qt::CaseInsensitive)) {
            return line.mid(separator + 1);
        }
        return QString();
    };

    int lineStart = 0;
    while (lineStart <= calendarData.size()) {
        int lineEnd = calendarData.indexOf(QLatin1Char('\n'), lineStart);
        if (lineEnd < 0) {
            lineEnd = calendarData.size();
        }
        int lineLength = lineEnd - lineStart;
        if (lineLength > 0 && calendarData.at(lineEnd - 1) == QLatin1Char('\r')) {
            --lineLength;
        }
        const QStringView line = QStringView(calendarData).mid(lineStart, lineLength);
        const bool foldedLine = !line.isEmpty()
            && (line.at(0) == QLatin1Char(' ') || line.at(0) == QLatin1Char('\t'));
        if (foldedLine && !logicalLine.isEmpty()) {
            logicalLine.append(line.mid(1));
        } else {
            if (!logicalLine.isEmpty()) {
                const QString uid = uidFromLogicalLine(logicalLine);
                if (!uid.isEmpty()) {
                    return uid;
                }
            }
            logicalLine = line.toString();
        }

        if (lineEnd == calendarData.size()) {
            break;
        }
        lineStart = lineEnd + 1;
    }

    return uidFromLogicalLine(logicalLine);
}

bool isEventResource(const DCalDavCalendarQuery::Resource &resource)
{
    if (resource.contentType.contains(QStringLiteral("vevent"), Qt::CaseInsensitive)) {
        return true;
    }
    return resource.contentType.isEmpty()
        && resource.href.endsWith(QStringLiteral(".ics"), Qt::CaseInsensitive);
}

bool hasEventComponent(const QString &calendarData)
{
    return calendarData.contains(QStringLiteral("BEGIN:VEVENT"), Qt::CaseInsensitive);
}

bool overlapsInitialSyncRange(const DSchedule::Ptr &schedule, const QDateTime &referenceTime)
{
    if (schedule.isNull()) {
        return false;
    }
    const QDateTime rangeStart = referenceTime.addMonths(-1);
    const QDateTime rangeEnd = referenceTime.addMonths(6);
    const QDateTime scheduleStart = schedule->dtStart();
    const QDateTime scheduleEnd = schedule->dtEnd();
    if (scheduleStart.isValid() && scheduleEnd.isValid()
        && !(scheduleEnd < rangeStart || scheduleStart > rangeEnd)) {
        return true;
    }
    if (!schedule->recurs() || schedule->recurrence() == nullptr) {
        return false;
    }

    qint64 duration = 0;
    if (scheduleStart.isValid() && scheduleEnd.isValid()) {
        duration = qMax<qint64>(0, scheduleStart.secsTo(scheduleEnd));
    }
    const QList<QDateTime> occurrences = schedule->recurrence()->timesInInterval(
        rangeStart.addSecs(-duration), rangeEnd);
    for (const QDateTime &occurrence : occurrences) {
        if (occurrence.isValid() && occurrence <= rangeEnd
            && occurrence.addSecs(duration) >= rangeStart) {
            return true;
        }
    }
    return false;
}

} // namespace

DCalDavIncrementalSync::DCalDavIncrementalSync(QObject *parent)
    : QObject(parent)
    , m_transport()
{
}

void DCalDavIncrementalSync::cancel()
{
    if (!m_running) {
        return;
    }
    m_transport.cancel();
    m_request.password.clear();
    m_callback = Callback();
    m_running = false;
}

void DCalDavIncrementalSync::start(const Request &request, const Callback &callback)
{
    if (m_running) {
        return;
    }

    m_request = request;
    m_callback = callback;
    m_result = Result();
    m_pendingResources.clear();
    m_mappingByHref.clear();
    m_remoteHrefs.clear();
    m_resourceIndex = 0;
    m_fallbackAttempted = false;
    m_resourceListHasEventFilter = false;
    m_syncCollectionMode = false;
    m_running = true;

    if (!DCalDavTransport::isSecureUrl(request.calendarUrl) || request.username.isEmpty()) {
        finish(false, QStringLiteral("Invalid CalDAV calendar URL or username."));
        return;
    }

    for (const DCalDavEventMappingInfo &mapping : request.existingMappings) {
        if (!mapping.href.isEmpty()) {
            m_mappingByHref.insert(mapping.href, mapping);
        }
    }

    // The first sync must use the product-defined range. Some providers return
    // empty calendar-data for that query, so matching resources are fetched by GET.
    const bool firstSync = !request.initialSyncCompleted;
    const bool providerHasNoSyncToken = request.syncToken == kNoSyncTokenMarker;
    if (firstSync) {
        sendResourceListRequest(true);
    } else if (request.syncToken.isEmpty() || providerHasNoSyncToken) {
        // Some providers, including WeCom, return only resource metadata for an
        // unbounded REPORT and reject the subsequent event GET. Reuse the
        // product-defined range query so calendar-data is returned inline.
        sendResourceListRequest(true);
    } else {
        sendRequest(false);
    }
}

void DCalDavIncrementalSync::sendResourceListRequest(bool firstSync)
{
    m_resourceListHasEventFilter = firstSync;
    const QDateTime referenceTime = m_request.referenceTime.isValid()
        ? m_request.referenceTime
        : QDateTime::currentDateTimeUtc();
    const DCalDavTransport::Request request = firstSync
        ? DCalDavCalendarQuery::firstSyncRequest(
              m_request.calendarUrl, m_request.username, m_request.password, referenceTime)
        : DCalDavCalendarQuery::resourceListRequest(
              m_request.calendarUrl, m_request.username, m_request.password);
    m_transport.send(request, [this](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            qCWarning(ServiceLogger) << "CalDAV resource list request failed"
                                     << "endpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                                     << "httpStatus:" << response.httpStatus
                                     << "transportError:" << static_cast<int>(response.error);
            m_result.failureResponse = response;
            finish(false, DCalDavUtils::transportErrorText(response));
            return;
        }

        DCalDavCalendarQuery::ResourceList resources;
        QString errorMessage;
        if (!DCalDavCalendarQuery::parseResourceList(response.body, resources, &errorMessage)) {
            finish(false, errorMessage);
            return;
        }

        const QUrl baseUrl = response.finalUrl.isValid() ? response.finalUrl : m_request.calendarUrl;
        int collectionResourceCount = 0;
        int metadataOnlyResourceCount = 0;
        int pendingResourceCount = 0;
        for (DCalDavCalendarQuery::Resource resource : resources) {
            resource.href = DCalDavDiscovery::resolveHref(baseUrl, resource.href).toString();
            if (resource.href.isEmpty()) {
                continue;
            }
            const QUrl resourceUrl(resource.href);
            if (resourceUrl == m_request.calendarUrl || resourceUrl.path().endsWith(QLatin1Char('/'))) {
                ++collectionResourceCount;
                continue;
            }
            if (resource.calendarData.isEmpty()) {
                ++metadataOnlyResourceCount;
            }
            m_remoteHrefs.insert(resource.href);
            if (!m_resourceListHasEventFilter && !isEventResource(resource)) {
                continue;
            }
            const auto mapping = m_mappingByHref.constFind(resource.href);
            if (mapping == m_mappingByHref.constEnd() || mapping->etag != resource.etag) {
                m_pendingResources.append(resource);
                ++pendingResourceCount;
            }
        }

        qCDebug(ServiceLogger) << "CalDAV resource list processed"
                               << "calendarEndpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                               << "responseResourceCount:" << resources.size()
                               << "collectionResourceCount:" << collectionResourceCount
                               << "metadataOnlyResourceCount:" << metadataOnlyResourceCount
                               << "pendingResourceCount:" << pendingResourceCount;
        fetchNextResource();
    });
}

bool DCalDavIncrementalSync::appendResourceCalendarData(
    const DCalDavCalendarQuery::Resource &resource, const QString &calendarData,
    QString *errorMessage)
{
    DCalDavCalendarQuery::RemoteEvent event;
    event.href = resource.href;
    event.etag = resource.etag;
    event.contentType = resource.contentType;
    event.calendarData = calendarData;
    event.uid = uidFromCalendarData(event.calendarData);
    if (event.calendarData.isEmpty() || !hasEventComponent(event.calendarData)
        || event.uid.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Remote CalDAV resource is missing a valid VEVENT UID.");
        }
        return false;
    }

    DSchedule::Ptr schedule;
    if (!DCalDavEventMapper::toSchedule(event, schedule, errorMessage)) {
        return false;
    }
    if (m_resourceListHasEventFilter) {
        const QDateTime referenceTime = m_request.referenceTime.isValid()
            ? m_request.referenceTime
            : QDateTime::currentDateTimeUtc();
        if (!overlapsInitialSyncRange(schedule, referenceTime)) {
            return true;
        }
    }
    m_result.remoteEvents.append(event);
    m_result.schedules.append(schedule);
    return true;
}

void DCalDavIncrementalSync::fetchNextResource()
{
    if (m_resourceIndex >= m_pendingResources.size()) {
        if (!m_resourceListHasEventFilter && !m_syncCollectionMode) {
            appendDeletedResources();
        }
        if (m_result.syncToken.isEmpty()) {
            m_result.syncToken = kNoSyncTokenMarker;
        }
        finish(true);
        return;
    }

    constexpr int kMultiGetBatchSize = 50;
    DCalDavCalendarQuery::ResourceList resourcesForMultiGet;
    while (m_resourceIndex < m_pendingResources.size()
           && resourcesForMultiGet.size() < kMultiGetBatchSize) {
        const DCalDavCalendarQuery::Resource resource = m_pendingResources.at(m_resourceIndex++);
        if (resource.calendarData.isEmpty()) {
            resourcesForMultiGet.append(resource);
            continue;
        }

        QString errorMessage;
        if (!appendResourceCalendarData(resource, resource.calendarData, &errorMessage)) {
            finish(false, errorMessage);
            return;
        }
    }

    if (resourcesForMultiGet.isEmpty()) {
        fetchNextResource();
        return;
    }
    requestCalendarDataBatch(resourcesForMultiGet);
}

void DCalDavIncrementalSync::requestCalendarDataBatch(
    const DCalDavCalendarQuery::ResourceList &resources)
{
    QStringList resourceHrefs;
    resourceHrefs.reserve(resources.size());
    for (const DCalDavCalendarQuery::Resource &resource : resources) {
        resourceHrefs.append(resource.href);
    }

    qCDebug(ServiceLogger) << "CalDAV fetching event data with calendar-multiget"
                             << "calendarEndpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                             << "resourceCount:" << resources.size();
    const DCalDavTransport::Request request = DCalDavCalendarQuery::resourceMultiGetRequest(
        m_request.calendarUrl, m_request.username, m_request.password, resourceHrefs);
    m_transport.send(request, [this, resources](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            qCWarning(ServiceLogger) << "CalDAV calendar-multiget request failed"
                                     << "calendarEndpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                                     << "resourceCount:" << resources.size()
                                     << "httpStatus:" << response.httpStatus
                                     << "transportError:" << static_cast<int>(response.error);
            if (response.httpStatus == 403 || response.httpStatus == 405 || response.httpStatus == 501) {
                qCWarning(ServiceLogger) << "CalDAV calendar-multiget is unavailable; using GET fallback"
                                         << "calendarEndpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                                         << "resourceCount:" << resources.size()
                                         << "httpStatus:" << response.httpStatus;
                m_result.failureResponse = DCalDavTransport::Response();
                fetchResourceByGet(resources, 0);
                return;
            }
            m_result.failureResponse = response;
            finish(false, DCalDavUtils::transportErrorText(response));
            return;
        }

        DCalDavCalendarQuery::RemoteEventList events;
        QString errorMessage;
        if (!DCalDavCalendarQuery::parseResponse(response.body, events, &errorMessage)) {
            finish(false, errorMessage);
            return;
        }

        const QUrl baseUrl = response.finalUrl.isValid() ? response.finalUrl : m_request.calendarUrl;
        QHash<QString, DCalDavCalendarQuery::RemoteEvent> eventByHref;
        for (DCalDavCalendarQuery::RemoteEvent event : events) {
            event.href = DCalDavDiscovery::resolveHref(baseUrl, event.href).toString();
            if (!event.href.isEmpty()) {
                eventByHref.insert(event.href, event);
            }
        }

        DCalDavCalendarQuery::ResourceList fallbackResources;
        int fetchedCount = 0;
        for (DCalDavCalendarQuery::Resource resource : resources) {
            const auto eventIt = eventByHref.constFind(resource.href);
            if (eventIt == eventByHref.constEnd() || eventIt->deleted
                || eventIt->calendarData.isEmpty()) {
                fallbackResources.append(resource);
                continue;
            }

            const DCalDavCalendarQuery::RemoteEvent &event = eventIt.value();
            if (!event.etag.isEmpty()) {
                resource.etag = event.etag;
            }
            if (!event.contentType.isEmpty()) {
                resource.contentType = event.contentType;
            }
            if (!appendResourceCalendarData(resource, event.calendarData, &errorMessage)) {
                finish(false, errorMessage);
                return;
            }
            ++fetchedCount;
        }

        qCDebug(ServiceLogger) << "CalDAV calendar-multiget response processed"
                               << "calendarEndpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                               << "requestedResourceCount:" << resources.size()
                               << "calendarDataCount:" << fetchedCount
                               << "getFallbackCount:" << fallbackResources.size();
        if (!fallbackResources.isEmpty()) {
            fetchResourceByGet(fallbackResources, 0);
            return;
        }
        fetchNextResource();
    });
}

void DCalDavIncrementalSync::fetchResourceByGet(
    const DCalDavCalendarQuery::ResourceList &resources, int index)
{
    if (index >= resources.size()) {
        fetchNextResource();
        return;
    }

    const DCalDavCalendarQuery::Resource resource = resources.at(index);
    qCDebug(ServiceLogger) << "CalDAV event requires GET fallback after calendar-multiget"
                             << "calendarEndpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                             << "resourcePresent:" << !resource.href.isEmpty()
                             << "contentType:" << resource.contentType
                             << "etagPresent:" << !resource.etag.isEmpty();
    const DCalDavTransport::Request request = DCalDavCalendarQuery::resourceGetRequest(
        QUrl(resource.href), m_request.username, m_request.password);
    m_transport.send(request, [this, resources, index, resource](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            qCWarning(ServiceLogger) << "CalDAV event GET fallback failed"
                                     << "calendarEndpoint:" << DCalDavTransport::urlForLog(m_request.calendarUrl)
                                     << "resourcePresent:" << !resource.href.isEmpty()
                                     << "httpStatus:" << response.httpStatus
                                     << "transportError:" << static_cast<int>(response.error);
            m_result.failureResponse = response;
            finish(false, DCalDavUtils::transportErrorText(response));
            return;
        }

        QString errorMessage;
        if (!appendResourceCalendarData(resource, QString::fromUtf8(response.body), &errorMessage)) {
            finish(false, errorMessage);
            return;
        }
        fetchResourceByGet(resources, index + 1);
    });
}

void DCalDavIncrementalSync::appendDeletedResources()
{
    for (auto it = m_mappingByHref.constBegin(); it != m_mappingByHref.constEnd(); ++it) {
        if (m_remoteHrefs.contains(it.key())) {
            continue;
        }
        DCalDavCalendarQuery::RemoteEvent event;
        event.href = it.key();
        event.uid = it->uid;
        event.deleted = true;
        m_result.remoteEvents.append(event);
    }
}

void DCalDavIncrementalSync::sendRequest(bool fullRange)
{
    DCalDavTransport::Request request;
    if (fullRange) {
        const QDateTime referenceTime = m_request.referenceTime.isValid()
            ? m_request.referenceTime
            : QDateTime::currentDateTimeUtc();
        request = DCalDavCalendarQuery::firstSyncRequest(
            m_request.calendarUrl, m_request.username, m_request.password, referenceTime);
    } else {
        request = DCalDavCalendarQuery::incrementalSyncRequest(
            m_request.calendarUrl, m_request.username, m_request.password, m_request.syncToken);
    }

    m_transport.send(request, [this, fullRange](const DCalDavTransport::Response &response) {
        if (response.error != DCalDavTransport::NoError) {
            if (!fullRange && shouldFallbackToFullRange(response)) {
                m_fallbackAttempted = true;
                m_result.usedFullRangeFallback = true;
                sendResourceListRequest(false);
            } else {
                m_result.failureResponse = response;
                finish(false, DCalDavUtils::transportErrorText(response));
            }
            return;
        }

        if (!fullRange && response.body.contains("valid-sync-token")) {
            m_fallbackAttempted = true;
            m_result.usedFullRangeFallback = true;
            sendResourceListRequest(false);
            return;
        }

        DCalDavCalendarQuery::RemoteEventList events;
        QString syncToken;
        QString errorMessage;
        if (!DCalDavCalendarQuery::parseResponseWithSyncToken(
                response.body, events, &syncToken, &errorMessage)) {
            finish(false, errorMessage);
            return;
        }

        const QUrl baseUrl = response.finalUrl.isValid() ? response.finalUrl : m_request.calendarUrl;
        m_syncCollectionMode = true;
        m_result.syncToken = syncToken;
        for (DCalDavCalendarQuery::RemoteEvent event : events) {
            event.href = DCalDavDiscovery::resolveHref(baseUrl, event.href).toString();
            if (event.href.isEmpty()) {
                continue;
            }
            if (event.deleted) {
                m_result.remoteEvents.append(event);
                continue;
            }
            if (event.calendarData.isEmpty()) {
                DCalDavCalendarQuery::Resource resource;
                resource.href = event.href;
                resource.etag = event.etag;
                resource.contentType = event.contentType;
                m_pendingResources.append(resource);
                continue;
            }
            if (!hasEventComponent(event.calendarData)) {
                continue;
            }
            event.uid = uidFromCalendarData(event.calendarData);
            DSchedule::Ptr schedule;
            if (!DCalDavEventMapper::toSchedule(event, schedule, &errorMessage)) {
                finish(false, errorMessage);
                return;
            }
            m_result.remoteEvents.append(event);
            m_result.schedules.append(schedule);
        }
        fetchNextResource();
    });
}

void DCalDavIncrementalSync::finish(bool success, const QString &errorMessage)
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

bool DCalDavIncrementalSync::shouldFallbackToFullRange(const DCalDavTransport::Response &response) const
{
    if (m_fallbackAttempted) {
        return false;
    }
    if (response.httpStatus == 409) {
        return true;
    }
    return response.httpStatus == 403 && response.body.contains("valid-sync-token");
}
