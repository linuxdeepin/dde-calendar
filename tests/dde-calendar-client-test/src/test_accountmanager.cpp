// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "../third-party_stub/stub.h"

#include "accountmanager.h"
#include "accountitem.h"
#include "dbus/dbusaccountmanagerrequest.h"
#include "dbus/dbusaccountrequest.h"
#include "dbus/dbusrequestbase.h"
#include "daccount.h"
#include "dcalendargeneralsettings.h"
#include "dcaldavaccountstatus.h"
#include "units.h"

#include <QTest>
#include <QSignalSpy>

// ---- Stub for isCommunityEdition (free function) ----
static bool stub_isCommunityEdition_false()
{
    return false;
}

static bool stub_isCommunityEdition_true()
{
    return true;
}

// ---- Stubs for DbusAccountManagerRequest methods ----
static void stub_void() {}
static void stub_void_bool(bool) {}
static void stub_void_int(int) {}
static void stub_void_str(const QString &) {}
static void stub_void_str_bool(const QString &, bool) {}
static void stub_void_str_str(const QString &, const QString &) {}
static void stub_void_callback(void*, CallbackFunc) {}
static void stub_void_int_str3(int, const QString &, const QString &, const QString &) {}
static void stub_void_str_int_str3(const QString &, int, const QString &, const QString &, const QString &) {}
static void stub_void_int_str4(int, const QString &, const QString &, const QString &, const QString &) {}
static void stub_void_str_int_str4(const QString &, int, const QString &, const QString &, const QString &, const QString &) {}
static void stub_void_source(DCalendarGeneralSettings::GeneralSettingSource) {}

static DCalendarGeneralSettings::GeneralSettingSource stub_source()
{
    return DCalendarGeneralSettings::Source_Database;
}

// ---- Helper to create a local account ----
static DAccount::Ptr makeLocalAccount()
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountID("local-001");
    account->setAccountName("Local account");
    account->setDbusPath("/com/deepin/dataserver/Calendar/Local");
    account->setDbusInterface("com.deepin.dataserver.Calendar.Local");
    return account;
}

static DAccount::Ptr makeUnionAccount()
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountID("union-001");
    account->setAccountName("UnionAccount");
    account->setDbusPath("/com/deepin/dataserver/Calendar/Union");
    account->setDbusInterface("com.deepin.dataserver.Calendar.Union");
    return account;
}

static DAccount::Ptr makeCalDavAccount()
{
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID("caldav-001");
    account->setAccountName("CalDavAccount");
    account->setDbusPath("/com/deepin/dataserver/Calendar/CalDav");
    account->setDbusInterface("com.deepin.dataserver.Calendar.CalDav");
    return account;
}

class TestAccountManager : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Stub isCommunityEdition and DbusAccountManagerRequest methods
        // before getInstance() to handle constructor calls safely
        stub.set(isCommunityEdition, stub_isCommunityEdition_false);
        stub.set(ADDR(DbusAccountManagerRequest, getCalDavAccountStatusList), stub_void);
        stub.set(ADDR(DbusAccountManagerRequest, getIsSupportUidAsync), stub_void);
        stub.set(ADDR(DbusAccountManagerRequest, getAccountList), stub_void);
        stub.set(ADDR(DbusAccountManagerRequest, getCalendarGeneralSettings), stub_void);
        stub.set(ADDR(DbusAccountManagerRequest, clientIsShow), stub_void_bool);
        stub.set(ADDR(DbusAccountManagerRequest, setFirstDayofWeek), stub_void_int);
        stub.set(ADDR(DbusAccountManagerRequest, setTimeFormatType), stub_void_int);
        stub.set(ADDR(DbusAccountManagerRequest, setFirstDayofWeekSource), stub_void_source);
        stub.set(ADDR(DbusAccountManagerRequest, setTimeFormatTypeSource), stub_void_source);
        stub.set(ADDR(DbusAccountManagerRequest, getFirstDayofWeekSource), stub_source);
        stub.set(ADDR(DbusAccountManagerRequest, getTimeFormatTypeSource), stub_source);
        stub.set(ADDR(DbusAccountManagerRequest, login), stub_void);
        stub.set(ADDR(DbusAccountManagerRequest, logout), stub_void);
        stub.set(ADDR(DbusAccountManagerRequest, downloadByAccountID), stub_void_str);
        stub.set(ADDR(DbusAccountManagerRequest, requestCalDavManualSync), stub_void_str_str);
        stub.set(ADDR(DbusAccountManagerRequest, setCallbackFunc), stub_void_callback);
        stub.set(ADDR(DbusAccountManagerRequest, uploadNetWorkAccountData), stub_void);
        stub.set(ADDR(DbusAccountManagerRequest, validateCalDavAccount), stub_void_int_str3);
        stub.set(ADDR(DbusAccountManagerRequest, validateCalDavAccountForUpdate), stub_void_str_int_str3);
        stub.set(ADDR(DbusAccountManagerRequest, createCalDavAccount), stub_void_int_str4);
        stub.set(ADDR(DbusAccountManagerRequest, updateCalDavAccount), stub_void_str_int_str4);
        stub.set(ADDR(DbusAccountManagerRequest, deleteCalDavAccountWithLocalDataOption), stub_void_str_bool);
        stub.set(ADDR(DbusAccountManagerRequest, resolveAllCalDavConflicts), stub_void_str_bool);
        stub.set(ADDR(DbusAccountManagerRequest, getCalDavAccountConfig), stub_void_str);

        // Also stub DbusAccountRequest methods that AccountItem uses
        stub.set(ADDR(DbusAccountRequest, querySchedulesWithParameter), stub_void);
        stub.set(ADDR(DbusAccountRequest, getScheduleTypeList), stub_void);
        stub.set(ADDR(DbusAccountRequest, getAccountInfo), stub_void);
        stub.set(ADDR(DbusAccountRequest, getSysColors), stub_void);

        mgr = AccountManager::getInstance();
    }

    void TearDown() override
    {
        // Clean up private state to avoid interference between tests
        mgr->m_localAccountItem.clear();
        mgr->m_unionAccountItem.clear();
        mgr->m_calDavAccountItems.clear();
        mgr->m_calDavAccountStatuses.clear();
        mgr->m_manualCalDavSyncAccounts.clear();
        mgr->m_pendingCalDavSyncAccounts.clear();
        mgr->m_manualUnionSyncAccounts.clear();
        mgr->m_pendingCalDavStatusChanges.clear();
        mgr->m_calDavStatusesInitialized = false;
        mgr->m_clientShowRequested = false;
        mgr->m_clientShowNotified = false;
        mgr->m_calDavProviderTypeOverrides.clear();
        mgr->m_pendingCalDavProviderType = -1;
        mgr->m_pendingCalDavDeleteAccountID.clear();
    }

    Stub stub;
    AccountManager *mgr = nullptr;
};

