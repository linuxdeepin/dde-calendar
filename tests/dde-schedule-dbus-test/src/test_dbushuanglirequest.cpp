// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/*
 * Branch checklist (from get_code_snippet):
 * Constructor:
 *   - calls DbusRequestBase(path, interface, sessionBus, parent)
 *   - no explicit branches
 * getFestivalMonth / getHuangLiDay / getHuangLiMonth / getLunarInfoBySolar / getLunarMonthCalendar:
 *   - each calls asyncCall with appropriate args
 *   - no explicit branches
 * slotCallFinished(CDBusPendingCallWatcher *call):
 *   - if (call->isError()) → qCWarning + return (error path)
 *   - if (call->getmember() == "getFestivalMonth") → create QDBusPendingReply<QString>, extract arg
 *   - fallthrough → call->deleteLater() (success, non-getFestivalMonth path)
 */

#include "dbushuanglirequest.h"
#include "cdbuspendingcallwatcher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QTimer>
#include <QEventLoop>
#include <QCoreApplication>

#include <gtest/gtest.h>

class DbusHuangLiRequestTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        request = new DbusHuangLiRequest();
    }

    void TearDown() override
    {
        delete request;
    }

    DbusHuangLiRequest *request = nullptr;
};

/* ---- Constructor ---- */

TEST_F(DbusHuangLiRequestTest, ConstructorSetsCorrectPath)
{
    EXPECT_EQ(QString("/com/deepin/dataserver/Calendar/HuangLi"), request->path());
}

TEST_F(DbusHuangLiRequestTest, ConstructorSetsCorrectInterface)
{
    EXPECT_EQ(QString("com.deepin.dataserver.Calendar.HuangLi"), request->interface());
}

TEST_F(DbusHuangLiRequestTest, ConstructorDoesNotCrashWithNullParent)
{
    DbusHuangLiRequest req(nullptr);
    EXPECT_EQ(QString("/com/deepin/dataserver/Calendar/HuangLi"), req.path());
}

/* ---- getFestivalMonth ---- */

TEST_F(DbusHuangLiRequestTest, GetFestivalMonthCreatesWatcher)
{
    request->getFestivalMonth(2024, 1);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getFestivalMonth"), watchers.last()->getmember());
}

TEST_F(DbusHuangLiRequestTest, GetFestivalMonthWithZeroValues)
{
    request->getFestivalMonth(0, 0);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getFestivalMonth"), watchers.last()->getmember());
}

TEST_F(DbusHuangLiRequestTest, GetFestivalMonthWithMaxValues)
{
    request->getFestivalMonth(9999, 12);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getFestivalMonth"), watchers.last()->getmember());
}

/* ---- getHuangLiDay ---- */

TEST_F(DbusHuangLiRequestTest, GetHuangLiDayCreatesWatcher)
{
    request->getHuangLiDay(2024, 6, 15);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getHuangLiDay"), watchers.last()->getmember());
}

TEST_F(DbusHuangLiRequestTest, GetHuangLiDayWithBoundaryValues)
{
    request->getHuangLiDay(2024, 1, 1);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getHuangLiDay"), watchers.last()->getmember());
}

/* ---- getHuangLiMonth ---- */

TEST_F(DbusHuangLiRequestTest, GetHuangLiMonthWithFillTrue)
{
    request->getHuangLiMonth(2024, 3, true);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getHuangLiMonth"), watchers.last()->getmember());
}

TEST_F(DbusHuangLiRequestTest, GetHuangLiMonthWithFillFalse)
{
    request->getHuangLiMonth(2024, 3, false);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getHuangLiMonth"), watchers.last()->getmember());
}

/* ---- getLunarInfoBySolar ---- */

TEST_F(DbusHuangLiRequestTest, GetLunarInfoBySolarCreatesWatcher)
{
    request->getLunarInfoBySolar(2024, 6, 15);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getLunarInfoBySolar"), watchers.last()->getmember());
}

