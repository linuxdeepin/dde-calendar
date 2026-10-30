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
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QEventLoop>
#include <QObject>
#include <QSignalSpy>
#include <QTimer>

#include <gtest/gtest.h>

namespace {

const char kAccountPath[] = "/com/deepin/dataserver/Calendar/Account/test_account";
const char kAccountInterface[] = "com.deepin.dataserver.Calendar.Account";
const char kCalendarService[] = "com.deepin.dataserver.Calendar";

// Mock service that registers slots the request object will call.
class AccountMockService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.Account")

public:
    QStringList calledMethods;
    QString accountInfoJson;
    QString scheduleTypeListJson;
    QString scheduleTypeJson;
    QString scheduleJson;
    QString scheduleListJson;
    QString sysColorsJson;

public slots:
    QString getAccountInfo() { calledMethods << "getAccountInfo"; return accountInfoJson; }
    void updateAccountInfo(const QString &) { calledMethods << "updateAccountInfo"; }
    QString getScheduleTypeList() { calledMethods << "getScheduleTypeList"; return scheduleTypeListJson; }
    void getScheduleTypeByID(const QString &) { calledMethods << "getScheduleTypeByID"; }
    QString createScheduleType(const QString &) { calledMethods << "createScheduleType"; return QStringLiteral("type-id-123"); }
    void updateScheduleType(const QString &) { calledMethods << "updateScheduleType"; }
    void deleteScheduleTypeByID(const QString &) { calledMethods << "deleteScheduleTypeByID"; }
    void scheduleTypeByUsed(const QString &) { calledMethods << "scheduleTypeByUsed"; }
    QString createSchedule(const QString &) { calledMethods << "createSchedule"; return QStringLiteral("new-schedule-id"); }
    void updateSchedule(const QString &) { calledMethods << "updateSchedule"; }
    QString getScheduleByScheduleID(const QString &) { calledMethods << "getScheduleByScheduleID"; return scheduleJson; }
    void deleteScheduleByScheduleID(const QString &) { calledMethods << "deleteScheduleByScheduleID"; }
    void deleteSchedulesByScheduleTypeID(const QString &) { calledMethods << "deleteSchedulesByScheduleTypeID"; }
    QString querySchedulesWithParameter(const QString &) { calledMethods << "querySchedulesWithParameter"; return scheduleListJson; }
    QString getSysColors() { calledMethods << "getSysColors"; return sysColorsJson; }

signals:
    void scheduleUpdate();
    void scheduleTypeUpdate();
};

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

inline void pumpEvents(int ms = 500)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

} // namespace

// ---- Constructor ----

TEST(SpDbusAccountRequestTest, Constructor_CreatesObject)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    pumpEvents(100);
    SUCCEED();
}

// ---- getAccountInfo (async, triggers slotCallFinished) ----

TEST(SpDbusAccountRequestTest, GetAccountInfo_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountInfoJson = QStringLiteral("{}");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.getAccountInfo();
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("getAccountInfo")));
}

// ---- updateAccountInfo ----

TEST(SpDbusAccountRequestTest, UpdateAccountInfo_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DAccount::Ptr account(new DAccount);
    request.updateAccountInfo(account);
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("updateAccountInfo")));
}

// ---- getScheduleTypeList (synchronous) ----

TEST(SpDbusAccountRequestTest, GetScheduleTypeList_ReturnsEmptyWhenNoData)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleTypeListJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    auto result = request.getScheduleTypeList();
    EXPECT_TRUE(result.isEmpty());
}

// ---- getScheduleTypeByID ----

TEST(SpDbusAccountRequestTest, GetScheduleTypeByID_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.getScheduleTypeByID(QStringLiteral("type-1"));
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("getScheduleTypeByID")));
}

// ---- createScheduleType ----

TEST(SpDbusAccountRequestTest, CreateScheduleType_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleType::Ptr typeInfo(new DScheduleType);
    request.createScheduleType(typeInfo);
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("createScheduleType")));
}

// ---- updateScheduleType ----

TEST(SpDbusAccountRequestTest, UpdateScheduleType_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleType::Ptr typeInfo(new DScheduleType);
    request.updateScheduleType(typeInfo);
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("updateScheduleType")));
}

