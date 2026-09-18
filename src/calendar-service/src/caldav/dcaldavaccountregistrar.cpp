// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavaccountregistrar.h"

#include "dcaldavcalendardeletehelper.h"

#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dcaldavcredentialstore.h"
#include "dcaldavcolorallocator.h"
#include "dcaldavsyncjobmanager.h"
#include "dcaldavrecoveryitem.h"
#include "dcaldavxmlreader.h"
#include "ddatabase.h"
#include "dscheduletype.h"
#include "commondef.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>

#include <QUrl>

#include <memory>

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
    discoveryRequest.requireReadableCalendar = false;
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

bool DCalDavAccountRegistrar::handleRemoteCalendarDeletion(
    const DCalDavCalendarInfo &calendar, QString *errorMessage)
{
    const DCalDavCategoryInfo::List categories =
        m_request.accountManagerDatabase->getCalDavCategoryMappings(
            m_request.account.accountId, calendar.calendarId);
    const QStringList typeIDs = DCalDavCalendarDeleteHelper::collectTypeIDs(
        calendar.scheduleTypeID, categories);
    const QStringList scheduleIDs = DCalDavCalendarDeleteHelper::collectScheduleIDs(
        m_request.localDatabase, typeIDs);

    return persistRemoteCalendarDeletionRecovery(calendar, typeIDs, errorMessage)
        && commitRemoteCalendarDeletion(calendar, typeIDs, scheduleIDs, errorMessage);
}

bool DCalDavAccountRegistrar::persistRemoteCalendarDeletionRecovery(
    const DCalDavCalendarInfo &calendar, const QStringList &typeIDs, QString *errorMessage)
{
    QJsonArray recoveryTypeIDs;
    for (const QString &typeID : typeIDs) {
        recoveryTypeIDs.append(typeID);
    }

    DCalDavRecoveryItem recoveryItem;
    recoveryItem.accountID = m_request.account.accountId;
    recoveryItem.localScheduleID = calendar.scheduleTypeID;
    recoveryItem.operationType = DCalDavRecoveryItem::RemoteDeleteCalendarOperation;
    recoveryItem.scheduleIcs = QStringLiteral("CALDAV-REMOTE-CALENDAR-DELETE");
    recoveryItem.calendarID = calendar.calendarId;
    recoveryItem.href = calendar.href;
    recoveryItem.originalIcs = QString::fromUtf8(
        QJsonDocument(recoveryTypeIDs).toJson(QJsonDocument::Compact));
    recoveryItem.createdAt = QDateTime::currentDateTimeUtc();
    if (m_request.localDatabase->upsertCalDavRecoveryItem(recoveryItem)) {
        return true;
    }

    if (errorMessage != nullptr) {
        *errorMessage = QStringLiteral(
            "Failed to persist removed CalDAV calendar recovery state.");
    }
    return false;
}

bool DCalDavAccountRegistrar::commitRemoteCalendarDeletion(
    const DCalDavCalendarInfo &calendar, const QStringList &typeIDs,
    const QStringList &scheduleIDs, QString *errorMessage)
{
    const auto clearRecovery = [this, &calendar]() {
        return m_request.localDatabase->deleteCalDavRecoveryItem(
            m_request.account.accountId, calendar.scheduleTypeID);
    };
    std::unique_ptr<SqlTransactionLocker> transaction(new SqlTransactionLocker(
        {m_request.localDatabase->getConnectionName(), DDataBase::NameAccountManager}));
    if (!transaction->isValid()) {
        clearRecovery();
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to start removed CalDAV calendar cleanup.");
        }
        return false;
    }

    if (!DCalDavCalendarDeleteHelper::deleteScheduleTypes(
            m_request.localDatabase, typeIDs, true, true)) {
        transaction->rollback();
        clearRecovery();
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to remove remotely deleted CalDAV calendar.");
        }
        return false;
    }

    const auto deleteManagerData = [this, &calendar, &scheduleIDs]() {
        return DCalDavCalendarDeleteHelper::deleteRemoteManagerData(
            m_request.accountManagerDatabase, m_request.account.accountId,
            calendar.calendarId, scheduleIDs);
    };
    if (!deleteManagerData()) {
        transaction->rollback();
        clearRecovery();
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to remove deleted CalDAV calendar metadata.");
        }
        return false;
    }

    if (!transaction->commit()) {
        bool repaired = false;
        if (transaction->hasPartialCommit()
            && transaction->committedConnectionNames().contains(
                m_request.localDatabase->getConnectionName())) {
            SqlTransactionLocker repairTransaction({DDataBase::NameAccountManager});
            repaired = repairTransaction.isValid()
                && deleteManagerData()
                && repairTransaction.commit();
        }
        if (!repaired) {
            if (!transaction->hasPartialCommit()) {
                clearRecovery();
            }
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral(
                    "Failed to commit removed CalDAV calendar cleanup.");
            }
            return false;
        }
    }

    if (!clearRecovery()) {
        qCWarning(ServiceLogger) << "Failed to clear removed CalDAV calendar recovery state.";
    }
    return true;
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
            // This href was discovered but is currently inaccessible. It is
            // not a remote deletion, so preserve its local metadata and only
            // disable it.
            discoveredHrefs.insert(collection.href);
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
        if (!scheduleTypeID.isEmpty()
            && m_request.accountManagerDatabase->hasPendingCalDavCalendarDelete(
                   m_request.account.accountId, scheduleTypeID)) {
            // Local deletion wins until the remote DELETE is acknowledged. Do
            // not re-enable or sync a collection that discovery still returns.
            continue;
        }
        // Some CalDAV servers omit current-user-privilege-set even though the
        // authenticated user can write. Try writes optimistically in that
        // case; an explicit privilege response without write remains read-only.
        const bool writable = !collection.privilegesKnown
            || (collection.privileges & DCalDavXmlReader::WritePrivilege);
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
        calendar.privileges = collection.privilegesKnown
            ? collection.privileges
            : (DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege);
        calendar.privilegesKnown = collection.privilegesKnown;
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

    for (const DCalDavCalendarInfo &calendar : existing) {
        if (discoveredHrefs.contains(calendar.href)) {
            continue;
        }
        if (m_request.accountManagerDatabase->hasPendingCalDavCalendarDelete(
                m_request.account.accountId, calendar.scheduleTypeID)) {
            continue;
        }

        if (!handleRemoteCalendarDeletion(calendar, errorMessage)) {
            return false;
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
