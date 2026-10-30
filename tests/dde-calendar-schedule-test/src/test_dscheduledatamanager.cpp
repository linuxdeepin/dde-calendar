// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "dscheduledatamanager.h"
#include "accountmanager.h"
#include "accountitem.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "dtypecolor.h"
#include "dschedule.h"
#include "dschedulequerypar.h"
#include "dbus/dbusaccountmanagerrequest.h"
#include "dbus/dbusaccountrequest.h"
#include "stub.h"

#include <QDateTime>

// ============================================================
// Global stub functions for DBus (needed before singleton init)
// ============================================================

static DAccount::List g_envAccountList;

static void stub_clientIsShow(bool) {}
static DAccount::List stub_getAccountList_env() { return g_envAccountList; }
static DScheduleType::List stub_getScheduleTypeList_env() { return DScheduleType::List(); }
static DTypeColor::List stub_getSysColors_env() { return DTypeColor::List(); }

// ============================================================
// Global test environment — initializes AccountManager + DScheduleDataManager
// ============================================================

class DScheduleDataManagerTestEnv : public ::testing::Environment {
public:
    void SetUp() override {
        m_stub = new Stub();
        m_stub->set(ADDR(DbusAccountManagerRequest, clientIsShow), stub_clientIsShow);
        m_stub->set(ADDR(DbusAccountManagerRequest, getAccountList), stub_getAccountList_env);
        m_stub->set(ADDR(DbusAccountRequest, getScheduleTypeList), stub_getScheduleTypeList_env);
        m_stub->set(ADDR(DbusAccountRequest, getSysColors), stub_getSysColors_env);
        AccountManager::getInstance();
        DScheduleDataManager::getInstance();
    }
    void TearDown() override {
        delete m_stub;
    }
private:
    Stub *m_stub = nullptr;
};

static int s_registerEnv = []() {
    ::testing::AddGlobalTestEnvironment(new DScheduleDataManagerTestEnv());
    return 0;
}();

// ============================================================
// Helpers
// ============================================================

static DAccount::Ptr createTestAccount()
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountID("test-account-id");
    account->setAccountName("test-account");
    account->setDbusPath("/test/path");
    account->setDbusInterface("com.test.Interface");
    return account;
}

static DScheduleType::Ptr createTestScheduleType(const QString &typeID,
                                                  DScheduleType::Privilege priv = DScheduleType::User)
{
    DScheduleType::Ptr type(new DScheduleType());
    type->setTypeID(typeID);
    type->setTypeName("TestType");
    type->setDisplayName("Test Type");
    type->setPrivilege(priv);
    type->setAccountID("test-account-id");
    DTypeColor color;
    color.setColorCode("#FF0000");
    type->setTypeColor(color);
    return type;
}

static AccountItem::Ptr createTestAccountItem(const DScheduleType::List &typeList = DScheduleType::List())
{
    DAccount::Ptr account = createTestAccount();
    AccountItem::Ptr item(new AccountItem(account));
    item->m_scheduleTypeList = typeList;
    return item;
}

// ============================================================
// Stub functions for AccountManager::getLocalAccountItem
// ============================================================

static AccountItem::Ptr g_stubLocalAccountItem;

static AccountItem::Ptr stub_getLocalAccountItem()
{
    return g_stubLocalAccountItem;
}

// Stub for AccountItem::createSchedule
static QString g_createScheduleResult = "new-schedule-id";
static QString stub_createSchedule(const DSchedule::Ptr &)
{
    return g_createScheduleResult;
}

// Stub for AccountItem::getScheduleByID
static DSchedule::Ptr g_stubSchedule;
static DSchedule::Ptr stub_getScheduleByID(const QString &)
{
    return g_stubSchedule;
}

// Stub for AccountItem::deleteScheduleByID
static bool g_deleteScheduleByIDCalled = false;
static void stub_deleteScheduleByID(const QString &, CallbackFunc)
{
    g_deleteScheduleByIDCalled = true;
}

// Stub for AccountItem::updateSchedule
static bool g_updateScheduleCalled = false;
static void stub_updateSchedule(const DSchedule::Ptr &, CallbackFunc)
{
    g_updateScheduleCalled = true;
}

