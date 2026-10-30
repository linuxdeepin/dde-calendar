// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "../third-party_stub/stub.h"

#include "accountitem.h"
#include "dbus/dbusaccountrequest.h"
#include "dbus/doanetworkdbus.h"
#include "daccount.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "dtypecolor.h"
#include "dschedulequerypar.h"
#include "cdbuspendingcallwatcher.h"

#include <QTest>
#include <QSignalSpy>

// ---- Stub functions for DOANetWorkDBus ----
static DOANetWorkDBus::NetWorkState stub_getNetWorkState_active()
{
    return DOANetWorkDBus::Active;
}

static DOANetWorkDBus::NetWorkState stub_getNetWorkState_disconnect()
{
    return DOANetWorkDBus::Disconnect;
}

// ---- Stub functions for DbusAccountRequest methods ----
static void stub_void() {}
static void stub_void_bool(bool) {}
static void stub_void_states(DAccount::AccountStates) {}
static void stub_void_str(const QString &) {}
static void stub_void_strlist(const QString &) {}
static void stub_void_callback(CallbackFunc) {}
static void stub_void_typeptr(const DScheduleType::Ptr &) {}
static void stub_void_schedptr(const DSchedule::Ptr &) {}
static void stub_void_typeptr_cb(const DScheduleType::Ptr &, CallbackFunc) {}
static void stub_void_schedptr_cb(const DSchedule::Ptr &, CallbackFunc) {}
static void stub_void_str_cb(const QString &, CallbackFunc) {}
static void stub_void_str_bool_cb(const QString &, bool, CallbackFunc) {}
static void stub_void_str_bool(const QString &, bool) {}
static void stub_void_str_str(const QString &, const QString &) {}
static void stub_void_str_str_str_bool(const QString &, const QString &, bool) {}
static void stub_void_queryptr(const DScheduleQueryPar::Ptr &) {}
static void stub_void_queryptr_cb(const DScheduleQueryPar::Ptr &, CallbackFunc) {}
static bool stub_bool_str(const QString &)
{
    return true;
}
static bool stub_bool_false_str(const QString &)
{
    return false;
}
static bool stub_bool_queryptr_json(const DScheduleQueryPar::Ptr &, QString &)
{
    return true;
}
static bool stub_bool_false_queryptr_json(const DScheduleQueryPar::Ptr &, QString &)
{
    return false;
}
static DSchedule::Ptr stub_schedptr_str(const QString &)
{
    return DSchedule::Ptr();
}
static QString stub_str()
{
    return QString("test");
}
static DAccount::AccountStates stub_accountStates()
{
    return DAccount::AccountStates(DAccount::Account_Open);
}
static bool stub_bool_true()
{
    return true;
}
static bool stub_bool_false()
{
    return false;
}
static DAccount::AccountSyncState stub_syncState()
{
    return DAccount::Sync_Normal;
}
static DAccount::SyncFreqType stub_syncFreq()
{
    return DAccount::SyncFreq_Maunal;
}

// Helper to create a local DAccount
static DAccount::Ptr createLocalAccount()
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountID("local-001");
    account->setAccountName("LocalAccount");
    account->setDbusPath("/com/deepin/dataserver/Calendar/Local");
    account->setDbusInterface("com.deepin.dataserver.Calendar.Local");
    return account;
}

static DAccount::Ptr createUnionAccount()
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountID("union-001");
    account->setAccountName("UnionAccount");
    account->setDbusPath("/com/deepin/dataserver/Calendar/Union");
    account->setDbusInterface("com.deepin.dataserver.Calendar.Union");
    return account;
}

class TestAccountItem : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Stub DbusAccountRequest constructor to avoid real DBus connection issues
        // The DbusAccountRequest inherits from DbusRequestBase -> QDBusAbstractInterface
        // which may work with sessionBus, so we try direct creation first.
        // If it fails, we can stub the constructor.

        account = createLocalAccount();
        item = AccountItem::Ptr(new AccountItem(account));
    }

    void TearDown() override
    {
        item.clear();
    }

    DAccount::Ptr account;
    AccountItem::Ptr item;
};

// getSyncMsg - all 4 states
TEST_F(TestAccountItem, GetSyncMsgNormal)
{
    EXPECT_FALSE(AccountItem::getSyncMsg(DAccount::Sync_Normal).isEmpty());
}

