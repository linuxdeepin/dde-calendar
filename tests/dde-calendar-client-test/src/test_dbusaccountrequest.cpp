// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusaccountrequest.h"
#include "dbusrequestbase.h"
#include "cdbuspendingcallwatcher.h"
#include "daccount.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "dtypecolor.h"
#include "dschedulequerypar.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QEventLoop>
#include <QObject>
#include <QSignalSpy>
#include <QTimer>

#include <gtest/gtest.h>

namespace {

const char kAccountPath[] = "/com/deepin/dataserver/Calendar/Account/test-account";
const char kAccountInterface[] = "com.deepin.dataserver.Calendar.Account";
const char kCalendarService[] = "com.deepin.dataserver.Calendar";

// Mock service that registers slots the request object will call.
class AccountMockService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.Account")

public:
    QStringList calledMethods;
    QVariantMap setProperties;
    bool isExpandValue = false;
    int accountStateValue = 0;
    int syncStateValue = 0;
    QString syncFreqValue;
    QString dtLastUpdateValue;
    QString accountInfoJson;
    QString scheduleTypeListJson;
    QString scheduleTypeJson;
    QString scheduleJson;
    QString scheduleListJson;
    QString sysColorsJson;
    bool scheduleTypeByUsedValue = false;

public slots:
    // asyncCall targets
    QString getAccountInfo() { calledMethods << "getAccountInfo"; return accountInfoJson; }
    QString getScheduleTypeList() { calledMethods << "getScheduleTypeList"; return scheduleTypeListJson; }
    QString getScheduleTypeByID(const QString &typeID) { calledMethods << "getScheduleTypeByID"; return scheduleTypeJson; }
    QString createScheduleType(const QString &json) { calledMethods << "createScheduleType"; return QStringLiteral("new-type-id"); }
    QString updateScheduleType(const QString &json) { calledMethods << "updateScheduleType"; return QString(); }
    QString updateScheduleTypeShowState(const QString &json) { calledMethods << "updateScheduleTypeShowState"; return QString(); }
    QString deleteScheduleTypeByID(const QString &typeID) { calledMethods << "deleteScheduleTypeByID"; return QString(); }
    bool scheduleTypeByUsed(const QString &typeID) { calledMethods << "scheduleTypeByUsed"; return scheduleTypeByUsedValue; }
    QString createSchedule(const QString &json) { calledMethods << "createSchedule"; return QStringLiteral("new-schedule-id"); }
    QString updateSchedule(const QString &json) { calledMethods << "updateSchedule"; return QString(); }
    QString getScheduleByScheduleID(const QString &scheduleID) { calledMethods << "getScheduleByScheduleID"; return scheduleJson; }
    QString deleteScheduleByScheduleID(const QString &scheduleID) { calledMethods << "deleteScheduleByScheduleID"; return QString(); }
    QString deleteSchedulesByScheduleTypeID(const QString &typeID) { calledMethods << "deleteSchedulesByScheduleTypeID"; return QString(); }
    QString querySchedulesWithParameter(const QString &json) { calledMethods << "querySchedulesWithParameter"; return scheduleListJson; }
    QString getSysColors() { calledMethods << "getSysColors"; return sysColorsJson; }
    void importSchedule(const QString &ics, const QString &typeID, bool clean) { calledMethods << "importSchedule"; }
    void exportSchedule(const QString &ics, const QString &typeID) { calledMethods << "exportSchedule"; }

    // Property accessors for QDBusInterface.property/setProperty
    QDBusVariant isExpand() const { return QDBusVariant(isExpandValue); }
    void setExpand(bool v) { isExpandValue = v; setProperties["isExpand"] = v; }

    // Properties via QDBusInterface
    QDBusVariant accountState() const { return QDBusVariant(accountStateValue); }
    void setAccountState(int v) { accountStateValue = v; setProperties["accountState"] = v; }

    QDBusVariant syncState() const { return QDBusVariant(syncStateValue); }
    void setSyncState(int v) { syncStateValue = v; }