// ---- getInstance ----
TEST_F(TestAccountManager, GetInstance)
{
    EXPECT_NE(mgr, nullptr);
    EXPECT_EQ(mgr, AccountManager::getInstance());
}

// ---- calDavFailureReason (static) ----
TEST_F(TestAccountManager, CalDavFailureReason)
{
    DCalDavAccountStatus status;
    // NoError → default
    status.failureCode = static_cast<int>(DCalDavErrorCode::NoError);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::InvalidRequest);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::CertificateInvalid);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::AuthenticationFailed);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::UnsupportedCalDav);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::ParseError);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::PermissionDenied);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::RateLimited);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::ServerUnavailable);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::Conflict);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::ResponseTooLarge);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::RequestTimedOut);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::NetworkUnavailable);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::NetworkError);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    status.failureCode = static_cast<int>(DCalDavErrorCode::StorageError);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());

    // Unknown → default
    status.failureCode = static_cast<int>(DCalDavErrorCode::Unknown);
    EXPECT_FALSE(AccountManager::calDavFailureReason(status).isEmpty());
}

// ---- calDavSyncFailedText (static) ----
TEST_F(TestAccountManager, CalDavSyncFailedText)
{
    EXPECT_FALSE(AccountManager::calDavSyncFailedText().isEmpty());
}

// ---- calDavSyncUiState ----
TEST_F(TestAccountManager, CalDavSyncUiState)
{
    // Not pending, Idle → no flags
    auto state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::Idle);
    EXPECT_FALSE(state.pending);
    EXPECT_FALSE(state.running);
    EXPECT_FALSE(state.failed);

    // Not pending, Running → running
    state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::Running);
    EXPECT_FALSE(state.pending);
    EXPECT_TRUE(state.running);
    EXPECT_FALSE(state.failed);

    // Not pending, Failed → failed
    state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::Failed);
    EXPECT_FALSE(state.pending);
    EXPECT_FALSE(state.running);
    EXPECT_TRUE(state.failed);

    // Not pending, AuthenticationRequired → failed
    state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::AuthenticationRequired);
    EXPECT_TRUE(state.failed);

    // Not pending, PermissionDenied → failed
    state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::PermissionDenied);
    EXPECT_TRUE(state.failed);

    // Not pending, RetryScheduled → failed
    state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::RetryScheduled);
    EXPECT_TRUE(state.failed);

    // Not pending, Succeeded → no flags
    state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::Succeeded);
    EXPECT_FALSE(state.pending);
    EXPECT_FALSE(state.running);
    EXPECT_FALSE(state.failed);

    // Pending → pending and running, not failed even if Failed status
    mgr->m_pendingCalDavSyncAccounts.insert("acc1", "req1");
    state = mgr->calDavSyncUiState("acc1", DCalDavSyncStatus::Failed);
    EXPECT_TRUE(state.pending);
    EXPECT_TRUE(state.running);
    EXPECT_FALSE(state.failed);
    mgr->m_pendingCalDavSyncAccounts.remove("acc1");
}

