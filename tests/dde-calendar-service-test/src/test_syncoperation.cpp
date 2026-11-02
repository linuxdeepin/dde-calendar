// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncoperation.h"

#include <gtest/gtest.h>

#include <QDBusMessage>
#include <QDBusArgument>
#include <QVariant>
#include <QVariantMap>
#include <QStringList>
#include <QThread>

class SyncoperationTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_op = new Syncoperation();
    }
    void TearDown() override
    {
        delete m_op;
        m_op = nullptr;
    }
    Syncoperation *m_op = nullptr;
};

// ---- Syncoperation::Syncoperation (constructor) ----

TEST_F(SyncoperationTest, ConstructorCreatesObject)
{
    EXPECT_NE(m_op, nullptr);
}

// ---- Syncoperation::~Syncoperation (destructor) ----

TEST_F(SyncoperationTest, DestructorDoesNotCrash)
{
    auto *op = new Syncoperation();
    delete op;
    SUCCEED();
}

// ---- Syncoperation::optlogin ----

TEST_F(SyncoperationTest, OptloginDoesNotCrash)
{
    m_op->optlogin();
    SUCCEED();
}

// ---- Syncoperation::optlogout ----

TEST_F(SyncoperationTest, OptlogoutDoesNotCrash)
{
    m_op->optlogout();
    SUCCEED();
}

// ---- Syncoperation::optUpload ----

TEST_F(SyncoperationTest, OptUploadWithoutDbusReturnsFailure)
{
    SyncoptResult result = m_op->optUpload("/test/key");
    EXPECT_FALSE(result.ret);
}

// ---- Syncoperation::optDownload ----

TEST_F(SyncoperationTest, OptDownloadWithoutDbusReturnsFailure)
{
    SyncoptResult result = m_op->optDownload("/test/key", "/test/path");
    EXPECT_FALSE(result.ret);
}

// ---- Syncoperation::optDelete ----

TEST_F(SyncoperationTest, OptDeleteWithoutDbusReturnsFailure)
{
    SyncoptResult result = m_op->optDelete("/test/key");
    EXPECT_FALSE(result.ret);
}

// ---- Syncoperation::optMetadata ----

TEST_F(SyncoperationTest, OptMetadataWithoutDbusReturnsFailure)
{
    SyncoptResult result = m_op->optMetadata("/test/key");
    EXPECT_FALSE(result.ret);
}

// ---- Syncoperation::optUserData ----

TEST_F(SyncoperationTest, OptUserDataWithoutDbusReturnsFalse)
{
    QVariantMap userInfoMap;
    bool ret = m_op->optUserData(userInfoMap);
    EXPECT_FALSE(ret);
}

// ---- Syncoperation::hasAvailable ----

TEST_F(SyncoperationTest, HasAvailableWithoutDbusReturnsFalse)
{
    bool ret = m_op->hasAvailable();
    EXPECT_FALSE(ret);
}

// ---- Syncoperation::optGetMainSwitcher ----

TEST_F(SyncoperationTest, OptGetMainSwitcherWithoutDbusReturnsFailure)
{
    SyncoptResult result = m_op->optGetMainSwitcher();
    EXPECT_FALSE(result.ret);
}

// ---- Syncoperation::optGetCalendarSwitcher ----

TEST_F(SyncoperationTest, OptGetCalendarSwitcherWithoutDbusReturnsFailure)
{
    SyncoptResult result = m_op->optGetCalendarSwitcher();
    EXPECT_FALSE(result.ret);
}

// ---- Syncoperation::slotDbusCall (private slot, accessible via -fno-access-control) ----

TEST_F(SyncoperationTest, SlotDbusCallSwitcherChangeDoesNotCrash)
{
    QDBusMessage msg = QDBusMessage::createSignal(
        "/test", "com.test.Interface", "SwitcherChange");
    m_op->slotDbusCall(msg);
    SUCCEED();
}

TEST_F(SyncoperationTest, SlotDbusCallLoginStatusDoesNotCrash)
{
    QDBusMessage msg = QDBusMessage::createSignal(
        "/test", "com.test.Interface", "LoginStatus");

    // Provide a QDBusArgument with int-array data so msg.arguments() is non-empty.
    // Source defect: slotDbusCall does not check msg.arguments().isEmpty()
    // before calling .first() — flagged in Mode 5 defect report.
    QDBusArgument arg;
    arg.beginArray(qMetaTypeId<int>());
    arg << 1;
    arg.endArray();
    msg << QVariant::fromValue(arg);

    m_op->slotDbusCall(msg);
    SUCCEED();
}

TEST_F(SyncoperationTest, SlotDbusCallUnknownMemberDoesNotCrash)
{
    QDBusMessage msg = QDBusMessage::createSignal(
        "/test", "com.test.Interface", "UnknownSignal");
    m_op->slotDbusCall(msg);
    SUCCEED();
}

// ---- Syncoperation::onPropertiesChanged (private slot, accessible via -fno-access-control) ----

TEST_F(SyncoperationTest, OnPropertiesChangedWithoutUserDataReturns)
{
    QVariantMap changedProperties;
    changedProperties.insert("OtherKey", QVariant("value"));
    m_op->onPropertiesChanged("com.test.Interface", changedProperties, QStringList());
    SUCCEED();
}

TEST_F(SyncoperationTest, OnPropertiesChangedWithUserDataReturns)
{
    QVariantMap changedProperties;
    changedProperties.insert("UserData", QVariant("somedata"));
    m_op->onPropertiesChanged("com.test.Interface", changedProperties, QStringList());
    SUCCEED();
}

TEST_F(SyncoperationTest, OnPropertiesChangedEmptyMapReturns)
{
    m_op->onPropertiesChanged("com.test.Interface", QVariantMap(), QStringList());
    SUCCEED();
}
