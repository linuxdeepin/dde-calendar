// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagerdatabase.h"
#include "daccount.h"
#include "ddatabase.h"
#include "dcaldavaccountinfo.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavcalendarinfo.h"
#include "dcaldavcategoryinfo.h"
#include "dcaldaveventmappinginfo.h"
#include "dcaldavoutboxitem.h"
#include "dcaldaverrorcode.h"
#include "dcalendargeneralsettings.h"
#include "units.h"

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>

class DAccountManagerDataBaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(dir->isValid());
        dbPath = dir->filePath("account-manager.db");
        localDbPath = dir->filePath("local.db");
        recreateDB();
    }

    void TearDown() override
    {
        db.close();
        QSqlDatabase::removeDatabase(db.connectionName());
        dir.reset();
    }

    void recreateDB()
    {
        db.close();
        QSqlDatabase::removeDatabase(db.connectionName());
        db = QSqlDatabase();
        database.setDBPath(dbPath);
        database.setLoaclDB(localDbPath);
        database.initDBData();
        db = QSqlDatabase::database(DDataBase::NameAccountManager);
    }

    DCalDavAccountInfo makeValidAccountInfo(const QString &id = QString())
    {
        DCalDavAccountInfo info;
        info.accountId = id.isEmpty() ? "caldav-acct-1" : id;
        info.providerType = 0;
        info.serverUrl = "https://caldav.example.com";
        info.username = "testuser";
        info.credentialRef = "secret-service:/test";
        info.accountColor = "#FF0000";
        return info;
    }

    DCalDavCalendarInfo makeValidCalendar(const QString &calId = "cal-1",
                                          const QString &acctId = "caldav-acct-1")
    {
        DCalDavCalendarInfo cal;
        cal.calendarId = calId;
        cal.accountId = acctId;
        cal.href = "/cal/" + calId;
        cal.displayName = "Test Calendar";
        cal.color = "#00FF00";
        cal.scheduleTypeID = "stype-" + calId;
        cal.syncToken = QStringLiteral("sync-token-123");
        cal.initialSyncCompleted = false;
        cal.privileges = 3;
        cal.privilegesKnown = true;
        cal.enabled = true;
        return cal;
    }

    DCalDavOutboxItem makeValidOutboxItem(const QString &opId = "op-1",
                                          const QString &acctId = "caldav-acct-1",
                                          const QString &schedId = "sched-1")
    {
        DCalDavOutboxItem item;
        item.operationID = opId;
        item.accountID = acctId;
        item.localScheduleID = schedId;
        item.operationType = DCalDavOutboxItem::CreateOperation;
        item.baseEtag = "etag-1";
        item.conflictIcs = "";
        item.serverIcs = "";
        item.retryCount = 0;
        item.failureType = DCalDavOutboxItem::NoFailure;
        return item;
    }

    DCalDavEventMappingInfo makeValidEventMapping(const QString &acctId = "caldav-acct-1",
                                                  const QString &calId = "cal-1")
    {
        DCalDavEventMappingInfo m;
        m.localScheduleID = "sched-1";
        m.accountID = acctId;
        m.calendarID = calId;
        m.uid = "uid-1";
        m.href = "/cal/1/event-1.ics";
        m.etag = "etag-1";
        m.originalIcs = "BEGIN:VCALENDAR\nEND:VCALENDAR";
        return m;
    }

    DCalDavCategoryInfo makeValidCategoryMapping(const QString &acctId = "caldav-acct-1",
                                                 const QString &calId = "cal-1")
    {
        DCalDavCategoryInfo m;
        m.accountId = acctId;
        m.calendarId = calId;
        m.categoryKey = "cat-key-1";
        m.scheduleTypeId = "stype-cal-1";
        return m;
    }

    DCalDavSkippedResource makeValidSkippedResource(const QString &calId = "cal-1")
    {
        DCalDavSkippedResource r;
        r.calendarId = calId;
        r.href = "/cal/1/skip-1.ics";
        r.etag = "etag-skip";
        r.reason = "Not Found";
        return r;
    }

    DAccountManagerDataBase database;
    std::unique_ptr<QTemporaryDir> dir;
    QString dbPath;
    QString localDbPath;
    QSqlDatabase db;
};

// ===== Constructor & Init =====

TEST_F(DAccountManagerDataBaseTest, Constructor)
{
    DAccountManagerDataBase db2;
    EXPECT_EQ(db2.getConnectionName(), QString(DDataBase::NameAccountManager));
}

TEST_F(DAccountManagerDataBaseTest, InitDBData)
{
    auto accounts = database.getAccountList();
    EXPECT_GE(accounts.size(), 1);
    EXPECT_EQ(accounts.first()->accountName(), QString("localAccount"));
}

TEST_F(DAccountManagerDataBaseTest, EnsureSchema)
{
    DAccountManagerDataBase db2;
    db2.setDBPath(dir->filePath("ensure-schema.db"));
    db2.setLoaclDB(localDbPath);
    db2.ensureSchema();
    auto accounts = db2.getAccountList();
    EXPECT_TRUE(accounts.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, SetLoaclDB)
{
    DAccountManagerDataBase db2;
    db2.setLoaclDB("/tmp/test-local.db");
    EXPECT_EQ(db2.getConnectionName(), QString(DDataBase::NameAccountManager));
}

// ===== Account CRUD =====

TEST_F(DAccountManagerDataBaseTest, GetAccountList)
{
    auto accounts = database.getAccountList();
    EXPECT_GE(accounts.size(), 1);
}

TEST_F(DAccountManagerDataBaseTest, GetAccountByID)
{
    auto accounts = database.getAccountList();
    ASSERT_FALSE(accounts.isEmpty());
    QString id = accounts.first()->accountID();
    auto account = database.getAccountByID(id);
    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->accountID(), id);
    EXPECT_EQ(account->accountName(), QString("localAccount"));
}

TEST_F(DAccountManagerDataBaseTest, GetAccountByID_NotFound)
{
    auto account = database.getAccountByID("nonexistent-id");
    EXPECT_EQ(account, nullptr);
}

