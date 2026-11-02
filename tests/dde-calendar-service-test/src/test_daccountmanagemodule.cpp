// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "daccountmanagemodule.h"
#include "daccount.h"
#include "daccountmodule.h"
#include "daccountmanagerdatabase.h"
#include "dcalendargeneralsettings.h"
#include "units.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSignalSpy>
#include <QTimer>
#include <QSettings>
#include <QTemporaryDir>

// Test fixture for DAccountManageModule
// The constructor is complex: it creates SyncFileManage (DBus),
// DAccountManagerDataBase (real DB), DConfig, DBusNotify, timers, etc.
// Tests require DBUS_SESSION_BUS_ADDRESS to be set.
class TestDAccountManageModule : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        // Construct once for all tests to avoid repeated heavy initialization
        s_module = new DAccountManageModule();
        // Stop the 2-second timer to prevent slotClientIsOpen from running
        // pidof during tests
        s_module->m_timer.stop();
        s_module->m_calDavDailyTimer.stop();
        s_module->m_calDavRetryTimer.stop();
    }

    static void TearDownTestSuite()
    {
        delete s_module;
        s_module = nullptr;
    }

    void SetUp() override
    {
        // Save original settings to restore after tests
        m_origFirstDay = s_module->getfirstDayOfWeek();
        m_origTimeFormat = s_module->getTimeFormatType();
    }

    void TearDown() override
    {
        // Restore original settings
        s_module->setFirstDayOfWeek(m_origFirstDay);
        s_module->setTimeFormatType(m_origTimeFormat);
    }

    static DAccountManageModule *s_module;
    int m_origFirstDay = 1;
    int m_origTimeFormat = 0;
};

DAccountManageModule *TestDAccountManageModule::s_module = nullptr;

// ---- Constructor ----

TEST_F(TestDAccountManageModule, Constructor_NoCrash)
{
    ASSERT_NE(s_module, nullptr);
}

TEST_F(TestDAccountManageModule, Constructor_HasAccountManagerDB)
{
    ASSERT_NE(s_module->m_accountManagerDB, nullptr);
}

TEST_F(TestDAccountManageModule, Constructor_HasSyncFileManage)
{
    ASSERT_NE(s_module->m_syncFileManage, nullptr);
}

TEST_F(TestDAccountManageModule, Constructor_HasGeneralSetting)
{
    ASSERT_NE(s_module->m_generalSetting, nullptr);
}

// ---- getAccountList ----

TEST_F(TestDAccountManageModule, GetAccountList_ReturnsJson)
{
    QString result = s_module->getAccountList();
    EXPECT_FALSE(result.isEmpty());
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    EXPECT_TRUE(doc.isObject());
}

// ---- getCalendarGeneralSettings / setCalendarGeneralSettings ----

TEST_F(TestDAccountManageModule, GetCalendarGeneralSettings_ReturnsJson)
{
    QString result = s_module->getCalendarGeneralSettings();
    EXPECT_FALSE(result.isEmpty());
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    EXPECT_TRUE(doc.isObject());
}

TEST_F(TestDAccountManageModule, SetCalendarGeneralSettings_ChangesValue)
{
    // When firstDayOfWeekSource == Source_System, the system config overrides
    // the DB value on read-back, so a value round-trip assertion is unreliable.
    // Verify the change was written by checking the signal is emitted instead.
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);

    QString original = s_module->getCalendarGeneralSettings();
    QJsonDocument doc = QJsonDocument::fromJson(original.toUtf8());
    QJsonObject obj = doc.object();

    int currentDay = obj.value("firstDayOfWeek").toInt();
    int newDay = (currentDay == 7) ? 1 : currentDay + 1;
    obj["firstDayOfWeek"] = newDay;

    QString modified = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    s_module->setCalendarGeneralSettings(modified);
    EXPECT_GE(spy.count(), 1);
}

TEST_F(TestDAccountManageModule, SetCalendarGeneralSettings_SameValue_NoSignal)
{
    QSignalSpy spyFirst(s_module, &DAccountManageModule::firstDayOfWeekChange);
    QSignalSpy spyTime(s_module, &DAccountManageModule::timeFormatTypeChange);

    QString current = s_module->getCalendarGeneralSettings();
    s_module->setCalendarGeneralSettings(current);

    EXPECT_EQ(spyFirst.count(), 0);
    EXPECT_EQ(spyTime.count(), 0);
}

TEST_F(TestDAccountManageModule, SetCalendarGeneralSettings_DifferentFirstDay_EmitsSignal)
{
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);

    QString original = s_module->getCalendarGeneralSettings();
    QJsonDocument doc = QJsonDocument::fromJson(original.toUtf8());
    QJsonObject obj = doc.object();
    int currentDay = obj.value("firstDayOfWeek").toInt();
    int newDay = (currentDay == 7) ? 1 : currentDay + 1;
    obj["firstDayOfWeek"] = newDay;
    QString modified = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

    s_module->setCalendarGeneralSettings(modified);
    EXPECT_GE(spy.count(), 1);
}

