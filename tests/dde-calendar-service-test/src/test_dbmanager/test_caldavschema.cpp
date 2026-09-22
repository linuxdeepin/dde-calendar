// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavprofile.h"
#include "dcaldavvalidationerror.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavaccountsync.h"
#include "dcaldavaccountregistrar.h"
#include "dcaldavdiscovery.h"
#include "dcaldavcalendarquery.h"
#include "dcaldavcredentialstore.h"
#include "dcaldavincrementalsync.h"
#include "dcaldavretrypolicy.h"
#include "dcaldaveventreconciler.h"
#include "dcaldaveventscheduleapplier.h"
#include "dcaldaveventmapper.h"
#include "dcaldavreadonlysync.h"
#include "dcaldavoutboxenqueuer.h"
#include "dcaldavoutboxprocessor.h"
#include "dcaldavrecoveryhandler.h"
#include "dcaldavaccountdeletioncleanup.h"
#include "dcaldavsyncstatemachine.h"
#include "dcaldavsyncjobmanager.h"
#include "dcaldavsyncstatusmapper.h"
#include "dcaldavxmlreader.h"
#include "mock_caldav_server.h"
#include "ddatabase.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "units.h"

#include <QEventLoop>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QVariant>
#include <QTimeZone>
#include <QTimer>
#include <QSslConfiguration>
#include <QSslSocket>

#include <gtest/gtest.h>

namespace {

bool tableExists(QSqlDatabase &database, const QString &tableName)
{
    QSqlQuery query(database);
    query.prepare("SELECT name FROM sqlite_master WHERE type = 'table' AND name = ?");
    query.addBindValue(QVariant(tableName));
    return query.exec() && query.next();
}

void trustMockCertificate(const QByteArray &certificate)
{
    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    configuration.addCaCertificate(QSslCertificate(certificate, QSsl::Pem));
    QSslConfiguration::setDefaultConfiguration(configuration);
}

bool createMockTlsCredentials(QByteArray &certificate, QByteArray &privateKey)
{
    const QString opensslPath = QStandardPaths::findExecutable(QStringLiteral("openssl"));
    if (opensslPath.isEmpty()) {
        return false;
    }

    QTemporaryDir directory;
    if (!directory.isValid()) {
        return false;
    }

    const QString certificatePath = directory.filePath(QStringLiteral("certificate.pem"));
    const QString privateKeyPath = directory.filePath(QStringLiteral("private-key.pem"));
    QProcess process;
    process.start(opensslPath, {
        QStringLiteral("req"),
        QStringLiteral("-x509"),
        QStringLiteral("-newkey"),
        QStringLiteral("rsa:2048"),
        QStringLiteral("-nodes"),
        QStringLiteral("-keyout"),
        privateKeyPath,
        QStringLiteral("-out"),
        certificatePath,
        QStringLiteral("-days"),
        QStringLiteral("3650"),
        QStringLiteral("-subj"),
        QStringLiteral("/CN=localhost"),
        QStringLiteral("-addext"),
        QStringLiteral("subjectAltName=DNS:localhost"),
    });
    if (!process.waitForFinished(5000) || process.exitCode() != 0) {
        return false;
    }

    QFile certificateFile(certificatePath);
    QFile privateKeyFile(privateKeyPath);
    if (!certificateFile.open(QIODevice::ReadOnly)
        || !privateKeyFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    certificate = certificateFile.readAll();
    privateKey = privateKeyFile.readAll();
    return !certificate.isEmpty() && !privateKey.isEmpty();
}

QByteArray headerValue(const QMap<QByteArray, QByteArray> &headers, const QByteArray &name)
{
    for (auto header = headers.cbegin(); header != headers.cend(); ++header) {
        if (header.key().compare(name, Qt::CaseInsensitive) == 0) {
            return header.value();
        }
    }
    return QByteArray();
}

bool waitForMockRequestCount(const MockCalDavServer &server, int count, int timeoutMs = 5000)
{
    QElapsedTimer timer;
    timer.start();
    while (server.requests().size() < count && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    return server.requests().size() >= count;
}

void processEventsFor(int durationMs)
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < durationMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
}

bool runIncrementalSync(const DCalDavIncrementalSync::Request &request,
                        DCalDavIncrementalSync::Result *result)
{
    if (result == nullptr) {
        return false;
    }

    DCalDavIncrementalSync synchronizer;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavIncrementalSync::Result &syncResult) {
        *result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }
    return callbackCalled;
}

} // namespace

TEST(CalDavProviderProfile, ProvidesExpectedDefaults)
{
    const DCalDavProviderProfile wecom = DCalDavProviderProfile::forProvider(DCalDavProviderProfile::Provider_WeCom);
    EXPECT_EQ(QStringLiteral("https://caldav.wecom.work"), wecom.serverUrl);
    EXPECT_TRUE(wecom.supportsWrite);

    const DCalDavProviderProfile tencent = DCalDavProviderProfile::forProvider(DCalDavProviderProfile::Provider_TencentMeeting);
    EXPECT_TRUE(tencent.supportsWrite);

    const DCalDavProviderProfile other = DCalDavProviderProfile::forProvider(DCalDavProviderProfile::Provider_Other);
    EXPECT_TRUE(other.serverUrl.isEmpty());
    EXPECT_TRUE(other.serverUrlEditable);
}

TEST(CalDavSchema, CreatesTablesIdempotently)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString connectionName = QStringLiteral("caldav-schema-test");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        database.setDatabaseName(directory.filePath("account-manager.db"));
        ASSERT_TRUE(database.open());

        const QStringList statements = {
            DDataBase::sql_create_caldavAccount,
            DDataBase::sql_create_caldavCalendar,
            DDataBase::sql_create_caldavEventMapping,
            DDataBase::sql_create_caldavOutbox,
            DDataBase::sql_create_caldavRecovery,
            DDataBase::sql_create_caldavAccountDeletionCleanup,
            DDataBase::sql_create_caldavSkippedResource,
        };
        for (const QString &statement : statements) {
            QSqlQuery query(database);
            ASSERT_TRUE(query.exec(statement));
            ASSERT_TRUE(query.exec(statement));
        }

        EXPECT_TRUE(tableExists(database, QStringLiteral("caldavAccount")));
        EXPECT_TRUE(tableExists(database, QStringLiteral("caldavCalendar")));
        EXPECT_TRUE(tableExists(database, QStringLiteral("caldavEventMapping")));
        EXPECT_TRUE(tableExists(database, QStringLiteral("caldavOutbox")));
        EXPECT_TRUE(tableExists(database, QStringLiteral("caldavRecovery")));
        EXPECT_TRUE(tableExists(database, QStringLiteral("caldavAccountDeletionCleanup")));
        EXPECT_TRUE(tableExists(database, QStringLiteral("caldavSkippedResource")));

        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

TEST(CalDavSkippedResource, PersistsUpdatesAndCleansUpRecords)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    DCalDavSkippedResource resource;
    resource.calendarId = QStringLiteral("calendar-1");
    resource.href = QStringLiteral("https://example.test/calendar/event.ics");
    resource.etag = QStringLiteral("\"etag-1\"");
    resource.reason = QStringLiteral("missing-vevent-uid");
    ASSERT_TRUE(database.upsertCalDavSkippedResource(QStringLiteral("account-1"), resource));

    DCalDavSkippedResource::List resources;
    ASSERT_TRUE(database.getCalDavSkippedResources(
        QStringLiteral("account-1"), resource.calendarId, resources));
    ASSERT_EQ(1, resources.size());
    EXPECT_EQ(resource.href, resources.first().href);
    EXPECT_EQ(resource.etag, resources.first().etag);
    EXPECT_EQ(resource.reason, resources.first().reason);

    resource.etag = QStringLiteral("\"etag-2\"");
    resource.reason = QStringLiteral("event-mapping-failed");
    ASSERT_TRUE(database.upsertCalDavSkippedResource(QStringLiteral("account-1"), resource));
    ASSERT_TRUE(database.getCalDavSkippedResources(
        QStringLiteral("account-1"), resource.calendarId, resources));
    ASSERT_EQ(1, resources.size());
    EXPECT_EQ(resource.etag, resources.first().etag);
    EXPECT_EQ(resource.reason, resources.first().reason);

    DCalDavSkippedResource otherCalendarResource = resource;
    otherCalendarResource.calendarId = QStringLiteral("calendar-2");
    ASSERT_TRUE(database.upsertCalDavSkippedResource(
        QStringLiteral("account-1"), otherCalendarResource));
    ASSERT_TRUE(database.deleteCalDavSkippedResource(
        QStringLiteral("account-1"), resource.calendarId, resource.href));
    resources.clear();
    ASSERT_TRUE(database.getCalDavSkippedResources(
        QStringLiteral("account-1"), resource.calendarId, resources));
    EXPECT_TRUE(resources.isEmpty());
    ASSERT_TRUE(database.deleteCalDavSkippedResourcesByCalendar(
        QStringLiteral("account-1"), otherCalendarResource.calendarId));
    resources.clear();
    ASSERT_TRUE(database.getCalDavSkippedResources(
        QStringLiteral("account-1"), otherCalendarResource.calendarId, resources));
    EXPECT_TRUE(resources.isEmpty());
}

TEST(CalDavRecovery, PersistsRecoveryItemsAndTracksSoftDeletedSchedules)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("recovery-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase database(account);
    database.setDBPath(directory.filePath(QStringLiteral("account.db")));
    database.initDBData();

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("Recovery event"));
    schedule->setDtStart(QDateTime(QDate(2026, 9, 3), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 9, 3), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(database.createSchedule(schedule).isEmpty());

    DCalDavRecoveryItem item;
    item.accountID = account->accountID();
    item.localScheduleID = schedule->uid();
    item.operationType = DCalDavRecoveryItem::DeleteOperation;
    item.scheduleIcs = DSchedule::toIcsString(schedule);
    item.calendarID = QStringLiteral("calendar-id");
    item.href = QStringLiteral("https://example.test/calendar/%1.ics").arg(schedule->uid());
    item.etag = QStringLiteral("\"etag\"");
    item.originalIcs = item.scheduleIcs;
    item.createdAt = QDateTime::currentDateTimeUtc();

    ASSERT_TRUE(database.upsertCalDavRecoveryItem(item));
    const DCalDavRecoveryItem::List items = database.getCalDavRecoveryItems(account->accountID());
    ASSERT_EQ(1, items.size());
    EXPECT_EQ(item.localScheduleID, items.first().localScheduleID);
    EXPECT_EQ(item.operationType, items.first().operationType);
    EXPECT_EQ(item.href, items.first().href);
    EXPECT_EQ(item.originalIcs, items.first().originalIcs);

    EXPECT_TRUE(database.scheduleExistsByScheduleID(schedule->uid()));
    EXPECT_FALSE(database.isScheduleDeletedByScheduleID(schedule->uid()));
    ASSERT_TRUE(database.deleteScheduleByScheduleID(schedule->uid()));
    EXPECT_TRUE(database.isScheduleDeletedByScheduleID(schedule->uid()));

    EXPECT_TRUE(database.deleteCalDavRecoveryItem(item.accountID, item.localScheduleID));
    EXPECT_TRUE(database.getCalDavRecoveryItems(account->accountID()).isEmpty());
}

TEST(CalDavRecovery, CompletesLocalOnlyDeleteAfterPartialCommit)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("local-only-delete-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setUid(QStringLiteral("local-only-schedule"));
    schedule->setSummary(QStringLiteral("Local only"));
    schedule->setDtStart(QDateTime(QDate(2026, 9, 16), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 9, 16), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());

    DCalDavRecoveryItem recovery;
    recovery.accountID = account->accountID();
    recovery.localScheduleID = schedule->uid();
    recovery.operationType = DCalDavRecoveryItem::DeleteOperation;
    recovery.scheduleIcs = DSchedule::toIcsString(schedule);
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(recovery));

    DCalDavOutboxItem staleCreate;
    staleCreate.operationID = QStringLiteral("stale-local-create");
    staleCreate.accountID = account->accountID();
    staleCreate.localScheduleID = schedule->uid();
    staleCreate.operationType = DCalDavOutboxItem::CreateOperation;
    ASSERT_TRUE(managerDatabase.upsertCalDavOutboxItem(staleCreate));
    ASSERT_TRUE(localDatabase.deleteScheduleByScheduleID(schedule->uid(), 1));

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, account->accountID());

    EXPECT_FALSE(localDatabase.scheduleExistsByScheduleID(schedule->uid()));
    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
    EXPECT_TRUE(managerDatabase.getCalDavOutboxItem(
                    account->accountID(), schedule->uid()).operationID.isEmpty());
}

TEST(CalDavRecovery, CompletesCalendarDeleteAfterPartialCommit)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("calendar-delete-recovery-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = QStringLiteral("https://example.test");
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/recovery");
    ASSERT_TRUE(managerDatabase.upsertCalDavAccountInfo(accountInfo));

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Recover calendar"));
    type->setDisplayName(QStringLiteral("Recover calendar"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setScheduleTypeID(type->typeID());
    schedule->setSummary(QStringLiteral("Recover calendar event"));
    schedule->setDtStart(QDateTime(QDate(2026, 9, 16), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 9, 16), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("recover-calendar");
    calendar.accountId = account->accountID();
    calendar.href = QStringLiteral("https://example.test/calendars/recover/");
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
    ASSERT_TRUE(managerDatabase.upsertCalDavCalendar(calendar));

    ASSERT_TRUE(localDatabase.deleteSchedulesByScheduleTypeID(type->typeID(), 0));
    ASSERT_TRUE(localDatabase.deleteScheduleTypeByID(type->typeID(), 0));

    QJsonArray recoveryTypeIDs;
    recoveryTypeIDs.append(type->typeID());
    DCalDavRecoveryItem recovery;
    recovery.accountID = account->accountID();
    recovery.localScheduleID = type->typeID();
    recovery.operationType = DCalDavRecoveryItem::DeleteCalendarOperation;
    recovery.scheduleIcs = QStringLiteral("CALDAV-CALENDAR-DELETE");
    recovery.calendarID = calendar.calendarId;
    recovery.href = calendar.href;
    recovery.originalIcs = QString::fromUtf8(
        QJsonDocument(recoveryTypeIDs).toJson(QJsonDocument::Compact));
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(recovery));

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, account->accountID());

    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type->typeID()).isNull());
    EXPECT_FALSE(localDatabase.getScheduleTypeByID(type->typeID(), 1).isNull());
    EXPECT_TRUE(localDatabase.isScheduleDeletedByScheduleID(schedule->uid()));
    const DCalDavCalendarInfo recoveredCalendar =
        managerDatabase.getCalDavCalendarByIDIncludingDisabled(
            account->accountID(), calendar.calendarId);
    EXPECT_FALSE(recoveredCalendar.calendarId.isEmpty());
    EXPECT_FALSE(recoveredCalendar.enabled);
    EXPECT_TRUE(managerDatabase.hasPendingCalDavCalendarDelete(
        account->accountID(), type->typeID()));
}

TEST(CalDavRecovery, ClearsCalendarDeleteRecoveryAfterFullRollback)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("calendar-delete-rollback-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Rollback calendar"));
    type->setDisplayName(QStringLiteral("Rollback calendar"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("rollback-calendar");
    calendar.accountId = account->accountID();
    calendar.href = QStringLiteral("https://example.test/calendars/rollback/");
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
    ASSERT_TRUE(managerDatabase.upsertCalDavCalendar(calendar));

    QJsonArray recoveryTypeIDs;
    recoveryTypeIDs.append(type->typeID());
    DCalDavRecoveryItem recovery;
    recovery.accountID = account->accountID();
    recovery.localScheduleID = type->typeID();
    recovery.operationType = DCalDavRecoveryItem::DeleteCalendarOperation;
    recovery.scheduleIcs = QStringLiteral("CALDAV-CALENDAR-DELETE");
    recovery.calendarID = calendar.calendarId;
    recovery.href = calendar.href;
    recovery.originalIcs = QString::fromUtf8(
        QJsonDocument(recoveryTypeIDs).toJson(QJsonDocument::Compact));
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(recovery));

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, account->accountID());

    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
    EXPECT_FALSE(localDatabase.getScheduleTypeByID(type->typeID()).isNull());
    EXPECT_TRUE(managerDatabase.getCalDavCalendarByIDIncludingDisabled(
                    account->accountID(), calendar.calendarId).enabled);
    EXPECT_FALSE(managerDatabase.hasPendingCalDavCalendarDelete(
        account->accountID(), type->typeID()));
}