TEST_F(DAccountManagerDataBaseTest, AddAccountInfo)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountName("NewAccount");
    account->setDisplayName("New Display");
    account->setAccountState(DAccount::Account_Open);
    account->setAccountType(DAccount::Account_Local);
    account->setDbName("new.db");
    account->setDbusPath("/new");
    account->setDbusInterface("new.iface");
    account->setIsExpandDisplay(true);
    account->setDtCreate(QDateTime::currentDateTime());

    QString id = database.addAccountInfo(account);
    EXPECT_FALSE(id.isEmpty());
    auto retrieved = database.getAccountByID(id);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->accountName(), QString("NewAccount"));
}

TEST_F(DAccountManagerDataBaseTest, AddAccountInfo_WithExistingID)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountID("preset-id-123");
    account->setAccountName("PresetAccount");
    account->setDisplayName("Preset");
    account->setAccountType(DAccount::Account_Local);
    account->setAccountState(DAccount::Account_Open);
    account->setDbName("preset.db");
    account->setDbusPath("/preset");
    account->setDbusInterface("preset.iface");
    account->setDtCreate(QDateTime::currentDateTime());

    QString id = database.addAccountInfo(account);
    EXPECT_EQ(id, QString("preset-id-123"));
}

TEST_F(DAccountManagerDataBaseTest, UpdateAccountInfo)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountName("ToUpdate");
    account->setDisplayName("Old");
    account->setAccountType(DAccount::Account_Local);
    account->setAccountState(DAccount::Account_Open);
    account->setDbName("update.db");
    account->setDbusPath("/update");
    account->setDbusInterface("update.iface");
    account->setDtCreate(QDateTime::currentDateTime());
    QString id = database.addAccountInfo(account);

    account->setAccountName("Updated");
    account->setDisplayName("New Display");
    bool res = database.updateAccountInfo(account);
    EXPECT_TRUE(res);
    auto retrieved = database.getAccountByID(id);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->accountName(), QString("Updated"));
    EXPECT_EQ(retrieved->displayName(), QString("New Display"));
}

TEST_F(DAccountManagerDataBaseTest, DeleteAccountInfo)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setAccountName("ToDelete");
    account->setAccountType(DAccount::Account_Local);
    account->setDtCreate(QDateTime::currentDateTime());
    QString id = database.addAccountInfo(account);

    bool res = database.deleteAccountInfo(id);
    EXPECT_TRUE(res);
    EXPECT_EQ(database.getAccountByID(id), nullptr);
}

TEST_F(DAccountManagerDataBaseTest, DeleteAccountInfo_NotFound)
{
    bool res = database.deleteAccountInfo("nonexistent");
    EXPECT_TRUE(res);
}

// ===== CalDAV Account =====

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_Insert)
{
    auto info = makeValidAccountInfo();
    EXPECT_TRUE(database.upsertCalDavAccountInfo(info));
    DCalDavAccountInfo retrieved;
    EXPECT_TRUE(database.getCalDavAccountInfo("caldav-acct-1", retrieved));
    EXPECT_EQ(retrieved.username, QString("testuser"));
    EXPECT_EQ(retrieved.serverUrl, QString("https://caldav.example.com"));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_Update)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    info.username = "updated-user";
    info.accountColor = "#0000FF";
    EXPECT_TRUE(database.upsertCalDavAccountInfo(info));
    DCalDavAccountInfo retrieved;
    ASSERT_TRUE(database.getCalDavAccountInfo(info.accountId, retrieved));
    EXPECT_EQ(retrieved.username, QString("updated-user"));
    EXPECT_EQ(retrieved.accountColor, QString("#0000FF"));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_EmptyAccountId)
{
    auto info = makeValidAccountInfo();
    info.accountId = "";
    EXPECT_FALSE(database.upsertCalDavAccountInfo(info));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_EmptyUsername)
{
    auto info = makeValidAccountInfo();
    info.username = "";
    EXPECT_FALSE(database.upsertCalDavAccountInfo(info));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_EmptyCredentialRef)
{
    auto info = makeValidAccountInfo();
    info.credentialRef = "";
    EXPECT_FALSE(database.upsertCalDavAccountInfo(info));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_InvalidServerUrl)
{
    auto info = makeValidAccountInfo();
    info.serverUrl = "http://not-https.com";
    EXPECT_FALSE(database.upsertCalDavAccountInfo(info));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_EmptyHost)
{
    auto info = makeValidAccountInfo();
    info.serverUrl = "https://";
    EXPECT_FALSE(database.upsertCalDavAccountInfo(info));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountInfo_InvalidCredentialRef)
{
    auto info = makeValidAccountInfo();
    info.credentialRef = "invalid-ref";
    EXPECT_FALSE(database.upsertCalDavAccountInfo(info));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavAccountInfo_EmptyID)
{
    DCalDavAccountInfo info;
    EXPECT_FALSE(database.getCalDavAccountInfo("", info));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavAccountInfo_NotFound)
{
    DCalDavAccountInfo info;
    EXPECT_FALSE(database.getCalDavAccountInfo("nonexistent", info));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavAccountStatusList_Empty)
{
    QString json = database.getCalDavAccountStatusList();
    EXPECT_FALSE(json.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavAccountStatusList_WithAccount)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    QString json = database.getCalDavAccountStatusList();
    EXPECT_FALSE(json.isEmpty());
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    EXPECT_TRUE(doc.isArray());
    EXPECT_GE(doc.array().size(), 1);
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavAccountInfo)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    EXPECT_TRUE(database.deleteCalDavAccountInfo(info.accountId));
    DCalDavAccountInfo retrieved;
    EXPECT_FALSE(database.getCalDavAccountInfo(info.accountId, retrieved));
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavAccountInfo_EmptyID)
{
    EXPECT_FALSE(database.deleteCalDavAccountInfo(""));
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavAccountInfo_NotFound)
{
    EXPECT_FALSE(database.deleteCalDavAccountInfo("nonexistent"));
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavAccountInfo_RemovesPendingNotification)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    ASSERT_TRUE(database.addPendingCalDavScheduleSyncNotification(info.accountId));
    EXPECT_TRUE(database.hasPendingCalDavScheduleSyncNotification(info.accountId));
    EXPECT_TRUE(database.deleteCalDavAccountInfo(info.accountId));
    EXPECT_FALSE(database.hasPendingCalDavScheduleSyncNotification(info.accountId));
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavAccountData)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    auto cal = makeValidCalendar();
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    auto mapping = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    EXPECT_TRUE(database.deleteCalDavAccountData(info.accountId));
    DCalDavAccountInfo retrieved;
    EXPECT_FALSE(database.getCalDavAccountInfo(info.accountId, retrieved));
    EXPECT_TRUE(database.getCalDavCalendarList(info.accountId).isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavAccountData_EmptyID)
{
    EXPECT_FALSE(database.deleteCalDavAccountData(""));
}

