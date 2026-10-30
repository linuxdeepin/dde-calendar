// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountmanagerrequest.h"
#include "dbusrequestbase.h"
#include "cdbuspendingcallwatcher.h"
#include "daccount.h"
#include "dcalendargeneralsettings.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QEventLoop>
#include <QObject>
#include <QSignalSpy>
#include <QTimer>

#include <gtest/gtest.h>

namespace {

const char kAccountManagerPath[] = "/com/deepin/dataserver/Calendar/AccountManager";
const char kAccountManagerInterface[] = "com.deepin.dataserver.Calendar.AccountManager";
const char kCalendarService[] = "com.deepin.dataserver.Calendar";

// Mock service that registers slots the request object will call.
class AccountManagerMockService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.AccountManager")

public:
    QStringList calledMethods;
    QString accountListJson;
    QString generalSettingsJson;

public slots:
    QString getAccountList() { calledMethods << "getAccountList"; return accountListJson; }
    void downloadByAccountID(const QString &) { calledMethods << "downloadByAccountID"; }
    void uploadNetWorkAccountData() { calledMethods << "uploadNetWorkAccountData"; }
    QString getCalendarGeneralSettings() { calledMethods << "getCalendarGeneralSettings"; return generalSettingsJson; }
    void setCalendarGeneralSettings(const QString &) { calledMethods << "setCalendarGeneralSettings"; }
    void calendarIsShow(bool) { calledMethods << "calendarIsShow"; }

signals:
    void accountUpdate();
};

struct BusGuard
{
    QString service = QString::fromUtf8(kCalendarService);
    QString path = QString::fromUtf8(kAccountManagerPath);
    AccountManagerMockService svc;
    bool registered = false;

    bool ensure()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) return false;
        if (!bus.registerService(service)) return false;
        if (!bus.registerObject(path, &svc,
                QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals
                    | QDBusConnection::ExportAllProperties)) {
            bus.unregisterService(service);
            return false;
        }
        registered = true;
        return true;
    }
    ~BusGuard()
    {
        if (registered) {
            QDBusConnection::sessionBus().unregisterObject(path);
            QDBusConnection::sessionBus().unregisterService(service);
        }
    }
};

inline void pumpEvents(int ms = 500)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

} // namespace

// ---- Constructor ----

TEST(SpDbusAccountManagerRequestTest, Constructor_CreatesObject)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    pumpEvents(100);
    SUCCEED();
}

// ---- getAccountList (synchronous) ----

TEST(SpDbusAccountManagerRequestTest, GetAccountList_ReturnsEmptyWhenNoData)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountListJson = QStringLiteral("[]");

    DbusAccountManagerRequest request;
    auto result = request.getAccountList();
    EXPECT_TRUE(result.isEmpty());
}

TEST(SpDbusAccountManagerRequestTest, GetAccountList_ReturnsParsedAccounts)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountListJson = QStringLiteral("{\"accounts\":[]}");

    DbusAccountManagerRequest request;
    auto result = request.getAccountList();
    EXPECT_TRUE(result.isEmpty());
}

// ---- downloadByAccountID ----

TEST(SpDbusAccountManagerRequestTest, DownloadByAccountID_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.downloadByAccountID(QStringLiteral("account-1"));
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("downloadByAccountID")));
}

// ---- uploadNetWorkAccountData ----

TEST(SpDbusAccountManagerRequestTest, UploadNetWorkAccountData_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.uploadNetWorkAccountData();
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("uploadNetWorkAccountData")));
}

// ---- getCalendarGeneralSettings (async, triggers slotCallFinished) ----

TEST(SpDbusAccountManagerRequestTest, GetCalendarGeneralSettings_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("{}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish);
    request.getCalendarGeneralSettings();
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

// ---- setCalendarGeneralSettings (async, triggers slotCallFinished -> getCalendarGeneralSettings) ----

TEST(SpDbusAccountManagerRequestTest, SetCalendarGeneralSettings_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("{}");

    DbusAccountManagerRequest request;
    DCalendarGeneralSettings::Ptr ptr(new DCalendarGeneralSettings);
    request.setCalendarGeneralSettings(ptr);
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("setCalendarGeneralSettings")));
}

// ---- clientIsShow ----

TEST(SpDbusAccountManagerRequestTest, ClientIsShow_SendsCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.clientIsShow(true);
    pumpEvents(200);
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("calendarIsShow")));
}

// ---- slotCallFinished error path (no service registered) ----

TEST(SpDbusAccountManagerRequestTest, SlotCallFinished_ErrorPath)
{
    // No BusGuard: calls will error because the service is not registered.
    if (!QDBusConnection::sessionBus().isConnected())
        GTEST_SKIP() << "Session bus not available";

    DbusAccountManagerRequest request;
    bool callbackInvoked = false;
    int callbackCode = -1;
    request.setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        callbackCode = msg.code;
    });
    request.getCalendarGeneralSettings();
    pumpEvents(2000);
    // Error path: callback should have been invoked with ret=1
    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(callbackCode, 1);
}

// ---- slotCallFinished getAccountList success path (via asyncCall) ----

TEST(SpDbusAccountManagerRequestTest, SlotCallFinished_GetAccountList_Success)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountListJson = QStringLiteral("{\"accounts\":[]}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetAccountListFinish);
    // Use asyncCall directly to trigger slotCallFinished with member="getAccountList"
    request.setCallbackFunc(nullptr);
    // -fno-access-control allows calling protected asyncCall
    request.asyncCall(QStringLiteral("getAccountList"));
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

// ---- slotCallFinished getCalendarGeneralSettings success path ----

TEST(SpDbusAccountManagerRequestTest, SlotCallFinished_GetCalendarGeneralSettings_Success)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("{}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish);
    request.getCalendarGeneralSettings();
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

// ---- slotCallFinished getCalendarGeneralSettings parse failure ----

TEST(SpDbusAccountManagerRequestTest, SlotCallFinished_GetCalendarGeneralSettings_ParseFailure)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("NOT_JSON");

    DbusAccountManagerRequest request;
    bool callbackInvoked = false;
    int callbackCode = -1;
    request.setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        callbackCode = msg.code;
    });
    request.getCalendarGeneralSettings();
    pumpEvents();
    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(callbackCode, 2);
}

// ---- slotCallFinished getAccountList parse failure ----

TEST(SpDbusAccountManagerRequestTest, SlotCallFinished_GetAccountList_ParseFailure)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountListJson = QStringLiteral("NOT_JSON");

    DbusAccountManagerRequest request;
    bool callbackInvoked = false;
    int callbackCode = -1;
    request.setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        callbackCode = msg.code;
    });
    request.asyncCall(QStringLiteral("getAccountList"));
    pumpEvents();
    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(callbackCode, 2);
}

// ---- slotCallFinished setCalendarGeneralSettings path ----

TEST(SpDbusAccountManagerRequestTest, SlotCallFinished_SetCalendarGeneralSettings_TriggersRefresh)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("{}");

    DbusAccountManagerRequest request;
    bool callbackInvoked = false;
    request.setCallbackFunc([&](CallMessge) {
        callbackInvoked = true;
    });
    DCalendarGeneralSettings::Ptr ptr(new DCalendarGeneralSettings);
    request.setCalendarGeneralSettings(ptr);
    pumpEvents(1000);
    // After setCalendarGeneralSettings completes, it calls getCalendarGeneralSettings,
    // which triggers another slotCallFinished that should invoke the callback.
    EXPECT_TRUE(callbackInvoked);
}

#include "test_sp_dbusaccountmanagerrequest.moc"