TEST(CalDavRecovery, CompletesRemoteCalendarDeleteAfterPartialCommit)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("remote-calendar-delete-recovery-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Remote removed calendar"));
    type->setDisplayName(QStringLiteral("Remote removed calendar"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setScheduleTypeID(type->typeID());
    schedule->setSummary(QStringLiteral("Remote removed event"));
    schedule->setDtStart(QDateTime(QDate(2026, 9, 16), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 9, 16), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("remote-removed-calendar");
    calendar.accountId = account->accountID();
    calendar.href = QStringLiteral("https://example.test/calendars/remote-removed/");
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege;
    ASSERT_TRUE(managerDatabase.upsertCalDavCalendar(calendar));

    QJsonArray recoveryTypeIDs;
    recoveryTypeIDs.append(type->typeID());
    DCalDavRecoveryItem recovery;
    recovery.accountID = account->accountID();
    recovery.localScheduleID = type->typeID();
    recovery.operationType = DCalDavRecoveryItem::RemoteDeleteCalendarOperation;
    recovery.scheduleIcs = QStringLiteral("CALDAV-REMOTE-CALENDAR-DELETE");
    recovery.calendarID = calendar.calendarId;
    recovery.href = calendar.href;
    recovery.originalIcs = QString::fromUtf8(
        QJsonDocument(recoveryTypeIDs).toJson(QJsonDocument::Compact));
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(recovery));

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, account->accountID());

    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type->typeID()).isNull());
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type->typeID(), 1).isNull());
    EXPECT_FALSE(localDatabase.scheduleExistsByScheduleID(schedule->uid()));
    EXPECT_TRUE(managerDatabase.getCalDavCalendarByIDIncludingDisabled(
                    account->accountID(), calendar.calendarId).calendarId.isEmpty());
    EXPECT_FALSE(managerDatabase.hasPendingCalDavCalendarDelete(
        account->accountID(), type->typeID()));
}

TEST(CalDavRecovery, RecoversCreateModifyAndDeleteAfterPartialCommit)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("recovery-partial-account"));
    account->setAccountType(DAccount::Account_CalDav);
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = QStringLiteral("https://example.test");
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(managerDatabase.upsertCalDavAccountInfo(accountInfo));

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("recovery-calendar");
    calendar.accountId = account->accountID();
    calendar.href = QStringLiteral("https://example.test/calendars/user/");
    calendar.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
    ASSERT_TRUE(managerDatabase.upsertCalDavCalendar(calendar));

    const auto makeSchedule = [](const QString &summary) {
        DSchedule::Ptr schedule(new DSchedule);
        schedule->setSummary(summary);
        schedule->setDtStart(QDateTime(QDate(2026, 9, 3), QTime(10, 0), Qt::UTC));
        schedule->setDtEnd(QDateTime(QDate(2026, 9, 3), QTime(11, 0), Qt::UTC));
        return schedule;
    };
    const auto makeRecovery = [&](const DSchedule::Ptr &schedule,
                                  DCalDavRecoveryItem::OperationType operation) {
        DCalDavRecoveryItem item;
        item.accountID = account->accountID();
        item.localScheduleID = schedule->uid();
        item.operationType = operation;
        item.scheduleIcs = DSchedule::toIcsString(schedule);
        item.calendarID = calendar.calendarId;
        item.href = calendar.href + schedule->uid() + QStringLiteral(".ics");
        return item;
    };
    const auto addMapping = [&](const DSchedule::Ptr &schedule) {
        DCalDavEventMappingInfo mapping;
        mapping.accountID = account->accountID();
        mapping.localScheduleID = schedule->uid();
        mapping.calendarID = calendar.calendarId;
        mapping.uid = schedule->uid();
        mapping.href = calendar.href + schedule->uid() + QStringLiteral(".ics");
        return managerDatabase.upsertCalDavEventMapping(mapping);
    };

    // Remote creation was accepted, but the local schedule transaction never committed.
    DSchedule::Ptr created = makeSchedule(QStringLiteral("create"));
    created->setUid(QStringLiteral("create-id"));
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(
        makeRecovery(created, DCalDavRecoveryItem::CreateOperation)));

    // The ICS payload can be damaged after the remote resource was created;
    // the persisted href must still let startup enqueue its compensating DELETE.
    DSchedule::Ptr corrupted = makeSchedule(QStringLiteral("corrupted create"));
    corrupted->setUid(QStringLiteral("corrupted-create-id"));
    DCalDavRecoveryItem corruptedItem = makeRecovery(
        corrupted, DCalDavRecoveryItem::CreateOperation);
    corruptedItem.scheduleIcs = QStringLiteral("not an ics payload");
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(corruptedItem));

    // The local-first creation committed, but its Create outbox item did not.
    DSchedule::Ptr localCreated = makeSchedule(QStringLiteral("local create"));
    ASSERT_FALSE(localDatabase.createSchedule(localCreated).isEmpty());
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(
        makeRecovery(localCreated, DCalDavRecoveryItem::LocalCreateOperation)));

    // The local edit committed, but adding the Modify outbox item did not.
    DSchedule::Ptr modified = makeSchedule(QStringLiteral("modify"));
    ASSERT_FALSE(localDatabase.createSchedule(modified).isEmpty());
    ASSERT_TRUE(addMapping(modified));
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(
        makeRecovery(modified, DCalDavRecoveryItem::ModifyOperation)));

    // The local soft-delete committed, but its Delete outbox item did not.
    DSchedule::Ptr deleted = makeSchedule(QStringLiteral("delete"));
    ASSERT_FALSE(localDatabase.createSchedule(deleted).isEmpty());
    ASSERT_TRUE(addMapping(deleted));
    ASSERT_TRUE(localDatabase.deleteScheduleByScheduleID(deleted->uid()));
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(
        makeRecovery(deleted, DCalDavRecoveryItem::DeleteOperation)));

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, account->accountID());

    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
    EXPECT_EQ(DCalDavOutboxItem::DeleteOperation,
              managerDatabase.getCalDavOutboxItem(account->accountID(), created->uid()).operationType);
    EXPECT_EQ(DCalDavOutboxItem::DeleteOperation,
              managerDatabase.getCalDavOutboxItem(account->accountID(), corrupted->uid()).operationType);
    EXPECT_EQ(DCalDavOutboxItem::CreateOperation,
              managerDatabase.getCalDavOutboxItem(account->accountID(), localCreated->uid()).operationType);
    EXPECT_EQ(DCalDavOutboxItem::ModifyOperation,
              managerDatabase.getCalDavOutboxItem(account->accountID(), modified->uid()).operationType);
    EXPECT_EQ(DCalDavOutboxItem::DeleteOperation,
              managerDatabase.getCalDavOutboxItem(account->accountID(), deleted->uid()).operationType);
}

TEST(CalDavAccountTransaction, RollsBackAccountAndCalDavConfigurationTogether)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountName(QStringLiteral("user"));
    account->setDisplayName(QStringLiteral("Calendar"));
    account->setDbName(QStringLiteral("account.db"));
    account->setDbusPath(QStringLiteral("/test/account"));
    account->setDbusInterface(QStringLiteral("com.deepin.test.account"));
    account->setDtCreate(QDateTime::currentDateTime());
    SqlTransactionLocker transaction({DDataBase::NameAccountManager});
    ASSERT_TRUE(transaction.isValid());
    ASSERT_FALSE(database.addAccountInfo(account).isEmpty());
    DCalDavAccountInfo invalid;
    invalid.accountId = account->accountID();
    EXPECT_FALSE(database.upsertCalDavAccountInfo(invalid));
    transaction.rollback();
    EXPECT_FALSE(database.getAccountByID(account->accountID()));
}

TEST(CalDavAccountDeletion, RemovesPendingOutboxWhenAccountIsForcedDeleted)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountName(QStringLiteral("user"));
    account->setDisplayName(QStringLiteral("Calendar"));
    account->setDbName(QStringLiteral("account.db"));
    account->setDbusPath(QStringLiteral("/test/account"));
    account->setDbusInterface(QStringLiteral("com.deepin.test.account"));
    account->setDtCreate(QDateTime::currentDateTime());
    ASSERT_FALSE(database.addAccountInfo(account).isEmpty());
    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = QStringLiteral("https://example.test");
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(accountInfo));
    DCalDavOutboxItem item;
    item.operationID = QStringLiteral("pending-operation");
    item.accountID = account->accountID();
    item.localScheduleID = QStringLiteral("schedule-id");
    item.operationType = DCalDavOutboxItem::DeleteOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    ASSERT_TRUE(database.hasCalDavOutboxItems(account->accountID()));
    ASSERT_TRUE(database.deleteCalDavAccountData(account->accountID()));
    EXPECT_FALSE(database.hasCalDavOutboxItems(account->accountID()));
    EXPECT_FALSE(database.getAccountByID(account->accountID()));
}

TEST(CalDavAccountDeletionCleanup, RemovesOnlyDeletedAccountDatabase)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();
    const QString deletedId = QStringLiteral("deleted-account");
    const QString pendingName = QStringLiteral("pending.db");
    QFile pending(directory.filePath(pendingName));
    ASSERT_TRUE(pending.open(QIODevice::WriteOnly));
    pending.close();
    ASSERT_TRUE(database.upsertCalDavAccountDeletionCleanup(deletedId, pendingName));
    DCalDavAccountDeletionCleanup::resume(&database, directory.path());
    EXPECT_FALSE(QFile::exists(pending.fileName()));
    EXPECT_TRUE(database.getCalDavAccountDeletionCleanups().isEmpty());
}

TEST(CalDavSyncStatusMapper, PrefersStructuredTransportResponse)
{
    DCalDavTransport::Response response;
    response.httpStatus = 401;
    EXPECT_EQ(DCalDavSyncStatus::AuthenticationRequired,
              DCalDavSyncStatusMapper::statusForFailure(response));

    response = DCalDavTransport::Response();
    response.error = DCalDavTransport::PermissionDenied;
    EXPECT_EQ(DCalDavSyncStatus::PermissionDenied,
              DCalDavSyncStatusMapper::statusForFailure(response));

    response = DCalDavTransport::Response();
    response.httpStatus = 412;
    EXPECT_EQ(DCalDavSyncStatus::Conflict,
              DCalDavSyncStatusMapper::statusForFailure(response));
}

TEST(CalDavAccountStatus, ClassifiesStructuredErrorCodes)
{
    EXPECT_EQ(DCalDavSyncStatus::AuthenticationRequired,
              DCalDavSyncStatus::fromErrorCode(DCalDavErrorCode::AuthenticationFailed));
    EXPECT_EQ(DCalDavSyncStatus::PermissionDenied,
              DCalDavSyncStatus::fromErrorCode(DCalDavErrorCode::PermissionDenied));
    EXPECT_EQ(DCalDavSyncStatus::Conflict,
              DCalDavSyncStatus::fromErrorCode(DCalDavErrorCode::Conflict));
    EXPECT_EQ(DCalDavSyncStatus::Failed,
              DCalDavSyncStatus::fromErrorCode(DCalDavErrorCode::RequestTimedOut));
    EXPECT_EQ(DCalDavSyncStatus::RetryScheduled,
              static_cast<DCalDavSyncStatus::Value>(7));
}

TEST(CalDavSyncStatusMapper, DoesNotUseFallbackText)
{
    DCalDavTransport::Response response;
    EXPECT_EQ(DCalDavErrorCode::NoError,
              DCalDavSyncStatusMapper::errorCodeForFailure(response));

    response.httpStatus = 401;
    EXPECT_EQ(DCalDavErrorCode::AuthenticationFailed,
              DCalDavSyncStatusMapper::errorCodeForFailure(response));
    response.httpStatus = 403;
    EXPECT_EQ(DCalDavErrorCode::PermissionDenied,
              DCalDavSyncStatusMapper::errorCodeForFailure(response));
    response.httpStatus = 429;
    EXPECT_EQ(DCalDavErrorCode::RateLimited,
              DCalDavSyncStatusMapper::errorCodeForFailure(response));
}

TEST(CalDavAccountStatus, SerializesWithoutCredentialData)
{
    DCalDavAccountStatus input;
    input.accountId = QStringLiteral("account-id");
    input.displayName = QStringLiteral("WeCom");
    input.providerType = DCalDavProviderProfile::Provider_WeCom;
    input.syncStatus = 2;
    input.lastSuccessfulSync = QDateTime::fromString(QStringLiteral("2026-08-10T12:00:00"), Qt::ISODate);
    input.failureReason = QStringLiteral("raw server reason");
    input.failureCode = static_cast<int>(DCalDavErrorCode::NetworkUnavailable);
    input.pendingOperationCount = 2;
    input.pendingDeleteCount = 1;
    input.conflictCount = 1;
    input.nextRetryAt = QDateTime::fromString(
        QStringLiteral("2026-08-11T12:05:00Z"), Qt::ISODate);

    const QString json = DCalDavAccountStatus::toJsonListString({input});
    EXPECT_FALSE(json.contains(QStringLiteral("credentialRef")));

    DCalDavAccountStatus::List parsed;
    ASSERT_TRUE(DCalDavAccountStatus::fromJsonListString(parsed, json));
    ASSERT_EQ(1, parsed.size());
    EXPECT_EQ(input.accountId, parsed.first().accountId);
    EXPECT_EQ(input.failureReason, parsed.first().failureReason);
    EXPECT_EQ(input.failureCode, parsed.first().failureCode);
    EXPECT_EQ(input.pendingOperationCount, parsed.first().pendingOperationCount);
    EXPECT_EQ(input.pendingDeleteCount, parsed.first().pendingDeleteCount);
    EXPECT_EQ(input.conflictCount, parsed.first().conflictCount);
    EXPECT_EQ(input.nextRetryAt, parsed.first().nextRetryAt);
}

TEST(CalDavCredentialStore, RejectsInvalidReferencesWithoutCallingSecretService)
{
    QString password;
    QString errorMessage;
    EXPECT_FALSE(DCalDavCredentialStore::readPassword(QStringLiteral("invalid reference"), password,
                                                       &errorMessage));
    EXPECT_TRUE(password.isEmpty());
    EXPECT_FALSE(errorMessage.isEmpty());

    QString credentialRef;
    EXPECT_FALSE(DCalDavCredentialStore::storePassword(QString(), QStringLiteral("password"),
                                                        credentialRef, &errorMessage));
    EXPECT_TRUE(credentialRef.isEmpty());
}

TEST(CalDavCredentialReference, ValidatesOpaqueReferences)
{
    EXPECT_TRUE(DCalDavCredentialReference::isValid(QStringLiteral("secret-service:/org/freedesktop/secrets/item_123")));
    EXPECT_TRUE(DCalDavCredentialReference::isValid(QString()));
    EXPECT_FALSE(DCalDavCredentialReference::isValid(QStringLiteral("invalid reference")));
    EXPECT_FALSE(DCalDavCredentialReference::isValid(QString(513, QLatin1Char('a'))));
}

TEST(CalDavDiscovery, BuildsSecureDiscoveryRequests)
{
    const QUrl serverUrl(QStringLiteral("https://caldav.wecom.work/calendar/"));
    const QList<QUrl> candidates = DCalDavDiscovery::discoveryCandidates(serverUrl);
    ASSERT_EQ(2, candidates.size());
    EXPECT_EQ(QStringLiteral("https://caldav.wecom.work/.well-known/caldav"), candidates.first().toString());
    EXPECT_EQ(serverUrl, candidates.last());

    const DCalDavTransport::Request request = DCalDavDiscovery::currentUserPrincipalRequest(
        candidates.first(), QStringLiteral("user"), QStringLiteral("password"));
    EXPECT_EQ(QByteArray("PROPFIND"), request.method);
    EXPECT_EQ(QByteArray("application/xml; charset=utf-8"), request.contentType);
    EXPECT_EQ(QByteArray("0"), request.headers.value("Depth"));
    EXPECT_TRUE(request.body.contains("current-user-principal"));

    const DCalDavTransport::Request homeRequest = DCalDavDiscovery::calendarHomeSetRequest(
        serverUrl, QStringLiteral("user"), QStringLiteral("password"));
    EXPECT_EQ(QByteArray("0"), homeRequest.headers.value("Depth"));
    EXPECT_TRUE(homeRequest.body.contains("calendar-home-set"));

    const DCalDavTransport::Request collectionsRequest = DCalDavDiscovery::calendarCollectionsRequest(
        serverUrl, QStringLiteral("user"), QStringLiteral("password"));
    EXPECT_EQ(QByteArray("1"), collectionsRequest.headers.value("Depth"));
    EXPECT_TRUE(collectionsRequest.body.contains("calendar-color"));
    EXPECT_TRUE(DCalDavTransport::isSecureUrl(request.url));
    EXPECT_FALSE(DCalDavTransport::isSecureUrl(QUrl(QStringLiteral("http://example.com"))));
}

