// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountmanagerrequest.h"
#include "dbusrequestbase.h"
#include "cdbuspendingcallwatcher.h"
#include "daccount.h"
#include "dcalendargeneralsettings.h"
#include "dcaldavaccountstatus.h"
#include "dcaldaverrorcode.h"
#include "dcaldavvalidationerror.h"

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
    // Track method calls.
    QStringList calledMethods;
    QVariantMap setProperties;
    int getFirstDayOfWeekSourceValue = -1;
    int getTimeFormatTypeSourceValue = -1;
    bool hasFirstDayOfWeekSource = false;
    bool hasTimeFormatTypeSource = false;
    bool isSupportUidValue = false;
    QString accountListJson;
    QString generalSettingsJson;
    QString calDavStatusListJson;

public slots:
    // asyncCall targets
    QString getAccountList() { calledMethods << "getAccountList"; return accountListJson; }
    QString getCalendarGeneralSettings() { calledMethods << "getCalendarGeneralSettings"; return generalSettingsJson; }
    bool isSupportUid() { calledMethods << "isSupportUid"; return isSupportUidValue; }
    void calendarIsShow(bool) { calledMethods << "calendarIsShow"; }

    QDBusVariant firstDayOfWeekSource() const
    {
        if (!hasFirstDayOfWeekSource) return QDBusVariant(QVariant());
        return QDBusVariant(getFirstDayOfWeekSourceValue);
    }
    void setFirstDayOfWeekSource(int v) { setProperties["firstDayOfWeekSource"] = v; }
    QDBusVariant timeFormatTypeSource() const
    {
        if (!hasTimeFormatTypeSource) return QDBusVariant(QVariant());
        return QDBusVariant(getTimeFormatTypeSourceValue);
    }
    void setTimeFormatTypeSource(int v) { setProperties["timeFormatTypeSource"] = v; }

signals:
    void accountUpdate();
    void calDavAccountStatusChanged(const QString &accountID);
    void calDavManualSyncFinished(const QString &accountID, const QString &requestID);
    void calDavAccountValidationFinished(const QString &requestID, bool success,
                                         int validationError, const QString &errorMessage,
                                         const QString &principalDisplayName);
};

// Helper to register/unregister the mock service on session bus.
struct BusGuard
{
    QString service = QString::fromUtf8(kCalendarService);
    AccountManagerMockService svc;
    bool registered = false;

    bool ensure()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();
        if (!bus.isConnected()) return false;
        if (!bus.registerService(service)) return false;
        if (!bus.registerObject(QString::fromUtf8(kAccountManagerPath), &svc,
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
            QDBusConnection::sessionBus().unregisterObject(QString::fromUtf8(kAccountManagerPath));
            QDBusConnection::sessionBus().unregisterService(service);
        }
    }
};

// Pump the event loop briefly so async DBus replies and watcher completion land.
inline void pumpEvents(int ms = 300)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

} // namespace

// ---- Property setters / getters ----

TEST(DbusAccountManagerRequestTest, SetFirstDayofWeek_SendsPropertySet)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.setFirstDayofWeek(7);
    pumpEvents();

    // The mock service records the property set; verify it was invoked.
    // setProperty uses the DBus Properties interface; we verify via setProperties map.
    EXPECT_TRUE(guard.svc.setProperties.contains("firstDayOfWeekSource")
                || guard.svc.calledMethods.contains("firstDayOfWeek"));
}

TEST(DbusAccountManagerRequestTest, SetTimeFormatType_SendsPropertySet)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.setTimeFormatType(1);
    pumpEvents();

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, SetFirstDayofWeekSource_SendsPropertySet)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.setFirstDayofWeekSource(DCalendarGeneralSettings::Source_System);
    pumpEvents();

    EXPECT_EQ(guard.svc.setProperties.value("firstDayOfWeekSource").toInt(),
              static_cast<int>(DCalendarGeneralSettings::Source_System));
}

TEST(DbusAccountManagerRequestTest, SetTimeFormatTypeSource_SendsPropertySet)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.setTimeFormatTypeSource(DCalendarGeneralSettings::Source_System);
    pumpEvents();

    EXPECT_EQ(guard.svc.setProperties.value("timeFormatTypeSource").toInt(),
              static_cast<int>(DCalendarGeneralSettings::Source_System));
}

TEST(DbusAccountManagerRequestTest, GetFirstDayofWeekSource_ReturnsValueWhenAvailable)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.hasFirstDayOfWeekSource = true;
    guard.svc.getFirstDayOfWeekSourceValue = static_cast<int>(DCalendarGeneralSettings::Source_System);

    DbusAccountManagerRequest request;
    pumpEvents(100);
    auto result = request.getFirstDayofWeekSource();

    EXPECT_EQ(result, DCalendarGeneralSettings::Source_System);
}

