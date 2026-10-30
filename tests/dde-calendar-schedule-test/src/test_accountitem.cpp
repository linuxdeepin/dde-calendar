// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "accountitem.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "dtypecolor.h"
#include "dschedule.h"
#include "dschedulequerypar.h"
#include "stub.h"

#include <QSignalSpy>

// ---- Stub data ----
static DScheduleType::List g_stubTypeList;
static DTypeColor::List g_stubColorList;
static QString g_stubScheduleID = "stub-schedule-id";
static DSchedule::Ptr g_stubSchedule;
static DSchedule::Map g_stubScheduleMap;
static bool g_createScheduleCalled = false;
static bool g_getScheduleByIDCalled = false;
static bool g_querySchedulesCalled = false;
static bool g_updateAccountInfoCalled = false;
static bool g_createScheduleTypeCalled = false;
static bool g_updateScheduleTypeCalled = false;
static bool g_deleteScheduleTypeCalled = false;
static bool g_scheduleTypeByUsedCalled = false;
static bool g_updateScheduleCalled = false;
static bool g_deleteScheduleByIDCalled = false;
static bool g_deleteSchedulesByTypeIDCalled = false;

// ---- Stub functions for blocking DBus methods ----
DScheduleType::List stub_getScheduleTypeList()
{
    return g_stubTypeList;
}

DTypeColor::List stub_getSysColors()
{
    return g_stubColorList;
}

QString stub_createSchedule(const DSchedule::Ptr &)
{
    g_createScheduleCalled = true;
    return g_stubScheduleID;
}

DSchedule::Ptr stub_getScheduleByID(const QString &)
{
    g_getScheduleByIDCalled = true;
    return g_stubSchedule;
}

DSchedule::Map stub_querySchedulesWithParameter(const DScheduleQueryPar::Ptr &)
{
    g_querySchedulesCalled = true;
    return g_stubScheduleMap;
}

// ---- Stub functions for async DBus methods ----
void stub_updateAccountInfo(const DAccount::Ptr &)
{
    g_updateAccountInfoCalled = true;
}

void stub_createScheduleType(const DScheduleType::Ptr &)
{
    g_createScheduleTypeCalled = true;
}

void stub_updateScheduleType(const DScheduleType::Ptr &)
{
    g_updateScheduleTypeCalled = true;
}

void stub_deleteScheduleTypeByID(const QString &)
{
    g_deleteScheduleTypeCalled = true;
}

void stub_scheduleTypeByUsed(const QString &)
{
    g_scheduleTypeByUsedCalled = true;
}

void stub_updateSchedule(const DSchedule::Ptr &)
{
    g_updateScheduleCalled = true;
}

void stub_deleteScheduleByScheduleID(const QString &)
{
    g_deleteScheduleByIDCalled = true;
}

void stub_deleteSchedulesByScheduleTypeID(const QString &)
{
    g_deleteSchedulesByTypeIDCalled = true;
}

// ---- Helpers ----
static DAccount::Ptr createTestAccount()
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountID("test-account-id");
    account->setAccountName("test-account");
    account->setDbusPath("/test/path");
    account->setDbusInterface("com.test.Interface");
    return account;
}

static DScheduleType::List createTestScheduleTypes()
{
    DScheduleType::List list;
    DScheduleType::Ptr type1(new DScheduleType());
    type1->setTypeID("type1");
    type1->setTypeName("Work");
    type1->setDisplayName("Work Schedule");
    type1->setPrivilege(DScheduleType::User);
    list.append(type1);

    DScheduleType::Ptr type2(new DScheduleType());
    type2->setTypeID("type2");
    type2->setTypeName("Personal");
    type2->setDisplayName("Personal Schedule");
    type2->setPrivilege(DScheduleType::Read);
    list.append(type2);

    DScheduleType::Ptr type3(new DScheduleType());
    type3->setTypeID("type3");
    type3->setTypeName("None");
    type3->setDisplayName("None Schedule");
    type3->setPrivilege(DScheduleType::None);
    list.append(type3);

    return list;
}

static DTypeColor::List createTestColorList()
{
    DTypeColor::List list;
    DTypeColor::Ptr color1(new DTypeColor());
    color1->setColorID("color1");
    color1->setColorCode("#FF0000");
    color1->setPrivilege(DTypeColor::PriSystem);
    list.append(color1);

    DTypeColor::Ptr color2(new DTypeColor());
    color2->setColorID("color2");
    color2->setColorCode("#00FF00");
    color2->setPrivilege(DTypeColor::PriUser);
    list.append(color2);

    return list;
}

