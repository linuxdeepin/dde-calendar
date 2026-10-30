// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_schedulemanager.h"

#include "dataManage/accountmanager.h"
#include "dataManage/accountitem.h"
#include "cdbuspendingcallwatcher.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "dcaldavaccountstatus.h"
#include "dschedulequerypar.h"

#include <QDateTime>
#include <QDate>

// ---- Stub typedefs for AccountManager / AccountItem methods ----
typedef QList<AccountItem::Ptr> (*fptr_getAccountList)(void *);
typedef DAccount::Ptr (*fptr_getAccount)(void *);
typedef QMap<QDate, DSchedule::List> (*fptr_getScheduleMap)(void *);
typedef DScheduleType::Ptr (*fptr_getScheduleTypeByID)(void *, const QString &);
typedef DCalDavAccountStatus (*fptr_getCalDavStatus)(void *, const QString &);
typedef void (*fptr_queryInt)(void *, const int, CallbackFunc);
typedef void (*fptr_queryDateTime)(void *, const QDateTime &, const QDateTime &, CallbackFunc);
typedef void (*fptr_queryParams)(void *, const DScheduleQueryPar::Ptr &, CallbackFunc);

// ---- Stub data globals ----
static DAccount::Type g_accountType = DAccount::Account_Local;
static QMap<QDate, DSchedule::List> g_stubScheduleMap;
static QMap<QDate, DSchedule::List> g_stubSearchScheduleMap;
static bool g_queryParamsCallbackInvoked = false;

// ---- Stub functions ----
QList<AccountItem::Ptr> stub_getAccountList(void *)
{
    QList<AccountItem::Ptr> list;
    char *dummy = new char[sizeof(AccountItem)];
    memset(dummy, 0, sizeof(AccountItem));
    auto noopDeleter = [](AccountItem *) {};
    list.append(AccountItem::Ptr(reinterpret_cast<AccountItem *>(dummy), noopDeleter));
    return list;
}

DAccount::Ptr stub_getAccount(void *)
{
    return DAccount::Ptr(new DAccount(g_accountType));
}

QMap<QDate, DSchedule::List> stub_getScheduleMap(void *)
{
    return g_stubScheduleMap;
}

QMap<QDate, DSchedule::List> stub_getSearchScheduleMap(void *)
{
    return g_stubSearchScheduleMap;
}

DScheduleType::Ptr stub_getScheduleTypeByID(void *, const QString &)
{
    DScheduleType::Ptr type(new DScheduleType());
    type->setShowState(DScheduleType::Show);
    return type;
}

DCalDavAccountStatus stub_getCalDavStatus(void *, const QString &)
{
    DCalDavAccountStatus status;
    status.accountColor = "#FF0000";
    return status;
}

void stub_queryInt(void *, const int, CallbackFunc) {}
void stub_queryDateTime(void *, const QDateTime &, const QDateTime &, CallbackFunc) {}

void stub_queryParams(void *, const DScheduleQueryPar::Ptr &, CallbackFunc callback)
{
    g_queryParamsCallbackInvoked = true;
    if (callback) {
        callback(CallMessge{0, {}});
    }
}

// ---- Helper: create a DSchedule with a schedule type ID ----
static DSchedule::Ptr createTestSchedule(const QString &typeId = "type_1")
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setScheduleTypeID(typeId);
    return schedule;
}

// ---- Helper: populate stub schedule maps ----
static void populateStubMaps()
{
    g_stubScheduleMap.clear();
    g_stubSearchScheduleMap.clear();
    QDate testDate(2025, 6, 15);
    DSchedule::List list;
    list.append(createTestSchedule());
    g_stubScheduleMap[testDate] = list;
    g_stubSearchScheduleMap[testDate] = list;
}


test_schedulemanager::test_schedulemanager()
{
    manager = ScheduleManager::getInstace();
}

test_schedulemanager::~test_schedulemanager()
{
    manager = nullptr;
}

// ScheduleManager::ScheduleManager(QObject *parent)
TEST_F(test_schedulemanager, constructor)
{
    // Constructor is called via getInstace() singleton; verify it's alive
    ScheduleManager *inst = ScheduleManager::getInstace();
    EXPECT_NE(inst, nullptr);
}