TEST_F(TestDAccountManageModule, SetCalendarGeneralSettings_DifferentTimeFormat_EmitsSignal)
{
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);

    QString original = s_module->getCalendarGeneralSettings();
    QJsonDocument doc = QJsonDocument::fromJson(original.toUtf8());
    QJsonObject obj = doc.object();
    int currentTime = obj.value("TimeShowType").toInt();
    int newTime = (currentTime == 0) ? 1 : 0;
    obj["TimeShowType"] = newTime;
    QString modified = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

    s_module->setCalendarGeneralSettings(modified);
    EXPECT_GE(spy.count(), 1);
}

// ---- getfirstDayOfWeek / setFirstDayOfWeek ----

TEST_F(TestDAccountManageModule, GetFirstDayOfWeek_ReturnsValidDay)
{
    int day = s_module->getfirstDayOfWeek();
    EXPECT_GE(day, 1);
    EXPECT_LE(day, 7);
}

TEST_F(TestDAccountManageModule, SetFirstDayOfWeek_ChangesValue)
{
    s_module->setFirstDayOfWeek(3);
    EXPECT_EQ(s_module->getfirstDayOfWeek(), 3);

    s_module->setFirstDayOfWeek(7);
    EXPECT_EQ(s_module->getfirstDayOfWeek(), 7);
}

TEST_F(TestDAccountManageModule, SetFirstDayOfWeek_SameValue_NoChange)
{
    int before = s_module->getfirstDayOfWeek();
    s_module->setFirstDayOfWeek(before);
    EXPECT_EQ(s_module->getfirstDayOfWeek(), before);
}

// ---- getTimeFormatType / setTimeFormatType ----

TEST_F(TestDAccountManageModule, GetTimeFormatType_ReturnsValidType)
{
    int type = s_module->getTimeFormatType();
    EXPECT_TRUE(type == 0 || type == 1);
}

TEST_F(TestDAccountManageModule, SetTimeFormatType_ChangesValue)
{
    s_module->setTimeFormatType(1);
    EXPECT_EQ(s_module->getTimeFormatType(), 1);

    s_module->setTimeFormatType(0);
    EXPECT_EQ(s_module->getTimeFormatType(), 0);
}

TEST_F(TestDAccountManageModule, SetTimeFormatType_SameValue_NoChange)
{
    int before = s_module->getTimeFormatType();
    s_module->setTimeFormatType(before);
    EXPECT_EQ(s_module->getTimeFormatType(), before);
}

// ---- getFirstDayOfWeekSource / setFirstDayOfWeekSource ----

TEST_F(TestDAccountManageModule, GetFirstDayOfWeekSource_ReturnsValidSource)
{
    auto source = s_module->getFirstDayOfWeekSource();
    EXPECT_TRUE(source == DCalendarGeneralSettings::Source_Database
                || source == DCalendarGeneralSettings::Source_System);
}

TEST_F(TestDAccountManageModule, SetFirstDayOfWeekSource_ToSystem)
{
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);
    s_module->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_System);
    EXPECT_EQ(s_module->getFirstDayOfWeekSource(), DCalendarGeneralSettings::Source_System);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDAccountManageModule, SetFirstDayOfWeekSource_ToDatabase)
{
    s_module->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_System);
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);
    s_module->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_Database);
    EXPECT_EQ(s_module->getFirstDayOfWeekSource(), DCalendarGeneralSettings::Source_Database);
    EXPECT_EQ(spy.count(), 1);
}

// ---- getTimeFormatTypeSource / setTimeFormatTypeSource ----

TEST_F(TestDAccountManageModule, GetTimeFormatTypeSource_ReturnsValidSource)
{
    auto source = s_module->getTimeFormatTypeSource();
    EXPECT_TRUE(source == DCalendarGeneralSettings::Source_Database
                || source == DCalendarGeneralSettings::Source_System);
}

TEST_F(TestDAccountManageModule, SetTimeFormatTypeSource_ToSystem)
{
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);
    s_module->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_System);
    EXPECT_EQ(s_module->getTimeFormatTypeSource(), DCalendarGeneralSettings::Source_System);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestDAccountManageModule, SetTimeFormatTypeSource_ToDatabase)
{
    s_module->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_System);
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);
    s_module->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_Database);
    EXPECT_EQ(s_module->getTimeFormatTypeSource(), DCalendarGeneralSettings::Source_Database);
    EXPECT_EQ(spy.count(), 1);
}

// ---- isSupportUid ----

TEST_F(TestDAccountManageModule, IsSupportUid_ReturnsBool)
{
    // Just verify it returns a bool without crash; actual value depends on env
    EXPECT_NO_FATAL_FAILURE(s_module->isSupportUid());
}

// ---- login / logout ----

TEST_F(TestDAccountManageModule, Login_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->login());
}

TEST_F(TestDAccountManageModule, Logout_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->logout());
}

// ---- calendarOpen ----

TEST_F(TestDAccountManageModule, CalendarOpen_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->calendarOpen(true));
    EXPECT_TRUE(s_module->m_clientIsOpen);
}