// ---- canWriteCalDavAccount ----
TEST_F(TestAccountManager, CanWriteCalDavAccount)
{
    // Empty accountId → false
    EXPECT_FALSE(mgr->canWriteCalDavAccount("nonexistent"));

    // Non-empty but supportsWrite=true → true
    DCalDavAccountStatus status;
    status.accountId = "caldav-001";
    status.supportsWrite = true;
    mgr->m_calDavAccountStatuses.insert("caldav-001", status);
    EXPECT_TRUE(mgr->canWriteCalDavAccount("caldav-001"));

    // supportsWrite=false → false
    status.supportsWrite = false;
    mgr->m_calDavAccountStatuses["caldav-001"] = status;
    EXPECT_FALSE(mgr->canWriteCalDavAccount("caldav-001"));
}

// ---- takeManualCalDavSyncRequest ----
TEST_F(TestAccountManager, TakeManualCalDavSyncRequest)
{
    // Not completed (Idle) → false
    EXPECT_FALSE(mgr->takeManualCalDavSyncRequest("acc1", DCalDavSyncStatus::Idle));

    // Completed but no manual mark → false
    EXPECT_FALSE(mgr->takeManualCalDavSyncRequest("acc1", DCalDavSyncStatus::Succeeded));

    // Completed but pending → false
    mgr->m_manualCalDavSyncAccounts.insert("acc1");
    mgr->m_pendingCalDavSyncAccounts.insert("acc1", "req1");
    EXPECT_FALSE(mgr->takeManualCalDavSyncRequest("acc1", DCalDavSyncStatus::Succeeded));
    mgr->m_pendingCalDavSyncAccounts.remove("acc1");

    // Completed + manual + not pending → true, mark removed
    EXPECT_TRUE(mgr->takeManualCalDavSyncRequest("acc1", DCalDavSyncStatus::Succeeded));
    EXPECT_FALSE(mgr->m_manualCalDavSyncAccounts.contains("acc1"));

    // Failed status also completed
    mgr->m_manualCalDavSyncAccounts.insert("acc2");
    EXPECT_TRUE(mgr->takeManualCalDavSyncRequest("acc2", DCalDavSyncStatus::Failed));
    EXPECT_FALSE(mgr->m_manualCalDavSyncAccounts.contains("acc2"));

    // AuthenticationRequired status
    mgr->m_manualCalDavSyncAccounts.insert("acc3");
    EXPECT_TRUE(mgr->takeManualCalDavSyncRequest("acc3", DCalDavSyncStatus::AuthenticationRequired));

    // PermissionDenied status
    mgr->m_manualCalDavSyncAccounts.insert("acc4");
    EXPECT_TRUE(mgr->takeManualCalDavSyncRequest("acc4", DCalDavSyncStatus::PermissionDenied));

    // RetryScheduled status
    mgr->m_manualCalDavSyncAccounts.insert("acc5");
    EXPECT_TRUE(mgr->takeManualCalDavSyncRequest("acc5", DCalDavSyncStatus::RetryScheduled));
}

// ---- isCalDavSyncPending ----
TEST_F(TestAccountManager, IsCalDavSyncPending)
{
    EXPECT_FALSE(mgr->isCalDavSyncPending("acc1"));
    mgr->m_pendingCalDavSyncAccounts.insert("acc1", "req1");
    EXPECT_TRUE(mgr->isCalDavSyncPending("acc1"));
    mgr->m_pendingCalDavSyncAccounts.remove("acc1");
    EXPECT_FALSE(mgr->isCalDavSyncPending("acc1"));
}

// ---- getCalDavAccountStatus ----
TEST_F(TestAccountManager, GetCalDavAccountStatus)
{
    DCalDavAccountStatus status;
    status.accountId = "caldav-001";
    status.displayName = "TestCalDav";
    mgr->m_calDavAccountStatuses.insert("caldav-001", status);
    auto result = mgr->getCalDavAccountStatus("caldav-001");
    EXPECT_EQ(result.accountId, "caldav-001");
    EXPECT_EQ(result.displayName, "TestCalDav");

    // Non-existent → default-constructed
    auto result2 = mgr->getCalDavAccountStatus("nonexistent");
    EXPECT_TRUE(result2.accountId.isEmpty());
}

// ---- getIsSupportUid ----
TEST_F(TestAccountManager, GetIsSupportUid)
{
    mgr->m_isSupportUid = true;
    EXPECT_TRUE(mgr->getIsSupportUid());
    mgr->m_isSupportUid = false;
    EXPECT_FALSE(mgr->getIsSupportUid());
}

// ---- getGeneralSettings ----
TEST_F(TestAccountManager, GetGeneralSettings)
{
    EXPECT_EQ(mgr->getGeneralSettings(), nullptr);
    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    mgr->m_settings = settings;
    EXPECT_EQ(mgr->getGeneralSettings(), settings);
}

