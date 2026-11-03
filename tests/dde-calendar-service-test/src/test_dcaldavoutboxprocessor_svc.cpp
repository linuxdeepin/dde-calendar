// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavoutboxprocessor.h"
#include "dcaldaverrorcode.h"
#include "dcaldavoutboxitem.h"
#include "dcaldavoutboxenqueuer.h"
#include "dcaldavprofile.h"
#include "dcaldavrecoveryitem.h"
#include "dcaldavvalidationerror.h"
#include "dcaldavtransport.h"
#include "dcaldavxmlreader.h"
#include "ddatabase.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dschedule.h"
#include "daccount.h"
#include "mock_caldav_server.h"

#include <QTemporaryDir>
#include <QDateTime>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QTimer>

#include <gtest/gtest.h>

namespace {

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

struct OutboxTestEnv {
    QTemporaryDir directory;
    DAccount::Ptr account;
    DAccountDataBase *localDatabase = nullptr;
    DAccountManagerDataBase *accountManagerDatabase = nullptr;
    QByteArray certificate;
    QByteArray privateKey;
    MockCalDavServer server;
    DCalDavAccountInfo accountInfo;
    DScheduleType::Ptr type;
    DCalDavCalendarInfo calendar;

    bool init()
    {
        if (!directory.isValid()) return false;
        if (!QSslSocket::supportsSsl()) return false;
        if (!createMockTlsCredentials(certificate, privateKey)) return false;
        trustMockCertificate(certificate);
        if (!server.start(certificate, privateKey)) return false;

        account.reset(new DAccount);
        account->setAccountID(QStringLiteral("mock-account"));
        account->setAccountType(DAccount::Account_CalDav);

        localDatabase = new DAccountDataBase(account);
        localDatabase->setDBPath(directory.filePath(QStringLiteral("local.db")));
        localDatabase->initDBData();

        accountManagerDatabase = new DAccountManagerDataBase;
        accountManagerDatabase->setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
        accountManagerDatabase->setLoaclDB(directory.filePath(QStringLiteral("local.db")));
        accountManagerDatabase->initDBData();

        accountInfo.accountId = account->accountID();
        accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
        accountInfo.serverUrl = server.url().toString();
        accountInfo.username = QStringLiteral("user");
        accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
        if (!accountManagerDatabase->upsertCalDavAccountInfo(accountInfo)) return false;

        type.reset(new DScheduleType(account->accountID()));
        type->setTypeName(QStringLiteral("Mock Calendar"));
        type->setDisplayName(QStringLiteral("Mock Calendar"));
        type->setPrivilege(DScheduleType::User);
        type->setShowState(DScheduleType::Show);
        if (localDatabase->createScheduleType(type).isEmpty()) return false;

        calendar.calendarId = QStringLiteral("mock-calendar");
        calendar.accountId = account->accountID();
        calendar.href = server.url().resolved(QUrl(QStringLiteral("/calendars/user/"))).toString();
        calendar.scheduleTypeID = type->typeID();
        calendar.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
        if (!accountManagerDatabase->upsertCalDavCalendar(calendar)) return false;

        return true;
    }

    ~OutboxTestEnv()
    {
        delete localDatabase;
        delete accountManagerDatabase;
    }

    DSchedule::Ptr createSchedule(const QString &summary)
    {
        DSchedule::Ptr schedule(new DSchedule);
        schedule->setScheduleTypeID(type->typeID());
        schedule->setSummary(summary);
        schedule->setDtStart(QDateTime(QDate(2026, 8, 31), QTime(12, 0), Qt::UTC));
        schedule->setDtEnd(QDateTime(QDate(2026, 8, 31), QTime(13, 0), Qt::UTC));
        localDatabase->createSchedule(schedule);
        return schedule;
    }

    bool processOutbox(DCalDavOutboxProcessor::Result &result, bool forceRetry = false)
    {
        DCalDavOutboxProcessor processor;
        DCalDavOutboxProcessor::Request request;
        request.accountID = account->accountID();
        request.username = accountInfo.username;
        request.password = QStringLiteral("pass");
        request.localDatabase = localDatabase;
        request.accountManagerDatabase = accountManagerDatabase;
        request.forceRetry = forceRetry;
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
    }
};

} // namespace