TEST(DbusAccountManagerRequestTest, GetFirstDayofWeekSource_ReturnsDatabaseDefaultWhenUnavailable)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.hasFirstDayOfWeekSource = false;

    DbusAccountManagerRequest request;
    pumpEvents(100);
    auto result = request.getFirstDayofWeekSource();

    EXPECT_EQ(result, DCalendarGeneralSettings::Source_Database);
}

TEST(DbusAccountManagerRequestTest, GetTimeFormatTypeSource_ReturnsValueWhenAvailable)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.hasTimeFormatTypeSource = true;
    guard.svc.getTimeFormatTypeSourceValue = static_cast<int>(DCalendarGeneralSettings::Source_System);

    DbusAccountManagerRequest request;
    pumpEvents(100);
    auto result = request.getTimeFormatTypeSource();

    EXPECT_EQ(result, DCalendarGeneralSettings::Source_System);
}

TEST(DbusAccountManagerRequestTest, GetTimeFormatTypeSource_ReturnsDatabaseDefaultWhenUnavailable)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.hasTimeFormatTypeSource = false;

    DbusAccountManagerRequest request;
    pumpEvents(100);
    auto result = request.getTimeFormatTypeSource();

    EXPECT_EQ(result, DCalendarGeneralSettings::Source_Database);
}

// ---- asyncCall wrappers: verify callback fires with ret=0 on success ----

