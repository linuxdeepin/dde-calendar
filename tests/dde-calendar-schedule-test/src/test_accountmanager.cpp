// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "accountmanager.h"
#include "accountitem.h"
#include "dscheduledatamanager.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "dtypecolor.h"
#include "dcalendargeneralsettings.h"
#include "dbus/dbusaccountmanagerrequest.h"
#include "dbus/dbusaccountrequest.h"
#include "cdbuspendingcallwatcher.h"
#include "stub.h"

#include <QSignalSpy>

// ============================================================
// Global stub functions
// ============================================================

static DAccount::List g_envAccountList;

static void stub_clientIsShow(bool) {}
static DAccount::List stub_getAccountList_env() { return g_envAccountList; }
static DScheduleType::List stub_getScheduleTypeList_env() { return DScheduleType::List(); }
static DTypeColor::List stub_getSysColors_env() { return DTypeColor::List(); }

static bool g_downloadCalled = false;
static bool g_uploadCalled = false;
static bool g_setSettingsCalled = false;

static void stub_downloadByAccountID(const QString &) { g_downloadCalled = true; }
static void stub_uploadNetWorkAccountData() { g_uploadCalled = true; }
static void stub_setCalendarGeneralSettings(DCalendarGeneralSettings::Ptr) { g_setSettingsCalled = true; }

// ============================================================
// Global test environment
// ============================================================

class CalendarDataTestEnv : public ::testing::Environment {
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
    ::testing::AddGlobalTestEnvironment(new CalendarDataTestEnv());
    return 0;
}();

// ============================================================
// Helpers
// ============================================================

static DAccount::Ptr createTestAccount(DAccount::Type type, const QString &id, const QString &name)
{
    DAccount::Ptr account(new DAccount(type));
    account->setAccountID(id);
    account->setAccountName(name);
    account->setDbusPath("/test/path");
    account->setDbusInterface("com.test.Interface");
    return account;
}

static DScheduleType::Ptr createTestScheduleType(const QString &typeID, const QString &typeName,
                                                  DScheduleType::Privilege priv,
                                                  const QString &accountID = QString(),
                                                  const QString &colorCode = "#FF0000")
{
    DScheduleType::Ptr type(new DScheduleType());
    type->setTypeID(typeID);
    type->setTypeName(typeName);
    type->setDisplayName(typeName);
    type->setPrivilege(priv);
    type->setAccountID(accountID);
    DTypeColor color;
    color.setColorCode(colorCode);
    type->setTypeColor(color);
    return type;
}

static AccountItem::Ptr createTestAccountItem(const DAccount::Ptr &account,
                                               const DScheduleType::List &typeList = DScheduleType::List())
{
    AccountItem::Ptr item(new AccountItem(account));
    item->m_scheduleTypeList = typeList;
    return item;
}

// ============================================================
// Test fixture
// ============================================================

class AccountManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto *mgr = AccountManager::getInstance();
        mgr->m_accountItemList.clear();
        mgr->m_localAccountItem.clear();
        mgr->m_unionAccountItem.clear();
        mgr->m_settings.clear();
        mgr->m_dataInitFinished = false;
        mgr->m_waitingCallList.clear();
        g_downloadCalled = false;
        g_uploadCalled = false;
        g_setSettingsCalled = false;
    }
};

// ============================================================
// Tests
// ============================================================

TEST_F(AccountManagerTest, GetInstance_ReturnsSamePointer)
{
    AccountManager *p1 = AccountManager::getInstance();
    AccountManager *p2 = AccountManager::getInstance();
    EXPECT_EQ(p1, p2);
}

TEST_F(AccountManagerTest, GetAccountList_InitiallyEmpty)
{
    EXPECT_TRUE(AccountManager::getInstance()->getAccountList().isEmpty());
}

TEST_F(AccountManagerTest, GetLocalAccountItem_InitiallyNull)
{
    EXPECT_TRUE(AccountManager::getInstance()->getLocalAccountItem().isNull());
}

TEST_F(AccountManagerTest, GetUnionAccountItem_InitiallyNull)
{
    EXPECT_TRUE(AccountManager::getInstance()->getUnionAccountItem().isNull());
}