TEST_F(TestAccountItem, GetSyncMsgNetworkAnomaly)
{
    EXPECT_FALSE(AccountItem::getSyncMsg(DAccount::Sync_NetworkAnomaly).isEmpty());
}

TEST_F(TestAccountItem, GetSyncMsgServerException)
{
    EXPECT_FALSE(AccountItem::getSyncMsg(DAccount::Sync_ServerException).isEmpty());
}

TEST_F(TestAccountItem, GetSyncMsgStorageFull)
{
    EXPECT_FALSE(AccountItem::getSyncMsg(DAccount::Sync_StorageFull).isEmpty());
}

// getAccount
TEST_F(TestAccountItem, GetAccount)
{
    EXPECT_EQ(item->getAccount()->accountID(), account->accountID());
}

// updateAccount - matching ID
TEST_F(TestAccountItem, UpdateAccountMatching)
{
    QSignalSpy spy(item.data(), &AccountItem::signalAccountDataUpdate);
    DAccount::Ptr updated(new DAccount(DAccount::Account_Local));
    updated->setAccountID("local-001");
    updated->setAccountName("UpdatedName");
    item->updateAccount(updated);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(item->getAccount()->accountName(), QString("UpdatedName"));
}

// updateAccount - non-matching ID
TEST_F(TestAccountItem, UpdateAccountNonMatching)
{
    QSignalSpy spy(item.data(), &AccountItem::signalAccountDataUpdate);
    DAccount::Ptr updated(new DAccount(DAccount::Account_Local));
    updated->setAccountID("different-id");
    item->updateAccount(updated);
    EXPECT_EQ(spy.count(), 0);
}

// getScheduleMap / getSearchScheduleMap
TEST_F(TestAccountItem, GetScheduleMap)
{
    auto map = item->getScheduleMap();
    EXPECT_EQ(map.size(), 0);
}

TEST_F(TestAccountItem, GetSearchScheduleMap)
{
    auto map = item->getSearchScheduleMap();
    EXPECT_EQ(map.size(), 0);
}

// getScheduleTypeList
TEST_F(TestAccountItem, GetScheduleTypeList)
{
    DScheduleType::Ptr type(new DScheduleType());
    type->setPrivilege(DScheduleType::User);
    item->m_scheduleTypeList.append(type);
    auto list = item->getScheduleTypeList();
    EXPECT_EQ(list.size(), 1);
}

// getScheduleTypeByID
TEST_F(TestAccountItem, GetScheduleTypeByIDFound)
{
    DScheduleType::Ptr type(new DScheduleType());
    type->setTypeID("type-001");
    item->m_scheduleTypeList.append(type);
    auto result = item->getScheduleTypeByID("type-001");
    EXPECT_NE(result, nullptr);
}

TEST_F(TestAccountItem, GetScheduleTypeByIDNotFound)
{
    auto result = item->getScheduleTypeByID("nonexistent");
    EXPECT_EQ(result, nullptr);
}

// getScheduleTypeByName
TEST_F(TestAccountItem, GetScheduleTypeByNameFound)
{
    DScheduleType::Ptr type(new DScheduleType());
    type->setTypeName("Work");
    type->setDisplayName("WorkSchedule");
    item->m_scheduleTypeList.append(type);
    auto result = item->getScheduleTypeByName("Work");
    EXPECT_NE(result, nullptr);
    auto result2 = item->getScheduleTypeByName("WorkSchedule");
    EXPECT_NE(result2, nullptr);
}

TEST_F(TestAccountItem, GetScheduleTypeByNameNotFound)
{
    auto result = item->getScheduleTypeByName("nonexistent");
    EXPECT_EQ(result, nullptr);
}

// getColorTypeList
TEST_F(TestAccountItem, GetColorTypeList)
{
    DTypeColor::Ptr color(new DTypeColor());
    item->m_typeColorList.append(color);
    auto list = item->getColorTypeList();
    EXPECT_EQ(list.size(), 1);
}

// isCanSyncShedule - local account always true
TEST_F(TestAccountItem, IsCanSyncSheduleLocal)
{
    EXPECT_TRUE(item->isCanSyncShedule());
}