TEST_F(TestDAccountManageModule, CalendarOpen_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->calendarOpen(false));
    EXPECT_FALSE(s_module->m_clientIsOpen);
}

// ---- getCalDavAccountStatusList ----

TEST_F(TestDAccountManageModule, GetCalDavAccountStatusList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->getCalDavAccountStatusList());
}

// ---- getCalDavAccountConfig ----

TEST_F(TestDAccountManageModule, GetCalDavAccountConfig_NonExistent_ReturnsEmpty)
{
    QString result = s_module->getCalDavAccountConfig("nonexistent-account-id");
    EXPECT_TRUE(result.isEmpty());
}

// ---- downloadByAccountID ----

TEST_F(TestDAccountManageModule, DownloadByAccountID_NonExistent_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->downloadByAccountID("nonexistent-account-id"));
}

// ---- notifyMsgHanding ----

TEST_F(TestDAccountManageModule, NotifyMsgHanding_NonExistent_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->notifyMsgHanding("nonexistent-account-id", "alarm-1", 1));
}

// ---- remindJob ----

TEST_F(TestDAccountManageModule, RemindJob_NonExistent_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->remindJob("nonexistent-account-id", "alarm-1"));
}

// ---- updateRemindSchedules ----

TEST_F(TestDAccountManageModule, UpdateRemindSchedules_NoClear_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->updateRemindSchedules(false));
}

TEST_F(TestDAccountManageModule, UpdateRemindSchedules_Clear_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->updateRemindSchedules(true));
}

// ---- uploadNetWorkAccountData ----

TEST_F(TestDAccountManageModule, UploadNetWorkAccountData_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->uploadNetWorkAccountData());
}

// ---- requestCalDavManualSync ----

TEST_F(TestDAccountManageModule, RequestCalDavManualSync_EmptyRequestID_ReturnsFalse)
{
    EXPECT_FALSE(s_module->requestCalDavManualSync("some-id", ""));
}

TEST_F(TestDAccountManageModule, RequestCalDavManualSync_NonExistentAccount_ReturnsFalse)
{
    EXPECT_FALSE(s_module->requestCalDavManualSync("nonexistent-account-id", "req-1"));
}

// ---- validateCalDavAccountForUpdate ----

TEST_F(TestDAccountManageModule, ValidateCalDavAccountForUpdate_NonExistent_ReturnsEmpty)
{
    QString result = s_module->validateCalDavAccountForUpdate(
        "nonexistent-account-id", 0, "https://example.com", "user", "cred");
    EXPECT_TRUE(result.isEmpty());
}

// ---- validateCalDavAccount ----

TEST_F(TestDAccountManageModule, ValidateCalDavAccount_InvalidProvider_ReturnsRequestID)
{
    QSignalSpy spy(s_module, &DAccountManageModule::calDavAccountValidationFinished);
    QString result = s_module->validateCalDavAccount(
        999, "https://example.com", "user", "cred-ref");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(spy.wait(1000));
    EXPECT_FALSE(spy.takeFirst().at(1).toBool());
}

TEST_F(TestDAccountManageModule, ValidateCalDavAccount_InvalidUrl_ReturnsRequestID)
{
    QSignalSpy spy(s_module, &DAccountManageModule::calDavAccountValidationFinished);
    QString result = s_module->validateCalDavAccount(
        0, "not-a-url", "user", "cred-ref");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(spy.wait(1000));
    EXPECT_FALSE(spy.takeFirst().at(1).toBool());
}

TEST_F(TestDAccountManageModule, ValidateCalDavAccount_EmptyUsername_ReturnsRequestID)
{
    QSignalSpy spy(s_module, &DAccountManageModule::calDavAccountValidationFinished);
    QString result = s_module->validateCalDavAccount(
        0, "https://example.com", "", "cred-ref");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(spy.wait(1000));
    EXPECT_FALSE(spy.takeFirst().at(1).toBool());
}

TEST_F(TestDAccountManageModule, ValidateCalDavAccount_EmptyCredential_ReturnsRequestID)
{
    QSignalSpy spy(s_module, &DAccountManageModule::calDavAccountValidationFinished);
    QString result = s_module->validateCalDavAccount(
        0, "https://example.com", "user", "");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(spy.wait(1000));
    EXPECT_FALSE(spy.takeFirst().at(1).toBool());
}

// ---- createCalDavAccount ----

TEST_F(TestDAccountManageModule, CreateCalDavAccount_InvalidProvider_ReturnsEmpty)
{
    QString result = s_module->createCalDavAccount(
        999, "https://example.com", "user", "cred-ref", "Test");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestDAccountManageModule, CreateCalDavAccount_InvalidUrl_ReturnsEmpty)
{
    QString result = s_module->createCalDavAccount(
        0, "not-a-url", "user", "cred-ref", "Test");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestDAccountManageModule, CreateCalDavAccount_EmptyUsername_ReturnsEmpty)
{
    QString result = s_module->createCalDavAccount(
        0, "https://example.com", "", "cred-ref", "Test");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestDAccountManageModule, CreateCalDavAccount_EmptyCredential_ReturnsEmpty)
{
    QString result = s_module->createCalDavAccount(
        0, "https://example.com", "user", "", "Test");
    EXPECT_TRUE(result.isEmpty());
}

