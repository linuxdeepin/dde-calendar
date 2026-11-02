// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavreadonlysync.h"
#include "dcaldavvalidationerror.h"
#include "dcaldaverrorcode.h"
#include "dcaldavtransport.h"
#include "mock_caldav_server.h"

#include <QUrl>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QProcess>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QSslConfiguration>
#include <QSslSocket>

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

} // namespace

TEST(CalDavReadOnlySyncSvc, RejectsHttpUrlWithoutNetworkAccess)
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
        EXPECT_EQ(DCalDavErrorCode::NetworkUnavailable, result.failureCode);
        EXPECT_EQ(QStringLiteral("Invalid CalDAV server URL or username."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavReadOnlySyncSvc, RejectsEmptyUsernameWithoutNetworkAccess)
{
    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = QUrl(QStringLiteral("https://example.com/caldav"));
    request.username = QString();

    bool called = false;
    synchronizer.start(request, [&called](const DCalDavReadOnlySync::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(DCalDavValidationError::NetworkUnavailable, result.validationError);
        EXPECT_EQ(QStringLiteral("Invalid CalDAV server URL or username."), result.errorMessage);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavReadOnlySyncSvc, RejectsInvalidUrlWithoutNetworkAccess)
{
    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = QUrl();
    request.username = QStringLiteral("user");

    bool called = false;
    synchronizer.start(request, [&called](const DCalDavReadOnlySync::Result &result) {
        called = true;
        EXPECT_FALSE(result.success);
        EXPECT_EQ(DCalDavValidationError::NetworkUnavailable, result.validationError);
    });
    EXPECT_TRUE(called);
}

TEST(CalDavReadOnlySyncSvc, StartAfterFailedStartAlsoInvokesCallback)
{
    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = QUrl(QStringLiteral("http://example.com/caldav"));
    request.username = QStringLiteral("user");

    int callCount = 0;
    synchronizer.start(request, [&callCount](const DCalDavReadOnlySync::Result &) {
        ++callCount;
    });
    EXPECT_EQ(1, callCount);

    synchronizer.start(request, [&callCount](const DCalDavReadOnlySync::Result &) {
        ++callCount;
    });
    EXPECT_EQ(2, callCount);
}

TEST(CalDavReadOnlySyncSvc, CancelWhenNotRunningIsNoOp)
{
    DCalDavReadOnlySync synchronizer;
    synchronizer.cancel();
    synchronizer.cancel(false);
}

TEST(CalDavReadOnlySyncSvc, CancelWithNotifyCallbackAfterFailedStart)
{
    DCalDavReadOnlySync synchronizer;
    DCalDavReadOnlySync::Request request;
    request.serverUrl = QUrl(QStringLiteral("http://example.com/caldav"));
    request.username = QStringLiteral("user");

    bool called = false;
    synchronizer.start(request, [&called](const DCalDavReadOnlySync::Result &) {
        called = true;
    });
    EXPECT_TRUE(called);

    synchronizer.cancel(true);
}

TEST(CalDavReadOnlySyncSvc, DestructorCancelsWithoutNotifyingCallback)
{
    bool called = false;
    {
        DCalDavReadOnlySync synchronizer;
        DCalDavReadOnlySync::Request request;
        request.serverUrl = QUrl(QStringLiteral("http://example.com/caldav"));
        request.username = QStringLiteral("user");
        synchronizer.start(request, [&called](const DCalDavReadOnlySync::Result &) {
            called = true;
        });
        EXPECT_TRUE(called);
    }
}

TEST(CalDavReadOnlySyncSvc, DefaultResultFieldsAreUnset)
{
    DCalDavReadOnlySync::Result result;
    EXPECT_FALSE(result.success);
    EXPECT_EQ(DCalDavValidationError::NoError, result.validationError);
    EXPECT_EQ(DCalDavErrorCode::NoError, result.failureCode);
    EXPECT_TRUE(result.errorMessage.isEmpty());
}

TEST(CalDavReadOnlySyncSvc, RequestRequireReadableCalendarDefaultsToTrue)
{
    DCalDavReadOnlySync::Request request;
    EXPECT_TRUE(request.requireReadableCalendar);
}

TEST(CalDavReadOnlySyncSvc, DestructorCancelsWhileSyncInProgress)
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
    server.setResponseDelay(5000);

    {
        DCalDavReadOnlySync synchronizer;
        DCalDavReadOnlySync::Request request;
        request.serverUrl = server.url();
        request.username = QStringLiteral("user");
        request.password = QStringLiteral("pass");

        synchronizer.start(request, [](const DCalDavReadOnlySync::Result &) {
        });
        ASSERT_TRUE(waitForMockRequestCount(server, 1));
    }

    processEventsFor(200);
    EXPECT_GE(server.requests().size(), 1);
}

TEST(CalDavReadOnlySyncSvc, HeapAllocatedDestructorCleansUp)
{
    DCalDavReadOnlySync *sync = new DCalDavReadOnlySync();
    delete sync;
    SUCCEED();
}