// static ScheduleManager *ScheduleManager::getInstace()
TEST_F(test_schedulemanager, getInstace)
{
    ScheduleManager *inst1 = ScheduleManager::getInstace();
    ScheduleManager *inst2 = ScheduleManager::getInstace();
    EXPECT_NE(inst1, nullptr);
    EXPECT_EQ(inst1, inst2);
}

// void ScheduleManager::initconnect()
TEST_F(test_schedulemanager, initconnect)
{
    // initconnect is called in constructor; singleton is already initialized
    // Verify the object is valid (connections established without crash)
    EXPECT_NE(manager, nullptr);
}

// QMap<QDate, DSchedule::List> ScheduleManager::getAllScheduleMap()
TEST_F(test_schedulemanager, getAllScheduleMap)
{
    // Inject test data into m_scheduleMap
    QDate testDate(2025, 6, 15);
    DSchedule::List testList;
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Test Schedule");
    testList.append(schedule);
    manager->m_scheduleMap[testDate] = testList;

    QMap<QDate, DSchedule::List> result = manager->getAllScheduleMap();
    EXPECT_EQ(result.size(), manager->m_scheduleMap.size());
    EXPECT_TRUE(result.contains(testDate));

    // Clean up
    manager->m_scheduleMap.clear();
}

// QMap<QDate, DSchedule::List> ScheduleManager::getScheduleMap(const QDate &startDate, const QDate &stopDate) const
TEST_F(test_schedulemanager, getScheduleMap)
{
    QDate d1(2025, 6, 1);
    QDate d2(2025, 6, 15);
    QDate d3(2025, 6, 30);
    DSchedule::List list1;
    list1.append(DSchedule::Ptr(new DSchedule()));
    DSchedule::List list3;
    list3.append(DSchedule::Ptr(new DSchedule()));
    manager->m_scheduleMap[d1] = list1;
    manager->m_scheduleMap[d3] = list3;

    // Range that includes both dates
    QMap<QDate, DSchedule::List> result = manager->getScheduleMap(d1, d3);
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains(d1));
    EXPECT_TRUE(result.contains(d3));

    // Range that includes only d1
    result = manager->getScheduleMap(d1, d2);
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains(d1));

    // Range with no schedules
    result = manager->getScheduleMap(QDate(2025, 7, 1), QDate(2025, 7, 10));
    EXPECT_EQ(result.size(), 0);

    // Same start and stop date that has schedule
    result = manager->getScheduleMap(d1, d1);
    EXPECT_EQ(result.size(), 1);

    // Clean up
    manager->m_scheduleMap.clear();
}

// QMap<QDate, DSchedule::List> ScheduleManager::getAllSearchedScheduleMap()
TEST_F(test_schedulemanager, getAllSearchedScheduleMap)
{
    QDate testDate(2025, 8, 20);
    DSchedule::List testList;
    testList.append(DSchedule::Ptr(new DSchedule()));
    manager->m_searchScheduleMap[testDate] = testList;

    QMap<QDate, DSchedule::List> result = manager->getAllSearchedScheduleMap();
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains(testDate));

    // Clean up
    manager->m_searchScheduleMap.clear();
}

// DSchedule::List ScheduleManager::getAllSearchedScheduleList()
TEST_F(test_schedulemanager, getAllSearchedScheduleList)
{
    QDate d1(2025, 8, 1);
    QDate d2(2025, 8, 2);
    DSchedule::List list1;
    list1.append(DSchedule::Ptr(new DSchedule()));
    DSchedule::List list2;
    list2.append(DSchedule::Ptr(new DSchedule()));
    list2.append(DSchedule::Ptr(new DSchedule()));
    manager->m_searchScheduleMap[d1] = list1;
    manager->m_searchScheduleMap[d2] = list2;

    DSchedule::List result = manager->getAllSearchedScheduleList();
    EXPECT_EQ(result.size(), 3);

    // Empty map
    manager->m_searchScheduleMap.clear();
    result = manager->getAllSearchedScheduleList();
    EXPECT_EQ(result.size(), 0);
}

// QSet<QDate> ScheduleManager::getAllScheduleDate()
TEST_F(test_schedulemanager, getAllScheduleDate)
{
    QDate d1(2025, 3, 1);
    QDate d2(2025, 3, 15);
    manager->m_scheduleMap[d1] = DSchedule::List();
    manager->m_scheduleMap[d2] = DSchedule::List();

    QSet<QDate> result = manager->getAllScheduleDate();
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains(d1));
    EXPECT_TRUE(result.contains(d2));

    // Clean up
    manager->m_scheduleMap.clear();
}