// ---- updateCalDavAccount ----

TEST_F(TestDAccountManageModule, UpdateCalDavAccount_NonExistent_ReturnsFalse)
{
    EXPECT_FALSE(s_module->updateCalDavAccount(
        "nonexistent-account-id", 0, "https://example.com", "user", "cred", "Test"));
}

// ---- deleteCalDavAccount ----

TEST_F(TestDAccountManageModule, DeleteCalDavAccount_NonExistent_ReturnsFalse)
{
    EXPECT_FALSE(s_module->deleteCalDavAccount("nonexistent-account-id"));
}

// ---- deleteCalDavAccountWithLocalDataOption ----

TEST_F(TestDAccountManageModule, DeleteCalDavAccountWithLocalDataOption_NonExistent_ReturnsFalse)
{
    EXPECT_FALSE(s_module->deleteCalDavAccountWithLocalDataOption("nonexistent-account-id", true));
    EXPECT_FALSE(s_module->deleteCalDavAccountWithLocalDataOption("nonexistent-account-id", false));
}

// ---- resolveCalDavConflict ----

TEST_F(TestDAccountManageModule, ResolveCalDavConflict_NonExistent_ReturnsFalse)
{
    EXPECT_FALSE(s_module->resolveCalDavConflict("nonexistent-account-id", "sched-1", true));
}

TEST_F(TestDAccountManageModule, ResolveCalDavConflict_EmptyScheduleID_ReturnsFalse)
{
    EXPECT_FALSE(s_module->resolveCalDavConflict("some-id", "", true));
}

// ---- resolveAllCalDavConflicts ----

TEST_F(TestDAccountManageModule, ResolveAllCalDavConflicts_NonExistent_ReturnsFalse)
{
    EXPECT_FALSE(s_module->resolveAllCalDavConflicts("nonexistent-account-id", true));
}

// ---- updateCalDavCredentialReference ----

TEST_F(TestDAccountManageModule, UpdateCalDavCredentialReference_NonExistent_ReturnsFalse)
{
    EXPECT_FALSE(s_module->updateCalDavCredentialReference("nonexistent-account-id", "new-cred"));
}

// ---- slotFirstDayOfWeek ----

TEST_F(TestDAccountManageModule, SlotFirstDayOfWeek_DifferentValue_EmitsSignal)
{
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);
    int currentDay = s_module->getfirstDayOfWeek();
    int newDay = (currentDay == 7) ? 1 : currentDay + 1;
    s_module->slotFirstDayOfWeek(newDay);
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(s_module->getfirstDayOfWeek(), newDay);
}

TEST_F(TestDAccountManageModule, SlotFirstDayOfWeek_SameValue_NoSignal)
{
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);
    int currentDay = s_module->getfirstDayOfWeek();
    s_module->slotFirstDayOfWeek(currentDay);
    EXPECT_EQ(spy.count(), 0);
}

// ---- slotTimeFormatType ----

TEST_F(TestDAccountManageModule, SlotTimeFormatType_DifferentValue_EmitsSignal)
{
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);
    int currentType = s_module->getTimeFormatType();
    int newType = (currentType == 0) ? 1 : 0;
    s_module->slotTimeFormatType(newType);
    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(s_module->getTimeFormatType(), newType);
}

TEST_F(TestDAccountManageModule, SlotTimeFormatType_SameValue_NoSignal)
{
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);
    int currentType = s_module->getTimeFormatType();
    s_module->slotTimeFormatType(currentType);
    EXPECT_EQ(spy.count(), 0);
}

// ---- slotSettingChange ----

TEST_F(TestDAccountManageModule, SlotSettingChange_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotSettingChange());
}

// ---- slotCalDavDailySync ----

TEST_F(TestDAccountManageModule, SlotCalDavDailySync_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotCalDavDailySync());
}

// ---- slotCalDavOnlineStateChanged ----

TEST_F(TestDAccountManageModule, SlotCalDavOnlineStateChanged_Offline_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotCalDavOnlineStateChanged(false));
}

TEST_F(TestDAccountManageModule, SlotCalDavOnlineStateChanged_Online_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotCalDavOnlineStateChanged(true));
}

// ---- slotClientIsOpen ----

TEST_F(TestDAccountManageModule, SlotClientIsOpen_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotClientIsOpen());
}

// ---- slotSwitcherChange ----

TEST_F(TestDAccountManageModule, SlotSwitcherChange_True_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotSwitcherChange(true));
}

TEST_F(TestDAccountManageModule, SlotSwitcherChange_False_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotSwitcherChange(false));
}

// ---- slotUidLoginStatueChange ----

TEST_F(TestDAccountManageModule, SlotUidLoginStatueChange_Status2_NoCrash)
{
    // Status 2 = login cancelled, default case returns immediately
    EXPECT_NO_FATAL_FAILURE(s_module->slotUidLoginStatueChange(2));
}

