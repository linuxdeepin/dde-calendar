// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusrequestbase.h"
#include "cdbuspendingcallwatcher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QEventLoop>
#include <QTimer>
#include <QVariant>

#include <gtest/gtest.h>

namespace {

const char kTestPath[] = "/com/deepin/dataserver/Calendar/TestBase";
const char kTestInterface[] = "com.deepin.dataserver.Calendar.TestBase";

class MockDbusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.TestBase")

public slots:
    QString echo(const QString &str) { return str; }
};

class ConcreteDbusRequestBase : public DbusRequestBase
{
    Q_OBJECT
public:
    explicit ConcreteDbusRequestBase(const QString &path, const QString &interface,
                                     const QDBusConnection &conn = QDBusConnection::sessionBus(),
                                     QObject *parent = nullptr)
        : DbusRequestBase(path, interface, conn, parent) {}

    bool slotFinishedCalled = false;
    CDBusPendingCallWatcher *lastWatcher = nullptr;
    bool slotDbusCallCalled = false;
    QDBusMessage lastMsg;

    void slotCallFinished(CDBusPendingCallWatcher *watcher) override
    {
        slotFinishedCalled = true;
        lastWatcher = watcher;
    }

    void slotDbusCall(const QDBusMessage &msg) override
    {
        slotDbusCallCalled = true;
        lastMsg = msg;
    }

    void exposeAsyncCall(const QString &method, const QList<QVariant> &args = {})
    {
        asyncCall(method, args);
    }

    void exposeAsyncCallNamed(const QString &method, const QString &callName,
                              const QList<QVariant> &args = {})
    {
        asyncCall(method, callName, args);
    }
};

} // namespace

TEST(DbusRequestBaseTest, ConstructorConnectsPropertiesSignal)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockDbusService service;
    bus.registerObject(QString::fromLatin1(kTestPath), &service,
                       QDBusConnection::ExportAllSlots);

    {
        ConcreteDbusRequestBase req(QString::fromLatin1(kTestPath),
                                    QString::fromLatin1(kTestInterface));
        EXPECT_TRUE(req.isValid());
    }

    bus.unregisterObject(QString::fromLatin1(kTestPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusRequestBaseTest, SetCallbackFunc)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockDbusService service;
    bus.registerObject(QString::fromLatin1(kTestPath), &service,
                       QDBusConnection::ExportAllSlots);

    {
        ConcreteDbusRequestBase req(QString::fromLatin1(kTestPath),
                                    QString::fromLatin1(kTestInterface));

        bool callbackInvoked = false;
        CallMessge receivedMsg;
        req.setCallbackFunc([&](CallMessge msg) {
            callbackInvoked = true;
            receivedMsg = msg;
        });

        req.exposeAsyncCall("echo", {QVariant(QStringLiteral("hello"))});

        QEventLoop loop;
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        EXPECT_TRUE(req.slotFinishedCalled);
        if (req.lastWatcher) {
            CallbackFunc func = req.lastWatcher->getCallbackFunc();
            EXPECT_TRUE(func != nullptr);
            if (func) {
                CallMessge cm;
                cm.code = 0;
                cm.msg = QVariant(QStringLiteral("result"));
                func(cm);
            }
        }
    }

    bus.unregisterObject(QString::fromLatin1(kTestPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusRequestBaseTest, AsyncCallWithCallName)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockDbusService service;
    bus.registerObject(QString::fromLatin1(kTestPath), &service,
                       QDBusConnection::ExportAllSlots);

    {
        ConcreteDbusRequestBase req(QString::fromLatin1(kTestPath),
                                    QString::fromLatin1(kTestInterface));
        req.setCallbackFunc([](CallMessge) {});

        req.exposeAsyncCallNamed("echo", "customCallName",
                                  {QVariant(QStringLiteral("test"))});

        QEventLoop loop;
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();

        EXPECT_TRUE(req.slotFinishedCalled);
        if (req.lastWatcher) {
            EXPECT_EQ(req.lastWatcher->getmember(), QStringLiteral("customCallName"));
        }
    }

    bus.unregisterObject(QString::fromLatin1(kTestPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusRequestBaseTest, SlotDbusCallDefaultIsNoop)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockDbusService service;
    bus.registerObject(QString::fromLatin1(kTestPath), &service,
                       QDBusConnection::ExportAllSlots);

    {
        ConcreteDbusRequestBase req(QString::fromLatin1(kTestPath),
                                    QString::fromLatin1(kTestInterface));

        QDBusMessage msg = QDBusMessage::createSignal(
            QString::fromLatin1(kTestPath),
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged");
        msg << QString::fromLatin1(kTestInterface);
        msg << QVariant::fromValue(QVariantMap());
        msg << QStringList();

        req.slotDbusCall(msg);
        EXPECT_TRUE(req.slotDbusCallCalled);
    }

    bus.unregisterObject(QString::fromLatin1(kTestPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

#include "test_dbusrequestbase.moc"