// ---- Tests ----

TEST(AccountItemTest, Constructor_WithValidAccount)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    EXPECT_EQ(account, item.getAccount());
    SUCCEED();
}

TEST(AccountItemTest, GetAccount_ReturnsCorrectAccount)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    EXPECT_EQ(account, item.getAccount());
    EXPECT_EQ("test-account-id", item.getAccount()->accountID());
}

TEST(AccountItemTest, GetScheduleMap_DefaultEmpty)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    EXPECT_TRUE(item.getScheduleMap().isEmpty());
}

TEST(AccountItemTest, ResetAccount_StubbedDBus)
{
    g_stubTypeList = createTestScheduleTypes();
    g_stubColorList = createTestColorList();

    Stub stub;
    stub.set(ADDR(DbusAccountRequest, getScheduleTypeList), stub_getScheduleTypeList);
    stub.set(ADDR(DbusAccountRequest, getSysColors), stub_getSysColors);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.resetAccount();

    // After reset, m_scheduleTypeList has 3 items, m_typeColorList has 2 items
    EXPECT_EQ(3u, item.m_scheduleTypeList.size());
    EXPECT_EQ(2u, item.m_typeColorList.size());
}

TEST(AccountItemTest, GetScheduleTypeList_FiltersNonePrivilege)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_scheduleTypeList = createTestScheduleTypes();

    // 3 types total, 1 has None privilege → 2 returned
    DScheduleType::List result = item.getScheduleTypeList();
    EXPECT_EQ(2, result.size());
}

TEST(AccountItemTest, GetScheduleTypeList_EmptyList)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    DScheduleType::List result = item.getScheduleTypeList();
    EXPECT_TRUE(result.isEmpty());
}

TEST(AccountItemTest, GetScheduleTypeByID_Found)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_scheduleTypeList = createTestScheduleTypes();

    DScheduleType::Ptr result = item.getScheduleTypeByID("type1");
    ASSERT_NE(nullptr, result);
    EXPECT_EQ("type1", result->typeID());
    EXPECT_EQ("Work", result->typeName());
}

TEST(AccountItemTest, GetScheduleTypeByID_NotFound)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_scheduleTypeList = createTestScheduleTypes();

    DScheduleType::Ptr result = item.getScheduleTypeByID("nonexistent");
    EXPECT_EQ(nullptr, result);
}

TEST(AccountItemTest, GetScheduleTypeByID_EmptyList)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    DScheduleType::Ptr result = item.getScheduleTypeByID("any");
    EXPECT_EQ(nullptr, result);
}

TEST(AccountItemTest, GetScheduleTypeByName_MatchTypeName)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_scheduleTypeList = createTestScheduleTypes();

    DScheduleType::Ptr result = item.getScheduleTypeByName("Work");
    ASSERT_NE(nullptr, result);
    EXPECT_EQ("Work", result->typeName());
}

TEST(AccountItemTest, GetScheduleTypeByName_MatchDisplayName)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_scheduleTypeList = createTestScheduleTypes();

    DScheduleType::Ptr result = item.getScheduleTypeByName("Personal Schedule");
    ASSERT_NE(nullptr, result);
    EXPECT_EQ("Personal", result->typeName());
}

TEST(AccountItemTest, GetScheduleTypeByName_NotFound)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_scheduleTypeList = createTestScheduleTypes();

    DScheduleType::Ptr result = item.getScheduleTypeByName("Nonexistent");
    EXPECT_EQ(nullptr, result);
}

TEST(AccountItemTest, GetColorTypeList_DefaultEmpty)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    EXPECT_TRUE(item.getColorTypeList().isEmpty());
}

TEST(AccountItemTest, GetColorTypeList_AfterSet)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_typeColorList = createTestColorList();
    EXPECT_EQ(2, item.getColorTypeList().size());
}

TEST(AccountItemTest, GetScheduleByID_Stubbed)
{
    g_stubSchedule = DSchedule::Ptr(new DSchedule());

    Stub stub;
    stub.set(ADDR(DbusAccountRequest, getScheduleByID), stub_getScheduleByID);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    g_getScheduleByIDCalled = false;
    DSchedule::Ptr result = item.getScheduleByID("schedule-1");
    EXPECT_TRUE(g_getScheduleByIDCalled);
    EXPECT_EQ(g_stubSchedule, result);
}