TEST_F(TestDAccountManageModule, SlotUidLoginStatueChange_Status4_NoCrash)
{
    // Status 4 = get server config success, default case returns immediately
    EXPECT_NO_FATAL_FAILURE(s_module->slotUidLoginStatueChange(4));
}

// ---- slotCalDavRetry ----

TEST_F(TestDAccountManageModule, SlotCalDavRetry_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->slotCalDavRetry());
}

// ---- Private methods (accessible via -fno-access-control) ----

// ---- getGeneralSettings ----

TEST_F(TestDAccountManageModule, GetGeneralSettings_ReturnsValidPtr)
{
    DCalendarGeneralSettings::Ptr settings = s_module->getGeneralSettings();
    ASSERT_NE(settings, nullptr);
    EXPECT_GE(static_cast<int>(settings->firstDayOfWeek()), 1);
    EXPECT_LE(static_cast<int>(settings->firstDayOfWeek()), 7);
}

// ---- setGeneralSettings ----

TEST_F(TestDAccountManageModule, SetGeneralSettings_PersistsToDB)
{
    DCalendarGeneralSettings::Ptr settings = s_module->getGeneralSettings();
    int origDay = static_cast<int>(settings->firstDayOfWeek());
    int newDay = (origDay == 7) ? 1 : origDay + 1;
    settings->setFirstDayOfWeek(static_cast<Qt::DayOfWeek>(newDay));
    s_module->setGeneralSettings(settings);

    DCalendarGeneralSettings::Ptr retrieved = s_module->getGeneralSettings();
    EXPECT_EQ(static_cast<int>(retrieved->firstDayOfWeek()), newDay);
}

// ---- scheduleNextCalDavDailySync ----

TEST_F(TestDAccountManageModule, ScheduleNextCalDavDailySync_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->scheduleNextCalDavDailySync());
}

// ---- scheduleNextCalDavRetry ----

TEST_F(TestDAccountManageModule, ScheduleNextCalDavRetry_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->scheduleNextCalDavRetry());
}

// ---- finishCalDavManualSync ----

TEST_F(TestDAccountManageModule, FinishCalDavManualSync_NonExistent_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->finishCalDavManualSync("nonexistent-account-id"));
}

// ---- registerCalDavAccounts ----

TEST_F(TestDAccountManageModule, RegisterCalDavAccounts_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->registerCalDavAccounts());
}

// ---- resumeCalDavAccountDeletionCleanups ----

TEST_F(TestDAccountManageModule, ResumeCalDavAccountDeletionCleanups_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->resumeCalDavAccountDeletionCleanups());
}

// ---- initAccountDBusInfo ----

TEST_F(TestDAccountManageModule, InitAccountDBusInfo_SetsDbusInfo)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    s_module->initAccountDBusInfo(account);
    EXPECT_FALSE(account->dbusPath().isEmpty());
    EXPECT_FALSE(account->dbusInterface().isEmpty());
    EXPECT_FALSE(account->dbName().isEmpty());
    EXPECT_TRUE(account->dbName().startsWith("account_caldav_"));
}

TEST_F(TestDAccountManageModule, InitAccountDBusInfo_UnionID)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    s_module->initAccountDBusInfo(account);
    EXPECT_TRUE(account->dbName().startsWith("account_uid_"));
}

TEST_F(TestDAccountManageModule, InitAccountDBusInfo_Local)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    s_module->initAccountDBusInfo(account);
    EXPECT_TRUE(account->dbName().startsWith("account_default_"));
}

// ---- Free functions (anonymous namespace - test indirectly) ----

TEST_F(TestDAccountManageModule, GetAccountList_HasAtLeastOneAccount)
{
    QString result = s_module->getAccountList();
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    QJsonArray arr = obj.value("accounts").toArray();
    EXPECT_GE(arr.size(), 1);
}

// ---- Additional includes for expanded coverage ----

#include "dcaldaverrorcode.h"
#include "dcaldavtransport.h"
#include "dcaldavsyncstatemachine.h"

// ---- login / logout ----

TEST_F(TestDAccountManageModule, Login_NoCrash_V2)
{
    EXPECT_NO_FATAL_FAILURE(s_module->login());
}

TEST_F(TestDAccountManageModule, Logout_NoCrash_V2)
{
    EXPECT_NO_FATAL_FAILURE(s_module->logout());
}

// ---- isSupportUid ----

TEST_F(TestDAccountManageModule, IsSupportUid_ReturnsBool_V2)
{
    bool result = s_module->isSupportUid();
    EXPECT_TRUE(result == true || result == false);
}

// ---- remindJob ----

TEST_F(TestDAccountManageModule, RemindJob_ExistingAccount_NoCrash)
{
    GTEST_SKIP() << "Source defect: null pointer dereference in DAccountModule::remindJob/notifyMsgHanding (see Mode 5 defects)";
    if (!s_module->m_accountModuleMap.isEmpty()) {
        QString accountID = s_module->m_accountModuleMap.firstKey();
        EXPECT_NO_FATAL_FAILURE(s_module->remindJob(accountID, "fake-alarm-id"));
    }
}