// Stub for AccountItem::querySchedulesWithParameter
static DSchedule::Map g_stubScheduleMap;
static DSchedule::Map stub_querySchedulesWithParameter(const DScheduleQueryPar::Ptr &)
{
    return g_stubScheduleMap;
}

// ============================================================
// Test fixture
// ============================================================

class DScheduleDataManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_stubLocalAccountItem.clear();
        g_createScheduleResult = "new-schedule-id";
        g_stubSchedule.clear();
        g_stubScheduleMap.clear();
        g_deleteScheduleByIDCalled = false;
        g_updateScheduleCalled = false;
    }
};

// ============================================================
// getInstance
// ============================================================

TEST_F(DScheduleDataManagerTest, GetInstance_ReturnsSamePointer)
{
    DScheduleDataManager *p1 = DScheduleDataManager::getInstance();
    DScheduleDataManager *p2 = DScheduleDataManager::getInstance();
    EXPECT_EQ(p1, p2);
}

// ============================================================
// createSchedule
// ============================================================

TEST_F(DScheduleDataManagerTest, CreateSchedule_NullAccount_ReturnsEmpty)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem.clear();

    DSchedule::Ptr schedule(new DSchedule());
    QString result = DScheduleDataManager::getInstance()->createSchedule(schedule);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(DScheduleDataManagerTest, CreateSchedule_ValidAccount_Delegates)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    stub.set(ADDR(AccountItem, createSchedule), stub_createSchedule);

    DSchedule::Ptr schedule(new DSchedule());
    QString result = DScheduleDataManager::getInstance()->createSchedule(schedule);
    EXPECT_EQ("new-schedule-id", result);
}

// ============================================================
// queryScheduleByScheduleID
// ============================================================

TEST_F(DScheduleDataManagerTest, QueryScheduleByScheduleID_NullAccount_ReturnsNull)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem.clear();

    DSchedule::Ptr result = DScheduleDataManager::getInstance()->queryScheduleByScheduleID("sched-1");
    EXPECT_TRUE(result.isNull());
}

TEST_F(DScheduleDataManagerTest, QueryScheduleByScheduleID_ValidAccount_Delegates)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    g_stubSchedule = DSchedule::Ptr(new DSchedule());
    stub.set(ADDR(AccountItem, getScheduleByID), stub_getScheduleByID);

    DSchedule::Ptr result = DScheduleDataManager::getInstance()->queryScheduleByScheduleID("sched-1");
    EXPECT_EQ(g_stubSchedule, result);
}

// ============================================================
// deleteScheduleByScheduleID
// ============================================================

TEST_F(DScheduleDataManagerTest, DeleteScheduleByScheduleID_NullAccount_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem.clear();

    bool result = DScheduleDataManager::getInstance()->deleteScheduleByScheduleID("sched-1");
    EXPECT_FALSE(result);
}

TEST_F(DScheduleDataManagerTest, DeleteScheduleByScheduleID_ValidAccount_ReturnsTrue)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    stub.set(ADDR(AccountItem, deleteScheduleByID), stub_deleteScheduleByID);

    bool result = DScheduleDataManager::getInstance()->deleteScheduleByScheduleID("sched-1");
    EXPECT_TRUE(result);
    EXPECT_TRUE(g_deleteScheduleByIDCalled);
}

// ============================================================
// updateSchedule
// ============================================================

TEST_F(DScheduleDataManagerTest, UpdateSchedule_NullAccount_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem.clear();

    DSchedule::Ptr schedule(new DSchedule());
    bool result = DScheduleDataManager::getInstance()->updateSchedule(schedule);
    EXPECT_FALSE(result);
}

TEST_F(DScheduleDataManagerTest, UpdateSchedule_ValidAccount_ReturnsTrue)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    stub.set(ADDR(AccountItem, updateSchedule), stub_updateSchedule);

    DSchedule::Ptr schedule(new DSchedule());
    bool result = DScheduleDataManager::getInstance()->updateSchedule(schedule);
    EXPECT_TRUE(result);
    EXPECT_TRUE(g_updateScheduleCalled);
}

// ============================================================
// isFestivalSchedule
// ============================================================

