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
// - constructor: connects finished -> signalCallFinished (covered by SignalEmitted test)
// - setCallbackFunc: stores func (covered by SetAndGetMapping tests)
// - getCallbackFunc: returns stored func / nullptr default (covered by two tests)
// - getmember: returns m_member (covered by Getmember_ReturnsConstructorValue)

TEST(CDBusPendingCallWatcherTest, Getmember_ReturnsConstructorValue)
{
    // Arrange
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.example.NoService", "/org/example/NoObject",
        "org.example.NoInterface", "NoMethod");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);

    // Act
    CDBusPendingCallWatcher watcher(call, QStringLiteral("testMethod"));
    QString member = watcher.getmember();

    // Assert
    EXPECT_EQ(member, QStringLiteral("testMethod"));
}

TEST(CDBusPendingCallWatcherTest, GetCallbackFunc_DefaultIsNull)
{
    // Arrange
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.example.NoService", "/org/example/NoObject",
        "org.example.NoInterface", "NoMethod");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    CDBusPendingCallWatcher watcher(call, QStringLiteral("method"));

    // Act
    CallbackFunc func = watcher.getCallbackFunc();

    // Assert
    EXPECT_EQ(func, nullptr);
}

TEST(CDBusPendingCallWatcherTest, SetAndGetCallbackFunc_RoundTripsValue)
{
    // Arrange
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.example.NoService", "/org/example/NoObject",
        "org.example.NoInterface", "NoMethod");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    CDBusPendingCallWatcher watcher(call, QStringLiteral("method"));

    bool invoked = false;
    int capturedCode = -1;
    CallbackFunc func = [&](CallMessge result) {
        invoked = true;
        capturedCode = result.code;
    };

    // Act
    watcher.setCallbackFunc(func);
    CallbackFunc retrieved = watcher.getCallbackFunc();
    ASSERT_NE(retrieved, nullptr);
    retrieved({0, QVariant()});

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(capturedCode, 0);
}

TEST(CDBusPendingCallWatcherTest, SignalCallFinished_EmittedWhenPendingCallFinishes)
{
    // Skip when no session bus is available — the pending call will never complete.
    if (!QDBusConnection::sessionBus().isConnected()) {
        GTEST_SKIP() << "DBus session bus not available";
    }

    // Arrange: use a call to a non-existent service so it finishes quickly with error.
    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.example.NoService", "/org/example/NoObject",
        "org.example.NoInterface", "NoMethod");
    QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
    CDBusPendingCallWatcher watcher(call, QStringLiteral("method"));

    QSignalSpy spy(&watcher, &CDBusPendingCallWatcher::signalCallFinished);

    // Act: wait for the pending call to finish (timeout 3s).
    spy.wait(3000);

    // Assert
    EXPECT_GE(spy.count(), 1);
    if (spy.count() >= 1) {
        // First argument should be the watcher pointer itself.
        EXPECT_EQ(spy.at(0).at(0).value<CDBusPendingCallWatcher *>(), &watcher);
    }
}