TEST_F(AccountManagerTest, GetGeneralSettings_InitiallyNull)
{
    EXPECT_TRUE(AccountManager::getInstance()->getGeneralSettings().isNull());
}

TEST_F(AccountManagerTest, SlotGetAccountListFinish_LocalAccount)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::List list;
    list.append(createTestAccount(DAccount::Account_Local, "local-1", "LocalAccount"));

    mgr->slotGetAccountListFinish(list);

    ASSERT_FALSE(mgr->m_localAccountItem.isNull());
    EXPECT_EQ("local-1", mgr->getLocalAccountItem()->getAccount()->accountID());
    EXPECT_EQ(1, mgr->getAccountList().size());
}

TEST_F(AccountManagerTest, SlotGetAccountListFinish_UnionAccount)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::List list;
    list.append(createTestAccount(DAccount::Account_UnionID, "union-1", "UnionAccount"));

    mgr->slotGetAccountListFinish(list);

    ASSERT_FALSE(mgr->m_unionAccountItem.isNull());
    EXPECT_EQ("union-1", mgr->getUnionAccountItem()->getAccount()->accountID());
    EXPECT_EQ(1, mgr->getAccountList().size());
}

TEST_F(AccountManagerTest, SlotGetAccountListFinish_BothAccounts)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::List list;
    list.append(createTestAccount(DAccount::Account_Local, "local-1", "LocalAccount"));
    list.append(createTestAccount(DAccount::Account_UnionID, "union-1", "UnionAccount"));

    mgr->slotGetAccountListFinish(list);

    ASSERT_FALSE(mgr->m_localAccountItem.isNull());
    ASSERT_FALSE(mgr->m_unionAccountItem.isNull());
    EXPECT_EQ(2, mgr->getAccountList().size());
}

TEST_F(AccountManagerTest, SlotGetAccountListFinish_CalDavIgnored)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::List list;
    list.append(createTestAccount(DAccount::Account_CalDav, "caldav-1", "CalDavAccount"));

    mgr->slotGetAccountListFinish(list);

    EXPECT_TRUE(mgr->m_localAccountItem.isNull());
    EXPECT_TRUE(mgr->m_unionAccountItem.isNull());
    EXPECT_EQ(0, mgr->getAccountList().size());
}

TEST_F(AccountManagerTest, SlotGetAccountListFinish_EmitsNoCrash_EmptyList)
{
    auto *mgr = AccountManager::getInstance();
    mgr->slotGetAccountListFinish(DAccount::List());
    EXPECT_EQ(0, mgr->getAccountList().size());
    SUCCEED();
}

TEST_F(AccountManagerTest, GetScheduleTypeByScheduleTypeId_Found)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::Ptr account = createTestAccount(DAccount::Account_Local, "acc-1", "Account1");
    DScheduleType::List typeList;
    typeList.append(createTestScheduleType("type-1", "Work", DScheduleType::User, "acc-1"));
    typeList.append(createTestScheduleType("type-2", "Personal", DScheduleType::Read, "acc-1"));
    AccountItem::Ptr item = createTestAccountItem(account, typeList);
    mgr->m_accountItemList.append(item);

    DScheduleType::Ptr result = mgr->getScheduleTypeByScheduleTypeId("type-2");
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ("type-2", result->typeID());
}

TEST_F(AccountManagerTest, GetScheduleTypeByScheduleTypeId_NotFound)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::Ptr account = createTestAccount(DAccount::Account_Local, "acc-1", "Account1");
    DScheduleType::List typeList;
    typeList.append(createTestScheduleType("type-1", "Work", DScheduleType::User, "acc-1"));
    AccountItem::Ptr item = createTestAccountItem(account, typeList);
    mgr->m_accountItemList.append(item);

    DScheduleType::Ptr result = mgr->getScheduleTypeByScheduleTypeId("nonexistent");
    EXPECT_TRUE(result.isNull());
}

TEST_F(AccountManagerTest, GetScheduleTypeByScheduleTypeId_EmptyList)
{
    DScheduleType::Ptr result = AccountManager::getInstance()->getScheduleTypeByScheduleTypeId("any");
    EXPECT_TRUE(result.isNull());
}