// ---- getAccountList / getLocalAccountItem / getUnionAccountItem ----
TEST_F(TestAccountManager, GetAccountList_Empty)
{
    EXPECT_TRUE(mgr->getAccountList().isEmpty());
    EXPECT_EQ(mgr->getLocalAccountItem(), nullptr);
    EXPECT_EQ(mgr->getUnionAccountItem(), nullptr);
}

// ---- resetAccount ----
TEST_F(TestAccountManager, ResetAccount)
{
    // Set some items, then reset
    mgr->m_localAccountItem = AccountItem::Ptr(new AccountItem(makeLocalAccount(), mgr));
    mgr->m_unionAccountItem = AccountItem::Ptr(new AccountItem(makeUnionAccount(), mgr));
    EXPECT_FALSE(mgr->getAccountList().isEmpty());

    mgr->resetAccount();
    EXPECT_EQ(mgr->getLocalAccountItem(), nullptr);
    EXPECT_EQ(mgr->getUnionAccountItem(), nullptr);
}

// ---- notifyClientIsShow / maybeNotifyClientIsShow ----
TEST_F(TestAccountManager, NotifyClientIsShow)
{
    // Before notify: maybeNotifyClientIsShow is a no-op (m_clientShowRequested=false)
    // After notify: calls clientIsShow(true) via stub
    QSignalSpy spy(mgr, &AccountManager::signalCalDavAccountStatusReady);

    // First set statuses initialized so maybeNotifyClientIsShow can be reached
    // via slotGetCalDavAccountStatusListFinish
    DCalDavAccountStatus::List list;
    mgr->slotGetCalDavAccountStatusListFinish(list);
    EXPECT_GE(spy.count(), 1);

    // Now call notifyClientIsShow — should trigger clientIsShow(true) via stub
    mgr->notifyClientIsShow();
    // m_clientShowNotified should be true now
    EXPECT_TRUE(mgr->m_clientShowNotified);

    // Calling again should not re-notify
    mgr->notifyClientIsShow();
}

// ---- slotGetGeneralSettingsFinish ----
TEST_F(TestAccountManager, SlotGetGeneralSettingsFinish)
{
    QSignalSpy spyInit(mgr, &AccountManager::signalDataInitFinished);
    QSignalSpy spyUpdate(mgr, &AccountManager::signalGeneralSettingsUpdate);

    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    mgr->slotGetGeneralSettingsFinish(settings);

    EXPECT_EQ(mgr->getGeneralSettings(), settings);
    EXPECT_EQ(spyInit.count(), 1);
    EXPECT_EQ(spyUpdate.count(), 1);
}

// ---- slotGetIsSupportUidFinish ----
TEST_F(TestAccountManager, SlotGetIsSupportUidFinish)
{
    // Without local account item
    mgr->slotGetIsSupportUidFinish(true);
    EXPECT_TRUE(mgr->m_isSupportUid);
    EXPECT_TRUE(mgr->m_isSupportUidLoaded);

    mgr->slotGetIsSupportUidFinish(false);
    EXPECT_FALSE(mgr->m_isSupportUid);

    // With local account item — should update name
    mgr->m_localAccountItem = AccountItem::Ptr(new AccountItem(makeLocalAccount(), mgr));
    mgr->m_localAccountItem->getAccount()->setAccountName("Event types");
    QSignalSpy spy(mgr, &AccountManager::signalAccountUpdate);
    mgr->slotGetIsSupportUidFinish(true);
    EXPECT_EQ(mgr->m_localAccountItem->getAccount()->accountName(), "Local account");
    EXPECT_GE(spy.count(), 1);

    // Change to false → name changes to "Event types"
    mgr->slotGetIsSupportUidFinish(false);
    EXPECT_EQ(mgr->m_localAccountItem->getAccount()->accountName(), "Event types");
}

// ---- slotGetCalDavAccountStatusListFinish ----
TEST_F(TestAccountManager, SlotGetCalDavAccountStatusListFinish_FirstInit)
{
    QSignalSpy spyReady(mgr, &AccountManager::signalCalDavAccountStatusReady);

    DCalDavAccountStatus::List list;
    DCalDavAccountStatus status;
    status.accountId = "caldav-001";
    status.displayName = "TestCalDav";
    status.syncStatus = DCalDavSyncStatus::Idle;
    list.append(status);

    mgr->slotGetCalDavAccountStatusListFinish(list);

    EXPECT_TRUE(mgr->m_calDavStatusesInitialized);
    EXPECT_EQ(mgr->m_calDavAccountStatuses.value("caldav-001").accountId, "caldav-001");
    EXPECT_GE(spyReady.count(), 1);
}

