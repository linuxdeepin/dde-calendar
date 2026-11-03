// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUS_CALL_STUB_H
#define DBUS_CALL_STUB_H

#include "../third-party_stub/stub.h"

#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include <QList>
#include <QString>
#include <QVariant>

// 确定性模拟“目标 D-Bus 服务不可用”的场景：
// 同时桩掉 QDBusConnection::call 与 QDBusAbstractInterface::callWithArgumentList，
// 使所有同步 D-Bus 调用都返回 ServiceUnknown 错误，与运行环境解耦。
// 注意：桩函数首参必须为 this 占位（Stub 库以函数指针方式调用成员函数）。
class ScopedDbusUnavailable
{
public:
    ScopedDbusUnavailable()
    {
        {
            typedef QDBusMessage (*fptr)(void *, const QDBusMessage &, QDBus::CallMode);
            fptr A_foo = (fptr)(&QDBusConnection::call);
            m_stub.set(A_foo, connectionCallUnavailable);
        }
        {
            typedef QDBusMessage (*fptr)(void *, QDBus::CallMode, const QString &,
                                         const QList<QVariant> &);
            fptr A_foo = (fptr)(&QDBusAbstractInterface::callWithArgumentList);
            m_stub.set(A_foo, interfaceCallUnavailable);
        }
        {
            // asyncCall / 生成的接口方法（如 SwitcherDump）走此路径
            typedef QDBusPendingCall (*fptr)(void *, QDBus::CallMode, const QString &,
                                             const QList<QVariant> &);
            fptr A_foo = (fptr)(&QDBusAbstractInterface::asyncCallWithArgumentList);
            m_stub.set(A_foo, asyncCallUnavailable);
        }
    }

private:
    static QDBusMessage connectionCallUnavailable(void *, const QDBusMessage &, QDBus::CallMode)
    {
        return unavailable();
    }
    static QDBusMessage interfaceCallUnavailable(void *, QDBus::CallMode, const QString &,
                                                 const QList<QVariant> &)
    {
        return unavailable();
    }
    static QDBusPendingCall asyncCallUnavailable(void *, QDBus::CallMode, const QString &,
                                                  const QList<QVariant> &)
    {
        return QDBusPendingCall::fromError(unavailable());
    }
    static QDBusMessage unavailable()
    {
        return QDBusMessage::createError(QDBusError::ServiceUnknown,
                                         QStringLiteral("stub: service unavailable"));
    }

    Stub m_stub;
};

#endif // DBUS_CALL_STUB_H