TEST(AccountItemTest, CreateSchedule_Stubbed)
{
    g_createScheduleCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, createSchedule), stub_createSchedule);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    DSchedule::Ptr schedule(new DSchedule());
    QString result = item.createSchedule(schedule);
    EXPECT_TRUE(g_createScheduleCalled);
    EXPECT_EQ(g_stubScheduleID, result);
}

TEST(AccountItemTest, QuerySchedulesWithParameter_Stubbed)
{
    g_querySchedulesCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_querySchedulesWithParameter);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    DScheduleQueryPar::Ptr params(new DScheduleQueryPar());
    DSchedule::Map result = item.querySchedulesWithParameter(params);
    EXPECT_TRUE(g_querySchedulesCalled);
    EXPECT_TRUE(result.isEmpty());
}

TEST(AccountItemTest, UpdateAccountInfo_Stubbed)
{
    g_updateAccountInfoCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, updateAccountInfo), stub_updateAccountInfo);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    bool callbackCalled = false;
    item.updateAccountInfo([&callbackCalled](CallMessge) { callbackCalled = true; });
    EXPECT_TRUE(g_updateAccountInfoCalled);
}

TEST(AccountItemTest, CreateJobType_SetsUserPrivilege)
{
    g_createScheduleTypeCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, createScheduleType), stub_createScheduleType);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    DScheduleType::Ptr typeInfo(new DScheduleType());
    typeInfo->setPrivilege(DScheduleType::None);
    item.createJobType(typeInfo, [](CallMessge) {});
    EXPECT_TRUE(g_createScheduleTypeCalled);
    EXPECT_EQ(DScheduleType::User, typeInfo->privilege());
}

TEST(AccountItemTest, UpdateScheduleType_Stubbed)
{
    g_updateScheduleTypeCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, updateScheduleType), stub_updateScheduleType);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    DScheduleType::Ptr typeInfo(new DScheduleType());
    item.updateScheduleType(typeInfo, [](CallMessge) {});
    EXPECT_TRUE(g_updateScheduleTypeCalled);
}

TEST(AccountItemTest, DeleteScheduleTypeByID_Stubbed)
{
    g_deleteScheduleTypeCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, deleteScheduleTypeByID), stub_deleteScheduleTypeByID);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.deleteScheduleTypeByID("type1", [](CallMessge) {});
    EXPECT_TRUE(g_deleteScheduleTypeCalled);
}

TEST(AccountItemTest, ScheduleTypeIsUsed_Stubbed)
{
    g_scheduleTypeByUsedCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, scheduleTypeByUsed), stub_scheduleTypeByUsed);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.scheduleTypeIsUsed("type1", [](CallMessge) {});
    EXPECT_TRUE(g_scheduleTypeByUsedCalled);
}

TEST(AccountItemTest, UpdateSchedule_Stubbed)
{
    g_updateScheduleCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, updateSchedule), stub_updateSchedule);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    DSchedule::Ptr schedule(new DSchedule());
    item.updateSchedule(schedule, [](CallMessge) {});
    EXPECT_TRUE(g_updateScheduleCalled);
}

TEST(AccountItemTest, DeleteScheduleByID_Stubbed)
{
    g_deleteScheduleByIDCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, deleteScheduleByScheduleID), stub_deleteScheduleByScheduleID);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.deleteScheduleByID("schedule-1", [](CallMessge) {});
    EXPECT_TRUE(g_deleteScheduleByIDCalled);
}

TEST(AccountItemTest, DeleteSchedulesByTypeID_Stubbed)
{
    g_deleteSchedulesByTypeIDCalled = false;
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, deleteSchedulesByScheduleTypeID), stub_deleteSchedulesByScheduleTypeID);

    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.deleteSchedulesByTypeID("type1", [](CallMessge) {});
    EXPECT_TRUE(g_deleteSchedulesByTypeIDCalled);
}

TEST(AccountItemTest, MonitorScheduleTypeData_FirstCall_CallbackNotInvoked)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    bool callbackCalled = false;
    item.monitorScheduleTypeData([&callbackCalled]() { callbackCalled = true; });
    EXPECT_FALSE(callbackCalled);
    EXPECT_FALSE(item.m_dataStatus.value("ScheduleType"));
}

