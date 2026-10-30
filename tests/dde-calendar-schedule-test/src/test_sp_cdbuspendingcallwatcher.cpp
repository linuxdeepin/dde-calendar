// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cdbuspendingcallwatcher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QSignalSpy>

#include <gtest/gtest.h>

// 分支清单：
// - constructor: connects finished -> signalCallFinished
// - setCallbackFunc: stores func
// - getCallbackFunc: returns stored func / nullptr default
// - getmember: returns m_member

namespace {

QDBusPendingCall makeDummyPendingCall(const QString &member)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.example.NoService", "/org/example/NoObject",
        "org.example.NoInterface", member);
    return QDBusConnection::sessionBus().asyncCall(msg);
}

} // namespace

TEST(SpCDBusPendingCallWatcherTest, Constructor_StoresMember)
{
    QDBusPendingCall call = makeDummyPendingCall("testMethod");
    CDBusPendingCallWatcher watcher(call, QStringLiteral("testMethod"));
    EXPECT_EQ(watcher.getmember(), QStringLiteral("testMethod"));
}

TEST(SpCDBusPendingCallWatcherTest, GetCallbackFunc_DefaultIsNull)
{
    QDBusPendingCall call = makeDummyPendingCall("method");
    CDBusPendingCallWatcher watcher(call, QStringLiteral("method"));
    EXPECT_EQ(watcher.getCallbackFunc(), nullptr);
}

TEST(SpCDBusPendingCallWatcherTest, SetAndGetCallbackFunc_RoundTripsValue)
{
    QDBusPendingCall call = makeDummyPendingCall("method");
    CDBusPendingCallWatcher watcher(call, QStringLiteral("method"));

    bool invoked = false;
    int capturedCode = -1;
    CallbackFunc func = [&](CallMessge result) {
        invoked = true;
        capturedCode = result.code;
    };

    watcher.setCallbackFunc(func);
    CallbackFunc retrieved = watcher.getCallbackFunc();
    ASSERT_NE(retrieved, nullptr);
    retrieved({0, QString()});
    EXPECT_TRUE(invoked);
    EXPECT_EQ(capturedCode, 0);
}

TEST(SpCDBusPendingCallWatcherTest, SignalCallFinished_EmittedWhenPendingCallFinishes)
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        GTEST_SKIP() << "DBus session bus not available";
    }

    QDBusPendingCall call = makeDummyPendingCall("method");
    CDBusPendingCallWatcher watcher(call, QStringLiteral("method"));

    QSignalSpy spy(&watcher, &CDBusPendingCallWatcher::signalCallFinished);
    spy.wait(3000);
    EXPECT_GE(spy.count(), 1);
}