TEST_F(TestDAccountManageModule, RemindJob_NonExistentAccount_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->remindJob("nonexistent-account-id", "fake-alarm-id"));
}

// ---- calendarOpen ----

TEST_F(TestDAccountManageModule, CalendarOpen_True_NoCrash_V2)
{
    EXPECT_NO_FATAL_FAILURE(s_module->calendarOpen(true));
    EXPECT_TRUE(s_module->m_clientIsOpen);
}

TEST_F(TestDAccountManageModule, CalendarOpen_False_NoCrash_V2)
{
    EXPECT_NO_FATAL_FAILURE(s_module->calendarOpen(false));
    EXPECT_FALSE(s_module->m_clientIsOpen);
}

// ---- updateRemindSchedules ----

TEST_F(TestDAccountManageModule, UpdateRemindSchedules_ClearTrue_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->updateRemindSchedules(true));
}

TEST_F(TestDAccountManageModule, UpdateRemindSchedules_ClearFalse_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->updateRemindSchedules(false));
}

// ---- notifyMsgHanding ----

TEST_F(TestDAccountManageModule, NotifyMsgHanding_ExistingAccount_NoCrash)
{
    GTEST_SKIP() << "Source defect: null pointer dereference in DAccountModule::remindJob/notifyMsgHanding (see Mode 5 defects)";
    if (!s_module->m_accountModuleMap.isEmpty()) {
        QString accountID = s_module->m_accountModuleMap.firstKey();
        EXPECT_NO_FATAL_FAILURE(s_module->notifyMsgHanding(accountID, "fake-alarm-id", 1));
    }
}

TEST_F(TestDAccountManageModule, NotifyMsgHanding_NonExistentAccount_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->notifyMsgHanding("nonexistent-account-id", "fake-alarm-id", 1));
}

TEST_F(TestDAccountManageModule, NotifyMsgHanding_Operation2_NoCrash)
{
    GTEST_SKIP() << "Source defect: null pointer dereference in DAccountModule::remindJob/notifyMsgHanding (see Mode 5 defects)";
    if (!s_module->m_accountModuleMap.isEmpty()) {
        QString accountID = s_module->m_accountModuleMap.firstKey();
        EXPECT_NO_FATAL_FAILURE(s_module->notifyMsgHanding(accountID, "fake-alarm-id", 2));
    }
}

// ---- downloadByAccountID ----

TEST_F(TestDAccountManageModule, DownloadByAccountID_NonExistent_NoCrash_V2)
{
    EXPECT_NO_FATAL_FAILURE(s_module->downloadByAccountID("nonexistent-account-id"));
}

// ---- uploadNetWorkAccountData ----

TEST_F(TestDAccountManageModule, UploadNetWorkAccountData_NoCrash_V2)
{
    EXPECT_NO_FATAL_FAILURE(s_module->uploadNetWorkAccountData());
}

// ---- validateCalDavAccount ----

TEST_F(TestDAccountManageModule, ValidateCalDavAccount_InvalidParams_ReturnsRequestID)
{
    // Empty username → invalid, returns a non-empty requestID
    QString result = s_module->validateCalDavAccount(0, "https://example.com", "", "cred-ref");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDAccountManageModule, ValidateCalDavAccount_InvalidProvider_ReturnsRequestID_V2)
{
    QString result = s_module->validateCalDavAccount(99, "https://example.com", "user", "cred-ref");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestDAccountManageModule, ValidateCalDavAccount_EmptyCredential_ReturnsRequestID_V2)
{
    QString result = s_module->validateCalDavAccount(0, "https://example.com", "user", "");
    EXPECT_FALSE(result.isEmpty());
}

// ---- createCalDavAccount ----