TEST_F(TestAccountManager, SlotGetCalDavAccountStatusListFinish_Subsequent)
{
    // First init
    DCalDavAccountStatus::List list1;
    DCalDavAccountStatus s1;
    s1.accountId = "caldav-001";
    s1.syncStatus = DCalDavSyncStatus::Idle;
    list1.append(s1);
    mgr->slotGetCalDavAccountStatusListFinish(list1);

    // Second call with changed status → should emit signalCalDavAccountStatusChanged
    QSignalSpy spyChanged(mgr, &AccountManager::signalCalDavAccountStatusChanged);
    QSignalSpy spyReady(mgr, &AccountManager::signalCalDavAccountStatusReady);

    DCalDavAccountStatus::List list2;
    DCalDavAccountStatus s2;
    s2.accountId = "caldav-001";
    s2.syncStatus = DCalDavSyncStatus::Failed; // changed
    list2.append(s2);
    mgr->slotGetCalDavAccountStatusListFinish(list2);

    EXPECT_GE(spyChanged.count(), 1);
    EXPECT_GE(spyReady.count(), 1);
}

TEST_F(TestAccountManager, SlotGetCalDavAccountStatusListFinish_WithProviderTypeOverride)
{
    // Set a provider type override
    mgr->m_calDavProviderTypeOverrides.insert("caldav-001", 5);

    DCalDavAccountStatus::List list;
    DCalDavAccountStatus s;
    s.accountId = "caldav-001";
    s.providerType = 0;
    list.append(s);
    mgr->slotGetCalDavAccountStatusListFinish(list);

    EXPECT_EQ(mgr->m_calDavAccountStatuses.value("caldav-001").providerType, 5);
}

TEST_F(TestAccountManager, SlotGetCalDavAccountStatusListFinish_PendingChanges)
{
    // Simulate a pending status change received before initialization
    mgr->m_pendingCalDavStatusChanges.insert("caldav-001");

    DCalDavAccountStatus::List list;
    DCalDavAccountStatus s;
    s.accountId = "caldav-001";
    list.append(s);

    QSignalSpy spyChanged(mgr, &AccountManager::signalCalDavAccountStatusChanged);
    mgr->slotGetCalDavAccountStatusListFinish(list);

    // Pending change should be flushed
    EXPECT_GE(spyChanged.count(), 1);
    EXPECT_TRUE(mgr->m_pendingCalDavStatusChanges.isEmpty());
}

// ---- slotGetAccountListFinish ----
TEST_F(TestAccountManager, SlotGetAccountListFinish_Empty)
{
    QSignalSpy spy(mgr, &AccountManager::signalAccountUpdate);
    DAccount::List list;
    mgr->slotGetAccountListFinish(list);
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(mgr->getLocalAccountItem(), nullptr);
    EXPECT_EQ(mgr->getUnionAccountItem(), nullptr);
}

TEST_F(TestAccountManager, SlotGetAccountListFinish_WithLocalAccount)
{
    DAccount::List list;
    list.append(makeLocalAccount());
    mgr->slotGetAccountListFinish(list);

    EXPECT_NE(mgr->getLocalAccountItem(), nullptr);
    EXPECT_EQ(mgr->getLocalAccountItem()->getAccount()->accountID(), "local-001");
}

TEST_F(TestAccountManager, SlotGetAccountListFinish_WithCalDavAccount)
{
    DAccount::List list;
    list.append(makeCalDavAccount());
    mgr->slotGetAccountListFinish(list);

    EXPECT_EQ(mgr->m_calDavAccountItems.size(), 1);
    EXPECT_EQ(mgr->m_calDavAccountItems[0]->getAccount()->accountID(), "caldav-001");
}

// ---- downloadByAccountID ----
TEST_F(TestAccountManager, DownloadByAccountID_UnionID)
{
    // Set up a union account item
    mgr->m_unionAccountItem = AccountItem::Ptr(new AccountItem(makeUnionAccount(), mgr));
    QSignalSpy spy(mgr, &AccountManager::signalSyncNum);

    mgr->downloadByAccountID("union-001");
    EXPECT_GE(spy.count(), 1);
    EXPECT_TRUE(mgr->m_manualUnionSyncAccounts.contains("union-001"));
}

TEST_F(TestAccountManager, DownloadByAccountID_CalDav)
{
    // Set up a CalDAV account item
    AccountItem::Ptr item(new AccountItem(makeCalDavAccount(), mgr));
    mgr->m_calDavAccountItems.append(item);

    QSignalSpy spy(mgr, &AccountManager::signalCalDavSyncPendingChanged);
    mgr->downloadByAccountID("caldav-001");
    EXPECT_GE(spy.count(), 1);
    EXPECT_TRUE(mgr->m_pendingCalDavSyncAccounts.contains("caldav-001"));
    EXPECT_TRUE(mgr->m_manualCalDavSyncAccounts.contains("caldav-001"));
}

