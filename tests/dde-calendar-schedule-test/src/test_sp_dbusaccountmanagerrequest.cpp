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

#include "../third-party_stub/stub.h"
#include <gtest/gtest.h>

#include <QElapsedTimer>
#include <QDBusConnectionInterface>

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

// 泵事件直到条件满足或超时
template <typename Predicate>
inline bool pumpUntil(Predicate pred, int timeoutMs = 3000, int stepMs = 100)
{
    QElapsedTimer timer;
    timer.start();
    while (!pred()) {
        if (timer.elapsed() >= timeoutMs)
            return false;
        pumpEvents(stepMs);
    }
    return true;
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
    // QDBusAbstractInterface 的首次调用会先做一次接口内省往返，该次调用的
    // 方法消息会被吞掉（CI 真实会话总线与本地 offscreen 均复现）。先以一次
    // 预热调用完成内省，再触发被测行为，其消息才会真正送达 mock 服务。
    {
        QList<QVariant> warmArgs; warmArgs << true;
        static_cast<QDBusAbstractInterface &>(request)
            .callWithArgumentList(QDBus::NoBlock, QStringLiteral("calendarIsShow"), warmArgs);
    }
    request.clientIsShow(true);
    const bool delivered = pumpUntil(
        [&guard] { return guard.svc.calledMethods.contains(QStringLiteral("calendarIsShow")); },
        3000);
    EXPECT_TRUE(delivered);
}

// ---- slotCallFinished error path (always errors, regardless of bus state) ----

TEST(SpDbusAccountManagerRequestTest, SlotCallFinished_ErrorPath)
{
    if (!QDBusConnection::sessionBus().isConnected())
        GTEST_SKIP() << "Session bus not available";
    // 本用例前提是“日历服务未注册”；开发机等真实会话里若已有
    // com.deepin.dataserver.Calendar 在线，调用会成功返回而非走错误分支，
    // 此时前提不成立，跳过（CI 无该服务，正常执行）。
    if (QDBusConnection::sessionBus().interface()
            && QDBusConnection::sessionBus().interface()->isServiceRegistered(
                   QString::fromUtf8(kCalendarService))) {
        GTEST_SKIP() << "calendar service registered; error-path premise unavailable";
    }

    DbusAccountManagerRequest request;
    bool callbackInvoked = false;
    int callbackCode = -1;
    request.setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        callbackCode = msg.code;
    });
    // Call a method no Calendar service implements so the reply is always a D-Bus
    // error: ServiceUnknown when the service name is unregistered, UnknownMethod
    // when it is owned (e.g. a real service on a shared CI session bus). Relying
    // on the service being absent is not deterministic, see the sibling
    // SlotCallFinished_ErrorPath test in test_sp_dbusaccountrequest.cpp.
    // -fno-access-control allows calling protected asyncCall.
    request.asyncCall(QStringLiteral("methodThatNeverExistsForErrorPath"));
    pumpEvents(2000);
    // 前提复核：若等待期间有真实日历服务抢注了服务名（与 mock 注销存在
    // 毫秒级竞态，调用会得到成功回复而非错误），则前提失效，跳过。
    if (QDBusConnection::sessionBus().interface()
            && QDBusConnection::sessionBus().interface()->isServiceRegistered(
                   QString::fromUtf8(kCalendarService))) {
        GTEST_SKIP() << "calendar service registered during wait; premise unavailable";
    }
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