TEST(CalDavOutboxProcessorSvc, EmptyAccountIDRejectsWithoutNetworkAccess)
{
    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.username = QStringLiteral("user");

    bool called = false;
    processor.start(request, [&called](const DCalDavOutboxProcessor::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(QStringLiteral("CalDAV Outbox request is incomplete."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavOutboxProcessorSvc, EmptyUsernameRejectsWithoutNetworkAccess)
{
    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = QStringLiteral("account-1");

    bool called = false;
    processor.start(request, [&called](const DCalDavOutboxProcessor::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(QStringLiteral("CalDAV Outbox request is incomplete."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavOutboxProcessorSvc, NullLocalDatabaseRejectsWithoutNetworkAccess)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = QStringLiteral("account-1");
    request.username = QStringLiteral("user");
    request.localDatabase = nullptr;
    request.accountManagerDatabase = &managerDatabase;

    bool called = false;
    processor.start(request, [&called](const DCalDavOutboxProcessor::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(QStringLiteral("CalDAV Outbox request is incomplete."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavOutboxProcessorSvc, NullManagerDatabaseRejectsWithoutNetworkAccess)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("null-manager-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = account->accountID();
    request.username = QStringLiteral("user");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = nullptr;

    bool called = false;
    processor.start(request, [&called](const DCalDavOutboxProcessor::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(QStringLiteral("CalDAV Outbox request is incomplete."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavOutboxProcessorSvc, StartAfterFailedStartAlsoInvokesCallback)
{
    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;

    int callCount = 0;
    processor.start(request, [&callCount](const DCalDavOutboxProcessor::Result &) {
        ++callCount;
    });
    EXPECT_EQ(1, callCount);

    processor.start(request, [&callCount](const DCalDavOutboxProcessor::Result &) {
        ++callCount;
    });
    EXPECT_EQ(2, callCount);
}

TEST(CalDavOutboxProcessorSvc, CancelWhenNotRunningIsNoOp)
{
    DCalDavOutboxProcessor processor;
    processor.cancel();
    processor.cancel(false);
}

TEST(CalDavOutboxProcessorSvc, CancelAfterFailedStartDoesNotReNotify)
{
    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;

    int callCount = 0;
    processor.start(request, [&callCount](const DCalDavOutboxProcessor::Result &) {
        ++callCount;
    });
    EXPECT_EQ(1, callCount);

    processor.cancel(true);
    EXPECT_EQ(1, callCount);
}

TEST(CalDavOutboxProcessorSvc, DestructorCancelsWithoutNotifyingCallback)
{
    int callCount = 0;
    {
        DCalDavOutboxProcessor processor;
        DCalDavOutboxProcessor::Request request;
        processor.start(request, [&callCount](const DCalDavOutboxProcessor::Result &) {
            ++callCount;
        });
        EXPECT_EQ(1, callCount);
    }
    EXPECT_EQ(1, callCount);
}

TEST(CalDavOutboxProcessorSvc, DefaultResultFieldsAreUnset)
{
    DCalDavOutboxProcessor::Result result;
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavErrorCode::NoError, result.failureCode);
    EXPECT_EQ(0, result.processedCount);
    EXPECT_EQ(0, result.retryScheduledCount);
    EXPECT_EQ(0, result.conflictRecordedCount);
    EXPECT_FALSE(result.requestAttempted);
}

TEST(CalDavOutboxProcessorSvc, CompletesImmediatelyForNonCalDavAccount)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("non-caldav-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = account->accountID();
    request.username = QStringLiteral("user");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &managerDatabase;

    bool called = false;
    processor.start(request, [&called](const DCalDavOutboxProcessor::Result &result) {
        called = true;
        EXPECT_TRUE(result.success);
    });
    EXPECT_TRUE(called);
}

// --- Integration tests with mock server ---

TEST(CalDavOutboxProcessorSvc, FetchEtagWhenServerOmitsEtagHeader)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    env.server.setResponseStatusForMethod("PUT", 204);
    // No setResponseEtag → response has no ETag header → fetchEtag triggered
    // Default PROPFIND returns 207 with <d:getetag> in body → parseEtag extracts it

    DSchedule::Ptr schedule = env.createSchedule(QStringLiteral("FetchEtag test event"));
    const QString scheduleID = schedule->uid();
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));

    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(env.processOutbox(result));
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.requestAttempted);

    // The mapping should have an etag from the PROPFIND response
    DCalDavEventMappingInfo mapping = env.accountManagerDatabase->getCalDavEventMappingByLocalScheduleID(
        env.account->accountID(), scheduleID);
    EXPECT_FALSE(mapping.href.isEmpty());
    EXPECT_FALSE(mapping.etag.isEmpty());
}

TEST(CalDavOutboxProcessorSvc, ModifyWithEmptyBaseEtagRecordsConflictAndKeepsLocal)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    // First: successful Create to establish mapping
    env.server.setResponseStatusForMethod("PUT", 204);
    env.server.setResponseEtag("\"mock-etag-1\"");

    DSchedule::Ptr schedule = env.createSchedule(QStringLiteral("FetchWriteEtag test event"));
    const QString scheduleID = schedule->uid();
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));

    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(env.processOutbox(result));
    ASSERT_TRUE(result.success);

    // Clear the mapping etag so the next Modify has empty baseEtag
    DCalDavEventMappingInfo mapping = env.accountManagerDatabase->getCalDavEventMappingByLocalScheduleID(
        env.account->accountID(), scheduleID);
    mapping.etag.clear();
    ASSERT_TRUE(env.accountManagerDatabase->upsertCalDavEventMapping(mapping));

    // Enqueue Modify — the outbox item should get empty baseEtag
    schedule->setSummary(QStringLiteral("Modified for fetchWriteEtag"));
    schedule->setLastModified(QDateTime::currentDateTimeUtc());
    ASSERT_TRUE(env.localDatabase->updateSchedule(schedule));
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::ModifyChange));

    // Verify baseEtag is empty
    DCalDavOutboxItem item = env.accountManagerDatabase->getCalDavOutboxItem(
        env.account->accountID(), scheduleID);
    EXPECT_TRUE(item.baseEtag.isEmpty());

    // 现行生产策略：Modify 且 baseEtag 为空时，不会自动拉取远端最新版本作为
    // 基线覆盖，而是转入冲突快照处理；当服务端未返回可用快照时，保留本地
    // 版本并记录一个可由用户选择的冲突（dcaldavoutboxprocessor.cpp 中
    // "Never turn an unknown baseline into the current remote version"）。
    ASSERT_TRUE(env.processOutbox(result));
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.requestAttempted);
    EXPECT_EQ(1, result.conflictRecordedCount);
    EXPECT_TRUE(result.errorMessage.contains(QLatin1String("conflict snapshot is empty")));
    // 本地日程必须保留（keep the local version）
    EXPECT_FALSE(env.localDatabase->getScheduleByScheduleID(scheduleID).isNull());
}

TEST(CalDavOutboxProcessorSvc, ProcessMissingScheduleConvertsToDeleteWhenMappingExists)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    env.server.setResponseStatusForMethod("DELETE", 204);

    // Create a mapping with a valid href for a schedule ID that does NOT exist in the local DB.
    // processMissingSchedule will convert the CreateOperation into a DeleteOperation.
    const QString ghostScheduleID = DDataBase::createUuid();
    DCalDavEventMappingInfo mapping;
    mapping.localScheduleID = ghostScheduleID;
    mapping.accountID = env.account->accountID();
    mapping.calendarID = env.calendar.calendarId;
    mapping.uid = ghostScheduleID;
    mapping.href = env.server.url().resolved(QUrl(QStringLiteral("/calendars/user/ghost.ics"))).toString();
    mapping.etag = QStringLiteral("\"mock-etag-1\"");
    ASSERT_TRUE(env.accountManagerDatabase->upsertCalDavEventMapping(mapping));

    // Insert an outbox item referencing the nonexistent schedule
    DCalDavOutboxItem item;
    item.operationID = DDataBase::createUuid();
    item.accountID = env.account->accountID();
    item.localScheduleID = ghostScheduleID;
    item.operationType = DCalDavOutboxItem::CreateOperation;
    item.failureType = DCalDavOutboxItem::NoFailure;
    ASSERT_TRUE(env.accountManagerDatabase->upsertCalDavOutboxItem(item));

    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(env.processOutbox(result));
    ASSERT_TRUE(result.success);
    EXPECT_TRUE(result.requestAttempted);
}

TEST(CalDavOutboxProcessorSvc, ProcessMissingScheduleRecordsFailureWhenNoMapping)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    // Insert an outbox item with a schedule ID that doesn't exist in the local DB.
    // No mapping exists → processMissingSchedule records a permanent failure (500).
    const QString ghostScheduleID = DDataBase::createUuid();
    DCalDavOutboxItem item;
    item.operationID = DDataBase::createUuid();
    item.accountID = env.account->accountID();
    item.localScheduleID = ghostScheduleID;
    item.operationType = DCalDavOutboxItem::CreateOperation;
    item.failureType = DCalDavOutboxItem::NoFailure;
    ASSERT_TRUE(env.accountManagerDatabase->upsertCalDavOutboxItem(item));

    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(env.processOutbox(result));
    EXPECT_FALSE(result.success);
    EXPECT_GT(result.permanentFailureCount, 0);
}

TEST(CalDavOutboxProcessorSvc, RepairCreateOperationWhenMappingAlreadyExists)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    env.server.setResponseStatusForMethod("PUT", 204);
    env.server.setResponseEtag("\"mock-etag-1\"");

    DSchedule::Ptr schedule = env.createSchedule(QStringLiteral("RepairCreate test event"));
    const QString scheduleID = schedule->uid();

    // First Create to establish mapping
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));
    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(env.processOutbox(result));
    ASSERT_TRUE(result.success);

    // Enqueue another Create for the same schedule — mapping.href exists → repairCreateOperation
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));

    // repairCreateOperation converts to ModifyOperation → sendWriteRequest
    ASSERT_TRUE(env.processOutbox(result));
    ASSERT_TRUE(result.success);
}