TEST_F(AccountManagerTest, GetAccountItemByAccountId_Found)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::Ptr account = createTestAccount(DAccount::Account_Local, "acc-1", "Account1");
    AccountItem::Ptr item = createTestAccountItem(account);
    mgr->m_accountItemList.append(item);

    AccountItem::Ptr result = mgr->getAccountItemByAccountId("acc-1");
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ("acc-1", result->getAccount()->accountID());
}

TEST_F(AccountManagerTest, GetAccountItemByAccountId_NotFound)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::Ptr account = createTestAccount(DAccount::Account_Local, "acc-1", "Account1");
    AccountItem::Ptr item = createTestAccountItem(account);
    mgr->m_accountItemList.append(item);

    AccountItem::Ptr result = mgr->getAccountItemByAccountId("nonexistent");
    EXPECT_TRUE(result.isNull());
}

TEST_F(AccountManagerTest, GetAccountItemByAccountName_Found)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::Ptr account = createTestAccount(DAccount::Account_Local, "acc-1", "AccountName1");
    AccountItem::Ptr item = createTestAccountItem(account);
    mgr->m_accountItemList.append(item);

    AccountItem::Ptr result = mgr->getAccountItemByAccountName("AccountName1");
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ("AccountName1", result->getAccount()->accountName());
}

TEST_F(AccountManagerTest, GetAccountItemByAccountName_NotFound)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::Ptr account = createTestAccount(DAccount::Account_Local, "acc-1", "AccountName1");
    AccountItem::Ptr item = createTestAccountItem(account);
    mgr->m_accountItemList.append(item);

    AccountItem::Ptr result = mgr->getAccountItemByAccountName("Nonexistent");
    EXPECT_TRUE(result.isNull());
}

TEST_F(AccountManagerTest, GetAccountItemByScheduleTypeId_Found)
{
    auto *mgr = AccountManager::getInstance();
    DAccount::Ptr account = createTestAccount(DAccount::Account_Local, "acc-1", "Account1");
    DScheduleType::List typeList;
    typeList.append(createTestScheduleType("type-1", "Work", DScheduleType::User, "acc-1"));
    AccountItem::Ptr item = createTestAccountItem(account, typeList);
    mgr->m_accountItemList.append(item);

    AccountItem::Ptr result = mgr->getAccountItemByScheduleTypeId("type-1");
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ("acc-1", result->getAccount()->accountID());
}

TEST_F(AccountManagerTest, ResetAccount_ClearsAndRefetches)
{
    auto *mgr = AccountManager::getInstance();
    mgr->m_dataInitFinished = true;
    mgr->m_localAccountItem = createTestAccountItem(createTestAccount(DAccount::Account_Local, "old", "Old"));
    mgr->m_unionAccountItem = createTestAccountItem(createTestAccount(DAccount::Account_UnionID, "old-u", "OldU"));

    Stub stub;
    DAccount::List newList;
    newList.append(createTestAccount(DAccount::Account_Local, "new-local", "NewLocal"));
    g_envAccountList = newList;
    stub.set(ADDR(DbusAccountManagerRequest, getAccountList), stub_getAccountList_env);

    mgr->resetAccount();

    EXPECT_FALSE(mgr->m_dataInitFinished);
    ASSERT_FALSE(mgr->m_localAccountItem.isNull());
    EXPECT_EQ("new-local", mgr->getLocalAccountItem()->getAccount()->accountID());
    g_envAccountList.clear();
}

TEST_F(AccountManagerTest, DownloadByAccountID_DelegatesToDbus)
{
    Stub stub;
    stub.set(ADDR(DbusAccountManagerRequest, downloadByAccountID), stub_downloadByAccountID);
    AccountManager::getInstance()->downloadByAccountID("acc-1", [](CallMessge) {});
    EXPECT_TRUE(g_downloadCalled);
}