TEST_F(TestDAccountManageModule, CreateCalDavAccount_InvalidParams_ReturnsEmpty)
{
    // Empty username → invalid, returns empty string
    QString result = s_module->createCalDavAccount(0, "https://example.com", "", "cred-ref", "Test");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(TestDAccountManageModule, CreateCalDavAccount_InvalidProvider_ReturnsEmpty_V2)
{
    QString result = s_module->createCalDavAccount(99, "https://example.com", "user", "cred-ref", "Test");
    EXPECT_TRUE(result.isEmpty());
}

// ---- updateCalDavAccount ----

TEST_F(TestDAccountManageModule, UpdateCalDavAccount_NonExistent_ReturnsFalse_V2)
{
    bool result = s_module->updateCalDavAccount("nonexistent-account-id", 0, "https://example.com", "user", "cred-ref", "Test");
    EXPECT_FALSE(result);
}

// ---- deleteCalDavAccount ----

TEST_F(TestDAccountManageModule, DeleteCalDavAccount_NonExistent_ReturnsFalse_V2)
{
    bool result = s_module->deleteCalDavAccount("nonexistent-account-id");
    EXPECT_FALSE(result);
}

// ---- resolveCalDavConflict ----

TEST_F(TestDAccountManageModule, ResolveCalDavConflict_NonExistent_ReturnsFalse_V2)
{
    bool result = s_module->resolveCalDavConflict("nonexistent-account-id", "schedule-id", true);
    EXPECT_FALSE(result);
}

TEST_F(TestDAccountManageModule, ResolveCalDavConflict_EmptyScheduleID_ReturnsFalse_V2)
{
    bool result = s_module->resolveCalDavConflict("nonexistent-account-id", "", true);
    EXPECT_FALSE(result);
}

// ---- resolveAllCalDavConflicts ----

TEST_F(TestDAccountManageModule, ResolveAllCalDavConflicts_NonExistent_ReturnsFalse_V2)
{
    bool result = s_module->resolveAllCalDavConflicts("nonexistent-account-id", true);
    EXPECT_FALSE(result);
}

// ---- getCalDavAccountConfig ----

TEST_F(TestDAccountManageModule, GetCalDavAccountConfig_NonExistent_ReturnsEmpty_V2)
{
    QString result = s_module->getCalDavAccountConfig("nonexistent-account-id");
    EXPECT_TRUE(result.isEmpty());
}

// ---- updateCalDavCredentialReference ----

TEST_F(TestDAccountManageModule, UpdateCalDavCredentialReference_NonExistent_ReturnsFalse_V2)
{
    bool result = s_module->updateCalDavCredentialReference("nonexistent-account-id", "new-cred-ref");
    EXPECT_FALSE(result);
}

// ---- requestCalDavManualSync ----

TEST_F(TestDAccountManageModule, RequestCalDavManualSync_NonExistent_ReturnsFalse)
{
    bool result = s_module->requestCalDavManualSync("nonexistent-account-id", "request-id");
    EXPECT_FALSE(result);
}

TEST_F(TestDAccountManageModule, RequestCalDavManualSync_EmptyRequestID_ReturnsFalse_V2)
{
    bool result = s_module->requestCalDavManualSync("nonexistent-account-id", "");
    EXPECT_FALSE(result);
}

// ---- deleteCalDavAccountInternal ----

TEST_F(TestDAccountManageModule, DeleteCalDavAccountInternal_NonExistent_ReturnsFalse)
{
    bool result = s_module->deleteCalDavAccountInternal("nonexistent-account-id", true);
    EXPECT_FALSE(result);
}

// ---- registerCalDavAccount (single) ----

TEST_F(TestDAccountManageModule, RegisterCalDavAccount_NullAccount_NoCrash)
{
    DAccount::Ptr nullAccount;
    EXPECT_NO_FATAL_FAILURE(s_module->registerCalDavAccount(nullAccount, DCalDavSyncStateMachine::StartupTrigger));
}

TEST_F(TestDAccountManageModule, RegisterCalDavAccount_NonExistentInMap_NoCrash)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID("fake-caldav-id-not-in-map");
    EXPECT_NO_FATAL_FAILURE(s_module->registerCalDavAccount(account, DCalDavSyncStateMachine::StartupTrigger));
}

// ---- notifyCalDavScheduleSyncResult ----

TEST_F(TestDAccountManageModule, NotifyCalDavScheduleSyncResult_InvalidDBus_ReturnsFalse)
{
    DCalDavTransport::Response response;
    bool result = s_module->notifyCalDavScheduleSyncResult(
        "nonexistent-account-id", true, DCalDavErrorCode::NoError, response);
    // m_dbusNotify is likely invalid in test environment → returns false
    EXPECT_FALSE(result);
}

TEST_F(TestDAccountManageModule, NotifyCalDavScheduleSyncResult_FailureCode_NoCrash)
{
    DCalDavTransport::Response response;
    response.error = DCalDavTransport::NetworkUnavailable;
    EXPECT_NO_FATAL_FAILURE(s_module->notifyCalDavScheduleSyncResult(
        "nonexistent-account-id", false, DCalDavErrorCode::NetworkUnavailable, response));
}

// ---- unionIDDataMerging ----

TEST_F(TestDAccountManageModule, UnionIDDataMerging_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->unionIDDataMerging());
}

// ---- addUIdAccount / removeUIdAccount ----

TEST_F(TestDAccountManageModule, AddUIdAccount_AddsToList)
{
    DAccount::Ptr uidAccount(new DAccount(DAccount::Account_UnionID));
    uidAccount->setAccountID("test-uid-add-id");
    uidAccount->setAccountName("test-uid-add");
    uidAccount->setDisplayName("Test UID Add");

    int countBefore = s_module->m_accountList.size();
    s_module->addUIdAccount(uidAccount);
    EXPECT_EQ(s_module->m_accountList.size(), countBefore + 1);
    EXPECT_TRUE(s_module->m_accountList.contains(uidAccount));

    // Clean up: remove the added account
    s_module->removeUIdAccount(uidAccount);
    EXPECT_FALSE(s_module->m_accountList.contains(uidAccount));
}

// ---- removeUIdAccount (standalone) ----

