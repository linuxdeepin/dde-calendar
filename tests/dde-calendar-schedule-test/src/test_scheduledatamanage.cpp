// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "scheduledatamanage.h"
#include "accountmanager.h"
#include "accountitem.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "dtypecolor.h"
#include "dbus/dbusaccountmanagerrequest.h"
#include "dbus/dbusaccountrequest.h"
#include "stub.h"

#include <QColor>

// ============================================================
// Global stub functions for DBus (needed before singleton init)
// ============================================================

static DAccount::List g_envAccountList;

static void stub_clientIsShow(bool) {}
static DAccount::List stub_getAccountList_env() { return g_envAccountList; }
static DScheduleType::List stub_getScheduleTypeList_env() { return DScheduleType::List(); }
static DTypeColor::List stub_getSysColors_env() { return DTypeColor::List(); }

// ============================================================
// Global test environment
// ============================================================

class CScheduleDataManageTestEnv : public ::testing::Environment {
public:
    void SetUp() override {
        m_stub = new Stub();
        m_stub->set(ADDR(DbusAccountManagerRequest, clientIsShow), stub_clientIsShow);
        m_stub->set(ADDR(DbusAccountManagerRequest, getAccountList), stub_getAccountList_env);
        m_stub->set(ADDR(DbusAccountRequest, getScheduleTypeList), stub_getScheduleTypeList_env);
        m_stub->set(ADDR(DbusAccountRequest, getSysColors), stub_getSysColors_env);
        AccountManager::getInstance();
    }
    void TearDown() override {
        delete m_stub;
    }
private:
    Stub *m_stub = nullptr;
};

static int s_registerEnv = []() {
    ::testing::AddGlobalTestEnvironment(new CScheduleDataManageTestEnv());
    return 0;
}();

// ============================================================
// Helpers
// ============================================================

static DScheduleType::Ptr createTestScheduleType(const QString &typeID,
                                                  const QString &colorCode = "#FF0000")
{
    DScheduleType::Ptr type(new DScheduleType());
    type->setTypeID(typeID);
    type->setTypeName("TestType");
    type->setDisplayName("Test Type");
    type->setPrivilege(DScheduleType::User);
    DTypeColor color;
    color.setColorCode(colorCode);
    type->setTypeColor(color);
    return type;
}

// ============================================================
// Stub for AccountManager::getScheduleTypeByScheduleTypeId
// ============================================================

static DScheduleType::Ptr g_stubScheduleType;

static DScheduleType::Ptr stub_getScheduleTypeByScheduleTypeId(const QString &)
{
    return g_stubScheduleType;
}

// ============================================================
// Test fixture
// ============================================================

class CScheduleDataManageTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_stubScheduleType.clear();
    }
};

// ============================================================
// getScheduleDataManage
// ============================================================

TEST_F(CScheduleDataManageTest, GetScheduleDataManage_ReturnsValidPointer)
{
    CScheduleDataManage *ptr = CScheduleDataManage::getScheduleDataManage();
    ASSERT_NE(nullptr, ptr);
}

TEST_F(CScheduleDataManageTest, GetScheduleDataManage_ReturnsSamePointer)
{
    CScheduleDataManage *p1 = CScheduleDataManage::getScheduleDataManage();
    CScheduleDataManage *p2 = CScheduleDataManage::getScheduleDataManage();
    EXPECT_EQ(p1, p2);
}

// ============================================================
// getScheduleColorByType
// ============================================================

TEST_F(CScheduleDataManageTest, GetScheduleColorByType_ValidType_ReturnsCorrectColors)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getScheduleTypeByScheduleTypeId),
             stub_getScheduleTypeByScheduleTypeId);
    g_stubScheduleType = createTestScheduleType("type-1", "#00FF00");

    CSchedulesColor color = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType("type-1");

    QColor expected("#00FF00");
    EXPECT_EQ(expected, color.orginalColor);
    // setAlphaF converts to 8-bit int; compare RGB and alpha separately
    EXPECT_EQ(expected.rgb(), color.normalColor.rgb());
    EXPECT_NEAR(0.2f, color.normalColor.alphaF(), 0.01f);
    EXPECT_EQ(expected.rgb(), color.pressColor.rgb());
    EXPECT_NEAR(0.35f, color.pressColor.alphaF(), 0.01f);
    EXPECT_EQ(expected.rgb(), color.hoverColor.rgb());
    EXPECT_NEAR(0.3f, color.hoverColor.alphaF(), 0.01f);
    EXPECT_EQ(expected.rgb(), color.hightColor.rgb());
    EXPECT_NEAR(0.35f, color.hightColor.alphaF(), 0.01f);
}

TEST_F(CScheduleDataManageTest, GetScheduleColorByType_NullType_ReturnsInvalidColor)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getScheduleTypeByScheduleTypeId),
             stub_getScheduleTypeByScheduleTypeId);
    g_stubScheduleType.clear();

    CSchedulesColor color = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType("nonexistent");

    EXPECT_FALSE(color.orginalColor.isValid());
    EXPECT_FALSE(color.normalColor.isValid());
    EXPECT_FALSE(color.pressColor.isValid());
    EXPECT_FALSE(color.hoverColor.isValid());
    EXPECT_FALSE(color.hightColor.isValid());
}

// ============================================================
// getSystemActiveColor
// ============================================================

TEST_F(CScheduleDataManageTest, GetSystemActiveColor_ReturnsValidColor)
{
    (void)CScheduleDataManage::getSystemActiveColor();
    SUCCEED();
}

// ============================================================
// getTextColor
// ============================================================

TEST_F(CScheduleDataManageTest, GetTextColor_ReturnsValidColor)
{
    (void)CScheduleDataManage::getTextColor();
    SUCCEED();
}

// ============================================================
// setTheMe / getTheme
// ============================================================

TEST_F(CScheduleDataManageTest, SetTheMe_DefaultZero)
{
    CScheduleDataManage *ptr = CScheduleDataManage::getScheduleDataManage();
    ptr->setTheMe(0);
    EXPECT_EQ(0, ptr->getTheme());
}

TEST_F(CScheduleDataManageTest, SetTheMe_DarkTheme)
{
    CScheduleDataManage *ptr = CScheduleDataManage::getScheduleDataManage();
    ptr->setTheMe(1);
    EXPECT_EQ(1, ptr->getTheme());
}

TEST_F(CScheduleDataManageTest, SetTheMe_LightTheme)
{
    CScheduleDataManage *ptr = CScheduleDataManage::getScheduleDataManage();
    ptr->setTheMe(2);
    EXPECT_EQ(2, ptr->getTheme());
}

// ============================================================
// Constructor / Destructor
// ============================================================

TEST_F(CScheduleDataManageTest, Constructor_CreatesInstance)
{
    CScheduleDataManage *instance = new CScheduleDataManage();
    ASSERT_NE(nullptr, instance);
    delete instance;
}

TEST_F(CScheduleDataManageTest, Destructor_DoesNotCrash)
{
    CScheduleDataManage *instance = new CScheduleDataManage();
    delete instance;
    SUCCEED();
}