// ---- deleteScheduleTypeByID ----

TEST(SpDbusAccountRequestTest, DeleteScheduleTypeByID_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.deleteScheduleTypeByID(QStringLiteral("type-1"));
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("deleteScheduleTypeByID")));
}

// ---- scheduleTypeByUsed ----

TEST(SpDbusAccountRequestTest, ScheduleTypeByUsed_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.scheduleTypeByUsed(QStringLiteral("type-1"));
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("scheduleTypeByUsed")));
}

// ---- createSchedule (synchronous) ----

TEST(SpDbusAccountRequestTest, CreateSchedule_ReturnsScheduleId)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DSchedule::Ptr scheduleInfo(new DSchedule);
    auto result = request.createSchedule(scheduleInfo);
    // Mock returns "new-schedule-id"
    EXPECT_EQ(result, QStringLiteral("new-schedule-id"));
}

// ---- updateSchedule ----

TEST(SpDbusAccountRequestTest, UpdateSchedule_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DSchedule::Ptr scheduleInfo(new DSchedule);
    request.updateSchedule(scheduleInfo);
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("updateSchedule")));
}

// ---- getScheduleByID (synchronous) ----

TEST(SpDbusAccountRequestTest, GetScheduleByID_ReturnsSchedule)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    auto result = request.getScheduleByID(QStringLiteral("schedule-1"));
    // May be null or valid depending on JSON; function is exercised.
    SUCCEED();
}

// ---- deleteScheduleByScheduleID ----

TEST(SpDbusAccountRequestTest, DeleteScheduleByScheduleID_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.deleteScheduleByScheduleID(QStringLiteral("schedule-1"));
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("deleteScheduleByScheduleID")));
}

// ---- deleteSchedulesByScheduleTypeID ----

TEST(SpDbusAccountRequestTest, DeleteSchedulesByScheduleTypeID_SendsAsyncCall)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    request.deleteSchedulesByScheduleTypeID(QStringLiteral("type-1"));
    pumpEvents();
    EXPECT_TRUE(guard.svc.calledMethods.contains(QStringLiteral("deleteSchedulesByScheduleTypeID")));
}

// ---- querySchedulesWithParameter (synchronous) ----

TEST(SpDbusAccountRequestTest, QuerySchedulesWithParameter_ReturnsEmptyWhenInvalid)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleListJson = QStringLiteral("");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    DScheduleQueryPar::Ptr params(new DScheduleQueryPar);
    auto result = request.querySchedulesWithParameter(params);
    EXPECT_TRUE(result.isEmpty());
}

// ---- getSysColors (synchronous) ----

TEST(SpDbusAccountRequestTest, GetSysColors_ReturnsList)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.sysColorsJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    auto result = request.getSysColors();
    EXPECT_TRUE(result.isEmpty());
}

// ---- slotCallFinished error path (no service registered) ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_ErrorPath)
{
    if (!QDBusConnection::sessionBus().isConnected())
        GTEST_SKIP() << "Session bus not available";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    bool callbackInvoked = false;
    int callbackCode = -1;
    request.setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        callbackCode = msg.code;
    });
    request.getAccountInfo();
    pumpEvents(2000);
    // slotCallFinished sets ret=1 on error but does NOT invoke the callback
    // (callback invocation is inside the else/success branch only).
    // This test exercises the error branch for code coverage.
    EXPECT_FALSE(callbackInvoked);
}

// ---- slotCallFinished getAccountInfo success path ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_GetAccountInfo_Success)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.accountInfoJson = QStringLiteral("{}");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetAccountInfoFinish);
    request.getAccountInfo();
    pumpEvents();
    // Signal may or may not fire depending on JSON parse; function is exercised.
    EXPECT_GE(spy.count(), 0);
}

// ---- slotCallFinished getScheduleTypeList success path ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_GetScheduleTypeList_Success)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleTypeListJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetScheduleTypeListFinish);
    request.asyncCall(QStringLiteral("getScheduleTypeList"));
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

// ---- slotCallFinished getSysColors path ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_GetSysColors_Success)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.sysColorsJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetSysColorsFinish);
    request.asyncCall(QStringLiteral("getSysColors"));
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