// ===== Schedule Sync Notification =====

TEST_F(DAccountManagerDataBaseTest, HasPendingCalDavScheduleSyncNotification_Empty)
{
    EXPECT_FALSE(database.hasPendingCalDavScheduleSyncNotification(""));
    EXPECT_FALSE(database.hasPendingCalDavScheduleSyncNotification("no-such-account"));
}

TEST_F(DAccountManagerDataBaseTest, AddPendingCalDavScheduleSyncNotification)
{
    QString acct = "notif-acct";
    EXPECT_FALSE(database.addPendingCalDavScheduleSyncNotification(""));
    EXPECT_TRUE(database.addPendingCalDavScheduleSyncNotification(acct));
    EXPECT_TRUE(database.hasPendingCalDavScheduleSyncNotification(acct));
    EXPECT_TRUE(database.addPendingCalDavScheduleSyncNotification(acct));
}

TEST_F(DAccountManagerDataBaseTest, RemovePendingCalDavScheduleSyncNotification)
{
    QString acct = "notif-acct-2";
    ASSERT_TRUE(database.addPendingCalDavScheduleSyncNotification(acct));
    EXPECT_FALSE(database.removePendingCalDavScheduleSyncNotification(""));
    EXPECT_TRUE(database.removePendingCalDavScheduleSyncNotification(acct));
    EXPECT_FALSE(database.hasPendingCalDavScheduleSyncNotification(acct));
}

TEST_F(DAccountManagerDataBaseTest, RemovePendingCalDavScheduleSyncNotificationIfNoCreates)
{
    QString acct = "notif-acct-3";
    ASSERT_TRUE(database.addPendingCalDavScheduleSyncNotification(acct));
    EXPECT_FALSE(database.removePendingCalDavScheduleSyncNotificationIfNoCreates(""));
    EXPECT_TRUE(database.removePendingCalDavScheduleSyncNotificationIfNoCreates(acct));
    EXPECT_FALSE(database.hasPendingCalDavScheduleSyncNotification(acct));
}

TEST_F(DAccountManagerDataBaseTest, RemovePendingNotificationIfNoCreates_KeepsWhenCreatesExist)
{
    QString acct = "notif-acct-4";
    ASSERT_TRUE(database.addPendingCalDavScheduleSyncNotification(acct));
    auto item = makeValidOutboxItem("op-create", acct, "sched-x");
    item.operationType = DCalDavOutboxItem::CreateOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    EXPECT_TRUE(database.removePendingCalDavScheduleSyncNotificationIfNoCreates(acct));
    EXPECT_TRUE(database.hasPendingCalDavScheduleSyncNotification(acct));
}

// ===== Account Deletion Cleanup =====

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavAccountDeletionCleanup)
{
    EXPECT_FALSE(database.upsertCalDavAccountDeletionCleanup("", "db1"));
    EXPECT_FALSE(database.upsertCalDavAccountDeletionCleanup("acct1", ""));
    EXPECT_TRUE(database.upsertCalDavAccountDeletionCleanup("acct1", "db1"));
    auto cleanups = database.getCalDavAccountDeletionCleanups();
    EXPECT_EQ(cleanups.value("acct1"), QString("db1"));
    EXPECT_TRUE(database.upsertCalDavAccountDeletionCleanup("acct1", "db2"));
    auto cleanups2 = database.getCalDavAccountDeletionCleanups();
    EXPECT_EQ(cleanups2.value("acct1"), QString("db2"));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavAccountDeletionCleanups_Empty)
{
    auto cleanups = database.getCalDavAccountDeletionCleanups();
    EXPECT_TRUE(cleanups.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavAccountDeletionCleanup)
{
    EXPECT_FALSE(database.deleteCalDavAccountDeletionCleanup(""));
    ASSERT_TRUE(database.upsertCalDavAccountDeletionCleanup("acct1", "db1"));
    EXPECT_TRUE(database.deleteCalDavAccountDeletionCleanup("acct1"));
    EXPECT_TRUE(database.getCalDavAccountDeletionCleanups().isEmpty());
}

// ===== Category Mapping =====

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavCategoryMapping)
{
    auto mapping = makeValidCategoryMapping();
    EXPECT_TRUE(database.upsertCalDavCategoryMapping(mapping));
    DCalDavCategoryInfo empty;
    empty.accountId = "";
    EXPECT_FALSE(database.upsertCalDavCategoryMapping(empty));
    empty.accountId = "a";
    empty.calendarId = "";
    EXPECT_FALSE(database.upsertCalDavCategoryMapping(empty));
    empty.calendarId = "c";
    empty.categoryKey = "";
    EXPECT_FALSE(database.upsertCalDavCategoryMapping(empty));
    empty.categoryKey = "k";
    empty.scheduleTypeId = "";
    EXPECT_FALSE(database.upsertCalDavCategoryMapping(empty));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMapping)
{
    auto mapping = makeValidCategoryMapping();
    ASSERT_TRUE(database.upsertCalDavCategoryMapping(mapping));
    auto result = database.getCalDavCategoryMapping(mapping.accountId, mapping.calendarId, mapping.categoryKey);
    EXPECT_EQ(result.accountId, mapping.accountId);
    EXPECT_EQ(result.calendarId, mapping.calendarId);
    EXPECT_EQ(result.categoryKey, mapping.categoryKey);
    EXPECT_EQ(result.scheduleTypeId, mapping.scheduleTypeId);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMapping_EmptyArgs)
{
    auto result = database.getCalDavCategoryMapping("", "cal-1", "cat-key-1");
    EXPECT_TRUE(result.accountId.isEmpty());
    result = database.getCalDavCategoryMapping("acct", "", "cat-key-1");
    EXPECT_TRUE(result.accountId.isEmpty());
    result = database.getCalDavCategoryMapping("acct", "cal-1", "");
    EXPECT_TRUE(result.accountId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMapping_NotFound)
{
    auto result = database.getCalDavCategoryMapping("noacct", "nocal", "nokey");
    EXPECT_TRUE(result.accountId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMappings)
{
    auto m1 = makeValidCategoryMapping();
    ASSERT_TRUE(database.upsertCalDavCategoryMapping(m1));
    auto m2 = makeValidCategoryMapping();
    m2.categoryKey = "cat-key-2";
    m2.scheduleTypeId = "stype-2";
    ASSERT_TRUE(database.upsertCalDavCategoryMapping(m2));
    auto list = database.getCalDavCategoryMappings(m1.accountId, m1.calendarId);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMappings_EmptyArgs)
{
    auto list = database.getCalDavCategoryMappings("", "cal-1");
    EXPECT_TRUE(list.isEmpty());
    list = database.getCalDavCategoryMappings("acct", "");
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMappingByScheduleTypeID)
{
    auto mapping = makeValidCategoryMapping();
    ASSERT_TRUE(database.upsertCalDavCategoryMapping(mapping));
    auto result = database.getCalDavCategoryMappingByScheduleTypeID(mapping.accountId, mapping.scheduleTypeId);
    EXPECT_EQ(result.categoryKey, mapping.categoryKey);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMappingByScheduleTypeID_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavCategoryMappingByScheduleTypeID("", "stype").accountId.isEmpty());
    EXPECT_TRUE(database.getCalDavCategoryMappingByScheduleTypeID("acct", "").accountId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCategoryMappingByScheduleTypeID_NotFound)
{
    EXPECT_TRUE(database.getCalDavCategoryMappingByScheduleTypeID("noacct", "nostype").accountId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavCategoryMappingByScheduleTypeID)
{
    auto mapping = makeValidCategoryMapping();
    ASSERT_TRUE(database.upsertCalDavCategoryMapping(mapping));
    EXPECT_FALSE(database.deleteCalDavCategoryMappingByScheduleTypeID("", "stype"));
    EXPECT_FALSE(database.deleteCalDavCategoryMappingByScheduleTypeID("acct", ""));
    EXPECT_TRUE(database.deleteCalDavCategoryMappingByScheduleTypeID(mapping.accountId, mapping.scheduleTypeId));
    auto result = database.getCalDavCategoryMappingByScheduleTypeID(mapping.accountId, mapping.scheduleTypeId);
    EXPECT_TRUE(result.accountId.isEmpty());
}

// ===== Credential Reference =====

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavCredentialReference)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    EXPECT_FALSE(database.updateCalDavCredentialReference("", "secret-service:/new"));
    EXPECT_FALSE(database.updateCalDavCredentialReference(info.accountId, "invalid-ref"));
    EXPECT_TRUE(database.updateCalDavCredentialReference(info.accountId, "secret-service:/new"));
    DCalDavAccountInfo retrieved;
    ASSERT_TRUE(database.getCalDavAccountInfo(info.accountId, retrieved));
    EXPECT_EQ(retrieved.credentialRef, QString("secret-service:/new"));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavCredentialReference_EmptyRef)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    EXPECT_TRUE(database.updateCalDavCredentialReference(info.accountId, ""));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavCredentialReference_NotFound)
{
    EXPECT_FALSE(database.updateCalDavCredentialReference("nonexistent", "secret-service:/test"));
}