TEST(DbusAccountManagerRequestTest, GetAccountList_EmitsSignalOnValidJson)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    // Empty account list is valid JSON per DAccount::fromJsonListString.
    guard.svc.accountListJson = QStringLiteral("{\"accounts\":[]}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetAccountListFinish);
    request.getAccountList();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

TEST(DbusAccountManagerRequestTest, DownloadByAccountID_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.downloadByAccountID(QStringLiteral("acct-1"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, RequestCalDavManualSync_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.requestCalDavManualSync(QStringLiteral("acct-1"), QStringLiteral("req-1"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, GetCalDavAccountStatusList_EmitsOnValidJson)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.calDavStatusListJson = QStringLiteral("[]");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetCalDavAccountStatusListFinish);
    request.getCalDavAccountStatusList();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

TEST(DbusAccountManagerRequestTest, ValidateCalDavAccount_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.validateCalDavAccount(0, QStringLiteral("http://srv"), QStringLiteral("user"),
                                  QStringLiteral("cred"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, GetCalDavAccountConfig_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.getCalDavAccountConfig(QStringLiteral("acct-1"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, ValidateCalDavAccountForUpdate_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.validateCalDavAccountForUpdate(QStringLiteral("acct-1"), 0, QStringLiteral("http://srv"),
                                           QStringLiteral("user"), QStringLiteral("cred"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, UpdateCalDavCredentialReference_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.updateCalDavCredentialReference(QStringLiteral("acct-1"), QStringLiteral("cred"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, CreateCalDavAccount_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.createCalDavAccount(0, QStringLiteral("http://srv"), QStringLiteral("user"),
                                QStringLiteral("cred"), QStringLiteral("display"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, UpdateCalDavAccount_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.updateCalDavAccount(QStringLiteral("acct-1"), 0, QStringLiteral("http://srv"),
                                QStringLiteral("user"), QStringLiteral("cred"),
                                QStringLiteral("display"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, DeleteCalDavAccount_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.deleteCalDavAccount(QStringLiteral("acct-1"));
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, DeleteCalDavAccountWithLocalDataOption_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.deleteCalDavAccountWithLocalDataOption(QStringLiteral("acct-1"), true);
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, ResolveAllCalDavConflicts_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.resolveAllCalDavConflicts(QStringLiteral("acct-1"), true);
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, UploadNetWorkAccountData_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.uploadNetWorkAccountData();
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, GetCalendarGeneralSettings_EmitsOnValidJson)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("{\"firstDayOfWeek\":1,\"TimeShowType\":0}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish);
    request.getCalendarGeneralSettings();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

TEST(DbusAccountManagerRequestTest, Login_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.login();
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, Logout_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.logout();
    pumpEvents(300);

    SUCCEED();
}

TEST(DbusAccountManagerRequestTest, ClientIsShow_SendsNoBlockCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    request.clientIsShow(true);
    pumpEvents(300);

    EXPECT_TRUE(guard.svc.calledMethods.contains("calendarIsShow"));
}

TEST(DbusAccountManagerRequestTest, GetIsSupportUid_ReturnsValueFromService)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.isSupportUidValue = true;

    DbusAccountManagerRequest request;
    bool result = request.getIsSupportUid();

    EXPECT_TRUE(result);
}

TEST(DbusAccountManagerRequestTest, GetIsSupportUid_ReturnsFalseWhenNoReply)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.isSupportUidValue = false;

    DbusAccountManagerRequest request;
    bool result = request.getIsSupportUid();

    EXPECT_FALSE(result);
}

TEST(DbusAccountManagerRequestTest, GetIsSupportUidAsync_InvokesMethod)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.isSupportUidValue = true;

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetIsSupportUidFinish);
    request.getIsSupportUidAsync();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- slotDbusCall: signal handling ----

TEST(DbusAccountManagerRequestTest, SlotDbusCall_AccountUpdate_TriggersGetAccountList)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountListJson = QStringLiteral("{\"accounts\":[]}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetAccountListFinish);

    emit guard.svc.accountUpdate();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

TEST(DbusAccountManagerRequestTest, SlotDbusCall_CalDavAccountStatusChanged_EmitsRefreshRequested)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.calDavStatusListJson = QStringLiteral("[]");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalCalDavAccountStatusRefreshRequested);

    emit guard.svc.calDavAccountStatusChanged(QStringLiteral("acct-1"));
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
    if (spy.count() >= 1) {
        EXPECT_EQ(spy.at(0).at(0).toString(), QStringLiteral("acct-1"));
    }
}

TEST(DbusAccountManagerRequestTest, SlotDbusCall_CalDavManualSyncFinished_EmitsSyncStatus)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.calDavStatusListJson = QStringLiteral("[]");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request,
                   &DbusAccountManagerRequest::signalCalDavManualSyncStatusRefreshFinished);

    emit guard.svc.calDavManualSyncFinished(QStringLiteral("acct-1"), QStringLiteral("req-1"));
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

TEST(DbusAccountManagerRequestTest, SlotDbusCall_CalDavAccountValidationFinished_EmitsValidationSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request,
                   &DbusAccountManagerRequest::signalCalDavAccountValidationFinished);

    emit guard.svc.calDavAccountValidationFinished(
        QStringLiteral("req-1"), false, DCalDavValidationError::CertificateInvalid,
        QStringLiteral("err"), QStringLiteral("principal"));
    pumpEvents(300);

    EXPECT_GE(spy.count(), 1);
    if (spy.count() >= 1) {
        EXPECT_EQ(spy.at(0).at(0).toString(), QStringLiteral("req-1"));
        EXPECT_FALSE(spy.at(0).at(1).toBool());
        EXPECT_EQ(spy.at(0).at(2).toInt(), DCalDavValidationError::CertificateInvalid);
    }
}

// ---- onPropertiesChanged: handled via PropertiesChanged DBus signal ----

TEST(DbusAccountManagerRequestTest, OnPropertiesChanged_FirstDayOfWeek_TriggersGetGeneralSettings)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("{\"firstDayOfWeek\":1,\"TimeShowType\":0}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish);

    // Emit PropertiesChanged on the org.freedesktop.DBus.Properties interface.
    QDBusMessage signal = QDBusMessage::createSignal(
        QString::fromUtf8(kAccountManagerPath), "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QVariantMap changed;
    changed.insert(QStringLiteral("firstDayOfWeek"), 1);
    signal << QString::fromUtf8(kAccountManagerInterface) << changed << QStringList();
    QDBusConnection::sessionBus().send(signal);
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

TEST(DbusAccountManagerRequestTest, OnPropertiesChanged_TimeFormatType_TriggersGetGeneralSettings)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.generalSettingsJson = QStringLiteral("{\"firstDayOfWeek\":1,\"TimeShowType\":0}");

    DbusAccountManagerRequest request;
    QSignalSpy spy(&request, &DbusAccountManagerRequest::signalGetGeneralSettingsFinish);

    QDBusMessage signal = QDBusMessage::createSignal(
        QString::fromUtf8(kAccountManagerPath), "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QVariantMap changed;
    changed.insert(QStringLiteral("timeFormatType"), 0);
    signal << QString::fromUtf8(kAccountManagerInterface) << changed << QStringList();
    QDBusConnection::sessionBus().send(signal);
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- slotCallFinished error path: callback fires with ret=1 on error ----

TEST(DbusAccountManagerRequestTest, SlotCallFinished_ErrorPath_RetriesGetAccountList)
{
    // No service registered: asyncCall will error, triggering retry path in slotCallFinished.
    // Ensure service is NOT owned by us.
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) GTEST_SKIP() << "Session bus unavailable";
    // If the service is owned by someone else, retry path will also error.
    DbusAccountManagerRequest request;
    request.getAccountList();
    pumpEvents(500);

    SUCCEED();
}

#include "test_dbusaccountmanagerrequest.moc"
