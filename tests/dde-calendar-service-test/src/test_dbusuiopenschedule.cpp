// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusuiopenschedule.h"
#include "../third-party_stub/stub.h"

#include <gtest/gtest.h>
#include <QtDBus/QtDBus>

static QDBusMessage stub_callWithArgumentList_opensched(QDBus::CallMode mode,
                                                        const QString &method,
                                                        const QList<QVariant> &args)
{
    Q_UNUSED(mode)
    Q_UNUSED(method)
    Q_UNUSED(args)
    return QDBusMessage();
}

class DbusUIOpenScheduleTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList),
                   stub_callWithArgumentList_opensched);
        m_schedule = new DbusUIOpenSchedule(
            "com.deepin.Calendar",
            "/com/deepin/Calendar",
            QDBusConnection::sessionBus());
    }
    void TearDown() override
    {
        delete m_schedule;
        m_schedule = nullptr;
    }
    Stub m_stub;
    DbusUIOpenSchedule *m_schedule = nullptr;
};

TEST_F(DbusUIOpenScheduleTest, StaticInterfaceName)
{
    EXPECT_STREQ(DbusUIOpenSchedule::staticInterfaceName(), "com.deepin.Calendar");
}

TEST_F(DbusUIOpenScheduleTest, Constructor)
{
    EXPECT_NE(m_schedule, nullptr);
}

TEST_F(DbusUIOpenScheduleTest, OpenScheduleDoesNotCrash)
{
    m_schedule->OpenSchedule("{\"key\":\"value\"}");
    SUCCEED();
}