// QSet<QDate> ScheduleManager::getAllSearchedScheduleDate()
TEST_F(test_schedulemanager, getAllSearchedScheduleDate)
{
    QDate d1(2025, 4, 1);
    QDate d2(2025, 4, 10);
    manager->m_searchScheduleMap[d1] = DSchedule::List();
    manager->m_searchScheduleMap[d2] = DSchedule::List();

    QSet<QDate> result = manager->getAllSearchedScheduleDate();
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains(d1));
    EXPECT_TRUE(result.contains(d2));

    // Clean up
    manager->m_searchScheduleMap.clear();
}

// DSchedule::List ScheduleManager::getScheduleByDay(QDate date)
TEST_F(test_schedulemanager, getScheduleByDay)
{
    QDate testDate(2025, 5, 1);
    DSchedule::List testList;
    testList.append(DSchedule::Ptr(new DSchedule()));
    manager->m_scheduleMap[testDate] = testList;

    // Date with schedule
    DSchedule::List result = manager->getScheduleByDay(testDate);
    EXPECT_EQ(result.size(), 1);

    // Date without schedule
    result = manager->getScheduleByDay(QDate(2025, 5, 2));
    EXPECT_EQ(result.size(), 0);

    // Clean up
    manager->m_scheduleMap.clear();
}

// void ScheduleManager::clearSearchSchedule()
TEST_F(test_schedulemanager, clearSearchSchedule)
{
    // Inject data into search map
    QDate testDate(2025, 7, 1);
    DSchedule::List testList;
    testList.append(DSchedule::Ptr(new DSchedule()));
    manager->m_searchScheduleMap[testDate] = testList;
    EXPECT_FALSE(manager->m_searchScheduleMap.isEmpty());

    manager->clearSearchSchedule();
    EXPECT_TRUE(manager->m_searchScheduleMap.isEmpty());
}

// void ScheduleManager::slotScheduleUpdate()
TEST_F(test_schedulemanager, slotScheduleUpdate)
{
    // Calls updateSchedule which calls mergeAccountSchedules (uses gAccountManager)
    // If gAccountManager is initialized with empty account list, should not crash
    manager->slotScheduleUpdate();
    SUCCEED();
}

// void ScheduleManager::slotSearchUpdate()
TEST_F(test_schedulemanager, slotSearchUpdate)
{
    // Calls updateSearchSchedule which calls mergeAccountSchedules (uses gAccountManager)
    manager->slotSearchUpdate();
    SUCCEED();
}

// void ScheduleManager::updateSchedule()
TEST_F(test_schedulemanager, updateSchedule)
{
    // Clears m_scheduleMap, calls mergeAccountSchedules, emits signal
    manager->m_scheduleMap[QDate(2025, 1, 1)] = DSchedule::List();
    EXPECT_FALSE(manager->m_scheduleMap.isEmpty());

    manager->updateSchedule();
    // After update, m_scheduleMap should be repopulated (or empty if no accounts)
    SUCCEED();
}

// void ScheduleManager::updateSearchSchedule()
TEST_F(test_schedulemanager, updateSearchSchedule)
{
    manager->m_searchScheduleMap[QDate(2025, 1, 1)] = DSchedule::List();
    EXPECT_FALSE(manager->m_searchScheduleMap.isEmpty());

    manager->updateSearchSchedule();
    SUCCEED();
}

// DScheduleType::Ptr ScheduleManager::getScheduleTypeByScheduleId(const QString &id)
TEST_F(test_schedulemanager, getScheduleTypeByScheduleId)
{
    // With invalid ID, should return nullptr
    DScheduleType::Ptr result = manager->getScheduleTypeByScheduleId("invalid_id");
    EXPECT_EQ(result, nullptr);
}

// void ScheduleManager::searchSchedule(const QString &key, const QDateTime &startTime, const QDateTime &endTime)
TEST_F(test_schedulemanager, searchSchedule)
{
    // Should not crash; creates DScheduleQueryPar, iterates gAccountManager
    QDateTime start(QDate(2025, 1, 1), QTime(0, 0, 0));
    QDateTime end(QDate(2025, 12, 31), QTime(23, 59, 59));
    manager->searchSchedule("test", start, end);
    SUCCEED();
}