// isCanSyncShedule - union account with network active
TEST(TestAccountItemUnion, IsCanSyncSheduleUnionActive)
{
    Stub stub;
    stub.set(ADDR(DOANetWorkDBus, getNetWorkState), stub_getNetWorkState_active);
    DAccount::Ptr account = createUnionAccount();
    account->setAccountState(DAccount::AccountStates(DAccount::Account_Open | DAccount::Account_Calendar));
    AccountItem item(account);
    EXPECT_TRUE(item.isCanSyncShedule());
}

// isCanSyncShedule - union account without calendar flag
TEST(TestAccountItemUnion, IsCanSyncSheduleUnionNoCalendar)
{
    Stub stub;
    stub.set(ADDR(DOANetWorkDBus, getNetWorkState), stub_getNetWorkState_active);
    DAccount::Ptr account = createUnionAccount();
    account->setAccountState(DAccount::AccountStates(DAccount::Account_Open));
    AccountItem item(account);
    EXPECT_FALSE(item.isCanSyncShedule());
}

// isCanSyncShedule - union account with network disconnected
TEST(TestAccountItemUnion, IsCanSyncSheduleUnionNoNetwork)
{
    Stub stub;
    stub.set(ADDR(DOANetWorkDBus, getNetWorkState), stub_getNetWorkState_disconnect);
    DAccount::Ptr account = createUnionAccount();
    account->setAccountState(DAccount::AccountStates(DAccount::Account_Open | DAccount::Account_Calendar));
    AccountItem item(account);
    EXPECT_FALSE(item.isCanSyncShedule());
}

// isCanSyncSetting - local account
TEST_F(TestAccountItem, IsCanSyncSettingLocal)
{
    EXPECT_TRUE(item->isCanSyncSetting());
}

// isCanSyncSetting - union account with settings flag
TEST(TestAccountItemUnion, IsCanSyncSettingUnion)
{
    Stub stub;
    stub.set(ADDR(DOANetWorkDBus, getNetWorkState), stub_getNetWorkState_active);
    DAccount::Ptr account = createUnionAccount();
    account->setAccountState(DAccount::AccountStates(DAccount::Account_Open | DAccount::Account_Setting));
    AccountItem item(account);
    EXPECT_TRUE(item.isCanSyncSetting());
}

// isCanSyncSetting - union account without settings flag
TEST(TestAccountItemUnion, IsCanSyncSettingUnionNoSetting)
{
    Stub stub;
    stub.set(ADDR(DOANetWorkDBus, getNetWorkState), stub_getNetWorkState_active);
    DAccount::Ptr account = createUnionAccount();
    account->setAccountState(DAccount::AccountStates(DAccount::Account_Open));
    AccountItem item(account);
    EXPECT_FALSE(item.isCanSyncSetting());
}

// isEnableForUosAccount - local account
TEST_F(TestAccountItem, IsEnableForUosAccountLocal)
{
    EXPECT_FALSE(item->isEnableForUosAccount());
}

// isEnableForUosAccount - union account active
TEST(TestAccountItemUnion, IsEnableForUosAccountUnion)
{
    Stub stub;
    stub.set(ADDR(DOANetWorkDBus, getNetWorkState), stub_getNetWorkState_active);
    DAccount::Ptr account = createUnionAccount();
    account->setAccountState(DAccount::AccountStates(DAccount::Account_Open));
    AccountItem item(account);
    EXPECT_TRUE(item.isEnableForUosAccount());
}

// isEnableForUosAccount - union account not open
TEST(TestAccountItemUnion, IsEnableForUosAccountUnionNotOpen)
{
    Stub stub;
    stub.set(ADDR(DOANetWorkDBus, getNetWorkState), stub_getNetWorkState_active);
    DAccount::Ptr account = createUnionAccount();
    AccountItem item(account);
    EXPECT_FALSE(item.isEnableForUosAccount());
}

// setAccountExpandStatus
TEST_F(TestAccountItem, SetAccountExpandStatus)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setAccountExpandStatus), stub_void_bool);
    EXPECT_NO_FATAL_FAILURE(item->setAccountExpandStatus(true));
}

// setAccountState
TEST_F(TestAccountItem, SetAccountState)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setAccountState), stub_void_states);
    stub.set(ADDR(DbusAccountRequest, setSyncFreq), stub_void_str);
    EXPECT_NO_FATAL_FAILURE(item->setAccountState(DAccount::Account_Open));
}

