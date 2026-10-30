// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "stub.h"

#include "scheduleservice.h"
#include "globaldef.h"
#include "reply.h"
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

class ScheduleServiceTest : public ::testing::Test
{
protected:
    Stub stub;

    void SetUp() override
    {
        stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList), stub_callWithArgumentList);
        stub.set(ADDR(AccountManager, resetAccount), stub_resetAccount);
    }
};

TEST_F(ScheduleServiceTest, Constructor)
{
    scheduleservice svc;
    SUCCEED();
}

TEST_F(ScheduleServiceTest, Destructor)
{
    scheduleservice *svc = new scheduleservice();
    delete svc;
    SUCCEED();
}

TEST_F(ScheduleServiceTest, ServiceName)
{
    scheduleservice svc;
    EXPECT_EQ(svc.serviceName(), QString(SERVICE_NAME));
}

TEST_F(ScheduleServiceTest, ServicePriority)
{
    scheduleservice svc;
    EXPECT_EQ(svc.servicePriority(), 0);
}

TEST_F(ScheduleServiceTest, CanHandle)
{
    scheduleservice svc;
    EXPECT_TRUE(svc.canHandle("anything"));
    EXPECT_TRUE(svc.canHandle(""));
}

TEST_F(ScheduleServiceTest, ServiceInvalidJson)
{
    scheduleservice svc;
    EXPECT_EQ(svc.service("invalid json"), ERR_FAIL);
}

TEST_F(ScheduleServiceTest, ServiceEmptyJsonObject)
{
    scheduleservice svc;
    EXPECT_EQ(svc.service("{}"), ERR_FAIL);
}

TEST_F(ScheduleServiceTest, ServiceMissingIntent)
{
    scheduleservice svc;
    EXPECT_EQ(svc.service(R"({"other":"value"})"), ERR_FAIL);
}

TEST_F(ScheduleServiceTest, ServiceEmptyIntent)
{
    scheduleservice svc;
    EXPECT_EQ(svc.service(R"({"intent":{}})"), ERR_FAIL);
}

TEST_F(ScheduleServiceTest, ServiceValidJson)
{
    scheduleservice svc;
    QString json = R"({"intent":{"semantic":[{"intent":"CREATE"}]}})";
    EXPECT_EQ(svc.service(json), ERR_SUCCESS);
}

TEST_F(ScheduleServiceTest, GetServiceReply)
{
    scheduleservice svc;
    Reply &reply = svc.getServiceReply();
    (void)reply;
    SUCCEED();
}

TEST_F(ScheduleServiceTest, GetScheduleManageTask)
{
    scheduleservice svc;
    ScheduleManageTask *task = svc.getScheduleManageTask();
    EXPECT_NE(task, nullptr);
}