TEST(CalDavOutboxProcessorSvc, ProcessCurrentItemOrAdvanceWhenItemDeletedMidFlight)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    // Use delay so we can race a DB modification against the response
    env.server.setResponseDelay(300);
    env.server.setResponseStatusForMethod("PUT", 204);
    env.server.setResponseEtag("\"mock-etag-1\"");

    DSchedule::Ptr schedule = env.createSchedule(QStringLiteral("RaceCondition test event"));
    const QString scheduleID = schedule->uid();
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));

    DCalDavOutboxProcessor::Result result;

    // Start processing in a way that doesn't block
    DCalDavOutboxProcessor processor;
    DCalDavOutboxProcessor::Request request;
    request.accountID = env.account->accountID();
    request.username = env.accountInfo.username;
    request.password = QStringLiteral("pass");
    request.localDatabase = env.localDatabase;
    request.accountManagerDatabase = env.accountManagerDatabase;

    bool called = false;
    QEventLoop loop;
    processor.start(request, [&](const DCalDavOutboxProcessor::Result &processorResult) {
        result = processorResult;
        called = true;
        loop.quit();
    });
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);

    // Wait for the PUT request to arrive at the server
    ASSERT_TRUE(waitForMockRequestCount(env.server, 1));

    // Delete the outbox item from DB while the response is delayed
    // This makes isCurrentItem return false → processCurrentItemOrAdvance
    env.accountManagerDatabase->deleteCalDavOutboxItem(env.account->accountID(), scheduleID);

    // Now wait for the delayed response and processing to complete
    if (!called) {
        loop.exec();
    }

    // The item was deleted, so processCurrentItemOrAdvance advances → finish(true) since no more items
    EXPECT_TRUE(called);
}