// setSyncFreq
TEST_F(TestAccountItem, SetSyncFreq)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setSyncFreq), stub_void_str);
    EXPECT_NO_FATAL_FAILURE(item->setSyncFreq(DAccount::SyncFreq_15Mins));
}

// getAccountState
TEST_F(TestAccountItem, GetAccountState)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, getAccountState), stub_accountStates);
    EXPECT_EQ(item->getAccountState(), DAccount::Account_Open);
}

// getSyncState
TEST_F(TestAccountItem, GetSyncState)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, getSyncState), stub_bool_true);
    EXPECT_TRUE(item->getSyncState());
}

// getSyncFreq
TEST_F(TestAccountItem, GetSyncFreq)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, getSyncFreq), stub_str);
    EXPECT_NO_FATAL_FAILURE(item->getSyncFreq());
}

// createJobType
TEST_F(TestAccountItem, CreateJobType)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, createScheduleType), stub_void_typeptr);
    DScheduleType::Ptr type(new DScheduleType());
    EXPECT_NO_FATAL_FAILURE(item->createJobType(type));
}

// updateScheduleType
TEST_F(TestAccountItem, UpdateScheduleType)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, updateScheduleType), stub_void_typeptr);
    DScheduleType::Ptr type(new DScheduleType());
    EXPECT_NO_FATAL_FAILURE(item->updateScheduleType(type));
}

// updateScheduleTypeShowState
TEST_F(TestAccountItem, UpdateScheduleTypeShowState)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, updateScheduleTypeShowState), stub_void_typeptr);
    DScheduleType::Ptr type(new DScheduleType());
    EXPECT_NO_FATAL_FAILURE(item->updateScheduleTypeShowState(type));
}

// deleteScheduleTypeByID
TEST_F(TestAccountItem, DeleteScheduleTypeByID)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, deleteScheduleTypeByID), stub_void_str);
    EXPECT_NO_FATAL_FAILURE(item->deleteScheduleTypeByID("type-001"));
}

// scheduleTypeIsUsed
TEST_F(TestAccountItem, ScheduleTypeIsUsed)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, scheduleTypeByUsed), stub_bool_str);
    EXPECT_TRUE(item->scheduleTypeIsUsed("type-001"));
}

// createSchedule
TEST_F(TestAccountItem, CreateSchedule)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, createSchedule), stub_void_schedptr);
    DSchedule::Ptr sched(new DSchedule());
    EXPECT_NO_FATAL_FAILURE(item->createSchedule(sched));
}

// updateSchedule
TEST_F(TestAccountItem, UpdateSchedule)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, updateSchedule), stub_void_schedptr);
    DSchedule::Ptr sched(new DSchedule());
    EXPECT_NO_FATAL_FAILURE(item->updateSchedule(sched));
}

// getScheduleByScheduleID
TEST_F(TestAccountItem, GetScheduleByScheduleID)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, getScheduleByScheduleID), stub_schedptr_str);
    EXPECT_EQ(item->getScheduleByScheduleID("sched-001"), nullptr);
}

// deleteScheduleByID
TEST_F(TestAccountItem, DeleteScheduleByID)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, deleteScheduleByScheduleID), stub_void_str);
    EXPECT_NO_FATAL_FAILURE(item->deleteScheduleByID("sched-001"));
}

// deleteSchedulesByTypeID
TEST_F(TestAccountItem, DeleteSchedulesByTypeID)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, deleteSchedulesByScheduleTypeID), stub_void_str);
    EXPECT_NO_FATAL_FAILURE(item->deleteSchedulesByTypeID("type-001"));
}

// querySchedulesWithParameter - int year overload
TEST_F(TestAccountItem, QuerySchedulesWithParameterYear)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_void_queryptr);
    EXPECT_NO_FATAL_FAILURE(item->querySchedulesWithParameter(2025));
}

// querySchedulesWithParameter - QDateTime start/end overload
TEST_F(TestAccountItem, QuerySchedulesWithParameterDateTime)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_void_queryptr);
    EXPECT_NO_FATAL_FAILURE(item->querySchedulesWithParameter(QDateTime(QDate(2025, 1, 1), QTime(0, 0)), QDateTime(QDate(2025, 12, 31), QTime(23, 59))));
}