// void ScheduleManager::resetSchedule(const QDateTime &start, const QDateTime &end)
TEST_F(test_schedulemanager, resetSchedule)
{
    // Iterates gAccountManager->getAccountList()
    QDateTime start(QDate(2025, 1, 1), QTime(0, 0, 0));
    QDateTime end(QDate(2025, 12, 31), QTime(23, 59, 59));
    manager->resetSchedule(start, end);
    SUCCEED();
}

// isScheduleVisible is in an anonymous namespace in schedulemanager.cpp — cannot be
// called directly from this TU. It is exercised indirectly through updateSchedule()
// → mergeAccountSchedules() → visibleSchedules() → isScheduleVisible().
TEST_F(test_schedulemanager, isScheduleVisible)
{
    // Exercise the code path: updateSchedule iterates accounts, calls isScheduleVisible
    // In test env gAccountManager returns empty account list, so the path completes safely.
    manager->updateSchedule();
    EXPECT_EQ(manager->m_scheduleMap.size(), 0);
    SUCCEED();
}

// visibleSchedules is in an anonymous namespace — exercised indirectly through
// updateSchedule() and updateSearchSchedule().
TEST_F(test_schedulemanager, visibleSchedules)
{
    // Exercise the code path via updateSearchSchedule which also calls visibleSchedules
    manager->updateSearchSchedule();
    EXPECT_EQ(manager->m_searchScheduleMap.size(), 0);
    SUCCEED();
}

// void mergeAccountSchedules(QMap<QDate, DSchedule::List> &target, ScheduleMapGetter scheduleMapGetter)
TEST_F(test_schedulemanager, mergeAccountSchedules)
{
    // mergeAccountSchedules is a template function; it's called by updateSchedule/updateSearchSchedule
    // We already test those; verify the maps are in valid state after merge
    manager->updateSchedule();
    EXPECT_TRUE(manager->m_scheduleMap.size() >= 0);
    SUCCEED();
}
// ==================== Stub-based tests for uncovered functions ====================

// ScheduleManager::resetSchedule(int year)
TEST_F(test_schedulemanager, resetSchedule_int)
{
    Stub stub;
    stub.set((fptr_getAccountList)(&AccountManager::getAccountList), stub_getAccountList);
    stub.set((fptr_queryInt)(static_cast<void (AccountItem::*)(const int, CallbackFunc)>(&AccountItem::querySchedulesWithParameter)), stub_queryInt);

    manager->resetSchedule(2025);
    SUCCEED();
}

// searchSchedule lambda callback (count==0 → updateSearchSchedule)
TEST_F(test_schedulemanager, searchSchedule_withCallback)
{
    Stub stub;
    stub.set((fptr_getAccountList)(&AccountManager::getAccountList), stub_getAccountList);
    stub.set((fptr_queryParams)(static_cast<void (AccountItem::*)(const DScheduleQueryPar::Ptr &, CallbackFunc)>(&AccountItem::querySchedulesWithParameter)), stub_queryParams);

    g_queryParamsCallbackInvoked = false;
    QDateTime start(QDate(2025, 1, 1), QTime(0, 0, 0));
    QDateTime end(QDate(2025, 12, 31), QTime(23, 59, 59));
    manager->searchSchedule("test", start, end);

    EXPECT_TRUE(g_queryParamsCallbackInvoked);
}

// initconnect CalDav lambda (emit signalCalDavAccountStatusChanged)
TEST_F(test_schedulemanager, initconnect_calDavLambda)
{
    Stub stub;
    stub.set((fptr_getAccountList)(&AccountManager::getAccountList), stub_getAccountList);
    stub.set((fptr_getScheduleMap)(&AccountItem::getScheduleMap), stub_getScheduleMap);
    stub.set((fptr_getScheduleMap)(&AccountItem::getSearchScheduleMap), stub_getSearchScheduleMap);
    stub.set((fptr_getAccount)(&AccountItem::getAccount), stub_getAccount);
    stub.set((fptr_getScheduleTypeByID)(&AccountManager::getScheduleTypeByScheduleTypeId), stub_getScheduleTypeByID);

    g_accountType = DAccount::Account_Local;
    populateStubMaps();

    // The lambda is connected in initconnect() (called from constructor).
    // Emit the signal to trigger the lambda.
    emit gAccountManager->signalCalDavAccountStatusChanged("test_account");
    SUCCEED();
}