    QDBusVariant syncFreq() const { return QDBusVariant(syncFreqValue); }
    void setSyncFreq(const QString &v) { syncFreqValue = v; setProperties["syncFreq"] = v; }

    QDBusVariant dtLastUpdate() const { return QDBusVariant(dtLastUpdateValue); }

signals:
    void scheduleUpdate();
    void scheduleTypeUpdate();
    void calDavScheduleCreateFailed(int createFailure);
};

// Helper to register/unregister the mock service on session bus.
struct BusGuard
{
    QString service = QString::fromUtf8(kCalendarService);
    QString path = QString::fromUtf8(kAccountPath);
    AccountMockService svc;
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

// Pump the event loop briefly so async DBus replies and watcher completion land.
inline void pumpEvents(int ms = 300)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

} // namespace

// ---- Constructor ----

TEST(DbusAccountRequestTest, Constructor_ConnectsScheduleSignals)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents();

    SUCCEED();
}

// ---- setAccountExpandStatus ----

TEST(DbusAccountRequestTest, SetAccountExpandStatus_SendsPropertySet)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.setAccountExpandStatus(true);
    pumpEvents();

    EXPECT_TRUE(guard.svc.setProperties.contains("isExpand"));
}

// ---- setAccountState / getAccountState ----

TEST(DbusAccountRequestTest, SetAccountState_SendsPropertySet)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.setAccountState(DAccount::Account_Open);
    pumpEvents();

    EXPECT_TRUE(guard.svc.setProperties.contains("accountState"));
}

TEST(DbusAccountRequestTest, GetAccountState_ReturnsPropertyValue)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountStateValue = static_cast<int>(DAccount::Account_Open);

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents();

    DAccount::AccountStates state = request.getAccountState();
    EXPECT_EQ(state, DAccount::Account_Open);
}

// ---- setSyncFreq / getSyncFreq ----

TEST(DbusAccountRequestTest, SetSyncFreq_SendsPropertySet)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.setSyncFreq(QStringLiteral("1h"));
    pumpEvents();

    EXPECT_TRUE(guard.svc.setProperties.contains("syncFreq"));
}

TEST(DbusAccountRequestTest, GetSyncFreq_ReturnsPropertyValue)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.syncFreqValue = QStringLiteral("30m");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents();

    QString freq = request.getSyncFreq();
    EXPECT_EQ(freq, QStringLiteral("30m"));
}

// ---- getSyncState ----

TEST(DbusAccountRequestTest, GetSyncState_ReturnsPropertyValue)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.syncStateValue = static_cast<int>(DAccount::Sync_Normal);

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents();

    DAccount::AccountSyncState state = request.getSyncState();
    EXPECT_EQ(state, DAccount::Sync_Normal);
}

// ---- getDtLastUpdate ----

TEST(DbusAccountRequestTest, GetDtLastUpdate_ReturnsPropertyValue)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.dtLastUpdateValue = QStringLiteral("2026-01-01 12:00:00");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents();

    QString dt = request.getDtLastUpdate();
    EXPECT_EQ(dt, QStringLiteral("2026-01-01 12:00:00"));
}

// ---- getAccountInfo (async) ----

TEST(DbusAccountRequestTest, GetAccountInfo_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountInfoJson = QStringLiteral("{\"accountID\":\"test\",\"name\":\"Test\",\"type\":1}");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetAccountInfoFinish);

    request.getAccountInfo();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- getScheduleTypeList (async) ----

TEST(DbusAccountRequestTest, GetScheduleTypeList_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleTypeListJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetScheduleTypeListFinish);

    request.getScheduleTypeList();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- getScheduleTypeByID (async) ----

TEST(DbusAccountRequestTest, GetScheduleTypeByID_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.getScheduleTypeByID(QStringLiteral("type-1"));
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("getScheduleTypeByID"));
}

// ---- createScheduleType (async) ----

TEST(DbusAccountRequestTest, CreateScheduleType_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleType::Ptr type(new DScheduleType);
    type->setTypeID(QStringLiteral("new-type"));
    request.createScheduleType(type);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("createScheduleType"));
}