TEST_F(DbusHuangLiRequestTest, GetLunarInfoBySolarWithZeroValues)
{
    request->getLunarInfoBySolar(0, 0, 0);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getLunarInfoBySolar"), watchers.last()->getmember());
}

/* ---- getLunarMonthCalendar ---- */

TEST_F(DbusHuangLiRequestTest, GetLunarMonthCalendarWithFillTrue)
{
    request->getLunarMonthCalendar(2024, 5, true);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getLunarMonthCalendar"), watchers.last()->getmember());
}

TEST_F(DbusHuangLiRequestTest, GetLunarMonthCalendarWithFillFalse)
{
    request->getLunarMonthCalendar(2024, 5, false);
    auto watchers = request->findChildren<CDBusPendingCallWatcher *>();
    ASSERT_FALSE(watchers.isEmpty());
    EXPECT_EQ(QString("getLunarMonthCalendar"), watchers.last()->getmember());
}

/* ---- slotCallFinished ---- */

TEST_F(DbusHuangLiRequestTest, SlotCallFinishedWithErrorReturnsEarly)
{
    // Create a pending call that will be in error state.
    // We use a real asyncCall to get a watcher, then manually trigger
    // slotCallFinished on a watcher that reports an error.
    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(
        QDBusMessage::createMethodCall(
            "com.deepin.nonexistent.Service",
            "/nonexistent/path",
            "nonexistent.interface",
            "nonexistentMethod"));
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(pendingCall, "testMethod", request);

    // Wait for the pending call to finish (it will be an error)
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();
    timer.stop();

    // The call should be in error state (nonexistent service)
    EXPECT_TRUE(watcher->isError());
    // slotCallFinished should return early without calling deleteLater
    request->slotCallFinished(watcher);
    // Watcher should NOT be deleted (deleteLater not called on error path)
    // The watcher is still a child of request, so it won't be a dangling pointer
    // Verify it's still valid
    EXPECT_TRUE(watcher->isError());

    delete watcher;
}

TEST_F(DbusHuangLiRequestTest, SlotCallFinishedWithGetFestivalMonthSuccess)
{
    // This test exercises the getFestivalMonth branch in slotCallFinished.
    // We need a watcher that is NOT in error state with member "getFestivalMonth".
    // Since we can't easily create a non-error pending call without a real DBus service,
    // we register a mock service on the session bus.
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    // Create a pending call to a real registered object to get a non-error reply.
    // We'll use QDBusConnection's own service as a simple ping.
    QDBusMessage pingMsg = QDBusMessage::createMethodCall(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "ListNames");
    QDBusPendingCall pendingCall = bus.asyncCall(pingMsg);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(pendingCall, "getFestivalMonth", request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();
    timer.stop();

    // The ListNames call should succeed
    if (watcher->isError()) {
        GTEST_SKIP() << "DBus ListNames call failed unexpectedly";
    }

    EXPECT_FALSE(watcher->isError());
    // This exercises the getFestivalMonth branch (creates QDBusPendingReply<QString>)
    // and then calls deleteLater().
    request->slotCallFinished(watcher);
    // deleteLater was called, so we can't use the watcher anymore.
    // Process events to ensure deleteLater takes effect.
    SUCCEED();
}

TEST_F(DbusHuangLiRequestTest, SlotCallFinishedWithNonFestivalMonthSuccess)
{
    // Exercises the success path where getmember() != "getFestivalMonth"
    // → falls through to deleteLater() without creating QDBusPendingReply.
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "ListNames");
    QDBusPendingCall pendingCall = bus.asyncCall(msg);
    CDBusPendingCallWatcher *watcher = new CDBusPendingCallWatcher(pendingCall, "getHuangLiDay", request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();
    timer.stop();

    if (watcher->isError()) {
        GTEST_SKIP() << "DBus ListNames call failed unexpectedly";
    }

    EXPECT_FALSE(watcher->isError());
    EXPECT_EQ(QString("getHuangLiDay"), watcher->getmember());
    // Exercises the non-getFestivalMonth success path → deleteLater
    request->slotCallFinished(watcher);
    SUCCEED();
}

