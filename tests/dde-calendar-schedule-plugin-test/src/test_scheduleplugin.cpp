// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "stub.h"

#include "scheduleplugin.h"
#include "scheduleservice.h"
#include "globaldef.h"
#include "reply.h"
#include "systemsemanticplugin.h"
#include "service.h"
#include "schedulemanagetask.h"
#include "accountmanager.h"

#include <QDBusAbstractInterface>
#include <QDBusMessage>
#include <QVariant>

static QDBusMessage stub_callWithArgumentList(QDBus::CallMode mode,
                                       const QString &method,
                                       const QList<QVariant> &args)
{
    Q_UNUSED(mode)
    Q_UNUSED(method)
    Q_UNUSED(args)
    QDBusMessage reply;
    return reply;
}

static void stub_resetAccount()
{
}

class SchedulePluginTest : public ::testing::Test
{
protected:
    Stub stub;

    void SetUp() override
    {
        stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList), stub_callWithArgumentList);
        stub.set(ADDR(AccountManager, resetAccount), stub_resetAccount);
    }
};

TEST_F(SchedulePluginTest, Constructor)
{
    Scheduleplugin plugin;
    SUCCEED();
}

TEST_F(SchedulePluginTest, Destructor)
{
    Scheduleplugin *plugin = new Scheduleplugin();
    delete plugin;
    SUCCEED();
}

TEST_F(SchedulePluginTest, Init)
{
    Scheduleplugin plugin;
    EXPECT_EQ(plugin.init(), 0);
}

TEST_F(SchedulePluginTest, Uninit)
{
    Scheduleplugin plugin;
    plugin.uninit();
    SUCCEED();
}

TEST_F(SchedulePluginTest, GetSupportService)
{
    Scheduleplugin plugin;
    QStringList services = plugin.getSupportService();
    EXPECT_EQ(services.size(), 1);
    EXPECT_TRUE(services.contains(SERVICE_NAME));
}

TEST_F(SchedulePluginTest, CreateServiceInvalid)
{
    Scheduleplugin plugin;
    IService *service = plugin.createService("invalid_service");
    EXPECT_EQ(service, nullptr);
}

TEST_F(SchedulePluginTest, CreateServiceValid)
{
    Scheduleplugin plugin;
    IService *service = plugin.createService(SERVICE_NAME);
    EXPECT_NE(service, nullptr);
    plugin.releaseService(service);
}

TEST_F(SchedulePluginTest, ReleaseServiceNull)
{
    Scheduleplugin plugin;
    plugin.releaseService(nullptr);
    SUCCEED();
}

TEST_F(SchedulePluginTest, ReleaseServiceNotInSet)
{
    Scheduleplugin plugin;
    IService *service = new scheduleservice();
    plugin.releaseService(service);
    SUCCEED();
}

TEST_F(SchedulePluginTest, NeedRunInDifferentThread)
{
    Scheduleplugin plugin;
    EXPECT_FALSE(plugin.needRunInDifferentThread());
}

TEST_F(SchedulePluginTest, IsAutoDetectSession)
{
    Scheduleplugin plugin;
    EXPECT_FALSE(plugin.isAutoDetectSession());
}

TEST_F(SchedulePluginTest, SlotSendMessageWithoutHandle)
{
    Scheduleplugin plugin;
    Reply reply;
    plugin.slotSendMessage(reply);
    SUCCEED();
}

TEST_F(SchedulePluginTest, SlotSendMessageWithHandle)
{
    Scheduleplugin plugin;
    bool callbackCalled = false;
    plugin.setMessageHandle([&callbackCalled](IServicePlugin *, Reply) {
        callbackCalled = true;
    });
    Reply reply;
    plugin.slotSendMessage(reply);
    EXPECT_TRUE(callbackCalled);
}