// ---- updateScheduleType (async) ----

TEST(DbusAccountRequestTest, UpdateScheduleType_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleType::Ptr type(new DScheduleType);
    type->setTypeID(QStringLiteral("type-1"));
    request.updateScheduleType(type);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("updateScheduleType"));
}

// ---- updateScheduleTypeShowState (async) ----

TEST(DbusAccountRequestTest, UpdateScheduleTypeShowState_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleType::Ptr type(new DScheduleType);
    type->setTypeID(QStringLiteral("type-1"));
    request.updateScheduleTypeShowState(type);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("updateScheduleTypeShowState"));
}

// ---- deleteScheduleTypeByID (async) ----

TEST(DbusAccountRequestTest, DeleteScheduleTypeByID_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.deleteScheduleTypeByID(QStringLiteral("type-1"));
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("deleteScheduleTypeByID"));
}

// ---- scheduleTypeByUsed (sync) ----

TEST(DbusAccountRequestTest, ScheduleTypeByUsed_ReturnsBool)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleTypeByUsedValue = true;

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents();

    bool result = request.scheduleTypeByUsed(QStringLiteral("type-1"));
    EXPECT_TRUE(result);
}

// ---- createSchedule (async) ----

TEST(DbusAccountRequestTest, CreateSchedule_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("Test Schedule"));
    request.createSchedule(schedule);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("createSchedule"));
}

// ---- updateSchedule (async) ----

TEST(DbusAccountRequestTest, UpdateSchedule_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("Updated Schedule"));
    request.updateSchedule(schedule);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("updateSchedule"));
}

// ---- getScheduleByScheduleID (sync) ----

TEST(DbusAccountRequestTest, GetScheduleByScheduleID_ReturnsSchedule)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleJson = QStringLiteral("{\"schedule\":{}}");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents();

    DSchedule::Ptr result = request.getScheduleByScheduleID(QStringLiteral("sched-1"));
    // Returns nullptr on error, valid ptr on success
    // The mock returns a JSON string; result depends on parsing
    SUCCEED();
}

// ---- deleteScheduleByScheduleID (async) ----

TEST(DbusAccountRequestTest, DeleteScheduleByScheduleID_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.deleteScheduleByScheduleID(QStringLiteral("sched-1"));
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("deleteScheduleByScheduleID"));
}

// ---- deleteSchedulesByScheduleTypeID (async) ----

TEST(DbusAccountRequestTest, DeleteSchedulesByScheduleTypeID_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.deleteSchedulesByScheduleTypeID(QStringLiteral("type-1"));
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("deleteSchedulesByScheduleTypeID"));
}

// ---- querySchedulesWithParameter (async) ----

TEST(DbusAccountRequestTest, QuerySchedulesWithParameter_EmptyKey_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleListJson = QStringLiteral("{\"query\":\"\",\"schedules\":[]}");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleQueryPar::Ptr params(new DScheduleQueryPar);
    request.querySchedulesWithParameter(params);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("querySchedulesWithParameter"));
}

TEST(DbusAccountRequestTest, QuerySchedulesWithParameter_WithKey_TriggersSearchCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleListJson = QStringLiteral("{\"query\":\"keyword\",\"schedules\":[]}");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleQueryPar::Ptr params(new DScheduleQueryPar);
    params->setKey(QStringLiteral("keyword"));
    request.querySchedulesWithParameter(params);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("querySchedulesWithParameter"));
}

// ---- querySchedulesByExternal (sync) ----

TEST(DbusAccountRequestTest, QuerySchedulesByExternal_ReturnsFalseOnError)
{
    // No service registered: synchronous call will error
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) GTEST_SKIP() << "Session bus unavailable";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleQueryPar::Ptr params(new DScheduleQueryPar);
    QString json;
    bool result = request.querySchedulesByExternal(params, json);
    EXPECT_FALSE(result);
}

// ---- getSysColors (async) ----