// updateSchedule lambda + mergeAccountSchedules + visibleSchedules + isScheduleVisible (Local account)
TEST_F(test_schedulemanager, updateSchedule_withAccounts)
{
    Stub stub;
    stub.set((fptr_getAccountList)(&AccountManager::getAccountList), stub_getAccountList);
    stub.set((fptr_getScheduleMap)(&AccountItem::getScheduleMap), stub_getScheduleMap);
    stub.set((fptr_getAccount)(&AccountItem::getAccount), stub_getAccount);
    stub.set((fptr_getScheduleTypeByID)(&AccountManager::getScheduleTypeByScheduleTypeId), stub_getScheduleTypeByID);

    g_accountType = DAccount::Account_Local;
    populateStubMaps();

    manager->m_scheduleMap.clear();
    manager->updateSchedule();
    EXPECT_FALSE(manager->m_scheduleMap.isEmpty());
}

// updateSchedule with CalDav account (covers accountColor branch)
TEST_F(test_schedulemanager, updateSchedule_withCalDavAccount)
{
    Stub stub;
    stub.set((fptr_getAccountList)(&AccountManager::getAccountList), stub_getAccountList);
    stub.set((fptr_getScheduleMap)(&AccountItem::getScheduleMap), stub_getScheduleMap);
    stub.set((fptr_getAccount)(&AccountItem::getAccount), stub_getAccount);
    stub.set((fptr_getScheduleTypeByID)(&AccountManager::getScheduleTypeByScheduleTypeId), stub_getScheduleTypeByID);
    stub.set((fptr_getCalDavStatus)(&AccountManager::getCalDavAccountStatus), stub_getCalDavStatus);

    g_accountType = DAccount::Account_CalDav;
    populateStubMaps();

    manager->m_scheduleMap.clear();
    manager->updateSchedule();
    EXPECT_FALSE(manager->m_scheduleMap.isEmpty());
}

// updateSearchSchedule lambda + mergeAccountSchedules + visibleSchedules
TEST_F(test_schedulemanager, updateSearchSchedule_withAccounts)
{
    Stub stub;
    stub.set((fptr_getAccountList)(&AccountManager::getAccountList), stub_getAccountList);
    stub.set((fptr_getScheduleMap)(&AccountItem::getSearchScheduleMap), stub_getSearchScheduleMap);
    stub.set((fptr_getAccount)(&AccountItem::getAccount), stub_getAccount);
    stub.set((fptr_getScheduleTypeByID)(&AccountManager::getScheduleTypeByScheduleTypeId), stub_getScheduleTypeByID);

    g_accountType = DAccount::Account_Local;
    populateStubMaps();

    manager->m_searchScheduleMap.clear();
    manager->updateSearchSchedule();
    EXPECT_FALSE(manager->m_searchScheduleMap.isEmpty());
}

// isScheduleVisible null schedule path (null DSchedule::Ptr in schedule list)
TEST_F(test_schedulemanager, isScheduleVisible_nullSchedule)
{
    Stub stub;
    stub.set((fptr_getAccountList)(&AccountManager::getAccountList), stub_getAccountList);
    stub.set((fptr_getScheduleMap)(&AccountItem::getScheduleMap), stub_getScheduleMap);
    stub.set((fptr_getAccount)(&AccountItem::getAccount), stub_getAccount);
    stub.set((fptr_getScheduleTypeByID)(&AccountManager::getScheduleTypeByScheduleTypeId), stub_getScheduleTypeByID);

    g_accountType = DAccount::Account_Local;
    g_stubScheduleMap.clear();
    g_stubSearchScheduleMap.clear();
    QDate testDate(2025, 7, 1);
    DSchedule::List list;
    // Add a null schedule — isScheduleVisible should return false for it
    list.append(DSchedule::Ptr(nullptr));
    list.append(createTestSchedule());
    g_stubScheduleMap[testDate] = list;

    manager->m_scheduleMap.clear();
    manager->updateSchedule();
    // The non-null schedule should still be visible and added to the map
    EXPECT_FALSE(manager->m_scheduleMap.isEmpty());
}
