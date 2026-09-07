// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavaccountsync.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavaccountstatus.h"
#include "dcaldaveventreconciler.h"
#include "dcaldavretrypolicy.h"
#include "dcaldavsyncstatusmapper.h"
#include "commondef.h"

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
    m_outboxProcessor.cancel();
    m_calendarSync.cancel();
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
    }

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
        m_result.failureResponse = result.failureResponse;
        m_result.failureCode = DCalDavSyncStatusMapper::errorCodeForFailure(m_result.failureResponse);
        m_result.createFailure = result.createFailure;
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

    const CalendarRequest &calendar = m_request.calendars.at(m_calendarIndex);
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
    const DCalDavEventMappingInfo::List existingMappings =
        m_request.accountManagerDatabase->getCalDavEventMappingList(
            m_request.accountID, calendar.calendar.calendarId);
    syncRequest.existingMappings = existingMappings;
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
            m_result.failureCode = DCalDavSyncStatusMapper::errorCodeForFailure(m_result.failureResponse);
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

        DCalDavEventReconciler::ActionList actions;
        QString errorMessage;
        if (!DCalDavEventReconciler::buildActions(
                syncResult.remoteEvents, existingMappings, actions, &errorMessage)) {
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