TEST_F(DScheduleDataManagerTest, IsFestivalSchedule_NonePrivilege_ReturnsTrue)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);

    DScheduleType::List typeList;
    typeList.append(createTestScheduleType("festival-type", DScheduleType::None));
    g_stubLocalAccountItem = createTestAccountItem(typeList);

    bool result = DScheduleDataManager::getInstance()->isFestivalSchedule("festival-type");
    EXPECT_TRUE(result);
}

TEST_F(DScheduleDataManagerTest, IsFestivalSchedule_UserPrivilege_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);

    DScheduleType::List typeList;
    typeList.append(createTestScheduleType("normal-type", DScheduleType::User));
    g_stubLocalAccountItem = createTestAccountItem(typeList);

    bool result = DScheduleDataManager::getInstance()->isFestivalSchedule("normal-type");
    EXPECT_FALSE(result);
}

// ============================================================
// querySchedulesWithParameter
// ============================================================

TEST_F(DScheduleDataManagerTest, QuerySchedulesWithParameter_NullAccount_ReturnsEmptyMap)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem.clear();

    DScheduleQueryPar::Ptr params(new DScheduleQueryPar());
    DSchedule::Map result = DScheduleDataManager::getInstance()->querySchedulesWithParameter(params);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(DScheduleDataManagerTest, QuerySchedulesWithParameter_ValidAccount_Delegates)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    stub.set(ADDR(AccountItem, querySchedulesWithParameter), stub_querySchedulesWithParameter);

    QDate date(2026, 1, 1);
    DSchedule::List list;
    list.append(DSchedule::Ptr(new DSchedule()));
    g_stubScheduleMap.insert(date, list);

    DScheduleQueryPar::Ptr params(new DScheduleQueryPar());
    DSchedule::Map result = DScheduleDataManager::getInstance()->querySchedulesWithParameter(params);
    EXPECT_EQ(1, result.size());
}

// ============================================================
// queryScheduleByRRule
// ============================================================

TEST_F(DScheduleDataManagerTest, QueryScheduleByRRule_DelegatesToQuerySchedulesWithParameter)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    stub.set(ADDR(AccountItem, querySchedulesWithParameter), stub_querySchedulesWithParameter);

    QDate date(2026, 3, 15);
    DSchedule::List list;
    list.append(DSchedule::Ptr(new DSchedule()));
    g_stubScheduleMap.insert(date, list);

    QDateTime start(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 12, 31), QTime(23, 59));
    DSchedule::Map result = DScheduleDataManager::getInstance()->queryScheduleByRRule(
        start, end, DScheduleQueryPar::RRule_Day);
    EXPECT_EQ(1, result.size());
}

// ============================================================
// queryScheduleByLimit
// ============================================================

TEST_F(DScheduleDataManagerTest, QueryScheduleByLimit_DelegatesToQuerySchedulesWithParameter)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    stub.set(ADDR(AccountItem, querySchedulesWithParameter), stub_querySchedulesWithParameter);

    QDate date(2026, 6, 1);
    DSchedule::List list;
    list.append(DSchedule::Ptr(new DSchedule()));
    g_stubScheduleMap.insert(date, list);

    QDateTime start(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 12, 31), QTime(23, 59));
    DSchedule::Map result = DScheduleDataManager::getInstance()->queryScheduleByLimit(
        start, end, 10);
    EXPECT_EQ(1, result.size());
}

// ============================================================
// queryScheduleBySummary
// ============================================================

TEST_F(DScheduleDataManagerTest, QueryScheduleBySummary_DelegatesToQuerySchedulesWithParameter)
{
    Stub stub;
    stub.set(ADDR(AccountManager, getLocalAccountItem), stub_getLocalAccountItem);
    g_stubLocalAccountItem = createTestAccountItem();
    stub.set(ADDR(AccountItem, querySchedulesWithParameter), stub_querySchedulesWithParameter);

    QDate date(2026, 9, 9);
    DSchedule::List list;
    list.append(DSchedule::Ptr(new DSchedule()));
    g_stubScheduleMap.insert(date, list);

    QDateTime start(QDate(2026, 1, 1), QTime(0, 0));
    QDateTime end(QDate(2026, 12, 31), QTime(23, 59));
    DSchedule::Map result = DScheduleDataManager::getInstance()->queryScheduleBySummary(
        start, end, "meeting");
    EXPECT_EQ(1, result.size());
}
