// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusnotify.h"
#include "../third-party_stub/stub.h"

#include <gtest/gtest.h>
#include <QtDBus/QtDBus>

static QDBusMessage stub_callWithArgumentList_notify(QDBus::CallMode mode,
                                                     const QString &method,
                                                     const QList<QVariant> &args)
{
    Q_UNUSED(mode)
    Q_UNUSED(method)
    Q_UNUSED(args)
    QDBusMessage reply;
    reply.setArguments({QVariant(static_cast<quint32>(42))});
    return reply;
}

static QDBusMessage::MessageType stub_type_reply()
{
    return QDBusMessage::ReplyMessage;
}

static QDBusMessage::MessageType stub_type_invalid()
{
    return QDBusMessage::InvalidMessage;
}

class DBusNotifyTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_notify = new DBusNotify(
            "org.freedesktop.Notifications",
            "/org/freedesktop/Notifications",
            "org.freedesktop.Notifications",
            QDBusConnection::sessionBus());
    }
    void TearDown() override
    {
        delete m_notify;
        m_notify = nullptr;
    }
    DBusNotify *m_notify = nullptr;
};

TEST_F(DBusNotifyTest, Constructor)
{
    EXPECT_NE(m_notify, nullptr);
}

TEST_F(DBusNotifyTest, NotifyWithValidReplyReturnsNotifyId)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList),
             stub_callWithArgumentList_notify);
    stub.set(ADDR(QDBusMessage, type), stub_type_reply);

    QList<QVariant> args;
    args << "app" << quint32(0) << "" << "title" << "body"
         << QStringList() << QVariantMap() << qint32(-1);

    int result = m_notify->Notify(args);
    EXPECT_EQ(result, 42);
}

TEST_F(DBusNotifyTest, NotifyWithNonReplyMessageReturnsMinusOne)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList),
             stub_callWithArgumentList_notify);
    stub.set(ADDR(QDBusMessage, type), stub_type_invalid);

    QList<QVariant> args;
    int result = m_notify->Notify(args);
    EXPECT_EQ(result, -1);
}

TEST_F(DBusNotifyTest, CloseNotificationDoesNotCrash)
{
    Stub stub;
    stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList),
             stub_callWithArgumentList_notify);

    m_notify->closeNotification(1);
    SUCCEED();
}