TEST(CalDavTransport, NormalizesAndRestrictsUrls)
{
    EXPECT_EQ(QStringLiteral("https://example.com/path"),
              DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("example.com/path"))).toString());
    EXPECT_EQ(QStringLiteral("https://example.com/path"),
              DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("//example.com/path#fragment"))).toString());
    EXPECT_FALSE(DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("http://example.com"))).isValid());
    EXPECT_FALSE(DCalDavTransport::normalizeUrl(QUrl(QStringLiteral("https://user:password@example.com"))).isValid());
    EXPECT_TRUE(DCalDavTransport::isSameOrigin(
        QUrl(QStringLiteral("https://example.com/a")), QUrl(QStringLiteral("https://example.com/b"))));
    EXPECT_FALSE(DCalDavTransport::isSameOrigin(
        QUrl(QStringLiteral("https://example.com")), QUrl(QStringLiteral("https://other.example.com"))));
    EXPECT_FALSE(DCalDavTransport::isSameOrigin(
        QUrl(QStringLiteral("https://example.com")), QUrl(QStringLiteral("http://example.com"))));
    EXPECT_FALSE(DCalDavDiscovery::resolveHref(
        QUrl(QStringLiteral("https://example.com/calendar/")),
        QStringLiteral("https://other.example.com/calendar/"))
        .isValid());
}

TEST(CalDavTransport, ClassifiesCertificateAndServerErrors)
{
    EXPECT_EQ(DCalDavTransport::CertificateInvalid,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 0, true, false, false));
    EXPECT_EQ(DCalDavTransport::AuthenticationFailed,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 401, false, false, false));
    EXPECT_EQ(DCalDavTransport::RateLimited,
              DCalDavTransport::classifyError(QNetworkReply::NoError, 429, false, false, false));
    EXPECT_EQ(DCalDavTransport::NetworkUnavailable,
              DCalDavTransport::classifyError(QNetworkReply::HostNotFoundError, 0, false, false, false));
}

TEST(CalDavXmlReader, ParsesDiscoveryPropertiesAndCalendars)
{
    const QByteArray xml = R"(
        <d:multistatus xmlns:d="DAV:" xmlns:c="urn:ietf:params:xml:ns:caldav" xmlns:apple="http://apple.com/ns/ical/">
          <d:response>
            <d:href>/calendar/</d:href>
            <d:propstat><d:prop>
              <d:current-user-principal><d:href>/principals/user/</d:href></d:current-user-principal>
              <c:calendar-home-set><d:href>/calendars/user/</d:href></c:calendar-home-set>
            </d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>
          </d:response>
          <d:response>
            <d:href>/calendars/user/work/</d:href>
            <d:propstat><d:prop>
              <d:displayname>Work</d:displayname>
              <apple:calendar-color>#00AA00FF</apple:calendar-color>
              <d:resourcetype><d:collection/><c:calendar/></d:resourcetype>
              <d:current-user-privilege-set>
                <d:privilege><d:read/></d:privilege>
                <d:privilege><d:write-content/></d:privilege>
              </d:current-user-privilege-set>
            </d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>
          </d:response>
          <d:response>
            <d:href>/calendars/user/addressbook/</d:href>
            <d:propstat><d:prop>
              <d:resourcetype><d:collection/></d:resourcetype>
            </d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>
          </d:response>
        </d:multistatus>)";

    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    ASSERT_TRUE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage));
    EXPECT_EQ(QStringLiteral("/principals/user/"), result.currentUserPrincipalHref);
    EXPECT_EQ(QStringLiteral("/calendars/user/"), result.calendarHomeSetHref);
    ASSERT_EQ(1, result.calendarCollections.size());
    EXPECT_EQ(QStringLiteral("/calendars/user/work/"), result.calendarCollections.first().href);
    EXPECT_EQ(QStringLiteral("Work"), result.calendarCollections.first().displayName);
    EXPECT_EQ(QStringLiteral("#00AA00FF"), result.calendarCollections.first().color);
    EXPECT_EQ(DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege,
              result.calendarCollections.first().privileges);
    EXPECT_EQ(QUrl(QStringLiteral("https://example.com/calendars/user/work/")),
              DCalDavDiscovery::resolveHref(QUrl(QStringLiteral("https://example.com/.well-known/caldav")),
                                             result.calendarCollections.first().href));
}

TEST(CalDavXmlReader, RejectsOversizedCollectionProperties)
{
    const QByteArray xml = QByteArray("<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">")
        + "<d:response><d:href>/calendar/</d:href><d:propstat><d:prop>"
        + "<d:resourcetype><d:collection/><c:calendar/></d:resourcetype>"
        + "<d:displayname>" + QByteArray(4097, 'x') + "</d:displayname>"
        + "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response></d:multistatus>";
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReader, RejectsMalformedXml)
{
    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery("<d:multistatus>", result, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(CalDavXmlReader, IncludesNestedElementText)
{
    DCalDavXmlStreamReader reader(
        QByteArrayLiteral("<root>before<child>inside</child>after</root>"));
    ASSERT_TRUE(reader.readNextStartElement());
    EXPECT_EQ(QStringLiteral("root"), reader.name());
    EXPECT_EQ(QStringLiteral("beforeinsideafter"),
              reader.readElementText(QXmlStreamReader::IncludeChildElements));
    EXPECT_FALSE(reader.hasError());
}

TEST(CalDavCalendarQuery, BuildsReadOnlyReportRequest)
{
    const DCalDavTransport::Request request = DCalDavCalendarQuery::firstSyncRequest(
        QUrl(QStringLiteral("https://dav.qq.com/calendar/work/")), QStringLiteral("user"), QStringLiteral("password"),
        QDateTime::fromString(QStringLiteral("2026-08-10T12:00:00Z"), Qt::ISODate));
    EXPECT_EQ(QByteArray("REPORT"), request.method);
    EXPECT_EQ(QByteArray("application/xml; charset=utf-8"), request.contentType);
    EXPECT_EQ(QByteArray("1"), request.headers.value("Depth"));
    EXPECT_TRUE(request.body.contains("calendar-query"));
    EXPECT_TRUE(request.body.contains("calendar-data"));
    EXPECT_TRUE(request.body.contains("VEVENT"));
    EXPECT_TRUE(request.body.contains("time-range start=\"20260710T120000Z\" end=\"20270210T120000Z\""));
    EXPECT_FALSE(request.body.contains("PUT"));
    EXPECT_FALSE(request.body.contains("DELETE"));
}

TEST(CalDavCalendarQuery, BuildsAndParsesResourceMetadataRequest)
{
    const DCalDavTransport::Request request = DCalDavCalendarQuery::resourceListRequest(
        QUrl(QStringLiteral("https://caldav.wecom.work/calendar/work/")),
        QStringLiteral("user"), QStringLiteral("password"));
    EXPECT_EQ(QByteArray("REPORT"), request.method);
    EXPECT_EQ(QByteArray("1"), request.headers.value("Depth"));
    EXPECT_TRUE(request.body.contains("calendar-query"));
    EXPECT_TRUE(request.body.contains("getetag"));
    EXPECT_TRUE(request.body.contains("getcontenttype"));
    EXPECT_FALSE(request.body.contains("calendar-data"));
    EXPECT_FALSE(request.body.contains("time-range"));

    const QByteArray xml = R"(
        <d:multistatus xmlns:d="DAV:">
          <d:response>
            <d:href>/calendar/work/event-1.ics</d:href>
            <d:propstat><d:prop>
              <d:getetag>"event-etag"</d:getetag>
              <d:getcontenttype>text/calendar; component=VEVENT</d:getcontenttype>
              <c:calendar-data xmlns:c="urn:ietf:params:xml:ns:caldav">BEGIN:VCALENDAR&#10;BEGIN:VEVENT&#10;UID:event-1&#10;SUMMARY:Planning&#10;END:VEVENT&#10;END:VCALENDAR&#10;</c:calendar-data>
            </d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>
          </d:response>
          <d:response>
            <d:href>/calendar/work/</d:href>
            <d:propstat><d:prop><d:getcontenttype>httpd/unix-directory</d:getcontenttype>
            </d:prop></d:propstat>
          </d:response>
          <d:response>
            <d:href>/calendar/work/deleted.ics</d:href>
            <d:propstat><d:status>HTTP/1.1 404 Not Found</d:status><d:prop>
              <d:getetag>"deleted-etag"</d:getetag>
            </d:prop></d:propstat>
          </d:response>
        </d:multistatus>)";

    DCalDavCalendarQuery::ResourceList resources;
    QString errorMessage;
    ASSERT_TRUE(DCalDavCalendarQuery::parseResourceList(xml, resources, &errorMessage));
    ASSERT_EQ(1, resources.size());
    EXPECT_EQ(QStringLiteral("/calendar/work/event-1.ics"), resources.first().href);
    EXPECT_EQ(QStringLiteral("\"event-etag\""), resources.first().etag);
    EXPECT_TRUE(resources.first().contentType.contains(QStringLiteral("VEVENT"), Qt::CaseInsensitive));
    EXPECT_TRUE(resources.first().calendarData.contains(QStringLiteral("UID:event-1")));
}

TEST(CalDavCalendarQuery, BuildsCalendarMultiGetRequest)
{
    const QStringList resourceHrefs = {
        QStringLiteral("https://dav.qq.com/calendar/work/event-1.ics"),
        QStringLiteral("https://dav.qq.com/calendar/work/event-2.ics?source=A&B"),
    };
    const DCalDavTransport::Request request = DCalDavCalendarQuery::resourceMultiGetRequest(
        QUrl(QStringLiteral("https://dav.qq.com/calendar/work/")), QStringLiteral("user"),
        QStringLiteral("password"), resourceHrefs);

    EXPECT_EQ(QByteArray("REPORT"), request.method);
    EXPECT_EQ(QByteArray("1"), request.headers.value("Depth"));
    EXPECT_TRUE(request.body.contains("calendar-multiget"));
    EXPECT_TRUE(request.body.contains("<d:href>/calendar/work/event-1.ics</d:href>"));
    EXPECT_TRUE(request.body.contains("/calendar/work/event-2.ics?source=A&amp;B"));
    EXPECT_FALSE(request.body.contains("https://dav.qq.com/calendar/work/event-1.ics"));
}

TEST(CalDavCalendarQuery, ParsesRemoteEventMetadataAndCalendarData)
{
    const QByteArray xml = R"(
        <d:multistatus xmlns:d="DAV:" xmlns:c="urn:ietf:params:xml:ns:caldav">
          <d:response>
            <d:href>/calendar/work/event-1.ics</d:href>
            <d:propstat><d:prop>
              <d:getetag>"event-etag"</d:getetag>
              <c:calendar-data>BEGIN:VCALENDAR&#10;BEGIN:VEVENT&#10;UID:event-1&#10;SUMMARY:Planning&#10;END:VEVENT&#10;END:VCALENDAR&#10;</c:calendar-data>
            </d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>
          </d:response>
        </d:multistatus>)";

    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    ASSERT_TRUE(DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage));
    ASSERT_EQ(1, events.size());
    EXPECT_EQ(QStringLiteral("/calendar/work/event-1.ics"), events.first().href);
    EXPECT_EQ(QStringLiteral("\"event-etag\""), events.first().etag);
    EXPECT_EQ(QStringLiteral("event-1"), events.first().uid);
    EXPECT_TRUE(events.first().calendarData.contains(QStringLiteral("SUMMARY:Planning")));
}

TEST(CalDavCalendarQuery, AcceptsSyncMetadataWithoutCalendarData)
{
    const QByteArray xml = R"(
        <d:multistatus xmlns:d="DAV:">
          <d:response><d:href>/calendar/work/event-1.ics</d:href><d:propstat><d:prop>
            <d:getetag>"event-etag-2"</d:getetag>
            <d:getcontenttype>text/calendar; component=VEVENT</d:getcontenttype>
          </d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response>
          <d:sync-token>next-token</d:sync-token>
        </d:multistatus>)";

    DCalDavCalendarQuery::RemoteEventList events;
    QString syncToken;
    QString errorMessage;
    ASSERT_TRUE(DCalDavCalendarQuery::parseResponseWithSyncToken(
        xml, events, &syncToken, &errorMessage));
    ASSERT_EQ(1, events.size());
    EXPECT_EQ(QStringLiteral("/calendar/work/event-1.ics"), events.first().href);
    EXPECT_EQ(QStringLiteral("\"event-etag-2\""), events.first().etag);
    EXPECT_TRUE(events.first().calendarData.isEmpty());
    EXPECT_EQ(QStringLiteral("next-token"), syncToken);
}

TEST(CalDavCalendarQuery, KeepsCalendarDataWithoutUidForSyncLayerValidation)
{
    const QByteArray xml = R"(
        <d:multistatus xmlns:d="DAV:" xmlns:c="urn:ietf:params:xml:ns:caldav">
          <d:response><d:href>/calendar/work/event.ics</d:href><d:propstat><d:prop>
            <c:calendar-data>BEGIN:VCALENDAR&#10;BEGIN:VEVENT&#10;SUMMARY:Missing UID&#10;END:VEVENT&#10;END:VCALENDAR</c:calendar-data>
          </d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat></d:response>
        </d:multistatus>)";

    DCalDavCalendarQuery::RemoteEventList events;
    QString errorMessage;
    ASSERT_TRUE(DCalDavCalendarQuery::parseResponse(xml, events, &errorMessage));
    ASSERT_EQ(1, events.size());
    EXPECT_TRUE(events.first().uid.isEmpty());
    EXPECT_TRUE(events.first().calendarData.contains(QStringLiteral("SUMMARY:Missing UID")));
}


TEST(CalDavEventMapper, ConvertsCalendarDataWithoutChangingRemoteIdentity)
{
    DCalDavCalendarQuery::RemoteEvent remoteEvent;
    remoteEvent.uid = QStringLiteral("event-1");
    remoteEvent.calendarData = QStringLiteral("BEGIN:VCALENDAR\r\n"
                                              "VERSION:2.0\r\n"
                                              "BEGIN:VEVENT\r\n"
                                              "UID:event-1\r\n"
                                              "DTSTART:20260810T090000Z\r\n"
                                              "DTEND:20260810T100000Z\r\n"
                                              "SUMMARY:Planning\r\n"
                                              "END:VEVENT\r\n"
                                              "END:VCALENDAR\r\n");

    DSchedule::Ptr schedule;
    QString errorMessage;
    ASSERT_TRUE(DCalDavEventMapper::toSchedule(remoteEvent, schedule, &errorMessage));
    ASSERT_TRUE(schedule);
    EXPECT_EQ(remoteEvent.uid, schedule->uid());
    EXPECT_EQ(QStringLiteral("Planning"), schedule->summary());
}

TEST(CalDavEventMapper, ConvertsUtcEventTimeToLocalTime)
{
    DCalDavCalendarQuery::RemoteEvent remoteEvent;
    remoteEvent.uid = QStringLiteral("event-timezone");
    remoteEvent.calendarData = QStringLiteral("BEGIN:VCALENDAR\r\n"
                                              "VERSION:2.0\r\n"
                                              "BEGIN:VEVENT\r\n"
                                              "UID:event-timezone\r\n"
                                              "DTSTART:20260825T113000Z\r\n"
                                              "DTEND:20260825T123000Z\r\n"
                                              "SUMMARY:Timezone test\r\n"
                                              "END:VEVENT\r\n"
                                              "END:VCALENDAR\r\n");

    DSchedule::Ptr schedule;
    QString errorMessage;
    ASSERT_TRUE(DCalDavEventMapper::toSchedule(remoteEvent, schedule, &errorMessage));
    ASSERT_TRUE(schedule);
    EXPECT_EQ(QDateTime::fromString(QStringLiteral("2026-08-25T11:30:00Z"), Qt::ISODate).toLocalTime(),
              schedule->dtStart());
    EXPECT_EQ(QDateTime::fromString(QStringLiteral("2026-08-25T12:30:00Z"), Qt::ISODate).toLocalTime(),
              schedule->dtEnd());
}