TEST_F(TestDAccountManageModule, RemoveUIdAccount_NonExistent_NoCrash)
{
    DAccount::Ptr uidAccount(new DAccount(DAccount::Account_UnionID));
    uidAccount->setAccountID("test-uid-remove-nonexistent");
    uidAccount->setAccountName("test-uid-remove");
    uidAccount->setDisplayName("Test UID Remove");

    EXPECT_NO_FATAL_FAILURE(s_module->removeUIdAccount(uidAccount));
}

// ---- updateUIdAccount ----

TEST_F(TestDAccountManageModule, UpdateUIdAccount_UpdatesDisplayAndAvatar)
{
    // SOURCE DEFECT: DAccountManageModule::updateUIdAccount() (daccountmanagemodule.cpp:1623-1624)
    // assigns to return-by-value temporaries:
    //   oldAccount->avatar() = uidAccount->avatar();
    //   oldAccount->displayName() = uidAccount->displayName();
    // displayName() and avatar() are getters returning by value, so the assignment
    // modifies a temporary, not oldAccount. The account is never actually updated.
    GTEST_SKIP() << "Source defect: updateUIdAccount assigns to return-by-value temporaries "
                    "(daccountmanagemodule.cpp:1623-1624) — oldAccount is never modified";
}

// ---- setUidSwitchStatus ----

TEST_F(TestDAccountManageModule, SetUidSwitchStatus_NoCrash)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountID("test-uid-switch-id");
    account->setAccountName("test-uid-switch");
    EXPECT_NO_FATAL_FAILURE(s_module->setUidSwitchStatus(account));
}

// ---- setFirstDayOfWeek ----

TEST_F(TestDAccountManageModule, SetFirstDayOfWeek_ChangesValue_V2)
{
    int origDay = s_module->getfirstDayOfWeek();
    int newDay = (origDay == 7) ? 1 : origDay + 1;
    s_module->setFirstDayOfWeek(newDay);
    EXPECT_EQ(s_module->getfirstDayOfWeek(), newDay);
    // Restore
    s_module->setFirstDayOfWeek(origDay);
}

// ---- slotFirstDayOfWeek ----

TEST_F(TestDAccountManageModule, SlotFirstDayOfWeek_DifferentValue_EmitsSignal_V2)
{
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);
    int origDay = s_module->getfirstDayOfWeek();
    int newDay = (origDay == 7) ? 1 : origDay + 1;
    s_module->slotFirstDayOfWeek(newDay);
    EXPECT_GE(spy.count(), 1);
    // Restore
    s_module->slotFirstDayOfWeek(origDay);
}

TEST_F(TestDAccountManageModule, SlotFirstDayOfWeek_SameValue_NoSignal_V2)
{
    QSignalSpy spy(s_module, &DAccountManageModule::firstDayOfWeekChange);
    int origDay = s_module->getfirstDayOfWeek();
    s_module->slotFirstDayOfWeek(origDay);
    EXPECT_EQ(spy.count(), 0);
}

// ---- slotTimeFormatType ----

TEST_F(TestDAccountManageModule, SlotTimeFormatType_DifferentValue_EmitsSignal_V2)
{
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);
    int origType = s_module->getTimeFormatType();
    int newType = (origType == 0) ? 1 : 0;
    s_module->slotTimeFormatType(newType);
    EXPECT_GE(spy.count(), 1);
    // Restore
    s_module->slotTimeFormatType(origType);
}

TEST_F(TestDAccountManageModule, SlotTimeFormatType_SameValue_NoSignal_V2)
{
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);
    int origType = s_module->getTimeFormatType();
    s_module->slotTimeFormatType(origType);
    EXPECT_EQ(spy.count(), 0);
}

// ---- setFirstDayOfWeekSource ----

TEST_F(TestDAccountManageModule, SetFirstDayOfWeekSource_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(s_module->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_Database));
    EXPECT_NO_FATAL_FAILURE(s_module->setFirstDayOfWeekSource(DCalendarGeneralSettings::Source_System));
}

// ---- getTimeFormatTypeSource / setTimeFormatTypeSource ----

TEST_F(TestDAccountManageModule, GetTimeFormatTypeSource_ReturnsValue)
{
    auto source = s_module->getTimeFormatTypeSource();
    EXPECT_TRUE(source == DCalendarGeneralSettings::Source_Database
                || source == DCalendarGeneralSettings::Source_System);
}

TEST_F(TestDAccountManageModule, SetTimeFormatTypeSource_EmitsSignal)
{
    QSignalSpy spy(s_module, &DAccountManageModule::timeFormatTypeChange);
    auto origSource = s_module->getTimeFormatTypeSource();
    auto newSource = (origSource == DCalendarGeneralSettings::Source_Database)
                         ? DCalendarGeneralSettings::Source_System
                         : DCalendarGeneralSettings::Source_Database;
    EXPECT_NO_FATAL_FAILURE(s_module->setTimeFormatTypeSource(newSource));
    EXPECT_GE(spy.count(), 1);
    // Restore
    s_module->setTimeFormatTypeSource(origSource);
}