TEST_F(TestAccountManager, DownloadByAccountID_CalDavAlreadyPending)
{
    // Set up a CalDAV account item
    AccountItem::Ptr item(new AccountItem(makeCalDavAccount(), mgr));
    mgr->m_calDavAccountItems.append(item);

    // Mark as already pending
    mgr->m_pendingCalDavSyncAccounts.insert("caldav-001", "existing-req");

    bool callbackCalled = false;
    CallbackFunc cb = [&callbackCalled](CallMessge) { callbackCalled = true; };
    mgr->downloadByAccountID("caldav-001", cb);
    EXPECT_TRUE(callbackCalled);
}

TEST_F(TestAccountManager, DownloadByAccountID_NonExistent)
{
    // Non-existent account → just calls dbus downloadByAccountID
    mgr->downloadByAccountID("nonexistent");
    // Should not crash
}

// ---- uploadNetWorkAccountData ----
TEST_F(TestAccountManager, UploadNetWorkAccountData)
{
    bool callbackCalled = false;
    CallbackFunc cb = [&callbackCalled](CallMessge) { callbackCalled = true; };
    mgr->uploadNetWorkAccountData(cb);
    // Should not crash; callback is set on dbus request
}

// ---- validateCalDavAccount ----
TEST_F(TestAccountManager, ValidateCalDavAccount)
{
    mgr->validateCalDavAccount(1, "http://server", "user", "cred");
    // Should not crash
}

// ---- validateCalDavAccountForUpdate ----
TEST_F(TestAccountManager, ValidateCalDavAccountForUpdate)
{
    mgr->validateCalDavAccountForUpdate("caldav-001", 1, "http://server", "user", "cred");
}

// ---- createCalDavAccount ----
TEST_F(TestAccountManager, CreateCalDavAccount)
{
    mgr->createCalDavAccount(1, "http://server", "user", "cred", "MyAccount");
    EXPECT_EQ(mgr->m_pendingCalDavProviderType, 1);
}

// ---- updateCalDavAccount ----
TEST_F(TestAccountManager, UpdateCalDavAccount)
{
    // Without existing status
    mgr->updateCalDavAccount("caldav-001", 2, "http://server", "user", "cred", "Updated");
    EXPECT_EQ(mgr->m_calDavProviderTypeOverrides.value("caldav-001"), 2);

    // With existing status → should emit signalCalDavAccountStatusChanged
    DCalDavAccountStatus status;
    status.accountId = "caldav-001";
    mgr->m_calDavAccountStatuses.insert("caldav-001", status);

    QSignalSpy spy(mgr, &AccountManager::signalCalDavAccountStatusChanged);
    mgr->updateCalDavAccount("caldav-001", 3, "http://server", "user", "cred", "Updated2");
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(mgr->m_calDavAccountStatuses.value("caldav-001").providerType, 3);
}

// ---- deleteCalDavAccountWithLocalDataOption ----
TEST_F(TestAccountManager, DeleteCalDavAccountWithLocalDataOption)
{
    mgr->deleteCalDavAccountWithLocalDataOption("caldav-001", true);
    EXPECT_EQ(mgr->m_pendingCalDavDeleteAccountID, "caldav-001");
}

// ---- resolveAllCalDavConflicts ----
TEST_F(TestAccountManager, ResolveAllCalDavConflicts)
{
    mgr->resolveAllCalDavConflicts("caldav-001", true);
    // Should not crash
}

// ---- getCalDavAccountConfig ----
TEST_F(TestAccountManager, GetCalDavAccountConfig)
{
    mgr->getCalDavAccountConfig("caldav-001");
    // Should not crash
}

// ---- setFirstDayofWeek ----
TEST_F(TestAccountManager, SetFirstDayofWeek)
{
    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    mgr->m_settings = settings;
    mgr->setFirstDayofWeek(static_cast<int>(Qt::Monday));
    // Should not crash
}

// ---- setTimeFormatType ----
TEST_F(TestAccountManager, SetTimeFormatType)
{
    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings());
    mgr->m_settings = settings;
    mgr->setTimeFormatType(static_cast<int>(DCalendarGeneralSettings::TwentyFour));
    // Should not crash
}

// ---- setFirstDayofWeekSource ----
TEST_F(TestAccountManager, SetFirstDayofWeekSource)
{
    mgr->setFirstDayofWeekSource(DCalendarGeneralSettings::Source_Database);
}

// ---- setTimeFormatTypeSource ----
TEST_F(TestAccountManager, SetTimeFormatTypeSource)
{
    mgr->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_System);
}

// ---- getFirstDayofWeekSource ----
TEST_F(TestAccountManager, GetFirstDayofWeekSource)
{
    auto result = mgr->getFirstDayofWeekSource();
    EXPECT_EQ(result, DCalendarGeneralSettings::Source_Database);
}

// ---- getTimeFormatTypeSource ----
TEST_F(TestAccountManager, GetTimeFormatTypeSource)
{
    auto result = mgr->getTimeFormatTypeSource();
    EXPECT_EQ(result, DCalendarGeneralSettings::Source_Database);
}