TEST(CalDavEventMapper, RejectsMismatchedRemoteIdentity)
{
    DCalDavCalendarQuery::RemoteEvent remoteEvent;
    remoteEvent.uid = QStringLiteral("event-2");
    remoteEvent.calendarData = QStringLiteral("BEGIN:VCALENDAR\r\n"
                                              "VERSION:2.0\r\n"
                                              "BEGIN:VEVENT\r\n"
                                              "UID:event-1\r\n"
                                              "DTSTART:20260810T090000Z\r\n"
                                              "DTEND:20260810T100000Z\r\n"
                                              "END:VEVENT\r\n"
                                              "END:VCALENDAR\r\n");

    DSchedule::Ptr schedule;
    QString errorMessage;
    EXPECT_FALSE(DCalDavEventMapper::toSchedule(remoteEvent, schedule, &errorMessage));
    EXPECT_EQ(QStringLiteral("Remote event UID does not match calendar data."), errorMessage);
}


TEST(CalDavReadOnlySync, RejectsInvalidRequestWithoutNetworkAccess)
{
    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = QUrl(QStringLiteral("http://example.com/caldav"));
    request.username = QStringLiteral("user");

    bool called = false;
    synchronizer.start(request, [&called](const DCalDavReadOnlySync::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(DCalDavValidationError::NetworkUnavailable, result.validationError);
        EXPECT_EQ(QStringLiteral("Invalid CalDAV server URL or username."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}


TEST(CalDavSyncStateMachine, QueuesTriggersReceivedWhileAccountIsRunning)
{
    DCalDavSyncStateMachine stateMachine;
    ASSERT_TRUE(stateMachine.registerAccount(QStringLiteral("account-1")));
    EXPECT_FALSE(stateMachine.registerAccount(QStringLiteral("account-1")));

    ASSERT_TRUE(stateMachine.requestSync(QStringLiteral("account-1"),
                                         DCalDavSyncStateMachine::StartupTrigger));
    ASSERT_TRUE(stateMachine.requestSync(QStringLiteral("account-1"),
                                         DCalDavSyncStateMachine::ManualTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued, stateMachine.stateFor(QStringLiteral("account-1")));

    EXPECT_EQ(QStringLiteral("account-1"), stateMachine.takeNextRunnableAccount());
    EXPECT_EQ(DCalDavSyncStateMachine::Running, stateMachine.stateFor(QStringLiteral("account-1")));
    EXPECT_TRUE(stateMachine.requestSync(QStringLiteral("account-1"),
                                         DCalDavSyncStateMachine::ForegroundTrigger));
    EXPECT_EQ(DCalDavSyncStateMachine::ForegroundTrigger,
              stateMachine.pendingTriggersFor(QStringLiteral("account-1")));

    ASSERT_TRUE(stateMachine.complete(QStringLiteral("account-1"), true));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued, stateMachine.stateFor(QStringLiteral("account-1")));
    EXPECT_EQ(QStringLiteral("account-1"), stateMachine.takeNextRunnableAccount());
    ASSERT_TRUE(stateMachine.complete(QStringLiteral("account-1"), true));
    EXPECT_EQ(DCalDavSyncStateMachine::Succeeded, stateMachine.stateFor(QStringLiteral("account-1")));
    EXPECT_TRUE(stateMachine.takeNextRunnableAccount().isEmpty());
}

TEST(CalDavSyncStateMachine, TakesOnlyRequestedAccountFromQueue)
{
    DCalDavSyncStateMachine stateMachine;
    ASSERT_TRUE(stateMachine.registerAccount(QStringLiteral("wecom")));
    ASSERT_TRUE(stateMachine.registerAccount(QStringLiteral("tencent")));

    ASSERT_TRUE(stateMachine.requestSync(QStringLiteral("wecom"),
                                         DCalDavSyncStateMachine::RetryTrigger));
    ASSERT_TRUE(stateMachine.requestSync(QStringLiteral("tencent"),
                                         DCalDavSyncStateMachine::ManualTrigger));

    EXPECT_EQ(QStringLiteral("tencent"),
              stateMachine.takeRunnableAccount(QStringLiteral("tencent")));
    EXPECT_EQ(DCalDavSyncStateMachine::Queued,
              stateMachine.stateFor(QStringLiteral("wecom")));
    EXPECT_EQ(QStringLiteral("wecom"), stateMachine.takeNextRunnableAccount());
}


TEST(CalDavSyncStateMachine, DoesNotScheduleWhenNoCalDavAccountExists)
{
    DCalDavSyncStateMachine stateMachine;
    EXPECT_EQ(0, stateMachine.requestSyncForAll(DCalDavSyncStateMachine::DailyTrigger));
    EXPECT_TRUE(stateMachine.takeNextRunnableAccount().isEmpty());
    EXPECT_FALSE(stateMachine.containsAccount(QStringLiteral("missing-account")));
}


TEST(CalDavCalendarQuery, BuildsIncrementalRequestAndKeepsDeletedResources)
{
    const DCalDavTransport::Request request = DCalDavCalendarQuery::incrementalSyncRequest(
        QUrl(QStringLiteral("https://dav.qq.com/calendar/work/")), QStringLiteral("user"), QStringLiteral("password"),
        QStringLiteral("token<&"));
    EXPECT_EQ(QByteArray("REPORT"), request.method);
    EXPECT_EQ(QByteArray("1"), request.headers.value("Depth"));
    EXPECT_TRUE(request.body.contains("sync-collection"));
    EXPECT_TRUE(request.body.contains("token&lt;&amp;"));

    const QByteArray xml = R"(
        <d:multistatus xmlns:d="DAV:" xmlns:c="urn:ietf:params:xml:ns:caldav">
          <d:sync-token>next-token</d:sync-token>
          <d:response>
            <d:href>/calendar/work/deleted.ics</d:href>
            <d:propstat><d:status>HTTP/1.1 404 Not Found</d:status></d:propstat>
          </d:response>
        </d:multistatus>)";
    DCalDavCalendarQuery::RemoteEventList events;
    QString syncToken;
    QString errorMessage;
    ASSERT_TRUE(DCalDavCalendarQuery::parseResponseWithSyncToken(xml, events, &syncToken, &errorMessage));
    ASSERT_EQ(1, events.size());
    EXPECT_TRUE(events.first().deleted);
    EXPECT_TRUE(events.first().uid.isEmpty());
    EXPECT_EQ(QStringLiteral("next-token"), syncToken);
}


TEST(CalDavIncrementalSync, RejectsInvalidCalendarWithoutStartingNetworkTask)
{
    DCalDavIncrementalSync synchronizer;
    DCalDavIncrementalSync::Request request;
    request.calendarUrl = QUrl(QStringLiteral("http://example.com/calendar/"));
    request.username = QStringLiteral("user");

    bool called = false;
    synchronizer.start(request, [&called](const DCalDavIncrementalSync::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_FALSE(result.usedFullRangeFallback);
        EXPECT_EQ(QStringLiteral("Invalid CalDAV calendar URL or username."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}


TEST(CalDavOutboxProcessor, RejectsIncompleteRequestWithoutNetworkAccess)
{
    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    bool called = false;
    processor.start(request, [&called](const DCalDavOutboxProcessor::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(QStringLiteral("CalDAV Outbox request is incomplete."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavRepository, ManualRetryIgnoresNetworkBackoffButNotConflicts)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    DCalDavAccountInfo account;
    account.accountId = QStringLiteral("manual-retry-account");
    account.providerType = DCalDavProviderProfile::Provider_QQMail;
    account.serverUrl = QStringLiteral("https://dav.qq.com");
    account.username = QStringLiteral("user@qq.com");
    account.credentialRef = QStringLiteral("secret-service:/manual_retry");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(account));

    const QDateTime future = QDateTime::currentDateTimeUtc().addSecs(3600);
    DCalDavOutboxItem networkItem;
    networkItem.operationID = QStringLiteral("network-operation");
    networkItem.accountID = account.accountId;
    networkItem.localScheduleID = QStringLiteral("schedule-network");
    networkItem.failureType = DCalDavOutboxItem::NetworkFailure;
    networkItem.nextRetryAt = future;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(networkItem));

    DCalDavOutboxItem conflictItem = networkItem;
    conflictItem.operationID = QStringLiteral("conflict-operation");
    conflictItem.localScheduleID = QStringLiteral("schedule-conflict");
    conflictItem.failureType = DCalDavOutboxItem::ConflictFailure;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(conflictItem));

    EXPECT_TRUE(database.getDueCalDavOutboxItems(account.accountId,
                                                  QDateTime::currentDateTimeUtc()).isEmpty());
    const DCalDavOutboxItem::List manualItems = database.getDueCalDavOutboxItems(
        account.accountId, QDateTime::currentDateTimeUtc(), true);
    ASSERT_EQ(1, manualItems.size());
    EXPECT_EQ(networkItem.operationID, manualItems.first().operationID);
}

TEST(CalDavRetryPolicy, UsesBackoffAndRetryAfterOnlyForTransientFailures)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::NetworkUnavailable;
    EXPECT_TRUE(DCalDavRetryPolicy::decide(response, 0).retry);
    EXPECT_EQ(60, DCalDavRetryPolicy::decide(response, 0).delaySeconds);
    EXPECT_EQ(5 * 60, DCalDavRetryPolicy::decide(response, 1).delaySeconds);
    EXPECT_EQ(15 * 60, DCalDavRetryPolicy::decide(response, 2).delaySeconds);

    response.error = DCalDavTransport::RateLimited;
    response.retryAfter = QByteArray("37");
    EXPECT_EQ(37, DCalDavRetryPolicy::decide(response, 0).delaySeconds);

    response.error = DCalDavTransport::AuthenticationFailed;
    EXPECT_FALSE(DCalDavRetryPolicy::decide(response, 0).retry);

    response.error = DCalDavTransport::NetworkError;
    EXPECT_TRUE(DCalDavRetryPolicy::decide(response, 0).retry);

    response.error = DCalDavTransport::ServerUnavailable;
    response.retryAfter = QByteArray("Wed, 02 Sep 2026 14:00:00 GMT");
    const DCalDavRetryPolicy::Decision dateDecision = DCalDavRetryPolicy::decide(response, 0);
    EXPECT_TRUE(dateDecision.retry);
    EXPECT_GE(dateDecision.delaySeconds, 0);
}


TEST(CalDavAccountStatus, UsesDiscoveredCalendarWritePrivileges)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    DCalDavAccountInfo account;
    account.accountId = QStringLiteral("read-only-account");
    account.providerType = DCalDavProviderProfile::Provider_QQMail;
    account.serverUrl = QStringLiteral("https://dav.qq.com");
    account.username = QStringLiteral("user@qq.com");
    account.credentialRef = QStringLiteral("secret-service:/read_only");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(account));

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("read-only-calendar");
    calendar.accountId = account.accountId;
    calendar.href = QStringLiteral("https://dav.qq.com/calendar/read-only/");
    calendar.scheduleTypeID = QStringLiteral("read-only-type");
    calendar.privileges = DCalDavXmlReader::ReadPrivilege;
    ASSERT_TRUE(database.upsertCalDavCalendar(calendar));

    DCalDavAccountStatus::List statuses;
    ASSERT_TRUE(DCalDavAccountStatus::fromJsonListString(
        statuses, database.getCalDavAccountStatusList()));
    ASSERT_EQ(1, statuses.size());
    EXPECT_FALSE(statuses.first().supportsWrite);
}

TEST(CalDavRepository, PersistsCalendarTokenAndAccountStatus)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    DCalDavAccountInfo insertedAccount;
    insertedAccount.accountId = QStringLiteral("account-1");
    insertedAccount.providerType = DCalDavProviderProfile::Provider_QQMail;
    insertedAccount.serverUrl = QStringLiteral("https://dav.qq.com");
    insertedAccount.username = QStringLiteral("user@qq.com");
    insertedAccount.credentialRef = QStringLiteral("secret-service:/org/freedesktop/secrets/collection/login/1");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(insertedAccount));

    DCalDavAccountInfo accountInfo;
    ASSERT_TRUE(database.getCalDavAccountInfo(QStringLiteral("account-1"), accountInfo));
    EXPECT_EQ(QStringLiteral("https://dav.qq.com"), accountInfo.serverUrl);
    EXPECT_EQ(QStringLiteral("user@qq.com"), accountInfo.username);
    EXPECT_EQ(insertedAccount.credentialRef, accountInfo.credentialRef);

    DCalDavAccountInfo updatedAccount = insertedAccount;
    updatedAccount.serverUrl = QStringLiteral("https://dav.qq.com/calendar/");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(updatedAccount));
    ASSERT_TRUE(database.getCalDavAccountInfo(updatedAccount.accountId, accountInfo));
    EXPECT_EQ(updatedAccount.serverUrl, accountInfo.serverUrl);

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("calendar-1");
    calendar.accountId = QStringLiteral("account-1");
    calendar.href = QStringLiteral("https://dav.qq.com/calendar/1/");
    calendar.displayName = QStringLiteral("Calendar");
    calendar.scheduleTypeID = QStringLiteral("type-1");
    calendar.privileges = DCalDavXmlReader::ReadPrivilege;
    calendar.privilegesKnown = true;
    ASSERT_TRUE(database.upsertCalDavCalendar(calendar));

    DCalDavCalendarInfo::List calendars = database.getCalDavCalendarList(QStringLiteral("account-1"));
    ASSERT_EQ(1, calendars.size());
    EXPECT_EQ(calendar.href, calendars.first().href);
    EXPECT_EQ(calendar.scheduleTypeID, calendars.first().scheduleTypeID);
    EXPECT_EQ(calendar.privileges, calendars.first().privileges);
    EXPECT_TRUE(calendars.first().privilegesKnown);
    EXPECT_FALSE(calendars.first().initialSyncCompleted);

    ASSERT_TRUE(database.updateCalDavCalendarInitialSyncCompleted(
        QStringLiteral("calendar-1"), true));
    calendars = database.getCalDavCalendarList(QStringLiteral("account-1"));
    ASSERT_EQ(1, calendars.size());
    EXPECT_TRUE(calendars.first().initialSyncCompleted);

    ASSERT_TRUE(database.updateCalDavCalendarSyncToken(QStringLiteral("calendar-1"), QStringLiteral("token-2")));
    calendars = database.getCalDavCalendarList(QStringLiteral("account-1"));
    ASSERT_EQ(1, calendars.size());
    EXPECT_EQ(QStringLiteral("token-2"), calendars.first().syncToken);
    const DCalDavCalendarInfo calendarByType = database.getCalDavCalendarByScheduleTypeID(
        QStringLiteral("account-1"), QStringLiteral("type-1"));
    EXPECT_EQ(calendar.calendarId, calendarByType.calendarId);
    EXPECT_TRUE(calendarByType.privilegesKnown);
    EXPECT_TRUE(calendarByType.initialSyncCompleted);

    const QDateTime syncTime = QDateTime::fromString(QStringLiteral("2026-08-10T12:00:00Z"), Qt::ISODate);
    ASSERT_TRUE(database.updateCalDavSyncStatus(QStringLiteral("account-1"), 2, syncTime, QString()));
    EXPECT_TRUE(database.getCalDavAccountStatusList().contains(QStringLiteral("2026-08-10T12:00:00Z")));
    ASSERT_TRUE(database.updateCalDavSyncStatus(QStringLiteral("account-1"), 1, QDateTime(), QString()));
    EXPECT_TRUE(database.getCalDavAccountStatusList().contains(QStringLiteral("2026-08-10T12:00:00Z")));
    ASSERT_TRUE(database.updateCalDavSyncStatus(QStringLiteral("account-1"), 3, QDateTime(),
                                                QStringLiteral("sync failed")));
    EXPECT_TRUE(database.getCalDavAccountStatusList().contains(QStringLiteral("2026-08-10T12:00:00Z")));

    DCalDavEventMappingInfo mapping;
    mapping.localScheduleID = QStringLiteral("local-schedule-1");
    mapping.accountID = QStringLiteral("account-1");
    mapping.calendarID = QStringLiteral("calendar-1");
    mapping.uid = QStringLiteral("event-1");
    mapping.href = QStringLiteral("https://dav.qq.com/calendar/1/event-1.ics");
    mapping.etag = QStringLiteral("\"etag-1\"");
    mapping.originalIcs = QStringLiteral("BEGIN:VCALENDAR");
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    const DCalDavEventMappingInfo::List mappingList = database.getCalDavEventMappingList(
        mapping.accountID, mapping.calendarID);
    ASSERT_EQ(1, mappingList.size());
    EXPECT_EQ(mapping.href, mappingList.first().href);
    const DCalDavEventMappingInfo stored = database.getCalDavEventMapping(mapping.accountID, mapping.href);
    EXPECT_EQ(mapping.localScheduleID, stored.localScheduleID);
    EXPECT_EQ(mapping.etag, stored.etag);
    const DCalDavEventMappingInfo mappingByLocal = database.getCalDavEventMappingByLocalScheduleID(
        mapping.accountID, mapping.localScheduleID);
    EXPECT_EQ(mapping.href, mappingByLocal.href);

    DCalDavOutboxItem outbox;
    outbox.operationID = QStringLiteral("operation-1");
    outbox.accountID = mapping.accountID;
    outbox.localScheduleID = mapping.localScheduleID;
    outbox.operationType = DCalDavOutboxItem::ModifyOperation;
    outbox.baseEtag = mapping.etag;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(outbox));
    EXPECT_TRUE(database.hasCalDavOutboxItems(outbox.accountID));
    EXPECT_EQ(outbox.operationID, database.getCalDavOutboxItem(
        outbox.accountID, outbox.localScheduleID).operationID);
    EXPECT_EQ(1, database.getDueCalDavOutboxItems(outbox.accountID,
                                                   QDateTime::currentDateTimeUtc()).size());
    outbox.failureType = DCalDavOutboxItem::PermissionFailure;
    outbox.nextRetryAt = QDateTime::fromString(QStringLiteral("2026-08-11T12:05:00Z"), Qt::ISODate);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(outbox));
    EXPECT_TRUE(database.getDueCalDavOutboxItems(outbox.accountID,
                                                  QDateTime::currentDateTimeUtc()).isEmpty());
    EXPECT_EQ(1, database.getDueCalDavOutboxItems(
        outbox.accountID, QDateTime::currentDateTimeUtc(), true).size());
    ASSERT_EQ(1, database.getCalDavBlockedOutboxItems(outbox.accountID).size());

    outbox.failureType = DCalDavOutboxItem::NetworkFailure;
    outbox.nextRetryAt = QDateTime::currentDateTimeUtc().addSecs(300);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(outbox));
    EXPECT_TRUE(database.getCalDavOutboxItem(
        outbox.accountID, outbox.localScheduleID).nextRetryAt.isValid());
    ASSERT_EQ(1, database.getCalDavRetryScheduledOutboxItems(
        outbox.accountID, QDateTime::currentDateTimeUtc()).size());
    EXPECT_TRUE(database.getCalDavBlockedOutboxItems(outbox.accountID).isEmpty());

    outbox.failureType = DCalDavOutboxItem::ConflictFailure;
    outbox.nextRetryAt = QDateTime::fromString(QStringLiteral("2026-08-11T12:05:00Z"), Qt::ISODate);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(outbox));
    EXPECT_TRUE(database.getDueCalDavOutboxItems(outbox.accountID,
                                                  QDateTime::currentDateTimeUtc()).isEmpty());
    ASSERT_EQ(1, database.getCalDavConflictItems(outbox.accountID).size());

    DCalDavOutboxItem staleOutbox = outbox;
    staleOutbox.operationID = QStringLiteral("stale-operation");
    EXPECT_FALSE(database.deleteCalDavOutboxItemIfCurrent(staleOutbox));
    EXPECT_EQ(outbox.operationID, database.getCalDavOutboxItem(
        outbox.accountID, outbox.localScheduleID).operationID);

    DCalDavAccountStatus::List statuses;
    ASSERT_TRUE(DCalDavAccountStatus::fromJsonListString(
        statuses, database.getCalDavAccountStatusList()));
    ASSERT_EQ(1, statuses.size());
    EXPECT_EQ(1, statuses.first().pendingOperationCount);
    EXPECT_EQ(1, statuses.first().conflictCount);
    EXPECT_FALSE(statuses.first().nextRetryAt.isValid());
    ASSERT_TRUE(database.deleteCalDavOutboxItemIfCurrent(outbox));

    ASSERT_TRUE(database.deleteCalDavAccountData(QStringLiteral("account-1")));
    EXPECT_TRUE(database.getCalDavCalendarList(QStringLiteral("account-1")).isEmpty());
    EXPECT_TRUE(database.getCalDavEventMappingList(QStringLiteral("account-1"),
                                                    QStringLiteral("calendar-1")).isEmpty());
    EXPECT_FALSE(database.getCalDavAccountInfo(QStringLiteral("account-1"), accountInfo));
    EXPECT_FALSE(database.deleteCalDavEventMapping(mapping.accountID, mapping.href));
    EXPECT_TRUE(database.getCalDavEventMapping(mapping.accountID, mapping.href).href.isEmpty());
}