// querySchedulesWithParameter - key + QDateTime overload
TEST_F(TestAccountItem, QuerySchedulesWithParameterKey)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_void_queryptr);
    EXPECT_NO_FATAL_FAILURE(item->querySchedulesWithParameter("test", QDateTime(QDate(2025, 1, 1), QTime(0, 0)), QDateTime(QDate(2025, 12, 31), QTime(23, 59))));
}

// querySchedulesWithParameter - DScheduleQueryPar overload
TEST_F(TestAccountItem, QuerySchedulesWithParameterQueryPar)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_void_queryptr);
    DScheduleQueryPar::Ptr par(new DScheduleQueryPar());
    EXPECT_NO_FATAL_FAILURE(item->querySchedulesWithParameter(par));
}

// querySchedulesByExternal - returns QString
TEST_F(TestAccountItem, QuerySchedulesByExternalReturn)
{
    Stub stub;
    stub.set((bool(DbusAccountRequest::*)(const DScheduleQueryPar::Ptr &, QString &))ADDR(DbusAccountRequest, querySchedulesByExternal), stub_bool_queryptr_json);
    QString result = item->querySchedulesByExternal("key", QDateTime(QDate(2025, 1, 1), QTime(0, 0)), QDateTime(QDate(2025, 12, 31), QTime(23, 59)));
    EXPECT_TRUE(result.isEmpty());
}

// querySchedulesByExternal - returns bool with out param
TEST_F(TestAccountItem, QuerySchedulesByExternalBool)
{
    Stub stub;
    stub.set((bool(DbusAccountRequest::*)(const DScheduleQueryPar::Ptr &, QString &))ADDR(DbusAccountRequest, querySchedulesByExternal), stub_bool_false_queryptr_json);
    QMap<QDate, DSchedule::List> out;
    bool result = item->querySchedulesByExternal("key", QDateTime(QDate(2025, 1, 1), QTime(0, 0)), QDateTime(QDate(2025, 12, 31), QTime(23, 59)), out);
    EXPECT_FALSE(result);
}

// importSchedule
TEST_F(TestAccountItem, ImportSchedule)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, importSchedule), stub_void_str_str_str_bool);
    EXPECT_NO_FATAL_FAILURE(item->importSchedule("test.ics", "type-001", true, nullptr));
}

// exportSchedule
TEST_F(TestAccountItem, ExportSchedule)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, exportSchedule), stub_void_str_str);
    EXPECT_NO_FATAL_FAILURE(item->exportSchedule("test.ics", "type-001", nullptr));
}

// resetAccount
TEST_F(TestAccountItem, ResetAccount)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_void_queryptr);
    stub.set(ADDR(DbusAccountRequest, getScheduleTypeList), stub_void);
    stub.set(ADDR(DbusAccountRequest, getSysColors), stub_void);
    EXPECT_NO_FATAL_FAILURE(item->resetAccount());
}

// getDtLastUpdate
TEST_F(TestAccountItem, GetDtLastUpdate)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, getDtLastUpdate), stub_str);
    EXPECT_EQ(item->getDtLastUpdate(), QString("test"));
}

// slotSearchUpdata - with preQuery
TEST_F(TestAccountItem, SlotSearchUpdataWithPreQuery)
{
    Stub stub;
    stub.set(ADDR(DbusAccountRequest, setCallbackFunc), stub_void_callback);
    stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_void_queryptr);
    DScheduleQueryPar::Ptr par(new DScheduleQueryPar());
    item->m_preQuery = par;
    EXPECT_NO_FATAL_FAILURE(item->slotSearchUpdata());
}

// slotSearchUpdata - without preQuery
TEST_F(TestAccountItem, SlotSearchUpdataNoPreQuery)
{
    item->m_preQuery = nullptr;
    EXPECT_NO_FATAL_FAILURE(item->slotSearchUpdata());
}

// slotGetAccountInfoFinish
TEST_F(TestAccountItem, SlotGetAccountInfoFinish)
{
    QSignalSpy spy(item.data(), &AccountItem::signalAccountDataUpdate);
    DAccount::Ptr newAccount(new DAccount(DAccount::Account_Local));
    newAccount->setAccountID("local-001");
    newAccount->setAccountName("NewName");
    item->slotGetAccountInfoFinish(newAccount);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(item->getAccount()->accountName(), QString("NewName"));
}

