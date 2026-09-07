// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCALDAVACCOUNTSYNC_H
#define DCALDAVACCOUNTSYNC_H

#include "dcaldavcalendarinfo.h"
#include "dcaldaverrorcode.h"
#include "dcaldavincrementalsync.h"
#include "dcaldaveventscheduleapplier.h"
#include "dcaldavoutboxprocessor.h"
#include "dcaldavvalidationerror.h"

#include <QObject>
#include <QVector>

#include <functional>

class DAccountDataBase;
class DAccountManagerDataBase;

class DCalDavAccountSync : public QObject
{
    Q_OBJECT
public:
    struct CalendarRequest {
        DCalDavCalendarInfo calendar;
        QString scheduleTypeID;
    };

    struct Request {
        QString accountID;
        QString username;
        QString password;
        QVector<CalendarRequest> calendars;
        DAccountDataBase *localDatabase = nullptr;
        DAccountManagerDataBase *accountManagerDatabase = nullptr;
        bool forceOutboxRetry = false;
        int retryCount = 0;
    };

    struct Result {
        bool success = false;
        QString errorMessage;
        DCalDavTransport::Response failureResponse;
        DCalDavErrorCode failureCode = DCalDavErrorCode::NoError;
        int createdCount = 0;
        int updatedCount = 0;
        int deletedCount = 0;
        int fullRangeFallbackCount = 0;
        DCalDavScheduleCreateError::Type createFailure = DCalDavScheduleCreateError::NoError;
    };

    typedef std::function<void(const Result &)> Callback;

    explicit DCalDavAccountSync(QObject *parent = nullptr);
    ~DCalDavAccountSync() override;

    /**
     * @brief Starts the account-wide Outbox flush and incremental calendar sync.
     * @param request Account credentials, target calendars, and local databases.
     * @param callback Invoked exactly once when synchronization completes or is cancelled.
     */
    void start(const Request &request, const Callback &callback);
    /**
     * @brief Cancels pending requests and optionally completes the callback.
     * @param notifyCallback Whether to report cancellation to the active caller.
     */
    void cancel(bool notifyCallback = true);

private:
    void flushOutbox();
    void syncNextCalendar();
    void finish(bool success, const QString &errorMessage = QString());
    void fail(const QString &errorMessage);

    DCalDavIncrementalSync m_calendarSync;
    DCalDavOutboxProcessor m_outboxProcessor;
    Request m_request;
    Callback m_callback;
    Result m_result;
    int m_calendarIndex = 0;
    int m_successfulCalendarCount = 0;
    int m_permissionDeniedCalendarCount = 0;
    QString m_lastPermissionDeniedError;
    bool m_running = false;
};

#endif // DCALDAVACCOUNTSYNC_H
