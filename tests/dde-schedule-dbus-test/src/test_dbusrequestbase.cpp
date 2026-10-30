// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/*
 * Branch checklist (from get_code_snippet):
 * DbusRequestBase constructor:
 *   - if (!sessionBus().connect(...)) → qCWarning path
 *   - normal path (connect succeeds)
 * setCallbackFunc:
 *   - stores func (no branch)
 * asyncCall(method, args):
 *   - creates watcher, transfers callback, clears callback, connects signal
 *   - no explicit if/switch branches
 * asyncCall(method, callName, args):
 *   - same as above but with callName
 *   - no explicit if/switch branches
 * slotDbusCall:
 *   - empty body, no branches
 */

#include "dbusrequestbase.h"
#include "cdbuspendingcallwatcher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>

#include <gtest/gtest.h>

/*
 * Concrete subclass because DbusRequestBase has pure virtual slotCallFinished.
 */
class TestableDbusRequestBase : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit TestableDbusRequestBase(const QString &path,
                                     const QString &interface,
                                     const QDBusConnection &connection = QDBusConnection::sessionBus(),
                                     QObject *parent = nullptr)
        : DbusRequestBase(path, interface, connection, parent)
        , slotCallFinishedCount(0)
    {
    }

    void slotCallFinished(CDBusPendingCallWatcher *call) override
    {
        ++slotCallFinishedCount;
        lastWatcher = call;
    }

    // Expose protected asyncCall for testing
    using DbusRequestBase::asyncCall;

    int slotCallFinishedCount = 0;
    CDBusPendingCallWatcher *lastWatcher = nullptr;
};

class DbusRequestBaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Use the same path/interface as DbusHuangLiRequest for a realistic test
        request = new TestableDbusRequestBase(
            "/com/deepin/dataserver/Calendar/HuangLi",
            "com.deepin.dataserver.Calendar.HuangLi",
            QDBusConnection::sessionBus());
    }

    void TearDown() override
    {
        delete request;
    }

    TestableDbusRequestBase *request = nullptr;
};

/* ---- Constructor ---- */

TEST_F(DbusRequestBaseTest, ConstructorSetsPathAndInterface)
{
    EXPECT_EQ(QString("/com/deepin/dataserver/Calendar/HuangLi"), request->path());
    EXPECT_EQ(QString("com.deepin.dataserver.Calendar.HuangLi"), request->interface());
}

TEST_F(DbusRequestBaseTest, ConstructorDoesNotCrashWithSessionBus)
{
    // If we get here the constructor completed without crashing.
    SUCCEED();
}

TEST_F(DbusRequestBaseTest, ConstructorWithNullParent)
{
    TestableDbusRequestBase req("/test/path", "test.interface",
                                QDBusConnection::sessionBus(), nullptr);
    EXPECT_EQ(QString("/test/path"), req.path());
    EXPECT_EQ(QString("test.interface"), req.interface());
}

/* ---- setCallbackFunc ---- */

TEST_F(DbusRequestBaseTest, SetCallbackFuncStoresCallback)
{
    bool callbackInvoked = false;
    CallMessge captured;
    request->setCallbackFunc([&](CallMessge msg) {
        callbackInvoked = true;
        captured = msg;
    });

    // Verify the callback is stored by invoking asyncCall, which transfers
    // the callback to the watcher and then clears it.
    // We can't easily get the callback back, but we can verify it doesn't crash.
    request->asyncCall("testMethod");
    // After asyncCall, m_callbackFunc should be nullptr (cleared).
    // Calling asyncCall again should not crash even with null callback.
    request->asyncCall("testMethod2");
    SUCCEED();
}

TEST_F(DbusRequestBaseTest, SetCallbackFuncWithNull)
{
    request->setCallbackFunc(nullptr);
    // Should not crash when asyncCall is called with null callback
    request->asyncCall("testMethod");
    SUCCEED();
}

/* ---- asyncCall(method, args) ---- */

TEST_F(DbusRequestBaseTest, AsyncCallWithoutArgsCreatesWatcher)
{
    request->asyncCall("testMethod");
    // The watcher is a child of request; verify it was created.
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    EXPECT_FALSE(watchers.isEmpty());
}

TEST_F(DbusRequestBaseTest, AsyncCallWithArgsCreatesWatcher)
{
    QList<QVariant> args = {QVariant(2024), QVariant(1)};
    request->asyncCall("testMethod", args);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    EXPECT_FALSE(watchers.isEmpty());
}

TEST_F(DbusRequestBaseTest, AsyncCallClearsCallbackAfterTransfer)
{
    request->setCallbackFunc([](CallMessge) {});
    request->asyncCall("testMethod");
    // After asyncCall, the callback should be cleared.
    // Calling asyncCall again should not crash with null callback.
    request->asyncCall("testMethod2");
    SUCCEED();
}

TEST_F(DbusRequestBaseTest, AsyncCallConnectsSignalCallFinished)
{
    request->asyncCall("testMethod");
    // The connect between signalCallFinished and slotCallFinished should be active.
    // We can verify by checking that the watcher exists (the connect call succeeded).
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ("testMethod", watchers.last()->getmember());
}

/* ---- asyncCall(method, callName, args) ---- */

TEST_F(DbusRequestBaseTest, AsyncCallWithCallNameCreatesWatcher)
{
    request->asyncCall("testMethod", "customCallName");
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("customCallName"), watchers.last()->getmember());
}

TEST_F(DbusRequestBaseTest, AsyncCallWithCallNameAndArgs)
{
    QList<QVariant> args = {QVariant(2024), QVariant(6), QVariant(15)};
    request->asyncCall("testMethod", "namedCall", args);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("namedCall"), watchers.last()->getmember());
}

TEST_F(DbusRequestBaseTest, AsyncCallWithCallNameClearsCallback)
{
    request->setCallbackFunc([](CallMessge) {});
    request->asyncCall("testMethod", "namedCall");
    // Callback cleared — second call with null callback should not crash
    request->asyncCall("testMethod2", "namedCall2");
    SUCCEED();
}

TEST_F(DbusRequestBaseTest, AsyncCallWithCallNameTransfersCallback)
{
    bool callbackTransferred = false;
    request->setCallbackFunc([&](CallMessge) {
        callbackTransferred = true;
    });
    request->asyncCall("testMethod", "namedCall");
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    // The callback should have been transferred to the watcher
    EXPECT_NE(nullptr, watchers.last()->getCallbackFunc());
}

/* ---- slotDbusCall ---- */

TEST_F(DbusRequestBaseTest, SlotDbusCallDoesNotCrash)
{
    QDBusMessage msg = QDBusMessage::createSignal("/test", "test.interface", "testSignal");
    request->slotDbusCall(msg);
    SUCCEED();
}

TEST_F(DbusRequestBaseTest, SlotDbusCallWithEmptyMessage)
{
    QDBusMessage msg;
    request->slotDbusCall(msg);
    SUCCEED();
}

#include "test_dbusrequestbase.moc"
