// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavaccountregistrar.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavcredentialstore.h"
#include "dcaldavcolorallocator.h"
#include "dcaldavsyncjobmanager.h"
#include "dcaldavxmlreader.h"
#include "ddatabase.h"
#include "dscheduletype.h"

#include <QCoreApplication>
#include <QSet>

#include <QUrl>

namespace {

QString findCalendarID(const DCalDavCalendarInfo::List &calendars, const QString &href,
                       QString *syncToken, bool *initialSyncCompleted)
{
    for (const DCalDavCalendarInfo &calendar : calendars) {
        if (calendar.href == href) {
            if (syncToken != nullptr) {
                *syncToken = calendar.syncToken;
            }
            if (initialSyncCompleted != nullptr) {
                *initialSyncCompleted = calendar.initialSyncCompleted;
            }
            return calendar.calendarId;
        }
    }
    return QString();
}

QString createCalendarScheduleType(DAccountDataBase *database, const QString &accountID,
                                    const QString &calendarID, const QString &displayName,
                                    const QString &calendarColor, bool writable,
                                    QString *errorMessage)
{
    Q_UNUSED(calendarColor);
    if (database == nullptr || accountID.isEmpty() || calendarID.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("CalDAV calendar type request is incomplete.");
        }
        return QString();
    }

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(DCalDavColorAllocator::nextColor(database));
    color.setPrivilege(DTypeColor::PriSystem);

    DScheduleType::Ptr type(new DScheduleType(accountID));
    const QString name = displayName.isEmpty()
        ? QCoreApplication::translate("DCalDavAccountRegistrar", "Calendar")
        : displayName;
    type->setTypeName(name);
    type->setDisplayName(name);
    type->setTypePath(calendarID);
    type->setTypeColor(color);
    type->setDescription(QStringLiteral("CalDAV calendar"));
    type->setPrivilege(writable ? DScheduleType::Privileges(
                                      DScheduleType::Read | DScheduleType::Write | DScheduleType::Delete)
                                : DScheduleType::Read);
    type->setShowState(DScheduleType::Show);
    type->setDtCreate(QDateTime::currentDateTime());
    type->setDeleted(0);

    if (!database->addTypeColor(color) || database->createScheduleType(type).isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to create CalDAV calendar type.");
        }
        return QString();
    }
    return type->typeID();
}

} // namespace

DCalDavAccountRegistrar::DCalDavAccountRegistrar(QObject *parent)
    : QObject(parent)
    , m_discovery(this)
{
}

void DCalDavAccountRegistrar::start(const Request &request, const Callback &callback)
{
    if (m_running) {
        return;
    }

    m_request = request;
    m_callback = callback;
    m_password.clear();
    m_running = true;

    if (request.account.accountId.isEmpty() || request.account.serverUrl.isEmpty()
        || request.account.username.isEmpty() || request.account.credentialRef.isEmpty()
        || request.localDatabase == nullptr
        || request.accountManagerDatabase == nullptr || request.jobManager == nullptr) {
        finish(false, QStringLiteral("CalDAV account registration request is incomplete."));
        return;
    }

    QString errorMessage;
    if (!DCalDavCredentialStore::readPassword(request.account.credentialRef, m_password, &errorMessage)) {
        finish(false, errorMessage, DCalDavTransport::Response(),
               DCalDavErrorCode::StorageError);
        return;
    }

    DCalDavReadOnlySync::Request discoveryRequest;
    discoveryRequest.serverUrl = QUrl(request.account.serverUrl);
    discoveryRequest.username = request.account.username;
    discoveryRequest.password = m_password;
    m_discovery.start(discoveryRequest, [this](const DCalDavReadOnlySync::Result &result) {
        if (!result.success) {
            finish(false, result.errorMessage, result.failureResponse, result.failureCode);
            return;
        }

        DCalDavAccountSync::Request syncRequest;
        syncRequest.accountID = m_request.account.accountId;
        syncRequest.username = m_request.account.username;
        syncRequest.password = m_password;
        syncRequest.localDatabase = m_request.localDatabase;
        syncRequest.accountManagerDatabase = m_request.accountManagerDatabase;
        syncRequest.retryCount = m_request.account.retryCount;
        QString errorMessage;
        if (!persistCalendars(result.discovery, syncRequest, &errorMessage)) {
            finish(false, errorMessage, DCalDavTransport::Response(),
                   DCalDavErrorCode::StorageError);
            return;
        }
        if (!m_request.jobManager->registerAccount(syncRequest)) {
            finish(false, QStringLiteral("CalDAV account is already syncing."),
                   DCalDavTransport::Response(), DCalDavErrorCode::Unknown);
            return;
        }
        if (m_request.triggerInitialSync) {
            m_request.jobManager->requestSync(
                m_request.account.accountId, DCalDavSyncStateMachine::StartupTrigger);
        }
        finish(true);
    });
}