TEST(CalDavOutboxProcessorSvc, StartLambdaChecksBlockedItemsForConflict)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    DSchedule::Ptr schedule = env.createSchedule(QStringLiteral("BlockedItem test event"));

    env.server.setResponseStatusForMethod("PUT", 204);
    env.server.setResponseEtag("\"mock-etag-1\"");
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));

    // Process with 403 → item gets PermissionFailure (not excluded from blocked items)
    env.server.clearResponseStatusForMethod("PUT");
    env.server.setResponseStatus(403);
    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(env.processOutbox(result));
    EXPECT_FALSE(result.success);

    // Call start without forceRetry → getCalDavBlockedOutboxItems returns the item
    // → std::any_of lambda runs → hasConflict=false → blocked items branch → finish(false)
    ASSERT_TRUE(env.processOutbox(result));
    EXPECT_FALSE(result.success);

    // With forceRetry=true → blocked items still checked by lambda, but continues to processNext
    env.server.setResponseStatusForMethod("PUT", 204);
    env.server.setResponseEtag("\"mock-etag-1\"");
    ASSERT_TRUE(env.processOutbox(result, true));
    EXPECT_TRUE(result.success);
}

TEST(CalDavOutboxProcessorSvc, DeleteCalendarOperationTriggersCompleteSuccessLambda)
{
    OutboxTestEnv env;
    ASSERT_TRUE(env.init());

    env.server.setResponseStatusForMethod("DELETE", 204);
    env.server.setResponseEtag("\"mock-etag-1\"");

    // First create a schedule and sync it to establish mapping
    DSchedule::Ptr schedule = env.createSchedule(QStringLiteral("DeleteCalendar test event"));
    const QString scheduleID = schedule->uid();
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueue(
        &*env.accountManagerDatabase, env.account->accountID(), schedule,
        DCalDavOutboxEnqueuer::CreateChange));
    DCalDavOutboxProcessor::Result result;
    ASSERT_TRUE(env.processOutbox(result));
    ASSERT_TRUE(result.success);

    // Enqueue a DeleteCalendar operation
    ASSERT_TRUE(DCalDavOutboxEnqueuer::enqueueCalendarDelete(
        &*env.accountManagerDatabase, env.account->accountID(), env.calendar));

    // Process — sendDeleteCalendarRequest → DELETE → 204 → handleWriteResponse → completeSuccess (DeleteCalendarOperation)
    ASSERT_TRUE(env.processOutbox(result));
    ASSERT_TRUE(result.success);
}