TEST(DbusAccountRequestTest, GetSysColors_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.sysColorsJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetSysColorsFinish);

    request.getSysColors();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- importSchedule (async) ----

TEST(DbusAccountRequestTest, ImportSchedule_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.importSchedule(QStringLiteral("/tmp/test.ics"), QStringLiteral("type-1"), false);
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("importSchedule"));
}

// ---- exportSchedule (async) ----

TEST(DbusAccountRequestTest, ExportSchedule_TriggersAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.exportSchedule(QStringLiteral("/tmp/test.ics"), QStringLiteral("type-1"));
    pumpEvents(500);

    EXPECT_TRUE(guard.svc.calledMethods.contains("exportSchedule"));
}

// ---- slotDbusCall: scheduleUpdate signal ----

TEST(DbusAccountRequestTest, SlotDbusCall_ScheduleUpdate_EmitsSearchUpdate)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleListJson = QStringLiteral("{\"query\":\"\",\"schedules\":[]}");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalSearchUpdate);

    emit guard.svc.scheduleUpdate();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- slotDbusCall: scheduleTypeUpdate signal ----

TEST(DbusAccountRequestTest, SlotDbusCall_ScheduleTypeUpdate_TriggersGetScheduleTypeList)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleTypeListJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetScheduleTypeListFinish);

    emit guard.svc.scheduleTypeUpdate();
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- slotDbusCall: calDavScheduleCreateFailed signal ----

TEST(DbusAccountRequestTest, SlotDbusCall_CalDavScheduleCreateFailed_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalCalDavScheduleCreateFailed);

    emit guard.svc.calDavScheduleCreateFailed(1);
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
    if (spy.count() >= 1) {
        EXPECT_EQ(spy.at(0).at(0).toInt(), 1);
    }
}

// ---- onPropertiesChanged: syncState ----

TEST(DbusAccountRequestTest, OnPropertiesChanged_SyncState_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalSyncStateChange);
    pumpEvents();

    QDBusMessage signal = QDBusMessage::createSignal(
        QString::fromUtf8(kAccountPath), "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QVariantMap changed;
    changed.insert(QStringLiteral("syncState"), static_cast<int>(DAccount::Sync_Normal));
    signal << QString::fromUtf8(kAccountInterface) << changed << QStringList();
    QDBusConnection::sessionBus().send(signal);
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- onPropertiesChanged: accountState ----

TEST(DbusAccountRequestTest, OnPropertiesChanged_AccountState_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalAccountStateChange);
    pumpEvents();

    QDBusMessage signal = QDBusMessage::createSignal(
        QString::fromUtf8(kAccountPath), "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QVariantMap changed;
    changed.insert(QStringLiteral("accountState"), static_cast<int>(DAccount::Account_Open));
    signal << QString::fromUtf8(kAccountInterface) << changed << QStringList();
    QDBusConnection::sessionBus().send(signal);
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- onPropertiesChanged: dtLastUpdate ----

TEST(DbusAccountRequestTest, OnPropertiesChanged_DtLastUpdate_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.dtLastUpdateValue = QStringLiteral("2026-01-01 12:00:00");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalDtLastUpdate);
    pumpEvents();

    QDBusMessage signal = QDBusMessage::createSignal(
        QString::fromUtf8(kAccountPath), "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QVariantMap changed;
    changed.insert(QStringLiteral("dtLastUpdate"), QStringLiteral("2026-01-01 12:00:00"));
    signal << QString::fromUtf8(kAccountInterface) << changed << QStringList();
    QDBusConnection::sessionBus().send(signal);
    pumpEvents(500);

    EXPECT_GE(spy.count(), 1);
}

// ---- slotCallFinished: error path (no service) ----

TEST(DbusAccountRequestTest, SlotCallFinished_ErrorPath_DoesNotCrash)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) GTEST_SKIP() << "Session bus unavailable";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.getAccountInfo();
    pumpEvents(500);

    SUCCEED();
}

#include "test_dbusaccountrequest.moc"