TEST_F(AccountManagerTest, UploadNetWorkAccountData_DelegatesToDbus)
{
    Stub stub;
    stub.set(ADDR(DbusAccountManagerRequest, uploadNetWorkAccountData), stub_uploadNetWorkAccountData);
    AccountManager::getInstance()->uploadNetWorkAccountData([](CallMessge) {});
    EXPECT_TRUE(g_uploadCalled);
}

TEST_F(AccountManagerTest, SetCalendarGeneralSettings_DelegatesToDbus)
{
    Stub stub;
    stub.set(ADDR(DbusAccountManagerRequest, setCalendarGeneralSettings), stub_setCalendarGeneralSettings);
    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    AccountManager::getInstance()->setCalendarGeneralSettings(settings, [](CallMessge) {});
    EXPECT_TRUE(g_setSettingsCalled);
}

TEST_F(AccountManagerTest, WaitingData_NotFinished_AppendsToList)
{
    auto *mgr = AccountManager::getInstance();
    bool called = false;
    mgr->waitingData([&called]() { called = true; });
    EXPECT_FALSE(called);
    EXPECT_EQ(1, mgr->m_waitingCallList.size());
}

TEST_F(AccountManagerTest, WaitingData_Finished_CallsImmediately)
{
    auto *mgr = AccountManager::getInstance();
    mgr->m_dataInitFinished = true;
    bool called = false;
    mgr->waitingData([&called]() { called = true; });
    EXPECT_TRUE(called);
    EXPECT_EQ(0, mgr->m_waitingCallList.size());
}

TEST_F(AccountManagerTest, ExecWaitingCall_CallsAllCallbacks)
{
    auto *mgr = AccountManager::getInstance();
    bool called1 = false, called2 = false;
    mgr->m_waitingCallList.append([&called1]() { called1 = true; });
    mgr->m_waitingCallList.append([&called2]() { called2 = true; });

    mgr->execWaitingCall();

    EXPECT_TRUE(called1);
    EXPECT_TRUE(called2);
    EXPECT_TRUE(mgr->m_dataInitFinished);
    EXPECT_EQ(0, mgr->m_waitingCallList.size());
}

TEST_F(AccountManagerTest, ExecWaitingCall_EmptyList)
{
    auto *mgr = AccountManager::getInstance();
    mgr->execWaitingCall();
    EXPECT_TRUE(mgr->m_dataInitFinished);
    EXPECT_EQ(0, mgr->m_waitingCallList.size());
}

TEST_F(AccountManagerTest, SlotGetGeneralSettingsFinish_StoresSettings)
{
    auto *mgr = AccountManager::getInstance();
    QSignalSpy spyInit(mgr, &AccountManager::signalDataInitFinished);
    QSignalSpy spyUpdate(mgr, &AccountManager::signalGeneralSettingsUpdate);

    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    settings->setTimeShowType(DCalendarGeneralSettings::Twelve);

    mgr->slotGetGeneralSettingsFinish(settings);

    EXPECT_EQ(settings, mgr->getGeneralSettings());
    EXPECT_EQ(1, spyInit.count());
    EXPECT_EQ(1, spyUpdate.count());
}

TEST_F(AccountManagerTest, SlotGetGeneralSettingsFinish_TriggersExecWaitingCall)
{
    auto *mgr = AccountManager::getInstance();
    bool called = false;
    mgr->waitingData([&called]() { called = true; });

    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    mgr->slotGetGeneralSettingsFinish(settings);

    EXPECT_TRUE(called);
    EXPECT_TRUE(mgr->m_dataInitFinished);
}

TEST_F(AccountManagerTest, SlotGetGeneralSettingsFinish_AlreadyFinished_NoDoubleExec)
{
    auto *mgr = AccountManager::getInstance();
    mgr->m_dataInitFinished = true;
    int callCount = 0;
    mgr->m_waitingCallList.append([&callCount]() { callCount++; });

    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    mgr->slotGetGeneralSettingsFinish(settings);

    EXPECT_EQ(0, callCount);
    EXPECT_EQ(1, mgr->m_waitingCallList.size());
}

TEST_F(AccountManagerTest, Destructor_DeletingDestructorCovered)
{
    AccountManager *tmp = new AccountManager();
    delete tmp;
    SUCCEED();
}