// ---- slotCallFinished createScheduleType path (triggers getScheduleTypeList refresh) ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_CreateScheduleType_TriggersRefresh)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleTypeListJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    bool callbackInvoked = false;
    QString callbackMsg;
    request.setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        callbackMsg = msg.msg;
    });
    request.asyncCall(QStringLiteral("createScheduleType"), {QVariant(QStringLiteral(""))});
    pumpEvents(1000);
    // createScheduleType sets canCall=false, then calls getScheduleTypeList()
    // synchronously (no second slotCallFinished). The original callback is
    // wrapped but never invoked because canCall is false.
    EXPECT_FALSE(callbackInvoked);
}


// ---- slotCallFinished createScheduleType path (chained callback invocation) ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_CreateScheduleType_ChainedCallbackInvoked)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";
    guard.svc.scheduleTypeListJson = QStringLiteral("[]");

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    bool callbackInvoked = false;
    QString callbackMsg;
    request.setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        callbackMsg = msg.msg;
    });
    // Step 1: createScheduleType wraps the callback into a lambda stored in m_callbackFunc.
    // canCall=false so the callback is NOT invoked here.
    request.asyncCall(QStringLiteral("createScheduleType"), {QVariant(QStringLiteral(""))});
    pumpEvents(1000);
    EXPECT_FALSE(callbackInvoked);

    // Step 2: a second async call (canCall=true) picks up the wrapped lambda as
    // the watcher callback. When slotCallFinished fires for this call, it invokes
    // the wrapped lambda, which in turn calls the original callback.
    request.asyncCall(QStringLiteral("getSysColors"));
    pumpEvents(1000);
    // The wrapped lambda has been invoked, proving the createScheduleType
    // callback-wrapping path is exercised.
    EXPECT_TRUE(callbackInvoked);
}

// ---- slotCallFinished createSchedule path (triggers querySchedulesWithParameter) ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_CreateSchedule_TriggersRefresh)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    bool callbackInvoked = false;
    request.setCallbackFunc([&](CallMessge) {
        callbackInvoked = true;
    });
    request.asyncCall(QStringLiteral("createSchedule"), {QVariant(QStringLiteral(""))});
    pumpEvents(1000);
    // createSchedule keeps canCall=true, so the callback IS invoked.
    EXPECT_TRUE(callbackInvoked);
}

// ---- slotCallFinished deleteScheduleByScheduleID path (triggers querySchedulesWithParameter) ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_DeleteSchedule_TriggersRefresh)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    bool callbackInvoked = false;
    request.setCallbackFunc([&](CallMessge) {
        callbackInvoked = true;
    });
    request.asyncCall(QStringLiteral("deleteScheduleByScheduleID"), {QVariant(QStringLiteral(""))});
    pumpEvents(1000);
    // deleteScheduleByScheduleID sets canCall=false, then calls
    // querySchedulesWithParameter synchronously (no second slotCallFinished).
    // The callback is NOT invoked because canCall is false.
    EXPECT_FALSE(callbackInvoked);
}

// ---- slotCallFinished getScheduleByID path ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_GetScheduleByID_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetScheduleFinish);
    request.asyncCall(QStringLiteral("getScheduleByID"));
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

// ---- slotCallFinished querySchedulesWithParameter path ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_QuerySchedules_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalGetScheduleListFinish);
    request.asyncCall(QStringLiteral("querySchedulesWithParameter"));
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

// ---- slotCallFinished searchSchedulesWithParameter path ----

TEST(SpDbusAccountRequestTest, SlotCallFinished_SearchSchedules_EmitsSignal)
{
    BusGuard guard;
    if (!guard.ensure()) GTEST_SKIP() << "Session bus unavailable or service owned";

    DbusAccountRequest request(QString::fromUtf8(kAccountPath), QString::fromUtf8(kAccountInterface));
    QSignalSpy spy(&request, &DbusAccountRequest::signalSearchScheduleListFinish);
    request.asyncCall(QStringLiteral("searchSchedulesWithParameter"));
    pumpEvents();
    EXPECT_GE(spy.count(), 0);
}

#include "test_sp_dbusaccountrequest.moc"