// ---- login ----
TEST_F(TestAccountManager, Login)
{
    mgr->login();
}

// ---- loginout ----
TEST_F(TestAccountManager, Loginout)
{
    mgr->loginout();
}

// ---- getScheduleTypeByScheduleTypeId ----
TEST_F(TestAccountManager, GetScheduleTypeByScheduleTypeId_NotFound)
{
    auto result = mgr->getScheduleTypeByScheduleTypeId("nonexistent");
    EXPECT_EQ(result, nullptr);
}

// ---- getAccountItemByScheduleTypeId ----
TEST_F(TestAccountManager, GetAccountItemByScheduleTypeId_NotFound)
{
    auto result = mgr->getAccountItemByScheduleTypeId("nonexistent");
    EXPECT_EQ(result, nullptr);
}

// ---- getAccountItemByAccountId ----
TEST_F(TestAccountManager, GetAccountItemByAccountId)
{
    // Empty list → not found
    EXPECT_EQ(mgr->getAccountItemByAccountId("local-001"), nullptr);

    // With local account
    mgr->m_localAccountItem = AccountItem::Ptr(new AccountItem(makeLocalAccount(), mgr));
    auto result = mgr->getAccountItemByAccountId("local-001");
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->getAccount()->accountID(), "local-001");

    // Not found
    EXPECT_EQ(mgr->getAccountItemByAccountId("nonexistent"), nullptr);
}

// ---- getAccountItemByAccountName ----
TEST_F(TestAccountManager, GetAccountItemByAccountName)
{
    // Empty list → not found
    EXPECT_EQ(mgr->getAccountItemByAccountName("LocalAccount"), nullptr);

    // With local account
    mgr->m_localAccountItem = AccountItem::Ptr(new AccountItem(makeLocalAccount(), mgr));
    auto result = mgr->getAccountItemByAccountName("Local account");
    EXPECT_NE(result, nullptr);

    // Not found
    EXPECT_EQ(mgr->getAccountItemByAccountName("nonexistent"), nullptr);
}

// ---- initConnect lambda coverage ----

// Callback capture stub for downloadByAccountID lambda
static CallbackFunc g_capturedCallback;
static void stub_setCallbackFunc_capture(void*, CallbackFunc func)
{
    g_capturedCallback = func;
}

// signalCalDavManualSyncStatusRefreshFinished → lambda (line 43)
TEST_F(TestAccountManager, InitConnectLambda_CalDavManualSyncStatusRefreshFinished)
{
    // Set up state so the lambda takes the statusChanged path
    mgr->m_pendingCalDavSyncAccounts.insert("caldav-001", "req-001");
    DCalDavAccountStatus status;
    status.accountId = "caldav-001";
    status.syncStatus = DCalDavSyncStatus::Idle;
    mgr->m_calDavAccountStatuses.insert("caldav-001", status);

    QSignalSpy spy(mgr, &AccountManager::signalCalDavAccountStatusChanged);
    emit mgr->m_dbusRequest->signalCalDavManualSyncStatusRefreshFinished("caldav-001", "req-001", true);
    EXPECT_GE(spy.count(), 1);
}

// signalCalDavAccountStatusRefreshRequested → lambda (line 60)
TEST_F(TestAccountManager, InitConnectLambda_CalDavAccountStatusRefreshRequested)
{
    mgr->m_calDavStatusesInitialized = false;
    emit mgr->m_dbusRequest->signalCalDavAccountStatusRefreshRequested("caldav-002");
    EXPECT_TRUE(mgr->m_pendingCalDavStatusChanges.contains("caldav-002"));
}

// signalValidateCalDavAccountStart → lambda (line 66)
TEST_F(TestAccountManager, InitConnectLambda_ValidateCalDavAccountStart)
{
    QSignalSpy spy(mgr, &AccountManager::signalCalDavAccountValidationStarted);
    emit mgr->m_dbusRequest->signalValidateCalDavAccountStart("req-001");
    EXPECT_GE(spy.count(), 1);
}

// signalValidateCalDavAccountForUpdateStart → lambda (line 75)
TEST_F(TestAccountManager, InitConnectLambda_ValidateCalDavAccountForUpdateStart)
{
    QSignalSpy spy(mgr, &AccountManager::signalCalDavAccountValidationForUpdateStarted);
    emit mgr->m_dbusRequest->signalValidateCalDavAccountForUpdateStart("req-002");
    EXPECT_GE(spy.count(), 1);
}

// signalCalDavAccountValidationFinished → lambda (line 88)
TEST_F(TestAccountManager, InitConnectLambda_CalDavAccountValidationFinished)
{
    QSignalSpy spy(mgr, &AccountManager::signalCalDavAccountValidationFinished);
    emit mgr->m_dbusRequest->signalCalDavAccountValidationFinished("req-003", true, 0, QString(), QString());
    EXPECT_GE(spy.count(), 1);
}