TEST(CalDavEventReconciler, BuildsCreateUpdateAndDeleteActions)
{
    DCalDavCalendarQuery::RemoteEvent createEvent;
    createEvent.href = QStringLiteral("https://example.com/create.ics");
    createEvent.uid = QStringLiteral("create-1");
    createEvent.calendarData = QStringLiteral("BEGIN:VCALENDAR");

    DCalDavCalendarQuery::RemoteEvent updateEvent = createEvent;
    updateEvent.href = QStringLiteral("https://example.com/update.ics");
    updateEvent.uid = QStringLiteral("update-1");

    DCalDavCalendarQuery::RemoteEvent deleteEvent;
    deleteEvent.href = QStringLiteral("https://example.com/delete.ics");
    deleteEvent.deleted = true;

    DCalDavEventMappingInfo updateMapping;
    updateMapping.localScheduleID = QStringLiteral("local-update");
    updateMapping.href = updateEvent.href;
    DCalDavEventMappingInfo deleteMapping;
    deleteMapping.localScheduleID = QStringLiteral("local-delete");
    deleteMapping.href = deleteEvent.href;

    DCalDavEventReconciler::ActionList actions;
    QString errorMessage;
    ASSERT_TRUE(DCalDavEventReconciler::buildActions(
        {createEvent, updateEvent, deleteEvent}, {updateMapping, deleteMapping}, actions, &errorMessage));
    ASSERT_EQ(3, actions.size());
    EXPECT_EQ(DCalDavEventReconciler::CreateAction, actions.at(0).type);
    EXPECT_EQ(DCalDavEventReconciler::UpdateAction, actions.at(1).type);
    EXPECT_EQ(QStringLiteral("local-update"), actions.at(1).existingMapping.localScheduleID);
    EXPECT_EQ(DCalDavEventReconciler::DeleteAction, actions.at(2).type);
    EXPECT_EQ(QStringLiteral("local-delete"), actions.at(2).existingMapping.localScheduleID);
}

TEST(CalDavEventReconciler, RejectsDuplicateRemoteHref)
{
    DCalDavCalendarQuery::RemoteEvent event;
    event.href = QStringLiteral("https://example.com/event.ics");
    event.uid = QStringLiteral("event-1");
    event.calendarData = QStringLiteral("BEGIN:VCALENDAR");

    DCalDavEventReconciler::ActionList actions;
    QString errorMessage;
    EXPECT_FALSE(DCalDavEventReconciler::buildActions({event, event}, {}, actions, &errorMessage));
    EXPECT_EQ(QStringLiteral("Remote event href is empty or duplicated."), errorMessage);
}


TEST(CalDavEventScheduleApplier, RejectsIncompleteRequestBeforeDatabaseAccess)
{
    DCalDavEventScheduleApplier::Request request;
    const DCalDavEventScheduleApplier::Result result = DCalDavEventScheduleApplier::apply(request);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QStringLiteral("CalDAV schedule apply request is incomplete."), result.errorMessage);
}


TEST(CalDavEventScheduleApplier, TrimsCategoryEdgesWithoutReordering)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("category-normalization-account"));

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavCalendarQuery::RemoteEvent event;
    event.href = QStringLiteral("https://example.test/calendars/user/category-event.ics");
    event.uid = QStringLiteral("category-event");
    event.calendarData = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\n"
        "BEGIN:VEVENT\r\nUID:category-event\r\n"
        "DTSTART:20260903T100000Z\r\nDTEND:20260903T110000Z\r\n"
        "SUMMARY:Category event\r\nCATEGORIES: Zebra , Alpha \r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n");

    DCalDavEventScheduleApplier::Request request;
    request.accountID = account->accountID();
    request.calendarID = QStringLiteral("category-calendar");
    request.actions.append({DCalDavEventReconciler::CreateAction, event, {}});
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;

    const DCalDavEventScheduleApplier::Result result = DCalDavEventScheduleApplier::apply(request);
    ASSERT_TRUE(result.success) << result.errorMessage.toStdString();
    ASSERT_EQ(1, result.createdCount);

    const DCalDavEventMappingInfo mapping = accountManagerDatabase.getCalDavEventMapping(
        request.accountID, event.href);
    ASSERT_FALSE(mapping.localScheduleID.isEmpty());
    const DSchedule::Ptr schedule = localDatabase.getScheduleByScheduleID(mapping.localScheduleID);
    ASSERT_TRUE(schedule);
    const DScheduleType::Ptr type = localDatabase.getScheduleTypeByID(schedule->scheduleTypeID());
    ASSERT_TRUE(type);
    EXPECT_EQ(QStringLiteral("Zebra, Alpha"), type->displayName());
}

TEST(CalDavAccountSync, RejectsIncompleteRequestBeforeDatabaseAccess)
{
    DCalDavAccountSync synchronizer;
    DCalDavAccountSync::Request request;

    bool called = false;
    synchronizer.start(request, [&called](const DCalDavAccountSync::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(QStringLiteral("CalDAV account sync request is incomplete."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}


TEST(CalDavAccountSync, FailsWhenSkippedResourceRecordsCannotBeLoaded)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("skipped-resource-query-account"));

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = QStringLiteral("https://caldav.example.test");
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavAccountInfo(accountInfo));

    DCalDavCalendarInfo calendar;
    calendar.accountId = account->accountID();
    calendar.calendarId = QStringLiteral("calendar-1");
    calendar.href = QStringLiteral("https://caldav.example.test/calendars/user/");
    calendar.displayName = QStringLiteral("Calendar");
    calendar.scheduleTypeID = QStringLiteral("schedule-type-1");
    calendar.syncToken = QStringLiteral("old-token");
    calendar.enabled = true;
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(calendar));

    QSqlQuery dropSkippedResourceTable(
        QSqlDatabase::database(accountManagerDatabase.getConnectionName()));
    ASSERT_TRUE(dropSkippedResourceTable.exec(QStringLiteral(
        "DROP TABLE caldavSkippedResource")));

    DCalDavAccountSync::Request request;
    request.accountID = account->accountID();
    request.username = accountInfo.username;
    request.password = QStringLiteral("password");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;
    request.calendars.append({calendar, calendar.scheduleTypeID});

    DCalDavAccountSync synchronizer;
    DCalDavAccountSync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    synchronizer.start(request, [&](const DCalDavAccountSync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QStringLiteral("Failed to load malformed CalDAV resource records."),
              result.errorMessage);
    const DCalDavCalendarInfo storedCalendar = accountManagerDatabase
        .getCalDavCalendarByIDIncludingDisabled(account->accountID(), calendar.calendarId);
    ASSERT_FALSE(storedCalendar.calendarId.isEmpty());
    EXPECT_EQ(QStringLiteral("old-token"), storedCalendar.syncToken);
}


TEST(CalDavSyncJobManager, RegistersAndUnregistersAccounts)
{
    DCalDavSyncJobManager manager;
    DCalDavAccountSync::Request request;
    request.accountID = QStringLiteral("account-1");
    ASSERT_TRUE(manager.registerAccount(request));
    EXPECT_TRUE(manager.registerAccount(request));
    EXPECT_TRUE(manager.containsAccount(QStringLiteral("account-1")));
    ASSERT_TRUE(manager.unregisterAccount(QStringLiteral("account-1")));
    EXPECT_FALSE(manager.containsAccount(QStringLiteral("account-1")));
}


TEST(CalDavXmlReader, RejectsExcessiveXmlDepth)
{
    QByteArray xml("<d:multistatus xmlns:d=\"DAV:\">");
    for (int i = 0; i < 70; ++i) {
        xml.append("<d:x>");
    }
    for (int i = 0; i < 70; ++i) {
        xml.append("</d:x>");
    }
    xml.append("</d:multistatus>");

    DCalDavXmlReader::DiscoveryResult result;
    QString errorMessage;
    EXPECT_FALSE(DCalDavXmlReader::parseDiscovery(xml, result, &errorMessage));
    EXPECT_EQ(QStringLiteral("DAV discovery response is too deeply nested."), errorMessage);
}

TEST(CalDavEventReconciler, MatchesMovedResourceByUid)
{
    DCalDavEventMappingInfo mapping;
    mapping.localScheduleID = QStringLiteral("local-event");
    mapping.uid = QStringLiteral("stable-uid");
    mapping.href = QStringLiteral("https://example.com/old.ics");

    DCalDavCalendarQuery::RemoteEvent event;
    event.href = QStringLiteral("https://example.com/new.ics");
    event.uid = mapping.uid;
    event.calendarData = QStringLiteral("BEGIN:VCALENDAR");

    DCalDavEventReconciler::ActionList actions;
    QString errorMessage;
    ASSERT_TRUE(DCalDavEventReconciler::buildActions({event}, {mapping}, actions, &errorMessage));
    ASSERT_EQ(1, actions.size());
    EXPECT_EQ(DCalDavEventReconciler::UpdateAction, actions.first().type);
    EXPECT_EQ(mapping.href, actions.first().existingMapping.href);
}

TEST(CalDavCommon, PrefersMasterEventOverDetachedException)
{
    const QString ics = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\n"
        "BEGIN:VEVENT\r\nUID:recurring-event\r\n"
        "RECURRENCE-ID:20260903T090000Z\r\n"
        "DTSTART:20260903T100000Z\r\nDTEND:20260903T110000Z\r\n"
        "SUMMARY:Detached exception\r\nEND:VEVENT\r\n"
        "BEGIN:VEVENT\r\nUID:recurring-event\r\n"
        "DTSTART:20260901T090000Z\r\nDTEND:20260901T100000Z\r\n"
        "RRULE:FREQ=DAILY;COUNT=5\r\nSUMMARY:Master event\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n");

    DSchedule::Ptr schedule;
    ASSERT_TRUE(DSchedule::fromIcsString(schedule, ics));
    ASSERT_TRUE(schedule);
    EXPECT_EQ(QStringLiteral("Master event"), schedule->summary());
}

TEST(CalDavCommon, SerializesNegativeTimezoneOffset)
{
    const QDateTime dateTime(QDate(2026, 8, 31), QTime(12, 0),
                             QTimeZone(QByteArrayLiteral("America/New_York")));
    EXPECT_EQ(QStringLiteral("2026-08-31T12:00:00-04:00"), dtToString(dateTime));
}

TEST(CalDavCancellation, AccountRegistrarNotifiesCallbackOnce)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(1000);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("cancel-registrar-account"));

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavSyncJobManager jobManager;
    DCalDavAccountRegistrar::Request request;
    request.account.accountId = account->accountID();
    request.account.serverUrl = server.url().toString();
    request.account.username = QStringLiteral("user");
    request.account.credentialRef = QStringLiteral("secret-service:/test");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;
    request.jobManager = &jobManager;
    request.credentialReader = [](const QString &, QString &password, QString *) {
        password = QStringLiteral("password");
        return true;
    };

    DCalDavAccountRegistrar registrar;
    int callbackCount = 0;
    DCalDavAccountRegistrar::Result result;
    registrar.start(request, [&](const DCalDavAccountRegistrar::Result &registrarResult) {
        ++callbackCount;
        result = registrarResult;
    });
    ASSERT_TRUE(waitForMockRequestCount(server, 1));
    registrar.cancel();
    processEventsFor(1200);

    EXPECT_EQ(1, callbackCount);
    EXPECT_EQ(1, server.requests().size());
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QStringLiteral("CalDAV account registration cancelled."), result.errorMessage);
    EXPECT_FALSE(jobManager.containsAccount(account->accountID()));
}

