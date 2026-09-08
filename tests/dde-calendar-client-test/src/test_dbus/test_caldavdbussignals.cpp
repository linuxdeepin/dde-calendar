// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountmanagerrequest.h"
#include "dbusaccountrequest.h"
#include "dcaldavvalidationerror.h"

#include <QDBusConnection>
#include <QEventLoop>
#include <QTimer>

#include <gtest/gtest.h>

namespace {

const char kAccountManagerPath[] = "/com/deepin/dataserver/Calendar/AccountManager";
const char kAccountManagerInterface[] = "com.deepin.dataserver.Calendar.AccountManager";
const char kAccountPath[] = "/com/deepin/dataserver/Calendar/test_caldav_account";
const char kAccountInterface[] = "com.deepin.dataserver.Calendar.TestCalDavAccount";

class AccountManagerSignalService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.AccountManager")

signals:
    void calDavAccountValidationFinished(const QString &requestID, bool success,
                                         int validationError, const QString &errorMessage,
                                         const QString &principalDisplayName);
};

class AccountSignalService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.TestCalDavAccount")

signals:
    void calDavScheduleCreateFailed(int createFailure);
};

} // namespace

TEST(CalDavDbusSignals, PreservesValidationAndCreateFailureArgumentOrder)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }
    if (!bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"))) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    AccountManagerSignalService managerService;
    AccountSignalService accountService;
    ASSERT_TRUE(bus.registerObject(QString::fromLatin1(kAccountManagerPath), &managerService,
                                   QDBusConnection::ExportAllSignals));
    ASSERT_TRUE(bus.registerObject(QString::fromLatin1(kAccountPath), &accountService,
                                   QDBusConnection::ExportAllSignals));

    DbusAccountManagerRequest managerRequest;
    DbusAccountRequest accountRequest(QString::fromLatin1(kAccountPath),
                                      QString::fromLatin1(kAccountInterface));

    const QString expectedRequestID = QStringLiteral("validation-request-id");
    const int expectedValidationError = DCalDavValidationError::CertificateInvalid;
    const QString expectedErrorMessage = QStringLiteral("internal error detail");
    const QString expectedPrincipalName = QStringLiteral("principal display name");
    const int expectedCreateFailure = DCalDavScheduleCreateError::PermissionDenied;
    bool validationReceived = false;
    bool createFailureReceived = false;
    QEventLoop loop;

    QObject::connect(&managerRequest,
                     &DbusAccountManagerRequest::signalCalDavAccountValidationFinished,
                     &loop,
                     [&](const QString &requestID, bool success, int validationError,
                         const QString &errorMessage, const QString &principalDisplayName) {
        EXPECT_EQ(expectedRequestID, requestID);
        EXPECT_FALSE(success);
        EXPECT_EQ(expectedValidationError, validationError);
        EXPECT_EQ(expectedErrorMessage, errorMessage);
        EXPECT_EQ(expectedPrincipalName, principalDisplayName);
        validationReceived = true;
        if (createFailureReceived) {
            loop.quit();
        }
    });
    QObject::connect(&accountRequest, &DbusAccountRequest::signalCalDavScheduleCreateFailed,
                     &loop, [&](int createFailure) {
        EXPECT_EQ(expectedCreateFailure, createFailure);
        createFailureReceived = true;
        if (validationReceived) {
            loop.quit();
        }
    });

    QTimer::singleShot(0, &managerService, [&]() {
        emit managerService.calDavAccountValidationFinished(
            expectedRequestID, false, expectedValidationError, expectedErrorMessage,
            expectedPrincipalName);
        emit accountService.calDavScheduleCreateFailed(expectedCreateFailure);
    });
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_TRUE(validationReceived);
    EXPECT_TRUE(createFailureReceived);
    bus.unregisterObject(QString::fromLatin1(kAccountPath));
    bus.unregisterObject(QString::fromLatin1(kAccountManagerPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

#include "test_caldavdbussignals.moc"
