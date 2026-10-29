// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "dbustimedate.h"

#include <QSignalSpy>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QVariant>
#include <QVariantMap>
#include <QList>

// DBusTimedate depends on a session bus; in test environments without
// the timedate service, the constructor still completes (DBus calls fail
// gracefully). We test the non-DBus-dependent logic paths.

// ============ Constructor (no DBus service) ============

TEST(DBusTimedateTest, Constructor_DoesNotCrashWithoutDBus)
{
    // The constructor connects to DBus signals and calls getHasDateTimeFormat().
    // Without a timedate service, it should still construct without crashing.
    DBusTimedate *timedate = nullptr;
    EXPECT_NO_FATAL_FAILURE({
        timedate = new DBusTimedate();
    });
    ASSERT_NE(timedate, nullptr);
    // Without DBus, m_hasDateTimeFormat should be false
    // so shortTimeFormat returns default 4
    EXPECT_EQ(timedate->shortTimeFormat(), 4);
    EXPECT_EQ(timedate->shortDateFormat(), 1);
    EXPECT_EQ(timedate->weekBegins(), Qt::Monday);
    delete timedate;
}

// ============ shortTimeFormat (default path) ============

TEST(DBusTimedateTest, ShortTimeFormat_DefaultIs4)
{
    DBusTimedate timedate;
    // Without DBus service, m_hasDateTimeFormat is false → returns 4
    EXPECT_EQ(timedate.shortTimeFormat(), 4);
}

// ============ shortDateFormat (default path) ============

TEST(DBusTimedateTest, ShortDateFormat_DefaultIs1)
{
    DBusTimedate timedate;
    EXPECT_EQ(timedate.shortDateFormat(), 1);
}

// ============ weekBegins (default path) ============

TEST(DBusTimedateTest, WeekBegins_DefaultIsMonday)
{
    DBusTimedate timedate;
    EXPECT_EQ(timedate.weekBegins(), Qt::Monday);
}

// ============ timeUpdated signal ============

TEST(DBusTimedateTest, TimeUpdated_EmitsTimeChanged)
{
    DBusTimedate timedate;
    QSignalSpy spy(&timedate, &DBusTimedate::TimeChanged);
    timedate.timeUpdated();
    EXPECT_EQ(spy.count(), 1);
}

TEST(DBusTimedateTest, TimeUpdated_MultipleCalls_EmitsEachTime)
{
    DBusTimedate timedate;
    QSignalSpy spy(&timedate, &DBusTimedate::TimeChanged);
    timedate.timeUpdated();
    timedate.timeUpdated();
    timedate.timeUpdated();
    EXPECT_EQ(spy.count(), 3);
}

// ============ propertiesChanged (error paths) ============

TEST(DBusTimedateTest, PropertiesChanged_WrongArgCount_NoSignal)
{
    DBusTimedate timedate;
    QSignalSpy spyTime(&timedate, &DBusTimedate::ShortTimeFormatChanged);
    QSignalSpy spyDate(&timedate, &DBusTimedate::ShortDateFormatChanged);

    // Create a message with wrong number of arguments (0 args)
    QDBusMessage msg = QDBusMessage::createSignal("/", "test.interface", "PropertiesChanged");
    timedate.propertiesChanged(msg);

    EXPECT_EQ(spyTime.count(), 0);
    EXPECT_EQ(spyDate.count(), 0);
}

TEST(DBusTimedateTest, PropertiesChanged_WrongInterface_NoSignal)
{
    DBusTimedate timedate;
    QSignalSpy spyTime(&timedate, &DBusTimedate::ShortTimeFormatChanged);

    // Create a message with 3 args but wrong interface name
    QDBusMessage msg = QDBusMessage::createSignal("/", "test.interface", "PropertiesChanged");
    QList<QVariant> args;
    args << QVariant("wrong.interface");
    QVariantMap changedProps;
    changedProps.insert("ShortTimeFormat", 5);
    args << QVariant(changedProps);
    args << QVariant(QStringList());
    msg.setArguments(args);

    timedate.propertiesChanged(msg);
    EXPECT_EQ(spyTime.count(), 0);
}

TEST(DBusTimedateTest, PropertiesChanged_TwoArgs_NoSignal)
{
    DBusTimedate timedate;
    QSignalSpy spyTime(&timedate, &DBusTimedate::ShortTimeFormatChanged);

    QDBusMessage msg = QDBusMessage::createSignal("/", "test.interface", "PropertiesChanged");
    QList<QVariant> args;
    args << QVariant(timedate.interface());
    args << QVariant(QVariantMap());
    msg.setArguments(args);

    timedate.propertiesChanged(msg);
    // Only 2 args → should return early
    EXPECT_EQ(spyTime.count(), 0);
}