TEST(CalDavCancellation, ReadOnlySyncNotifiesCallbackOnce)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(1000);

    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = server.url();
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");

    int callbackCount = 0;
    DCalDavReadOnlySync::Result result;
    synchronizer.start(request, [&](const DCalDavReadOnlySync::Result &syncResult) {
        ++callbackCount;
        result = syncResult;
    });
    ASSERT_TRUE(waitForMockRequestCount(server, 1));
    synchronizer.cancel();
    processEventsFor(1200);

    EXPECT_EQ(1, callbackCount);
    EXPECT_EQ(1, server.requests().size());
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QStringLiteral("CalDAV validation cancelled."), result.errorMessage);
    EXPECT_EQ(DCalDavValidationError::Other, result.validationError);
}

TEST(CalDavCancellation, IncrementalSyncNotifiesCallbackOnce)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(1000);

    DCalDavIncrementalSync synchronizer;
    DCalDavIncrementalSync::Request request;
    request.calendarUrl = server.url().resolved(QUrl(QStringLiteral("/calendars/user/")));
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.calendarId = QStringLiteral("calendar");

    int callbackCount = 0;
    DCalDavIncrementalSync::Result result;
    synchronizer.start(request, [&](const DCalDavIncrementalSync::Result &syncResult) {
        ++callbackCount;
        result = syncResult;
    });
    ASSERT_TRUE(waitForMockRequestCount(server, 1));
    synchronizer.cancel();
    processEventsFor(1200);

    EXPECT_EQ(1, callbackCount);
    EXPECT_EQ(1, server.requests().size());
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QStringLiteral("CalDAV calendar synchronization cancelled."), result.errorMessage);
    EXPECT_EQ(DCalDavErrorCode::Unknown, result.failureCode);
}

TEST(CalDavCancellation, AccountSyncNotifiesCallbackOnce)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(1000);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("cancel-account"));

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = server.url().toString();
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavAccountInfo(accountInfo));

    DCalDavCalendarInfo calendar;
    calendar.accountId = account->accountID();
    calendar.calendarId = QStringLiteral("calendar");
    calendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/"))).toString();
    calendar.scheduleTypeID = QStringLiteral("schedule-type");
    calendar.initialSyncCompleted = true;
    calendar.syncToken = QStringLiteral("old-token");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(calendar));

    DCalDavAccountSync::Request request;
    request.accountID = account->accountID();
    request.username = accountInfo.username;
    request.password = QStringLiteral("password");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;
    request.calendars.append({calendar, calendar.scheduleTypeID});

    DCalDavAccountSync synchronizer;
    int callbackCount = 0;
    DCalDavAccountSync::Result result;
    synchronizer.start(request, [&](const DCalDavAccountSync::Result &syncResult) {
        ++callbackCount;
        result = syncResult;
    });
    ASSERT_TRUE(waitForMockRequestCount(server, 1));
    synchronizer.cancel();
    processEventsFor(1200);

    EXPECT_EQ(1, callbackCount);
    EXPECT_EQ(1, server.requests().size());
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QStringLiteral("CalDAV synchronization cancelled."), result.errorMessage);
}

TEST(CalDavCancellation, OutboxProcessorNotifiesCallbackOnce)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseDelay(1000);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("cancel-outbox-account"));

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = server.url().toString();
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavAccountInfo(accountInfo));

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Outbox cancellation calendar"));
    type->setDisplayName(QStringLiteral("Outbox cancellation calendar"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::Write);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setUid(QStringLiteral("cancel-outbox-event"));
    schedule->setScheduleTypeID(type->typeID());
    schedule->setSummary(QStringLiteral("Outbox cancellation event"));
    schedule->setDtStart(QDateTime(QDate(2026, 9, 20), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 9, 20), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.accountId = account->accountID();
    calendar.calendarId = QStringLiteral("cancel-outbox-calendar");
    calendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/"))).toString();
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
    calendar.privilegesKnown = true;
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(calendar));

    DCalDavOutboxItem item;
    item.operationID = QStringLiteral("cancel-outbox-operation");
    item.accountID = account->accountID();
    item.localScheduleID = schedule->uid();
    item.operationType = DCalDavOutboxItem::CreateOperation;
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavOutboxItem(item));

    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = account->accountID();
    request.username = accountInfo.username;
    request.password = QStringLiteral("password");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;

    int callbackCount = 0;
    DCalDavOutboxProcessor::Result result;
    processor.start(request, [&](const DCalDavOutboxProcessor::Result &processorResult) {
        ++callbackCount;
        result = processorResult;
    });
    ASSERT_TRUE(waitForMockRequestCount(server, 1));
    processor.cancel();
    processEventsFor(1200);

    EXPECT_EQ(1, callbackCount);
    EXPECT_EQ(1, server.requests().size());
    EXPECT_FALSE(result.success);
    EXPECT_EQ(QStringLiteral("CalDAV Outbox processing cancelled."), result.errorMessage);
}

TEST(CalDavIntegration, ClassifiesAuthenticationFailure)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatus(401);

    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = server.url();
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");

    DCalDavReadOnlySync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavReadOnlySync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavValidationError::AuthenticationFailed, result.validationError);
    EXPECT_EQ(DCalDavTransport::AuthenticationFailed, result.failureResponse.error);
    EXPECT_EQ(1, server.connectionCount());
}

TEST(CalDavIntegration, ClassifiesUnsupportedCalDavServer)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatus(404);

    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = server.url();
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");

    DCalDavReadOnlySync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavReadOnlySync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavValidationError::UnsupportedCalDav, result.validationError);
    EXPECT_TRUE(result.errorMessage.isEmpty());
}

TEST(CalDavIntegration, ClassifiesMalformedDiscoveryAsParseError)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    const QByteArray malformedResponse =
        "<d:multistatus xmlns:d=\"DAV:\"><d:response>";
    server.setResponseBodyForTarget("/.well-known/caldav", malformedResponse);
    server.setResponseBodyForTarget("/", malformedResponse);

    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = server.url();
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");

    DCalDavReadOnlySync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavReadOnlySync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavValidationError::ParseError, result.validationError);
    EXPECT_EQ(2, server.connectionCount());
}

TEST(CalDavIntegration, ClassifiesNoReadableCalendarAsUnsupportedCalDav)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseBodyForTarget(
        "/calendars/",
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\" xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "<d:response><d:href>/calendars/user/</d:href><d:propstat><d:prop>"
        "<d:displayname>Not a calendar</d:displayname>"
        "</d:prop><d:status>HTTP/1.1 200 OK</d:status></d:propstat>"
        "</d:response></d:multistatus>");

    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = server.url();
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");

    DCalDavReadOnlySync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavReadOnlySync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavValidationError::UnsupportedCalDav, result.validationError);
    EXPECT_TRUE(result.errorMessage.isEmpty());
}

TEST(CalDavIntegration, DiscoversAndReadsFromLocalHttpsServer)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));

    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = server.url();
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");

    DCalDavReadOnlySync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavReadOnlySync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    ASSERT_TRUE(result.success);
    EXPECT_EQ(DCalDavValidationError::NoError, result.validationError);
    ASSERT_EQ(1, result.schedules.size());
    EXPECT_EQ(QStringLiteral("mock-event-1"), result.schedules.first()->uid());
    ASSERT_EQ(4, server.requests().size());
    EXPECT_EQ(QByteArray("PROPFIND"), server.requests().at(0).method);
    EXPECT_EQ(QByteArray("/.well-known/caldav"), server.requests().at(0).target);
    EXPECT_EQ(QByteArray("PROPFIND"), server.requests().at(1).method);
    EXPECT_EQ(QByteArray("/principal/"), server.requests().at(1).target);
    EXPECT_EQ(QByteArray("PROPFIND"), server.requests().at(2).method);
    EXPECT_EQ(QByteArray("/calendars/"), server.requests().at(2).target);
    EXPECT_EQ(QByteArray("REPORT"), server.requests().at(3).method);
    EXPECT_EQ(QByteArray("/calendars/user/"), server.requests().at(3).target);
}

TEST(CalDavIntegration, FetchesMetadataOnlyCalendarQueryWithMultiGet)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setCalendarQueryReturnsCalendarData(false);

    DCalDavIncrementalSync synchronizer;
    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

    DCalDavIncrementalSync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavIncrementalSync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(1, result.schedules.size());
    EXPECT_EQ(QStringLiteral("mock-event-1"), result.schedules.first()->uid());
    ASSERT_EQ(2, server.requests().size());
    EXPECT_EQ(QByteArray("REPORT"), server.requests().at(0).method);
    EXPECT_TRUE(server.requests().at(0).body.contains("calendar-query"));
    EXPECT_TRUE(server.requests().at(0).body.contains("time-range"));
    EXPECT_EQ(QByteArray("REPORT"), server.requests().at(1).method);
    EXPECT_TRUE(server.requests().at(1).body.contains("calendar-multiget"));
    EXPECT_FALSE(server.requests().at(1).body.contains("https://localhost"));
}

TEST(CalDavIntegration, DetectsRemoteDeletionWithoutSyncTokenOutsideQueryRange)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));

    DCalDavIncrementalSync synchronizer;
    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.syncToken = QStringLiteral("dde-calendar:no-sync-token");
    request.initialSyncCompleted = true;
    request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

    DCalDavEventMappingInfo deletedMapping;
    deletedMapping.accountID = QStringLiteral("account");
    deletedMapping.calendarID = QStringLiteral("calendar");
    deletedMapping.localScheduleID = QStringLiteral("local-deleted-event");
    deletedMapping.uid = QStringLiteral("remote-deleted-event");
    deletedMapping.href = calendarUrl.resolved(QUrl(QStringLiteral("deleted-event.ics"))).toString();
    deletedMapping.etag = QStringLiteral("\"deleted-etag\"");
    request.existingMappings.append(deletedMapping);

    DCalDavEventMappingInfo existingMapping;
    existingMapping.accountID = QStringLiteral("account");
    existingMapping.calendarID = QStringLiteral("calendar");
    existingMapping.localScheduleID = QStringLiteral("local-existing-event");
    existingMapping.uid = QStringLiteral("mock-event-1");
    existingMapping.href = calendarUrl.resolved(QUrl(QStringLiteral("mock-event-1.ics"))).toString();
    existingMapping.etag = QStringLiteral("\"mock-etag-1\"");
    request.existingMappings.append(existingMapping);

    DCalDavIncrementalSync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavIncrementalSync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(2, server.requests().size());
    EXPECT_TRUE(server.requests().at(0).body.contains("getetag"));
    EXPECT_TRUE(server.requests().at(0).body.contains("<c:comp-filter name=\"VEVENT\"/>"));
    EXPECT_TRUE(server.requests().at(1).body.contains("time-range"));

    bool deletedEventFound = false;
    for (const DCalDavCalendarQuery::RemoteEvent &event : result.remoteEvents) {
        if (event.deleted && event.href == deletedMapping.href) {
            deletedEventFound = true;
            break;
        }
    }
    EXPECT_TRUE(deletedEventFound);
    EXPECT_TRUE(result.schedules.isEmpty());

    DCalDavEventReconciler::ActionList actions;
    QString errorMessage;
    ASSERT_TRUE(DCalDavEventReconciler::buildActions(
        result.remoteEvents, request.existingMappings, actions, &errorMessage));
    ASSERT_EQ(1, actions.size());
    EXPECT_EQ(DCalDavEventReconciler::DeleteAction, actions.first().type);
    EXPECT_EQ(deletedMapping.localScheduleID, actions.first().existingMapping.localScheduleID);
}

TEST(CalDavIntegration, DetectsRemoteDeletionAfterInvalidSyncTokenFallback)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setInvalidSyncTokenOnce(true);

    DCalDavIncrementalSync synchronizer;
    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.syncToken = QStringLiteral("old-token");
    request.initialSyncCompleted = true;
    request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

    DCalDavEventMappingInfo deletedMapping;
    deletedMapping.accountID = QStringLiteral("account");
    deletedMapping.calendarID = QStringLiteral("calendar");
    deletedMapping.localScheduleID = QStringLiteral("local-deleted-event");
    deletedMapping.uid = QStringLiteral("remote-deleted-event");
    deletedMapping.href = calendarUrl.resolved(QUrl(QStringLiteral("deleted-event.ics"))).toString();
    deletedMapping.etag = QStringLiteral("\"deleted-etag\"");
    request.existingMappings.append(deletedMapping);

    DCalDavEventMappingInfo existingMapping;
    existingMapping.accountID = QStringLiteral("account");
    existingMapping.calendarID = QStringLiteral("calendar");
    existingMapping.localScheduleID = QStringLiteral("local-existing-event");
    existingMapping.uid = QStringLiteral("mock-event-1");
    existingMapping.href = calendarUrl.resolved(QUrl(QStringLiteral("mock-event-1.ics"))).toString();
    existingMapping.etag = QStringLiteral("\"mock-etag-1\"");
    request.existingMappings.append(existingMapping);

    DCalDavIncrementalSync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavIncrementalSync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.usedFullRangeFallback);
    ASSERT_EQ(3, server.requests().size());
    EXPECT_TRUE(server.requests().at(0).body.contains("sync-collection"));
    EXPECT_TRUE(server.requests().at(1).body.contains("getetag"));
    EXPECT_FALSE(server.requests().at(1).body.contains("time-range"));
    EXPECT_TRUE(server.requests().at(2).body.contains("time-range"));

    const auto deletedEvent = std::find_if(
        result.remoteEvents.cbegin(), result.remoteEvents.cend(),
        [&deletedMapping](const DCalDavCalendarQuery::RemoteEvent &event) {
            return event.deleted && event.href == deletedMapping.href;
        });
    EXPECT_NE(result.remoteEvents.cend(), deletedEvent);
    EXPECT_TRUE(result.schedules.isEmpty());
}

TEST(CalDavIntegration, FallsBackToGetWhenCalendarMultiGetIsForbidden)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setCalendarQueryReturnsCalendarData(false);
    server.setCalendarMultiGetResponseStatus(403);
    server.setResponseStatusForMethod("GET", 200);

    DCalDavIncrementalSync synchronizer;
    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

    DCalDavIncrementalSync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavIncrementalSync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(1, result.schedules.size());
    EXPECT_EQ(QStringLiteral("mock-event-1"), result.schedules.first()->uid());
    EXPECT_EQ(DCalDavTransport::NoError, result.failureResponse.error);
    ASSERT_EQ(3, server.requests().size());
    EXPECT_EQ(QByteArray("REPORT"), server.requests().at(0).method);
    EXPECT_TRUE(server.requests().at(0).body.contains("calendar-query"));
    EXPECT_TRUE(server.requests().at(0).body.contains("time-range"));
    EXPECT_EQ(QByteArray("REPORT"), server.requests().at(1).method);
    EXPECT_TRUE(server.requests().at(1).body.contains("calendar-multiget"));
    EXPECT_FALSE(server.requests().at(1).body.contains("https://localhost"));
    EXPECT_EQ(QByteArray("GET"), server.requests().at(2).method);
    EXPECT_EQ(QByteArray("/calendars/user/mock-event-1.ics"), server.requests().at(2).target);
}