// ===== Calendar CRUD =====

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavCalendar_Insert)
{
    auto cal = makeValidCalendar();
    EXPECT_TRUE(database.upsertCalDavCalendar(cal));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavCalendar_Update)
{
    auto cal = makeValidCalendar();
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    cal.displayName = "Updated Calendar";
    EXPECT_TRUE(database.upsertCalDavCalendar(cal));
    auto list = database.getCalDavCalendarList(cal.accountId);
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.first().displayName, QString("Updated Calendar"));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavCalendar_EmptyFields)
{
    DCalDavCalendarInfo cal;
    EXPECT_FALSE(database.upsertCalDavCalendar(cal));
    cal.calendarId = "c";
    EXPECT_FALSE(database.upsertCalDavCalendar(cal));
    cal.accountId = "a";
    EXPECT_FALSE(database.upsertCalDavCalendar(cal));
    cal.href = "/h";
    EXPECT_TRUE(database.upsertCalDavCalendar(cal));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarList)
{
    auto cal1 = makeValidCalendar("cal-1");
    ASSERT_TRUE(database.upsertCalDavCalendar(cal1));
    auto cal2 = makeValidCalendar("cal-2");
    ASSERT_TRUE(database.upsertCalDavCalendar(cal2));
    auto list = database.getCalDavCalendarList(cal1.accountId);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarList_EmptyID)
{
    auto list = database.getCalDavCalendarList("");
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarList_NotFound)
{
    auto list = database.getCalDavCalendarList("noacct");
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByScheduleTypeID)
{
    auto cal = makeValidCalendar();
    cal.enabled = true;
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    auto result = database.getCalDavCalendarByScheduleTypeID(cal.accountId, cal.scheduleTypeID);
    EXPECT_EQ(result.calendarId, cal.calendarId);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByScheduleTypeID_Disabled)
{
    auto cal = makeValidCalendar();
    cal.enabled = false;
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    auto result = database.getCalDavCalendarByScheduleTypeID(cal.accountId, cal.scheduleTypeID);
    EXPECT_TRUE(result.calendarId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByScheduleTypeID_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavCalendarByScheduleTypeID("", "stype").calendarId.isEmpty());
    EXPECT_TRUE(database.getCalDavCalendarByScheduleTypeID("acct", "").calendarId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByScheduleTypeID_NotFound)
{
    EXPECT_TRUE(database.getCalDavCalendarByScheduleTypeID("noacct", "nostype").calendarId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByScheduleTypeIDIncludingDisabled)
{
    auto cal = makeValidCalendar();
    cal.enabled = false;
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    auto result = database.getCalDavCalendarByScheduleTypeIDIncludingDisabled(cal.accountId, cal.scheduleTypeID);
    EXPECT_EQ(result.calendarId, cal.calendarId);
    EXPECT_FALSE(result.enabled);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByScheduleTypeIDIncludingDisabled_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavCalendarByScheduleTypeIDIncludingDisabled("", "stype").calendarId.isEmpty());
    EXPECT_TRUE(database.getCalDavCalendarByScheduleTypeIDIncludingDisabled("acct", "").calendarId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByScheduleTypeIDIncludingDisabled_NotFound)
{
    EXPECT_TRUE(database.getCalDavCalendarByScheduleTypeIDIncludingDisabled("noacct", "nostype").calendarId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByIDIncludingDisabled)
{
    auto cal = makeValidCalendar();
    cal.enabled = false;
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    auto result = database.getCalDavCalendarByIDIncludingDisabled(cal.accountId, cal.calendarId);
    EXPECT_EQ(result.calendarId, cal.calendarId);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByIDIncludingDisabled_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavCalendarByIDIncludingDisabled("", "cal-1").calendarId.isEmpty());
    EXPECT_TRUE(database.getCalDavCalendarByIDIncludingDisabled("acct", "").calendarId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavCalendarByIDIncludingDisabled_NotFound)
{
    EXPECT_TRUE(database.getCalDavCalendarByIDIncludingDisabled("noacct", "nocal").calendarId.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavCalendarData)
{
    auto cal = makeValidCalendar();
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    auto mapping = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    EXPECT_TRUE(database.deleteCalDavCalendarData(cal.accountId, cal.calendarId));
    EXPECT_TRUE(database.getCalDavCalendarList(cal.accountId).isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavCalendarData_NoTransaction)
{
    auto cal = makeValidCalendar();
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    EXPECT_TRUE(database.deleteCalDavCalendarData(cal.accountId, cal.calendarId, false));
    EXPECT_TRUE(database.getCalDavCalendarList(cal.accountId).isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavCalendarData_EmptyArgs)
{
    EXPECT_FALSE(database.deleteCalDavCalendarData("", "cal-1"));
    EXPECT_FALSE(database.deleteCalDavCalendarData("acct", ""));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavCalendarSyncToken)
{
    auto cal = makeValidCalendar();
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    EXPECT_FALSE(database.updateCalDavCalendarSyncToken("", "new-token"));
    EXPECT_TRUE(database.updateCalDavCalendarSyncToken(cal.calendarId, "new-token"));
    auto list = database.getCalDavCalendarList(cal.accountId);
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.first().syncToken, QString("new-token"));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavCalendarSyncToken_NotFound)
{
    EXPECT_FALSE(database.updateCalDavCalendarSyncToken("nonexistent", "token"));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavCalendarInitialSyncCompleted)
{
    auto cal = makeValidCalendar();
    ASSERT_TRUE(database.upsertCalDavCalendar(cal));
    EXPECT_FALSE(database.updateCalDavCalendarInitialSyncCompleted("", true));
    EXPECT_TRUE(database.updateCalDavCalendarInitialSyncCompleted(cal.calendarId, true));
    auto list = database.getCalDavCalendarList(cal.accountId);
    ASSERT_EQ(list.size(), 1);
    EXPECT_TRUE(list.first().initialSyncCompleted);
    EXPECT_TRUE(database.updateCalDavCalendarInitialSyncCompleted(cal.calendarId, false));
}

// ===== Sync Status & Retry =====

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavSyncStatus)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    EXPECT_FALSE(database.updateCalDavSyncStatus("", 1, QDateTime(), "", DCalDavErrorCode::NoError));
    QDateTime now = QDateTime::currentDateTime();
    EXPECT_TRUE(database.updateCalDavSyncStatus(info.accountId, DCalDavSyncStatus::Succeeded, now, "", DCalDavErrorCode::NoError));
    EXPECT_TRUE(database.updateCalDavSyncStatus(info.accountId, DCalDavSyncStatus::Failed, QDateTime(), "Network error", DCalDavErrorCode::NetworkError));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavSyncStatus_NotFound)
{
    EXPECT_FALSE(database.updateCalDavSyncStatus("nonexistent", 1, QDateTime(), "", DCalDavErrorCode::NoError));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavRetryState)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    QDateTime next = QDateTime::currentDateTime().addSecs(60);
    EXPECT_FALSE(database.updateCalDavRetryState("", 1, next, "", DCalDavErrorCode::NoError));
    EXPECT_FALSE(database.updateCalDavRetryState(info.accountId, -1, next, "", DCalDavErrorCode::NoError));
    EXPECT_FALSE(database.updateCalDavRetryState(info.accountId, 1, QDateTime(), "", DCalDavErrorCode::NoError));
    EXPECT_TRUE(database.updateCalDavRetryState(info.accountId, 2, next, "retry reason", DCalDavErrorCode::NetworkError));
}

TEST_F(DAccountManagerDataBaseTest, UpdateCalDavRetryState_NotFound)
{
    QDateTime next = QDateTime::currentDateTime().addSecs(60);
    EXPECT_FALSE(database.updateCalDavRetryState("nonexistent", 1, next, "", DCalDavErrorCode::NoError));
}

TEST_F(DAccountManagerDataBaseTest, ClearCalDavRetryState)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    QDateTime next = QDateTime::currentDateTime().addSecs(60);
    ASSERT_TRUE(database.updateCalDavRetryState(info.accountId, 3, next, "reason", DCalDavErrorCode::NetworkError));
    EXPECT_FALSE(database.clearCalDavRetryState(""));
    EXPECT_TRUE(database.clearCalDavRetryState(info.accountId));
}

TEST_F(DAccountManagerDataBaseTest, ClearCalDavRetryState_NotFound)
{
    EXPECT_FALSE(database.clearCalDavRetryState("nonexistent"));
}

TEST_F(DAccountManagerDataBaseTest, DueCalDavAccountRetryIDs)
{
    EXPECT_TRUE(database.dueCalDavAccountRetryIDs(QDateTime()).isEmpty());
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    QDateTime past = QDateTime::currentDateTime().addSecs(-60);
    ASSERT_TRUE(database.updateCalDavRetryState(info.accountId, 1, past, "reason", DCalDavErrorCode::NetworkError));
    auto ids = database.dueCalDavAccountRetryIDs(QDateTime::currentDateTime());
    EXPECT_TRUE(ids.contains(info.accountId));
}

TEST_F(DAccountManagerDataBaseTest, DueCalDavOutboxAccountIDs)
{
    EXPECT_TRUE(database.dueCalDavOutboxAccountIDs(QDateTime()).isEmpty());
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    auto item = makeValidOutboxItem("op-retry", info.accountId, "sched-retry");
    item.failureType = DCalDavOutboxItem::NetworkFailure;
    item.nextRetryAt = QDateTime::currentDateTimeUtc().addSecs(-60);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto ids = database.dueCalDavOutboxAccountIDs(QDateTime::currentDateTime());
    EXPECT_TRUE(ids.contains(info.accountId));
}

TEST_F(DAccountManagerDataBaseTest, EarliestCalDavRetryAt_Empty)
{
    EXPECT_FALSE(database.earliestCalDavRetryAt().isValid());
}

TEST_F(DAccountManagerDataBaseTest, EarliestCalDavRetryAt_WithRetry)
{
    auto info = makeValidAccountInfo();
    ASSERT_TRUE(database.upsertCalDavAccountInfo(info));
    QDateTime future = QDateTime::currentDateTime().addSecs(120);
    ASSERT_TRUE(database.updateCalDavRetryState(info.accountId, 1, future, "reason", DCalDavErrorCode::NetworkError));
    auto earliest = database.earliestCalDavRetryAt();
    EXPECT_TRUE(earliest.isValid());
}

TEST_F(DAccountManagerDataBaseTest, EarliestCalDavOutboxRetryAt_Empty)
{
    EXPECT_FALSE(database.earliestCalDavOutboxRetryAt().isValid());
}

TEST_F(DAccountManagerDataBaseTest, EarliestCalDavOutboxRetryAt_WithData)
{
    auto item = makeValidOutboxItem("op-early", "acct-early", "sched-early");
    item.failureType = DCalDavOutboxItem::NetworkFailure;
    item.nextRetryAt = QDateTime::currentDateTime().addSecs(30);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto earliest = database.earliestCalDavOutboxRetryAt();
    EXPECT_TRUE(earliest.isValid());
}

// ===== Event Mapping =====

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavEventMapping_Insert)
{
    auto mapping = makeValidEventMapping();
    EXPECT_TRUE(database.upsertCalDavEventMapping(mapping));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavEventMapping_Update)
{
    auto mapping = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    mapping.etag = "etag-2";
    EXPECT_TRUE(database.upsertCalDavEventMapping(mapping));
    auto result = database.getCalDavEventMapping(mapping.accountID, mapping.href);
    EXPECT_EQ(result.etag, QString("etag-2"));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavEventMapping_EmptyFields)
{
    DCalDavEventMappingInfo m;
    EXPECT_FALSE(database.upsertCalDavEventMapping(m));
    m.localScheduleID = "ls";
    EXPECT_FALSE(database.upsertCalDavEventMapping(m));
    m.accountID = "a";
    EXPECT_FALSE(database.upsertCalDavEventMapping(m));
    m.calendarID = "c";
    EXPECT_FALSE(database.upsertCalDavEventMapping(m));
    m.uid = "u";
    EXPECT_FALSE(database.upsertCalDavEventMapping(m));
    m.href = "h";
    EXPECT_TRUE(database.upsertCalDavEventMapping(m));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMapping)
{
    auto mapping = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    auto result = database.getCalDavEventMapping(mapping.accountID, mapping.href);
    EXPECT_EQ(result.uid, mapping.uid);
    EXPECT_EQ(result.etag, mapping.etag);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMapping_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavEventMapping("", "href").localScheduleID.isEmpty());
    EXPECT_TRUE(database.getCalDavEventMapping("acct", "").localScheduleID.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMapping_NotFound)
{
    EXPECT_TRUE(database.getCalDavEventMapping("noacct", "nohref").localScheduleID.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMappingByLocalScheduleID)
{
    auto mapping = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    auto result = database.getCalDavEventMappingByLocalScheduleID(mapping.accountID, mapping.localScheduleID);
    EXPECT_EQ(result.href, mapping.href);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMappingByLocalScheduleID_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavEventMappingByLocalScheduleID("", "ls").localScheduleID.isEmpty());
    EXPECT_TRUE(database.getCalDavEventMappingByLocalScheduleID("acct", "").localScheduleID.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMappingByLocalScheduleID_NotFound)
{
    EXPECT_TRUE(database.getCalDavEventMappingByLocalScheduleID("noacct", "nols").localScheduleID.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMappingList)
{
    auto m1 = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(m1));
    auto m2 = makeValidEventMapping();
    m2.localScheduleID = "sched-2";
    m2.href = "/cal/1/event-2.ics";
    m2.uid = "uid-2";
    ASSERT_TRUE(database.upsertCalDavEventMapping(m2));
    auto list = database.getCalDavEventMappingList(m1.accountID, m1.calendarID);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavEventMappingList_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavEventMappingList("", "cal-1").isEmpty());
    EXPECT_TRUE(database.getCalDavEventMappingList("acct", "").isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavEventMapping)
{
    auto mapping = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    EXPECT_FALSE(database.deleteCalDavEventMapping("", "href"));
    EXPECT_FALSE(database.deleteCalDavEventMapping("acct", ""));
    EXPECT_TRUE(database.deleteCalDavEventMapping(mapping.accountID, mapping.href));
    EXPECT_TRUE(database.getCalDavEventMapping(mapping.accountID, mapping.href).localScheduleID.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavEventMapping_NotFound)
{
    EXPECT_FALSE(database.deleteCalDavEventMapping("noacct", "nohref"));
}

// ===== Skipped Resources =====

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavSkippedResource)
{
    auto acctId = "caldav-acct-1";
    auto resource = makeValidSkippedResource();
    EXPECT_FALSE(database.upsertCalDavSkippedResource("", resource));
    resource.calendarId = "";
    EXPECT_FALSE(database.upsertCalDavSkippedResource(acctId, resource));
    resource.calendarId = "cal-1";
    resource.href = "";
    EXPECT_FALSE(database.upsertCalDavSkippedResource(acctId, resource));
    resource.href = "/cal/1/skip-1.ics";
    resource.reason = "";
    EXPECT_FALSE(database.upsertCalDavSkippedResource(acctId, resource));
    resource.reason = "Not Found";
    EXPECT_TRUE(database.upsertCalDavSkippedResource(acctId, resource));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavSkippedResource_Update)
{
    auto acctId = "caldav-acct-1";
    auto resource = makeValidSkippedResource();
    ASSERT_TRUE(database.upsertCalDavSkippedResource(acctId, resource));
    resource.reason = "Gone";
    EXPECT_TRUE(database.upsertCalDavSkippedResource(acctId, resource));
    DCalDavSkippedResource::List list;
    ASSERT_TRUE(database.getCalDavSkippedResources(acctId, resource.calendarId, list));
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list.first().reason, QString("Gone"));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavSkippedResources)
{
    auto acctId = "caldav-acct-1";
    auto resource = makeValidSkippedResource();
    ASSERT_TRUE(database.upsertCalDavSkippedResource(acctId, resource));
    DCalDavSkippedResource::List list;
    EXPECT_FALSE(database.getCalDavSkippedResources("", "cal-1", list));
    EXPECT_FALSE(database.getCalDavSkippedResources(acctId, "", list));
    EXPECT_TRUE(database.getCalDavSkippedResources(acctId, "cal-1", list));
    EXPECT_EQ(list.size(), 1);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavSkippedResources_NotFound)
{
    DCalDavSkippedResource::List list;
    EXPECT_TRUE(database.getCalDavSkippedResources("noacct", "nocal", list));
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavSkippedResource)
{
    auto acctId = "caldav-acct-1";
    auto resource = makeValidSkippedResource();
    ASSERT_TRUE(database.upsertCalDavSkippedResource(acctId, resource));
    EXPECT_FALSE(database.deleteCalDavSkippedResource("", "cal-1", "href"));
    EXPECT_FALSE(database.deleteCalDavSkippedResource(acctId, "", "href"));
    EXPECT_FALSE(database.deleteCalDavSkippedResource(acctId, "cal-1", ""));
    EXPECT_TRUE(database.deleteCalDavSkippedResource(acctId, resource.calendarId, resource.href));
    DCalDavSkippedResource::List list;
    database.getCalDavSkippedResources(acctId, resource.calendarId, list);
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavSkippedResourcesByCalendar)
{
    auto acctId = "caldav-acct-1";
    auto r1 = makeValidSkippedResource();
    ASSERT_TRUE(database.upsertCalDavSkippedResource(acctId, r1));
    auto r2 = makeValidSkippedResource();
    r2.href = "/cal/1/skip-2.ics";
    ASSERT_TRUE(database.upsertCalDavSkippedResource(acctId, r2));
    EXPECT_FALSE(database.deleteCalDavSkippedResourcesByCalendar("", "cal-1"));
    EXPECT_FALSE(database.deleteCalDavSkippedResourcesByCalendar(acctId, ""));
    EXPECT_TRUE(database.deleteCalDavSkippedResourcesByCalendar(acctId, "cal-1"));
    DCalDavSkippedResource::List list;
    database.getCalDavSkippedResources(acctId, "cal-1", list);
    EXPECT_TRUE(list.isEmpty());
}

