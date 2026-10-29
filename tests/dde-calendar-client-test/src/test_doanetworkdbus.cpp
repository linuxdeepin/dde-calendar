// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "doanetworkdbus.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>
#include <QVariant>
#include <QVariantMap>

#include <gtest/gtest.h>

namespace {

bool networkManagerAvailable()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected())
        return false;
    return bus.interface()->isServiceRegistered(
        QStringLiteral("org.freedesktop.NetworkManager"));
}

void sendPropertiesChanged(const QString &testPath, const QString &interfaceName,
                           const QVariantMap &changedProps)
{
    QDBusMessage msg = QDBusMessage::createSignal(
        testPath, "org.freedesktop.DBus.Properties", "PropertiesChanged");
    msg << interfaceName;
    msg << QVariant::fromValue(changedProps);
    msg << QStringList();
    QDBusConnection::sessionBus().send(msg);
}

} // namespace

TEST(DOANetWorkDBusTest, ConstructorCreatesInstance)
{
    DOANetWorkDBus dbus;
    SUCCEED();
}

TEST(DOANetWorkDBusTest, GetNetWorkState)
{
    DOANetWorkDBus dbus;
    DOANetWorkDBus::NetWorkState state = dbus.getNetWorkState();
    EXPECT_TRUE(state == DOANetWorkDBus::Active ||
                state == DOANetWorkDBus::Disconnect ||
                state == DOANetWorkDBus::Connecting ||
                state == DOANetWorkDBus::unknow);
}

TEST(DOANetWorkDBusTest, GetNetWorkStateActiveWhenServiceRunning)
{
    if (!networkManagerAvailable()) {
        GTEST_SKIP() << "NetworkManager is not available on system bus";
    }
    DOANetWorkDBus dbus;
    DOANetWorkDBus::NetWorkState state = dbus.getNetWorkState();
    EXPECT_EQ(state, DOANetWorkDBus::Active);
}

TEST(DOANetWorkDBusTest, PropertiesChangedIgnoresWrongArgCount)
{
    DOANetWorkDBus dbus;

    QDBusMessage msg = QDBusMessage::createSignal(
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");
    msg << QStringLiteral("org.freedesktop.NetworkManager");

    QSignalSpy spy(&dbus, &DOANetWorkDBus::sign_NetWorkChange);
    dbus.propertiesChanged(msg);
    EXPECT_EQ(spy.count(), 0);
}

TEST(DOANetWorkDBusTest, PropertiesChangedIgnoresWrongInterface)
{
    DOANetWorkDBus dbus;

    QDBusMessage msg = QDBusMessage::createSignal(
        "/org/freedesktop/NetworkManager",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");
    msg << QStringLiteral("org.some.other.Interface");
    QVariantMap changedProps;
    changedProps.insert("State", QVariant(70));
    msg << QVariant::fromValue(changedProps);
    msg << QStringList();

    QSignalSpy spy(&dbus, &DOANetWorkDBus::sign_NetWorkChange);
    dbus.propertiesChanged(msg);
    EXPECT_EQ(spy.count(), 0);
}

TEST(DOANetWorkDBusTest, PropertiesChangedEmitsActiveOnState70)
{
    DOANetWorkDBus dbus;
    qRegisterMetaType<DOANetWorkDBus::NetWorkState>("DOANetWorkDBus::NetWorkState");

    QDBusConnection bus = QDBusConnection::sessionBus();
    ASSERT_TRUE(bus.isConnected());

    const QString testPath = QStringLiteral("/test/propchange70");

    ASSERT_TRUE(bus.connect(QString(), testPath,
                            "org.freedesktop.DBus.Properties", "PropertiesChanged",
                            "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage))));

    QSignalSpy spy(&dbus, &DOANetWorkDBus::sign_NetWorkChange);

    QVariantMap changedProps;
    changedProps.insert("State", QVariant(70));
    sendPropertiesChanged(testPath, QStringLiteral("org.freedesktop.NetworkManager"), changedProps);

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() >= 1) {
        EXPECT_EQ(spy.takeFirst().at(0).value<DOANetWorkDBus::NetWorkState>(),
                  DOANetWorkDBus::Active);
    }

    bus.disconnect(QString(), testPath,
                   "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage)));
}

TEST(DOANetWorkDBusTest, PropertiesChangedEmitsDisconnectOnState20)
{
    DOANetWorkDBus dbus;
    qRegisterMetaType<DOANetWorkDBus::NetWorkState>("DOANetWorkDBus::NetWorkState");

    QDBusConnection bus = QDBusConnection::sessionBus();
    ASSERT_TRUE(bus.isConnected());

    const QString testPath = QStringLiteral("/test/propchange20");

    ASSERT_TRUE(bus.connect(QString(), testPath,
                            "org.freedesktop.DBus.Properties", "PropertiesChanged",
                            "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage))));

    QSignalSpy spy(&dbus, &DOANetWorkDBus::sign_NetWorkChange);

    QVariantMap changedProps;
    changedProps.insert("State", QVariant(20));
    sendPropertiesChanged(testPath, QStringLiteral("org.freedesktop.NetworkManager"), changedProps);

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_EQ(spy.count(), 1);
    if (spy.count() >= 1) {
        EXPECT_EQ(spy.takeFirst().at(0).value<DOANetWorkDBus::NetWorkState>(),
                  DOANetWorkDBus::Disconnect);
    }

    bus.disconnect(QString(), testPath,
                   "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage)));
}

TEST(DOANetWorkDBusTest, PropertiesChangedIgnoresOtherStateValues)
{
    DOANetWorkDBus dbus;
    qRegisterMetaType<DOANetWorkDBus::NetWorkState>("DOANetWorkDBus::NetWorkState");

    QDBusConnection bus = QDBusConnection::sessionBus();
    ASSERT_TRUE(bus.isConnected());

    const QString testPath = QStringLiteral("/test/propchange99");

    ASSERT_TRUE(bus.connect(QString(), testPath,
                            "org.freedesktop.DBus.Properties", "PropertiesChanged",
                            "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage))));

    QSignalSpy spy(&dbus, &DOANetWorkDBus::sign_NetWorkChange);

    QVariantMap changedProps;
    changedProps.insert("State", QVariant(99));
    sendPropertiesChanged(testPath, QStringLiteral("org.freedesktop.NetworkManager"), changedProps);

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_EQ(spy.count(), 0);

    bus.disconnect(QString(), testPath,
                   "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage)));
}

TEST(DOANetWorkDBusTest, PropertiesChangedIgnoresNonStateProps)
{
    DOANetWorkDBus dbus;
    qRegisterMetaType<DOANetWorkDBus::NetWorkState>("DOANetWorkDBus::NetWorkState");

    QDBusConnection bus = QDBusConnection::sessionBus();
    ASSERT_TRUE(bus.isConnected());

    const QString testPath = QStringLiteral("/test/propchangeother");

    ASSERT_TRUE(bus.connect(QString(), testPath,
                            "org.freedesktop.DBus.Properties", "PropertiesChanged",
                            "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage))));

    QSignalSpy spy(&dbus, &DOANetWorkDBus::sign_NetWorkChange);

    QVariantMap changedProps;
    changedProps.insert("Connectivity", QVariant(4));
    sendPropertiesChanged(testPath, QStringLiteral("org.freedesktop.NetworkManager"), changedProps);

    QEventLoop loop;
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();

    EXPECT_EQ(spy.count(), 0);

    bus.disconnect(QString(), testPath,
                   "org.freedesktop.DBus.Properties", "PropertiesChanged",
                   "sa{sv}as", &dbus, SLOT(propertiesChanged(QDBusMessage)));
}