// slotGetScheduleTypeListFinish
TEST_F(TestAccountItem, SlotGetScheduleTypeListFinish)
{
    QSignalSpy spy(item.data(), &AccountItem::signalScheduleTypeUpdate);
    DScheduleType::List list;
    DScheduleType::Ptr type(new DScheduleType());
    type->setTypeID("type-001");
    type->setPrivilege(DScheduleType::Read);
    list.append(type);
    item->slotGetScheduleTypeListFinish(list);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(item->getScheduleTypeList().size(), 1);
}

// slotGetScheduleTypeListFinish - with schedule cleanup
TEST_F(TestAccountItem, SlotGetScheduleTypeListFinishCleanup)
{
    DSchedule::Ptr sched(new DSchedule());
    sched->setScheduleTypeID("deleted-type");
    DSchedule::List schedList;
    schedList.append(sched);
    item->m_scheduleMap[QDate(2025, 6, 15)] = schedList;
    item->m_searchedScheduleMap[QDate(2025, 6, 15)] = schedList;

    DScheduleType::List list;
    DScheduleType::Ptr type(new DScheduleType());
    type->setTypeID("type-001");
    list.append(type);
    item->slotGetScheduleTypeListFinish(list);
    // Schedules with "deleted-type" should be removed
    EXPECT_EQ(item->getScheduleMap().size(), 0);
    EXPECT_EQ(item->getSearchScheduleMap().size(), 0);
}

// slotGetScheduleListFinish
TEST_F(TestAccountItem, SlotGetScheduleListFinish)
{
    QSignalSpy spy(item.data(), &AccountItem::signalScheduleUpdate);
    QMap<QDate, DSchedule::List> map;
    DSchedule::Ptr sched(new DSchedule());
    DSchedule::List list;
    list.append(sched);
    map[QDate(2025, 6, 15)] = list;
    item->slotGetScheduleListFinish(map);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(item->getScheduleMap().size(), 1);
}

// slotSearchScheduleListFinish
TEST_F(TestAccountItem, SlotSearchScheduleListFinish)
{
    QSignalSpy spy(item.data(), &AccountItem::signalSearchScheduleUpdate);
    QMap<QDate, DSchedule::List> map;
    DSchedule::Ptr sched(new DSchedule());
    DSchedule::List list;
    list.append(sched);
    map[QDate(2025, 6, 15)] = list;
    item->slotSearchScheduleListFinish(map);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(item->getSearchScheduleMap().size(), 1);
}

// slotGetSysColorsFinish
TEST_F(TestAccountItem, SlotGetSysColorsFinish)
{
    DTypeColor::List list;
    DTypeColor::Ptr color(new DTypeColor());
    list.append(color);
    item->slotGetSysColorsFinish(list);
    EXPECT_EQ(item->getColorTypeList().size(), 1);
}

// slotAccountStateChange
TEST_F(TestAccountItem, SlotAccountStateChange)
{
    QSignalSpy spy(item.data(), &AccountItem::signalAccountStateChange);
    item->slotAccountStateChange(DAccount::Account_Open);
    EXPECT_EQ(spy.count(), 1);
}

// slotSyncStateChange
TEST_F(TestAccountItem, SlotSyncStateChange)
{
    QSignalSpy spy(item.data(), &AccountItem::signalSyncStateChange);
    item->slotSyncStateChange(DAccount::Sync_Normal);
    EXPECT_EQ(spy.count(), 1);
}

// signalCalDavScheduleCreateFailed lambda coverage (initConnect line 33)
TEST_F(TestAccountItem, InitConnectLambda_CalDavScheduleCreateFailed)
{
    QSignalSpy spy(item.data(), &AccountItem::signalCalDavScheduleCreateFailed);
    emit item->m_dbusRequest->signalCalDavScheduleCreateFailed(1);
    EXPECT_GE(spy.count(), 1);
    if (spy.count() > 0) {
        QList<QVariant> args = spy.takeFirst();
        EXPECT_EQ(args.at(1).toInt(), 1);
    }
}