// ===== Outbox =====

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavOutboxItem_Insert)
{
    auto item = makeValidOutboxItem();
    EXPECT_TRUE(database.upsertCalDavOutboxItem(item));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavOutboxItem_Update)
{
    auto item = makeValidOutboxItem();
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    item.baseEtag = "updated-etag";
    EXPECT_TRUE(database.upsertCalDavOutboxItem(item));
    auto result = database.getCalDavOutboxItem(item.accountID, item.localScheduleID);
    EXPECT_EQ(result.baseEtag, QString("updated-etag"));
}

TEST_F(DAccountManagerDataBaseTest, UpsertCalDavOutboxItem_EmptyFields)
{
    DCalDavOutboxItem item;
    EXPECT_FALSE(database.upsertCalDavOutboxItem(item));
    item.operationID = "op";
    EXPECT_FALSE(database.upsertCalDavOutboxItem(item));
    item.accountID = "a";
    EXPECT_FALSE(database.upsertCalDavOutboxItem(item));
    item.localScheduleID = "ls";
    EXPECT_TRUE(database.upsertCalDavOutboxItem(item));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavOutboxItem)
{
    auto item = makeValidOutboxItem();
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto result = database.getCalDavOutboxItem(item.accountID, item.localScheduleID);
    EXPECT_EQ(result.operationID, item.operationID);
    EXPECT_EQ(result.operationType, item.operationType);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavOutboxItem_EmptyArgs)
{
    EXPECT_TRUE(database.getCalDavOutboxItem("", "ls").operationID.isEmpty());
    EXPECT_TRUE(database.getCalDavOutboxItem("acct", "").operationID.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavOutboxItem_NotFound)
{
    EXPECT_TRUE(database.getCalDavOutboxItem("noacct", "nols").operationID.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, HasPendingCalDavCalendarDelete)
{
    EXPECT_FALSE(database.hasPendingCalDavCalendarDelete("", "stype"));
    EXPECT_FALSE(database.hasPendingCalDavCalendarDelete("acct", ""));
    auto item = makeValidOutboxItem("op-del-cal", "acct-del", "sched-del");
    item.operationType = DCalDavOutboxItem::DeleteCalendarOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    EXPECT_TRUE(database.hasPendingCalDavCalendarDelete("acct-del", "sched-del"));
}

TEST_F(DAccountManagerDataBaseTest, HasPendingCalDavCalendarDelete_NoDeleteOp)
{
    auto item = makeValidOutboxItem("op-create", "acct-nd", "sched-nd");
    item.operationType = DCalDavOutboxItem::CreateOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    EXPECT_FALSE(database.hasPendingCalDavCalendarDelete("acct-nd", "sched-nd"));
}

TEST_F(DAccountManagerDataBaseTest, HasCalDavOutboxItems)
{
    EXPECT_FALSE(database.hasCalDavOutboxItems(""));
    EXPECT_FALSE(database.hasCalDavOutboxItems("noacct"));
    auto item = makeValidOutboxItem();
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    EXPECT_TRUE(database.hasCalDavOutboxItems(item.accountID));
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavConflictItems)
{
    EXPECT_TRUE(database.getCalDavConflictItems("").isEmpty());
    auto item = makeValidOutboxItem("op-conflict", "acct-conflict", "sched-conflict");
    item.failureType = DCalDavOutboxItem::ConflictFailure;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto list = database.getCalDavConflictItems("acct-conflict");
    EXPECT_EQ(list.size(), 1);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavBlockedOutboxItems)
{
    EXPECT_TRUE(database.getCalDavBlockedOutboxItems("").isEmpty());
    auto item = makeValidOutboxItem("op-blocked", "acct-blocked", "sched-blocked");
    item.failureType = DCalDavOutboxItem::PermissionFailure;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto list = database.getCalDavBlockedOutboxItems("acct-blocked");
    EXPECT_EQ(list.size(), 1);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavBlockedOutboxItems_ExcludesNoFailure)
{
    auto item = makeValidOutboxItem("op-nofail", "acct-nf", "sched-nf");
    item.failureType = DCalDavOutboxItem::NoFailure;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto list = database.getCalDavBlockedOutboxItems("acct-nf");
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavRetryScheduledOutboxItems)
{
    EXPECT_TRUE(database.getCalDavRetryScheduledOutboxItems("", QDateTime::currentDateTime()).isEmpty());
    EXPECT_TRUE(database.getCalDavRetryScheduledOutboxItems("acct", QDateTime()).isEmpty());
    auto item = makeValidOutboxItem("op-sched", "acct-sched", "sched-sched");
    item.failureType = DCalDavOutboxItem::NetworkFailure;
    item.nextRetryAt = QDateTime::currentDateTime().addSecs(3600);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto list = database.getCalDavRetryScheduledOutboxItems("acct-sched", QDateTime::currentDateTime());
    EXPECT_EQ(list.size(), 1);
}

TEST_F(DAccountManagerDataBaseTest, GetCalDavRetryScheduledOutboxItems_NotFuture)
{
    auto item = makeValidOutboxItem("op-past", "acct-past", "sched-past");
    item.failureType = DCalDavOutboxItem::NetworkFailure;
    item.nextRetryAt = QDateTime::currentDateTimeUtc().addSecs(-60);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto list = database.getCalDavRetryScheduledOutboxItems("acct-past", QDateTime::currentDateTime());
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, GetDueCalDavOutboxItems_IgnoreRetryAt)
{
    EXPECT_TRUE(database.getDueCalDavOutboxItems("", QDateTime::currentDateTime(), true).isEmpty());
    auto item = makeValidOutboxItem("op-due1", "acct-due", "sched-due1");
    item.failureType = DCalDavOutboxItem::NoFailure;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto item2 = makeValidOutboxItem("op-due2", "acct-due", "sched-due2");
    item2.failureType = DCalDavOutboxItem::PermanentFailure;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item2));
    auto list = database.getDueCalDavOutboxItems("acct-due", QDateTime::currentDateTime(), true);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(DAccountManagerDataBaseTest, GetDueCalDavOutboxItems_WithRetryAt)
{
    auto item = makeValidOutboxItem("op-due3", "acct-due2", "sched-due3");
    item.failureType = DCalDavOutboxItem::NoFailure;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto item2 = makeValidOutboxItem("op-due4", "acct-due2", "sched-due4");
    item2.failureType = DCalDavOutboxItem::NetworkFailure;
    item2.nextRetryAt = QDateTime::currentDateTime().addSecs(-60);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item2));
    auto list = database.getDueCalDavOutboxItems("acct-due2", QDateTime::currentDateTime(), false);
    EXPECT_EQ(list.size(), 2);
}