TEST(AccountItemTest, MonitorScheduleTypeData_StatusTrue_CallbackInvoked)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    item.m_dataStatus.insert("ScheduleType", true);
    bool callbackCalled = false;
    item.monitorScheduleTypeData([&callbackCalled]() { callbackCalled = true; });
    EXPECT_TRUE(callbackCalled);
}

TEST(AccountItemTest, MonitorScheduleTypeData_MultipleCallbacks_StoredInList)
{
    // DEFECT: monitorScheduleTypeData does not append the new callback when the
    // key already exists in m_callbackMap — it copies the existing list but never
    // calls funcList.append(callback) in the else branch. Only the first callback
    // is retained. (See defects.json / defects-summary.md)
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    bool cb1 = false, cb2 = false;
    item.monitorScheduleTypeData([&cb1]() { cb1 = true; });
    item.monitorScheduleTypeData([&cb2]() { cb2 = true; });
    EXPECT_FALSE(cb1);
    EXPECT_FALSE(cb2);
    // Due to the defect, only 1 callback is stored instead of 2
    EXPECT_EQ(1, item.m_callbackMap.value("ScheduleType").size());
}

TEST(AccountItemTest, SlotGetAccountInfoFinish_UpdatesAccount)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    QSignalSpy spy(&item, &AccountItem::signalAccountDataUpdate);

    DAccount::Ptr newAccount(new DAccount(DAccount::Account_Local));
    newAccount->setAccountID("new-id");
    item.slotGetAccountInfoFinish(newAccount);

    EXPECT_EQ(newAccount, item.m_account);
    EXPECT_EQ("new-id", item.getAccount()->accountID());
    EXPECT_EQ(1, spy.count());
}

TEST(AccountItemTest, SlotGetScheduleTypeListFinish_UpdatesList)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    QSignalSpy spy(&item, &AccountItem::signalScheduleTypeUpdate);

    DScheduleType::List testList = createTestScheduleTypes();
    item.slotGetScheduleTypeListFinish(testList);

    EXPECT_EQ(3, item.m_scheduleTypeList.size());
    EXPECT_TRUE(item.m_dataStatus.value("ScheduleType"));
    EXPECT_EQ(1, spy.count());
}

TEST(AccountItemTest, SlotGetScheduleTypeListFinish_TriggersCallbacks)
{
    // DEFECT: due to monitorScheduleTypeData not appending subsequent callbacks,
    // only cb1 is stored and triggered. cb2 is silently dropped. (See defects.json)
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);

    bool cb1 = false, cb2 = false;
    item.monitorScheduleTypeData([&cb1]() { cb1 = true; });
    item.monitorScheduleTypeData([&cb2]() { cb2 = true; });

    DScheduleType::List testList = createTestScheduleTypes();
    item.slotGetScheduleTypeListFinish(testList);

    // Only cb1 is triggered because cb2 was never stored (defect)
    EXPECT_TRUE(cb1);
    EXPECT_FALSE(cb2);
}

TEST(AccountItemTest, SlotGetScheduleListFinish_UpdatesMap)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    QSignalSpy spy(&item, &AccountItem::signalScheduleUpdate);

    QMap<QDate, DSchedule::List> testMap;
    QDate date(2026, 1, 1);
    DSchedule::List list;
    list.append(DSchedule::Ptr(new DSchedule()));
    testMap.insert(date, list);

    item.slotGetScheduleListFinish(testMap);

    EXPECT_EQ(1, item.m_scheduleMap.size());
    EXPECT_EQ(1, spy.count());
}

TEST(AccountItemTest, SlotSearchScheduleListFinish_UpdatesMap)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);
    QSignalSpy spy(&item, &AccountItem::signalSearchScheduleUpdate);

    QMap<QDate, DSchedule::List> testMap;
    QDate date(2026, 6, 15);
    DSchedule::List list;
    list.append(DSchedule::Ptr(new DSchedule()));
    testMap.insert(date, list);

    item.slotSearchScheduleListFinish(testMap);

    EXPECT_EQ(1, item.m_searchedScheduleMap.size());
    EXPECT_EQ(1, spy.count());
}

TEST(AccountItemTest, SlotGetSysColorsFinish_UpdatesColorList)
{
    DAccount::Ptr account = createTestAccount();
    AccountItem item(account);

    DTypeColor::List testList = createTestColorList();
    item.slotGetSysColorsFinish(testList);

    EXPECT_EQ(2, item.m_typeColorList.size());
}