TEST(CalDavIntegration, SkipsMalformedRemoteEventWithoutFailingSync)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setCalendarQueryReturnsCalendarData(false);
    server.setCalendarMultiGetResponseStatus(403);
    server.setResponseStatusForMethod("GET", 200);
    server.setResponseBodyForTarget(
        "/calendars/user/mock-event-1.ics",
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\n"
        "DTSTART:20260831T120000Z\r\nDTEND:20260831T130000Z\r\n"
        "SUMMARY:Missing UID\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n");

    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.calendarId = QStringLiteral("calendar");
    request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

    DCalDavIncrementalSync::Result result;
    ASSERT_TRUE(runIncrementalSync(request, &result));
    EXPECT_TRUE(result.success);
    EXPECT_EQ(DCalDavErrorCode::NoError, result.failureCode);
    EXPECT_TRUE(result.errorMessage.isEmpty());
    EXPECT_TRUE(result.schedules.isEmpty());
    EXPECT_TRUE(result.remoteEvents.isEmpty());
    ASSERT_EQ(1, result.skippedResources.size());
    EXPECT_EQ(QStringLiteral("calendar"), result.skippedResources.first().calendarId);
    EXPECT_EQ(calendarUrl.resolved(QUrl(QStringLiteral("mock-event-1.ics"))).toString(),
              result.skippedResources.first().href);
    EXPECT_EQ(QStringLiteral("missing-vevent-uid"), result.skippedResources.first().reason);
    EXPECT_TRUE(result.clearedSkippedResourceHrefs.isEmpty());
    ASSERT_EQ(3, server.requests().size());
    EXPECT_EQ(QByteArray("GET"), server.requests().last().method);
}

TEST(CalDavIntegration, SkipsRemoteEventsWithoutVeventOrWithInvalidIcs)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    const QList<QPair<QByteArray, QString>> malformedEvents = {
        {QByteArrayLiteral(
             "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VTODO\r\n"
             "UID:todo-1\r\nSUMMARY:Not an event\r\nEND:VTODO\r\nEND:VCALENDAR\r\n"),
         QStringLiteral("missing-vevent")},
        {QByteArrayLiteral(
             "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\n"
             "UID:invalid-ics\r\nDTSTART:20260831T120000Z\r\n"
             "SUMMARY:Invalid event\r\n"),
         QStringLiteral("event-mapping-failed")},
    };

    for (const auto &malformedEvent : malformedEvents) {
        MockCalDavServer server;
        ASSERT_TRUE(server.start(certificate, privateKey));
        server.setCalendarQueryReturnsCalendarData(false);
        server.setCalendarMultiGetResponseStatus(403);
        server.setResponseStatusForMethod("GET", 200);
        server.setResponseBodyForTarget(
            "/calendars/user/mock-event-1.ics", malformedEvent.first);

        DCalDavIncrementalSync::Request request;
        QUrl calendarUrl = server.url();
        calendarUrl.setPath(QStringLiteral("/calendars/user/"));
        request.calendarUrl = calendarUrl;
        request.username = QStringLiteral("user");
        request.password = QStringLiteral("password");
        request.calendarId = QStringLiteral("calendar");
        request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

        DCalDavIncrementalSync::Result result;
        ASSERT_TRUE(runIncrementalSync(request, &result));
        EXPECT_TRUE(result.success);
        EXPECT_EQ(DCalDavErrorCode::NoError, result.failureCode);
        EXPECT_TRUE(result.schedules.isEmpty());
        EXPECT_TRUE(result.remoteEvents.isEmpty());
        ASSERT_EQ(1, result.skippedResources.size());
        EXPECT_EQ(malformedEvent.second, result.skippedResources.first().reason);
    }
}

TEST(CalDavIntegration, PersistsAndClearsMalformedResourceAcrossAccountSync)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setCalendarQueryReturnsCalendarData(false);
    server.setCalendarMultiGetResponseStatus(403);
    server.setResponseStatusForMethod("GET", 200);
    server.setResponseBodyForTarget(
        "/calendars/user/mock-event-1.ics",
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\n"
        "DTSTART:20260831T120000Z\r\nDTEND:20260831T130000Z\r\n"
        "SUMMARY:Missing UID\r\nEND:VEVENT\r\nEND:VCALENDAR\r\n");

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("malformed-resource-account"));

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = server.url().toString();
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavAccountInfo(accountInfo));

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Malformed event calendar"));
    type->setDisplayName(QStringLiteral("Malformed event calendar"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::Read);
    type->setShowState(DScheduleType::Show);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("calendar");
    calendar.accountId = account->accountID();
    calendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/"))).toString();
    calendar.displayName = QStringLiteral("Mock Calendar");
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege;
    calendar.privilegesKnown = true;
    calendar.initialSyncCompleted = true;
    calendar.syncToken = QStringLiteral("old-token");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(calendar));

    DCalDavAccountSync::Request request;
    request.accountID = account->accountID();
    request.username = accountInfo.username;
    request.password = QStringLiteral("password");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;
    request.calendars.append({calendar, type->typeID()});

    auto runAccountSync = [&](DCalDavAccountSync::Result *result) {
        DCalDavAccountSync synchronizer;
        bool callbackCalled = false;
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        synchronizer.start(request, [&](const DCalDavAccountSync::Result &syncResult) {
            *result = syncResult;
            callbackCalled = true;
            loop.quit();
        });
        timeout.start(5000);
        if (!callbackCalled) {
            loop.exec();
        }
        return callbackCalled;
    };

    DCalDavAccountSync::Result firstResult;
    ASSERT_TRUE(runAccountSync(&firstResult));
    ASSERT_TRUE(firstResult.success) << firstResult.errorMessage.toStdString();
    EXPECT_EQ(0, firstResult.createdCount);

    DCalDavSkippedResource::List skippedResources;
    ASSERT_TRUE(accountManagerDatabase.getCalDavSkippedResources(
        account->accountID(), calendar.calendarId, skippedResources));
    ASSERT_EQ(1, skippedResources.size());
    EXPECT_EQ(QStringLiteral("missing-vevent-uid"), skippedResources.first().reason);
    EXPECT_EQ(QStringLiteral("mock-token-1"),
              accountManagerDatabase.getCalDavCalendarByIDIncludingDisabled(
                  account->accountID(), calendar.calendarId).syncToken);

    server.setResponseBodyForTarget(
        "/calendars/user/mock-event-1.ics",
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\nBEGIN:VEVENT\r\n"
        "UID:mock-event-1\r\nDTSTART:20260831T120000Z\r\n"
        "DTEND:20260831T130000Z\r\nSUMMARY:Recovered event\r\n"
        "END:VEVENT\r\nEND:VCALENDAR\r\n");

    DCalDavAccountSync::Result secondResult;
    ASSERT_TRUE(runAccountSync(&secondResult));
    ASSERT_TRUE(secondResult.success) << secondResult.errorMessage.toStdString();
    EXPECT_EQ(1, secondResult.createdCount);

    skippedResources.clear();
    ASSERT_TRUE(accountManagerDatabase.getCalDavSkippedResources(
        account->accountID(), calendar.calendarId, skippedResources));
    EXPECT_TRUE(skippedResources.isEmpty());

    const DCalDavEventMappingInfo::List mappings = accountManagerDatabase
        .getCalDavEventMappingList(account->accountID(), calendar.calendarId);
    ASSERT_EQ(1, mappings.size());
    EXPECT_EQ(QStringLiteral("mock-event-1"), mappings.first().uid);
    const DSchedule::Ptr recoveredSchedule = localDatabase.getScheduleByScheduleID(
        mappings.first().localScheduleID);
    ASSERT_TRUE(recoveredSchedule);
    EXPECT_EQ(QStringLiteral("Recovered event"), recoveredSchedule->summary());
}

TEST(CalDavIntegration, TreatsCalendarMultiGetMissingResourceAsDeleted)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setCalendarQueryReturnsCalendarData(false);
    server.setCalendarMultiGetResponseBody(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:multistatus xmlns:d=\"DAV:\">"
        "<d:response><d:href>/calendars/user/mock-event-1.ics</d:href>"
        "<d:propstat><d:prop/><d:status>HTTP/1.1 404 Not Found</d:status>"
        "</d:propstat></d:response></d:multistatus>");

    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

    DCalDavEventMappingInfo mapping;
    mapping.accountID = QStringLiteral("account");
    mapping.calendarID = QStringLiteral("calendar");
    mapping.localScheduleID = QStringLiteral("local-event");
    mapping.uid = QStringLiteral("mock-event-1");
    mapping.href = calendarUrl.resolved(QUrl(QStringLiteral("mock-event-1.ics"))).toString();
    mapping.etag = QStringLiteral("\"old-etag\"");
    request.existingMappings.append(mapping);

    DCalDavIncrementalSync::Result result;
    ASSERT_TRUE(runIncrementalSync(request, &result));
    ASSERT_TRUE(result.success);
    ASSERT_EQ(1, result.remoteEvents.size());
    EXPECT_TRUE(result.remoteEvents.first().deleted);
    EXPECT_EQ(mapping.href, result.remoteEvents.first().href);
    EXPECT_EQ(mapping.uid, result.remoteEvents.first().uid);
    EXPECT_TRUE(result.schedules.isEmpty());
    ASSERT_EQ(2, server.requests().size());
    EXPECT_TRUE(server.requests().at(1).body.contains("calendar-multiget"));
}

TEST(CalDavIntegration, TreatsGetFallbackNotFoundAsDeleted)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setCalendarQueryReturnsCalendarData(false);
    server.setCalendarMultiGetResponseStatus(403);
    server.setResponseStatusForTarget("/calendars/user/mock-event-1.ics", 404);

    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.referenceTime = QDateTime(QDate(2026, 8, 31), QTime(0, 0), Qt::UTC);

    DCalDavEventMappingInfo mapping;
    mapping.accountID = QStringLiteral("account");
    mapping.calendarID = QStringLiteral("calendar");
    mapping.localScheduleID = QStringLiteral("local-event");
    mapping.uid = QStringLiteral("mock-event-1");
    mapping.href = calendarUrl.resolved(QUrl(QStringLiteral("mock-event-1.ics"))).toString();
    mapping.etag = QStringLiteral("\"old-etag\"");
    request.existingMappings.append(mapping);

    DCalDavIncrementalSync::Result result;
    ASSERT_TRUE(runIncrementalSync(request, &result));
    ASSERT_TRUE(result.success);
    ASSERT_EQ(1, result.remoteEvents.size());
    EXPECT_TRUE(result.remoteEvents.first().deleted);
    EXPECT_EQ(mapping.href, result.remoteEvents.first().href);
    EXPECT_EQ(mapping.uid, result.remoteEvents.first().uid);
    EXPECT_TRUE(result.schedules.isEmpty());
    ASSERT_EQ(3, server.requests().size());
    EXPECT_EQ(QByteArray("GET"), server.requests().last().method);
}

TEST(CalDavIntegration, TreatsMissingCalendarCollectionAsAllResourcesDeleted)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/calendars/user/", 404);

    DCalDavIncrementalSync::Request request;
    QUrl calendarUrl = server.url();
    calendarUrl.setPath(QStringLiteral("/calendars/user/"));
    request.calendarUrl = calendarUrl;
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");

    DCalDavEventMappingInfo mapping;
    mapping.accountID = QStringLiteral("account");
    mapping.calendarID = QStringLiteral("calendar");
    mapping.localScheduleID = QStringLiteral("local-event");
    mapping.uid = QStringLiteral("remote-event");
    mapping.href = calendarUrl.resolved(QUrl(QStringLiteral("remote-event.ics"))).toString();
    request.existingMappings.append(mapping);

    DCalDavIncrementalSync::Result result;
    ASSERT_TRUE(runIncrementalSync(request, &result));
    ASSERT_TRUE(result.success);
    ASSERT_EQ(1, result.remoteEvents.size());
    EXPECT_TRUE(result.remoteEvents.first().deleted);
    EXPECT_EQ(mapping.href, result.remoteEvents.first().href);
    EXPECT_TRUE(result.schedules.isEmpty());
    ASSERT_EQ(1, server.requests().size());
}

TEST(CalDavIntegration, DropsCalendarThatDisappearsDuringDiscovery)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/calendars/user/", 404);

    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = server.url();
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.requireReadableCalendar = false;

    DCalDavReadOnlySync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    synchronizer.start(request, [&](const DCalDavReadOnlySync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.discovery.calendarCollections.isEmpty());
    ASSERT_EQ(4, server.requests().size());
    EXPECT_EQ(QByteArray("REPORT"), server.requests().last().method);
    EXPECT_EQ(QByteArray("/calendars/user/"), server.requests().last().target);
}