TEST_F(DAccountManagerDataBaseTest, GetDueCalDavOutboxItems_WithRetryAt_NotDue)
{
    auto item = makeValidOutboxItem("op-notdue", "acct-nd2", "sched-nd2");
    item.failureType = DCalDavOutboxItem::NetworkFailure;
    item.nextRetryAt = QDateTime::currentDateTime().addSecs(3600);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    auto list = database.getDueCalDavOutboxItems("acct-nd2", QDateTime::currentDateTime(), false);
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavOutboxItem)
{
    auto item = makeValidOutboxItem();
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    EXPECT_FALSE(database.deleteCalDavOutboxItem("", "ls"));
    EXPECT_FALSE(database.deleteCalDavOutboxItem("acct", ""));
    EXPECT_TRUE(database.deleteCalDavOutboxItem(item.accountID, item.localScheduleID));
    EXPECT_FALSE(database.hasCalDavOutboxItems(item.accountID));
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavCalendarEventOutboxItems)
{
    auto mapping = makeValidEventMapping();
    ASSERT_TRUE(database.upsertCalDavEventMapping(mapping));
    auto item = makeValidOutboxItem("op-cal-ev", mapping.accountID, mapping.localScheduleID);
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    EXPECT_FALSE(database.deleteCalDavCalendarEventOutboxItems("", "cal-1"));
    EXPECT_FALSE(database.deleteCalDavCalendarEventOutboxItems("acct", ""));
    EXPECT_TRUE(database.deleteCalDavCalendarEventOutboxItems(mapping.accountID, mapping.calendarID));
    EXPECT_FALSE(database.hasCalDavOutboxItems(mapping.accountID));
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavOutboxItemIfCurrent)
{
    auto item = makeValidOutboxItem();
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));
    EXPECT_TRUE(database.deleteCalDavOutboxItemIfCurrent(item));
    EXPECT_FALSE(database.deleteCalDavOutboxItemIfCurrent(item));
}