// signalCreateCalDavAccountFinish → lambda (line 103)
TEST_F(TestAccountManager, InitConnectLambda_CreateCalDavAccountFinish)
{
    mgr->m_pendingCalDavProviderType = 1;
    QSignalSpy spy(mgr, &AccountManager::signalCreateCalDavAccountFinish);
    emit mgr->m_dbusRequest->signalCreateCalDavAccountFinish("caldav-new");
    EXPECT_GE(spy.count(), 1);
    EXPECT_TRUE(mgr->m_calDavAccountStatuses.contains("caldav-new"));
    EXPECT_EQ(mgr->m_pendingCalDavProviderType, -1);
}

// signalDeleteCalDavAccountFinish → lambda (line 119)
TEST_F(TestAccountManager, InitConnectLambda_DeleteCalDavAccountFinish)
{
    mgr->m_pendingCalDavDeleteAccountID = "caldav-del";
    mgr->m_calDavAccountStatuses.insert("caldav-del", DCalDavAccountStatus());
    mgr->m_calDavProviderTypeOverrides.insert("caldav-del", 1);
    mgr->m_manualCalDavSyncAccounts.insert("caldav-del");

    QSignalSpy spy(mgr, &AccountManager::signalDeleteCalDavAccountFinish);
    emit mgr->m_dbusRequest->signalDeleteCalDavAccountFinish(true);
    EXPECT_GE(spy.count(), 1);
    EXPECT_FALSE(mgr->m_calDavAccountStatuses.contains("caldav-del"));
    EXPECT_TRUE(mgr->m_pendingCalDavDeleteAccountID.isEmpty());
}

// ---- D0 destructor coverage ----
TEST_F(TestAccountManager, DeletingDestructor)
{
    AccountManager *tempMgr = new AccountManager();
    EXPECT_NE(tempMgr, nullptr);
    // delete triggers D0 (deleting destructor)
    EXPECT_NO_FATAL_FAILURE(delete tempMgr);
}

// ---- downloadByAccountID lambda coverage ----
TEST_F(TestAccountManager, DownloadByAccountID_CalDavLambdaCallback)
{
    g_capturedCallback = nullptr;
    stub.reset(ADDR(DbusAccountManagerRequest, setCallbackFunc));
    stub.set(ADDR(DbusAccountManagerRequest, setCallbackFunc), stub_setCallbackFunc_capture);

    AccountItem::Ptr item(new AccountItem(makeCalDavAccount(), mgr));
    mgr->m_calDavAccountItems.append(item);

    bool originalCalled = false;
    CallbackFunc originalCb = [&originalCalled](CallMessge) { originalCalled = true; };

    mgr->downloadByAccountID("caldav-001", originalCb);

    ASSERT_TRUE((bool)g_capturedCallback);

    // Call the captured lambda with failure (code != 0)
    CallMessge msg;
    msg.code = 1;
    g_capturedCallback(msg);

    EXPECT_TRUE(originalCalled);
    EXPECT_FALSE(mgr->m_pendingCalDavSyncAccounts.contains("caldav-001"));
    EXPECT_FALSE(mgr->m_manualCalDavSyncAccounts.contains("caldav-001"));

    // Restore stub
    stub.reset(ADDR(DbusAccountManagerRequest, setCallbackFunc));
    stub.set(ADDR(DbusAccountManagerRequest, setCallbackFunc), stub_void_callback);
}

// ---- slotGetAccountListFinish lambda coverage ----
TEST_F(TestAccountManager, SlotGetAccountListFinish_UnionSyncStateChangeLambda)
{
    DAccount::List list;
    list.append(makeUnionAccount());
    mgr->slotGetAccountListFinish(list);

    ASSERT_NE(mgr->m_unionAccountItem, nullptr);

    // Test 1: Sync_Normal without manual request → returns early (no signalSyncNum)
    QSignalSpy spy1(mgr, &AccountManager::signalSyncNum);
    emit mgr->m_unionAccountItem->signalSyncStateChange(DAccount::Sync_Normal);
    EXPECT_EQ(spy1.count(), 0);

    // Test 2: Non-normal state → emits signalSyncNum
    QSignalSpy spy2(mgr, &AccountManager::signalSyncNum);
    emit mgr->m_unionAccountItem->signalSyncStateChange(DAccount::Sync_NetworkAnomaly);
    EXPECT_GE(spy2.count(), 1);

    // Test 3: Sync_Normal with manual request → emits signalSyncNum
    mgr->m_manualUnionSyncAccounts.insert("union-001");
    QSignalSpy spy3(mgr, &AccountManager::signalSyncNum);
    emit mgr->m_unionAccountItem->signalSyncStateChange(DAccount::Sync_Normal);
    EXPECT_GE(spy3.count(), 1);
}