TEST(CalDavIntegration, KeepsAccountSuccessfulWhenOneCalendarIsForbidden)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatusForTarget("/calendars/user/forbidden/", 403);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("partial-access-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = server.url().toString();
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavAccountInfo(accountInfo));

    DTypeColor allowedColor;
    allowedColor.setColorID(DDataBase::createUuid());
    allowedColor.setColorCode(QStringLiteral("#4381D5"));
    allowedColor.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(allowedColor));

    DScheduleType::Ptr allowedType(new DScheduleType(account->accountID()));
    allowedType->setTypeName(QStringLiteral("Allowed calendar"));
    allowedType->setDisplayName(QStringLiteral("Allowed calendar"));
    allowedType->setTypeColor(allowedColor);
    allowedType->setPrivilege(DScheduleType::Read);
    allowedType->setShowState(DScheduleType::Show);
    allowedType->setDtCreate(QDateTime::currentDateTime());
    ASSERT_FALSE(localDatabase.createScheduleType(allowedType).isEmpty());

    DCalDavCalendarInfo allowedCalendar;
    allowedCalendar.calendarId = QStringLiteral("allowed-calendar");
    allowedCalendar.accountId = account->accountID();
    allowedCalendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/allowed/"))).toString();
    allowedCalendar.displayName = QStringLiteral("Allowed calendar");
    allowedCalendar.scheduleTypeID = allowedType->typeID();
    allowedCalendar.privileges = DCalDavXmlReader::ReadPrivilege;
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(allowedCalendar));

    DCalDavCalendarInfo forbiddenCalendar;
    forbiddenCalendar.calendarId = QStringLiteral("forbidden-calendar");
    forbiddenCalendar.accountId = account->accountID();
    forbiddenCalendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/forbidden/"))).toString();
    forbiddenCalendar.displayName = QStringLiteral("Forbidden calendar");
    forbiddenCalendar.scheduleTypeID = allowedType->typeID();
    forbiddenCalendar.privileges = DCalDavXmlReader::ReadPrivilege;
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(forbiddenCalendar));

    DCalDavAccountSync synchronizer;
    DCalDavAccountSync::Request request;
    request.accountID = account->accountID();
    request.username = accountInfo.username;
    request.password = QStringLiteral("password");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;
    request.calendars.append({allowedCalendar, allowedType->typeID()});
    request.calendars.append({forbiddenCalendar, allowedType->typeID()});

    DCalDavAccountSync::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    synchronizer.start(request, [&](const DCalDavAccountSync::Result &syncResult) {
        result = syncResult;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_TRUE(result.success) << "error=" << result.errorMessage.toStdString();
    EXPECT_EQ(DCalDavTransport::NoError, result.failureResponse.error);
    DCalDavAccountStatus::List statusList;
    ASSERT_TRUE(DCalDavAccountStatus::fromJsonListString(
        statusList, accountManagerDatabase.getCalDavAccountStatusList()));
    ASSERT_EQ(1, statusList.size());
    EXPECT_EQ(DCalDavSyncStatus::Succeeded, statusList.first().syncStatus);
    EXPECT_TRUE(statusList.first().failureReason.isEmpty());
    const DCalDavCalendarInfo::List calendars = accountManagerDatabase.getCalDavCalendarList(
        account->accountID());
    ASSERT_EQ(2, calendars.size());
    const auto inaccessible = std::find_if(calendars.cbegin(), calendars.cend(),
                                           [](const DCalDavCalendarInfo &calendar) {
                                               return calendar.calendarId == QStringLiteral("forbidden-calendar");
                                           });
    ASSERT_NE(calendars.cend(), inaccessible);
    EXPECT_FALSE(inaccessible->enabled);
}

TEST(CalDavIntegration, SendsWriteRequestAndClassifiesServerResponses)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatus(204);
    server.setResponseEtag("\\\"mock-etag-2\\\"");

    DCalDavTransport transport;
    DCalDavTransport::Request request;
    request.url = server.url().resolved(QUrl(QStringLiteral("/calendars/user/event.ics")));
    request.method = "PUT";
    request.contentType = "text/calendar; charset=utf-8";
    request.username = QStringLiteral("user");
    request.password = QStringLiteral("password");
    request.headers.insert("If-Match", "\\\"mock-etag-1\\\"");
    request.body = "BEGIN:VCALENDAR\\r\\nEND:VCALENDAR\\r\\n";

    DCalDavTransport::Response response;
    bool callbackCalled = false;
    QEventLoop loop;
    transport.send(request, [&](const DCalDavTransport::Response &transportResponse) {
        response = transportResponse;
        callbackCalled = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_EQ(204, response.httpStatus);
    EXPECT_EQ(DCalDavTransport::NoError, response.error);
    EXPECT_EQ(QByteArray("\\\"mock-etag-2\\\""), response.etag);
    ASSERT_EQ(1, server.requests().size());
    EXPECT_EQ(QByteArray("PUT"), server.requests().first().method);
    EXPECT_EQ(QByteArray("/calendars/user/event.ics"), server.requests().first().target);
    EXPECT_EQ(request.body, server.requests().first().body);

    const QList<int> statuses = {401, 403, 409, 412, 429, 500, 502, 503, 504};
    const QList<DCalDavTransport::Error> errors = {
        DCalDavTransport::AuthenticationFailed,
        DCalDavTransport::PermissionDenied,
        DCalDavTransport::NoError,
        DCalDavTransport::NoError,
        DCalDavTransport::RateLimited,
        DCalDavTransport::ServerUnavailable,
        DCalDavTransport::ServerUnavailable,
        DCalDavTransport::ServerUnavailable,
        DCalDavTransport::ServerUnavailable,
    };
    for (int i = 0; i < statuses.size(); ++i) {
        server.setResponseStatus(statuses.at(i));
        response = DCalDavTransport::Response();
        callbackCalled = false;
        transport.send(request, [&](const DCalDavTransport::Response &transportResponse) {
            response = transportResponse;
            callbackCalled = true;
            loop.quit();
        });
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        if (!callbackCalled) {
            loop.exec();
        }
        ASSERT_TRUE(callbackCalled);
        EXPECT_EQ(statuses.at(i), response.httpStatus);
        EXPECT_EQ(errors.at(i), response.error);
    }
}

TEST(CalDavIntegration, RestoresCalendarWhenRemoteDeleteIsRejected)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatus(204);
    server.setResponseStatusForTarget(QByteArray("/calendars/user/rejected/"), 403);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("rejected-calendar-delete-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = server.url().toString();
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/rejected_delete");
    ASSERT_TRUE(managerDatabase.upsertCalDavAccountInfo(accountInfo));

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Rejected calendar"));
    type->setDisplayName(QStringLiteral("Rejected calendar"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setScheduleTypeID(type->typeID());
    schedule->setSummary(QStringLiteral("Rejected calendar event"));
    schedule->setDtStart(QDateTime(QDate(2026, 9, 16), QTime(12, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 9, 16), QTime(13, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("rejected-calendar");
    calendar.accountId = account->accountID();
    calendar.href = server.url().resolved(
        QUrl(QStringLiteral("/calendars/user/rejected/"))).toString();
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
    calendar.enabled = false;
    ASSERT_TRUE(managerDatabase.upsertCalDavCalendar(calendar));

    ASSERT_TRUE(localDatabase.deleteSchedulesByScheduleTypeID(type->typeID(), 0));
    ASSERT_TRUE(localDatabase.deleteScheduleTypeByID(type->typeID(), 0));
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueueCalendarDelete(
        &managerDatabase, account->accountID(), calendar));

    QJsonArray recoveryTypeIDs;
    recoveryTypeIDs.append(type->typeID());
    DCalDavRecoveryItem recovery;
    recovery.accountID = account->accountID();
    recovery.localScheduleID = type->typeID();
    recovery.operationType = DCalDavRecoveryItem::DeleteCalendarOperation;
    recovery.scheduleIcs = QStringLiteral("CALDAV-CALENDAR-DELETE");
    recovery.calendarID = calendar.calendarId;
    recovery.href = calendar.href;
    recovery.originalIcs = QString::fromUtf8(
        QJsonDocument(recoveryTypeIDs).toJson(QJsonDocument::Compact));
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(recovery));

    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = account->accountID();
    request.username = accountInfo.username;
    request.password = QStringLiteral("password");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &managerDatabase;

    DCalDavOutboxProcessor::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    processor.start(request, [&](const DCalDavOutboxProcessor::Result &processorResult) {
        result = processorResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_TRUE(result.success) << result.errorMessage.toStdString();
    EXPECT_EQ(1, result.restoredCalendarCount);
    ASSERT_EQ(1, server.requests().size());
    EXPECT_EQ(QByteArray("DELETE"), server.requests().first().method);
    EXPECT_FALSE(localDatabase.getScheduleTypeByID(type->typeID()).isNull());
    EXPECT_FALSE(localDatabase.isScheduleDeletedByScheduleID(schedule->uid()));
    EXPECT_TRUE(managerDatabase.getCalDavCalendarByIDIncludingDisabled(
                    account->accountID(), calendar.calendarId).enabled);
    EXPECT_TRUE(managerDatabase.getCalDavOutboxItem(
                    account->accountID(), type->typeID()).operationID.isEmpty());
    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
}

TEST(CalDavIntegration, KeepsLocalScheduleWhenCachedReadOnlyCalendarReturnsForbidden)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatus(403);

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("read-only-cache-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = server.url().toString();
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/read_only_cache");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavAccountInfo(accountInfo));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Read-only cached calendar"));
    type->setDisplayName(QStringLiteral("Read-only cached calendar"));
    type->setPrivilege(DScheduleType::User);
    type->setShowState(DScheduleType::Show);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("read-only-cached-calendar");
    calendar.accountId = account->accountID();
    calendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/"))).toString();
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege;
    calendar.privilegesKnown = true;
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(calendar));

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setScheduleTypeID(type->typeID());
    schedule->setSummary(QStringLiteral("Local event retained after 403"));
    schedule->setDtStart(QDateTime(QDate(2026, 9, 22), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 9, 22), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());
    const QString scheduleID = schedule->uid();

    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));
    ASSERT_TRUE(localDatabase.scheduleExistsByScheduleID(scheduleID));

    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = account->accountID();
    request.username = accountInfo.username;
    request.password = QStringLiteral("password");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &accountManagerDatabase;

    DCalDavOutboxProcessor::Result result;
    bool callbackCalled = false;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    processor.start(request, [&](const DCalDavOutboxProcessor::Result &processorResult) {
        result = processorResult;
        callbackCalled = true;
        loop.quit();
    });
    timeout.start(5000);
    if (!callbackCalled) {
        loop.exec();
    }

    ASSERT_TRUE(callbackCalled);
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavTransport::PermissionDenied, result.failureResponse.error);
    EXPECT_EQ(DCalDavScheduleCreateError::PermissionDenied, result.createFailure);

    const DSchedule::Ptr retainedSchedule = localDatabase.getScheduleByScheduleID(scheduleID);
    ASSERT_TRUE(retainedSchedule);
    EXPECT_EQ(QStringLiteral("Local event retained after 403"), retainedSchedule->summary());

    const DCalDavOutboxItem outbox = accountManagerDatabase.getCalDavOutboxItem(
        account->accountID(), scheduleID);
    EXPECT_EQ(DCalDavOutboxItem::PermissionFailure, outbox.failureType);
    EXPECT_FALSE(outbox.operationID.isEmpty());
    ASSERT_FALSE(server.requests().isEmpty());
    EXPECT_EQ(QByteArray("PUT"), server.requests().last().method);
}

TEST(CalDavIntegration, ProcessesOutboxCreateModifyConflictRetryAndDelete)
{
    if (!QSslSocket::supportsSsl()) {
        GTEST_SKIP() << "Qt SSL backend is unavailable";
    }

    QByteArray certificate;
    QByteArray privateKey;
    ASSERT_TRUE(createMockTlsCredentials(certificate, privateKey));
    trustMockCertificate(certificate);

    MockCalDavServer server;
    ASSERT_TRUE(server.start(certificate, privateKey));
    server.setResponseStatus(204);
    server.setResponseEtag("\"mock-etag-1\"");

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount);
    account->setAccountID(QStringLiteral("mock-account"));
    account->setAccountType(DAccount::Account_CalDav);

    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase accountManagerDatabase;
    accountManagerDatabase.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    accountManagerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    accountManagerDatabase.initDBData();

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = server.url().toString();
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavAccountInfo(accountInfo));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Mock Calendar"));
    type->setDisplayName(QStringLiteral("Mock Calendar"));
    type->setPrivilege(DScheduleType::User);
    type->setShowState(DScheduleType::Show);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("mock-calendar");
    calendar.accountId = account->accountID();
    calendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/"))).toString();
    calendar.scheduleTypeID = type->typeID();
    calendar.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavCalendar(calendar));

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setScheduleTypeID(type->typeID());
    schedule->setSummary(QStringLiteral("Mock outbox event"));
    schedule->setDtStart(QDateTime(QDate(2026, 8, 31), QTime(12, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 8, 31), QTime(13, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());
    const QString scheduleID = schedule->uid();
    EXPECT_TRUE(localDatabase.scheduleExistsByScheduleID(scheduleID));
    EXPECT_FALSE(localDatabase.scheduleExistsByScheduleID(QStringLiteral("missing-schedule")));

    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));

    auto processOutbox = [&](DCalDavOutboxProcessor::Result &result) {
        DCalDavOutboxProcessor processor;
        DCalDavOutboxProcessor::Request request;
        request.accountID = account->accountID();
        request.username = accountInfo.username;
        request.password = QStringLiteral("password");
        request.localDatabase = &localDatabase;
        request.accountManagerDatabase = &accountManagerDatabase;
        bool called = false;
        QEventLoop loop;
        processor.start(request, [&](const DCalDavOutboxProcessor::Result &processorResult) {
            result = processorResult;
            called = true;
            loop.quit();
        });
        QTimer::singleShot(5000, &loop, &QEventLoop::quit);
        if (!called) {
            loop.exec();
        }
        return called;
    };

    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(processOutbox(result));
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(accountManagerDatabase.getCalDavOutboxItem(
        account->accountID(), scheduleID).operationID.isEmpty());
    DCalDavEventMappingInfo mapping = accountManagerDatabase.getCalDavEventMappingByLocalScheduleID(
        account->accountID(), scheduleID);
    ASSERT_FALSE(mapping.href.isEmpty());
    EXPECT_EQ(QStringLiteral("\"mock-etag-1\""), mapping.etag);

    DSchedule::Ptr permissionDeniedSchedule(new DSchedule);
    permissionDeniedSchedule->setScheduleTypeID(type->typeID());
    permissionDeniedSchedule->setSummary(QStringLiteral("Permission denied outbox event"));
    permissionDeniedSchedule->setDtStart(QDateTime(QDate(2026, 8, 31), QTime(14, 0), Qt::UTC));
    permissionDeniedSchedule->setDtEnd(QDateTime(QDate(2026, 8, 31), QTime(15, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(permissionDeniedSchedule).isEmpty());
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), permissionDeniedSchedule,
        DCalDavOutboxEnqueuer::CreateChange));
    server.setResponseStatus(403);
    ASSERT_TRUE(processOutbox(result));
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavScheduleCreateError::PermissionDenied, result.createFailure);
    EXPECT_EQ(DCalDavTransport::PermissionDenied, result.failureResponse.error);
    ASSERT_TRUE(accountManagerDatabase.deleteCalDavOutboxItem(
        account->accountID(), permissionDeniedSchedule->uid()));
    server.setResponseStatus(204);

    schedule->setSummary(QStringLiteral("Updated mock outbox event"));
    schedule->setLastModified(QDateTime::currentDateTimeUtc());
    ASSERT_TRUE(localDatabase.updateSchedule(schedule));
    server.setResponseEtag("\"mock-etag-2\"");
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::ModifyChange));
    ASSERT_TRUE(processOutbox(result));
    ASSERT_TRUE(result.success);
    mapping = accountManagerDatabase.getCalDavEventMappingByLocalScheduleID(
        account->accountID(), scheduleID);
    EXPECT_EQ(QStringLiteral("\"mock-etag-2\""), mapping.etag);
    ASSERT_GE(server.requests().size(), 3);
    EXPECT_EQ(QByteArray("\"mock-etag-1\""),
              headerValue(server.requests().at(2).headers, "If-Match"));
    EXPECT_TRUE(server.requests().at(2).body.contains("Updated mock outbox event"));

    // Preserve detached recurrence exceptions when a local edit rewrites the
    // master VEVENT in a resource containing multiple VEVENT components.
    mapping.originalIcs = QStringLiteral(
        "BEGIN:VCALENDAR\r\nVERSION:2.0\r\n"
        "BEGIN:VEVENT\r\nUID:%1\r\n"
        "DTSTART:20260831T120000Z\r\nDTEND:20260831T130000Z\r\n"
        "RRULE:FREQ=DAILY;COUNT=2\r\nSUMMARY:Remote master\r\nEND:VEVENT\r\n"
        "BEGIN:VEVENT\r\nUID:%1\r\nRECURRENCE-ID:20260831T120000Z\r\n"
        "DTSTART:20260831T140000Z\r\nDTEND:20260831T150000Z\r\n"
        "SUMMARY:Remote exception\r\nEND:VEVENT\r\n"
        "END:VCALENDAR\r\n").arg(scheduleID);
    ASSERT_TRUE(accountManagerDatabase.upsertCalDavEventMapping(mapping));
    schedule->setSummary(QStringLiteral("Updated event with exception"));
    schedule->setLastModified(QDateTime::currentDateTimeUtc());
    ASSERT_TRUE(localDatabase.updateSchedule(schedule));
    server.setResponseStatus(204);
    server.setResponseEtag("\"mock-etag-3\"");
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::ModifyChange));
    ASSERT_TRUE(processOutbox(result));
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(server.requests().last().body.contains("Updated event with exception"));
    EXPECT_TRUE(server.requests().last().body.contains("RECURRENCE-ID"));

    server.setResponseStatus(412);
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::ModifyChange));
    ASSERT_TRUE(processOutbox(result));
    EXPECT_FALSE(result.success);
    const DCalDavOutboxItem conflict = accountManagerDatabase.getCalDavOutboxItem(
        account->accountID(), scheduleID);
    EXPECT_EQ(DCalDavOutboxItem::ConflictFailure, conflict.failureType);
    EXPECT_FALSE(conflict.conflictIcs.isEmpty());

    // A conflict snapshot fetch is a separate network request.  A transient
    // failure there must schedule a retry instead of permanently blocking the
    // conflict as if a server version had been captured.
    server.setResponseStatus(412);
    server.setResponseStatusForMethod("GET", 503);
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::ModifyChange));
    ASSERT_TRUE(processOutbox(result));
    EXPECT_FALSE(result.success);
    const DCalDavOutboxItem snapshotRetry = accountManagerDatabase.getCalDavOutboxItem(
        account->accountID(), scheduleID);
    EXPECT_EQ(DCalDavOutboxItem::NetworkFailure, snapshotRetry.failureType);
    EXPECT_TRUE(snapshotRetry.nextRetryAt.isValid());
    server.clearResponseStatusForMethod("GET");

    server.setResponseStatus(503);
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::ModifyChange));
    ASSERT_TRUE(processOutbox(result));
    EXPECT_FALSE(result.success);
    const DCalDavOutboxItem retry = accountManagerDatabase.getCalDavOutboxItem(
        account->accountID(), scheduleID);
    EXPECT_EQ(DCalDavOutboxItem::NetworkFailure, retry.failureType);
    EXPECT_TRUE(retry.nextRetryAt.isValid());

    server.setResponseStatus(204);
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &accountManagerDatabase, account->accountID(), schedule,
        DCalDavOutboxEnqueuer::DeleteChange));
    EXPECT_EQ(QStringLiteral("\"mock-etag-3\""),
              accountManagerDatabase.getCalDavOutboxItem(
                  account->accountID(), scheduleID).baseEtag);
    ASSERT_TRUE(processOutbox(result));
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(accountManagerDatabase.getCalDavOutboxItem(
        account->accountID(), scheduleID).operationID.isEmpty());
    EXPECT_TRUE(accountManagerDatabase.getCalDavEventMappingByLocalScheduleID(
        account->accountID(), scheduleID).href.isEmpty());
    EXPECT_TRUE(localDatabase.getScheduleByScheduleID(scheduleID).isNull());
    ASSERT_GE(server.requests().size(), 5);
    const MockCalDavServer::Request &deleteRequest = server.requests().last();
    EXPECT_EQ(QByteArray("DELETE"), deleteRequest.method);
    EXPECT_EQ(QByteArray("\"mock-etag-3\""),
              headerValue(deleteRequest.headers, "If-Match"));
}