TEST_F(DAccountManagerDataBaseTest, DeleteCalDavOutboxItemIfCurrent_EmptyFields)
{
    DCalDavOutboxItem item;
    EXPECT_FALSE(database.deleteCalDavOutboxItemIfCurrent(item));
}

// ===== General Settings =====

TEST_F(DAccountManagerDataBaseTest, GetCalendarGeneralSettings)
{
    auto settings = database.getCalendarGeneralSettings();
    ASSERT_NE(settings, nullptr);
    EXPECT_EQ(settings->firstDayOfWeek(), Qt::Sunday);
    EXPECT_EQ(settings->timeShowType(), DCalendarGeneralSettings::TwentyFour);
}

TEST_F(DAccountManagerDataBaseTest, SetCalendarGeneralSettings)
{
    DCalendarGeneralSettings::Ptr settings(new DCalendarGeneralSettings);
    settings->setFirstDayOfWeek(Qt::Monday);
    settings->setTimeShowType(DCalendarGeneralSettings::Twelve);
    database.setCalendarGeneralSettings(settings);
    auto retrieved = database.getCalendarGeneralSettings();
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->firstDayOfWeek(), Qt::Monday);
    EXPECT_EQ(retrieved->timeShowType(), DCalendarGeneralSettings::Twelve);
}