QString DCalDavAccountRegistrar::findScheduleTypeID(
    const QString &calendarID, const DCalDavXmlReader::CalendarCollection &,
    const DCalDavCalendarInfo::List &existing, QString *)
{
    for (const DCalDavCalendarInfo &calendar : existing) {
        if (calendar.calendarId == calendarID) {
            return calendar.scheduleTypeID;
        }
    }
    // CalDAV event types are created from CATEGORIES after ICS parsing.
    return QString();
}

bool DCalDavAccountRegistrar::persistCalendars(const DCalDavXmlReader::DiscoveryResult &discovery,
                                               DCalDavAccountSync::Request &syncRequest,
                                               QString *errorMessage)
{
    DCalDavCalendarInfo::List existing = m_request.accountManagerDatabase->getCalDavCalendarList(
        m_request.account.accountId);
    QSet<QString> discoveredHrefs;

    for (const DCalDavXmlReader::CalendarCollection &collection : discovery.calendarCollections) {
        const QUrl href(collection.href);
        if (!href.isValid() || href.scheme() != QStringLiteral("https")) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("CalDAV discovery returned an insecure calendar URL.");
            }
            return false;
        }
        if (collection.privilegesKnown
            && !(collection.privileges & DCalDavXmlReader::ReadPrivilege)) {
            for (DCalDavCalendarInfo &calendar : existing) {
                if (calendar.href == collection.href) {
                    calendar.enabled = false;
                    if (!m_request.accountManagerDatabase->upsertCalDavCalendar(calendar)) {
                        if (errorMessage != nullptr) {
                            *errorMessage = QStringLiteral("Failed to disable inaccessible CalDAV calendar.");
                        }
                        return false;
                    }
                    break;
                }
            }
            continue;
        }

        QString syncToken;
        bool initialSyncCompleted = false;
        QString calendarID = findCalendarID(existing, collection.href, &syncToken,
                                             &initialSyncCompleted);
        if (calendarID.isEmpty()) {
            calendarID = DDataBase::createUuid();
        }

        QString scheduleTypeID = findScheduleTypeID(calendarID, collection, existing, errorMessage);
        const bool writable = (collection.privileges & DCalDavXmlReader::WritePrivilege) != 0;
        if (scheduleTypeID.isEmpty()) {
            scheduleTypeID = createCalendarScheduleType(
                m_request.localDatabase, m_request.account.accountId, calendarID,
                collection.displayName, collection.color, writable, errorMessage);
            if (scheduleTypeID.isEmpty()) {
                return false;
            }
        } else {
            const DScheduleType::Ptr type = m_request.localDatabase->getScheduleTypeByID(scheduleTypeID);
            if (type.isNull()) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("Failed to load CalDAV calendar type.");
                }
                return false;
            }
            type->setPrivilege(writable ? DScheduleType::Privileges(
                                             DScheduleType::Read | DScheduleType::Write | DScheduleType::Delete)
                                       : DScheduleType::Read);
            if (!m_request.localDatabase->updateScheduleType(type)) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("Failed to update CalDAV calendar permissions.");
                }
                return false;
            }
        }

        DCalDavCalendarInfo calendar;
        calendar.calendarId = calendarID;
        calendar.accountId = m_request.account.accountId;
        calendar.href = collection.href;
        calendar.displayName = collection.displayName;
        calendar.color = collection.color;
        calendar.scheduleTypeID = scheduleTypeID;
        calendar.privileges = collection.privileges;
        calendar.syncToken = syncToken;
        calendar.initialSyncCompleted = initialSyncCompleted;
        calendar.enabled = true;
        if (!m_request.accountManagerDatabase->upsertCalDavCalendar(calendar)) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Failed to persist discovered CalDAV calendar.");
            }
            return false;
        }

        DCalDavAccountSync::CalendarRequest calendarRequest;
        calendarRequest.calendar = calendar;
        calendarRequest.scheduleTypeID = calendar.scheduleTypeID;
        syncRequest.calendars.append(calendarRequest);
        discoveredHrefs.insert(collection.href);
    }

    for (DCalDavCalendarInfo &calendar : existing) {
        if (calendar.enabled && !discoveredHrefs.contains(calendar.href)) {
            calendar.enabled = false;
            if (!m_request.accountManagerDatabase->upsertCalDavCalendar(calendar)) {
                if (errorMessage != nullptr) {
                    *errorMessage = QStringLiteral("Failed to disable removed CalDAV calendar.");
                }
                return false;
            }
        }
    }
    return true;
}

void DCalDavAccountRegistrar::finish(bool success, const QString &errorMessage,
                                       const DCalDavTransport::Response &failureResponse,
                                       DCalDavErrorCode failureCode)
{
    if (!m_running) {
        return;
    }

    Result result;
    result.success = success;
    result.errorMessage = errorMessage;
    result.failureResponse = failureResponse;
    result.failureCode = success ? DCalDavErrorCode::NoError : failureCode;
    if (success) {
        result.calendarCount = m_request.accountManagerDatabase->getCalDavCalendarList(
            m_request.account.accountId).size();
        m_request.accountManagerDatabase->clearCalDavRetryState(m_request.account.accountId);
    }
    m_password.clear();
    m_running = false;
    if (m_callback) {
        const Callback callback = m_callback;
        m_callback = Callback();
        callback(result);
    }
}
