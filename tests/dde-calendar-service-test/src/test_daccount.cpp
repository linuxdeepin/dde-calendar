// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "daccount.h"
#include "units.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

// ============ Constructor ============

TEST(DAccountTest, Constructor_DefaultIsLocal)
{
    DAccount account;
    EXPECT_EQ(account.accountType(), DAccount::Account_Local);
    EXPECT_TRUE(account.displayName().isEmpty());
    EXPECT_TRUE(account.accountID().isEmpty());
    EXPECT_TRUE(account.accountName().isEmpty());
    EXPECT_TRUE(account.dbName().isEmpty());
    EXPECT_TRUE(account.dbusPath().isEmpty());
    EXPECT_TRUE(account.avatar().isEmpty());
    EXPECT_TRUE(account.description().isEmpty());
    EXPECT_EQ(account.syncTag(), 0);
    EXPECT_EQ(account.accountState(), DAccount::AccountStates(DAccount::Account_Invalid));
    EXPECT_EQ(account.syncState(), DAccount::Sync_Normal);
    EXPECT_EQ(account.syncFreq(), DAccount::SyncFreq_15Mins);
    EXPECT_EQ(account.intervalTime(), 0);
    EXPECT_TRUE(account.isExpandDisplay());
}

TEST(DAccountTest, Constructor_WithUnionIDType)
{
    DAccount account(DAccount::Account_UnionID);
    EXPECT_EQ(account.accountType(), DAccount::Account_UnionID);
    EXPECT_TRUE(account.isNetWorkAccount());
}

TEST(DAccountTest, Constructor_WithCalDavType)
{
    DAccount account(DAccount::Account_CalDav);
    EXPECT_EQ(account.accountType(), DAccount::Account_CalDav);
    EXPECT_TRUE(account.isNetWorkAccount());
}

// ============ isNetWorkAccount ============

TEST(DAccountTest, IsNetWorkAccount_LocalReturnsFalse)
{
    DAccount account(DAccount::Account_Local);
    EXPECT_FALSE(account.isNetWorkAccount());
}

TEST(DAccountTest, IsNetWorkAccount_UnionIDReturnsTrue)
{
    DAccount account(DAccount::Account_UnionID);
    EXPECT_TRUE(account.isNetWorkAccount());
}

// ============ Getters/Setters ============

TEST(DAccountTest, DisplayName_SetterGetter)
{
    DAccount account;
    account.setDisplayName("My Calendar");
    EXPECT_EQ(account.displayName(), "My Calendar");
}

TEST(DAccountTest, AccountID_SetterGetter)
{
    DAccount account;
    account.setAccountID("id-123");
    EXPECT_EQ(account.accountID(), "id-123");
}

TEST(DAccountTest, AccountName_SetterGetter)
{
    DAccount account;
    account.setAccountName("accountName");
    EXPECT_EQ(account.accountName(), "accountName");
}

TEST(DAccountTest, DbusPath_SetterGetter)
{
    DAccount account;
    account.setDbusPath("/com/deepin/calendar");
    EXPECT_EQ(account.dbusPath(), "/com/deepin/calendar");
}

TEST(DAccountTest, AccountType_SetterGetter)
{
    DAccount account;
    account.setAccountType(DAccount::Account_CalDav);
    EXPECT_EQ(account.accountType(), DAccount::Account_CalDav);
}

TEST(DAccountTest, IsExpandDisplay_SetterGetter)
{
    DAccount account;
    account.setIsExpandDisplay(false);
    EXPECT_FALSE(account.isExpandDisplay());
}

TEST(DAccountTest, SyncTag_SetterGetter)
{
    DAccount account;
    account.setSyncTag(42);
    EXPECT_EQ(account.syncTag(), 42);
}

TEST(DAccountTest, SyncState_SetterGetter)
{
    DAccount account;
    account.setSyncState(DAccount::Sync_ServerException);
    EXPECT_EQ(account.syncState(), DAccount::Sync_ServerException);
}

TEST(DAccountTest, Avatar_SetterGetter)
{
    DAccount account;
    account.setAvatar("/path/to/avatar.png");
    EXPECT_EQ(account.avatar(), "/path/to/avatar.png");
}

TEST(DAccountTest, Description_SetterGetter)
{
    DAccount account;
    account.setDescription("Test description");
    EXPECT_EQ(account.description(), "Test description");
}

TEST(DAccountTest, DtCreate_SetterGetter)
{
    DAccount account;
    QDateTime dt(QDate(2026, 1, 1), QTime(10, 0));
    account.setDtCreate(dt);
    EXPECT_EQ(account.dtCreate(), dt);
}

TEST(DAccountTest, DtDelete_SetterGetter)
{
    DAccount account;
    QDateTime dt(QDate(2026, 6, 1), QTime(12, 0));
    account.setDtDelete(dt);
    EXPECT_EQ(account.dtDelete(), dt);
}

TEST(DAccountTest, DtUpdate_SetterGetter)
{
    DAccount account;
    QDateTime dt(QDate(2026, 3, 15), QTime(8, 30));
    account.setDtUpdate(dt);
    EXPECT_EQ(account.dtUpdate(), dt);
}

TEST(DAccountTest, DbName_SetterGetter)
{
    DAccount account;
    account.setDbName("calendar.db");
    EXPECT_EQ(account.dbName(), "calendar.db");
}

TEST(DAccountTest, CloudPath_SetterGetter)
{
    DAccount account;
    account.setCloudPath("/cloud/path");
    EXPECT_EQ(account.cloudPath(), "/cloud/path");
}

TEST(DAccountTest, SyncFreq_SetterGetter)
{
    DAccount account;
    account.setSyncFreq(DAccount::SyncFreq_1hour);
    EXPECT_EQ(account.syncFreq(), DAccount::SyncFreq_1hour);
}

TEST(DAccountTest, IntervalTime_SetterGetter)
{
    DAccount account;
    account.setIntervalTime(30);
    EXPECT_EQ(account.intervalTime(), 30);
}

TEST(DAccountTest, DbusInterface_SetterGetter)
{
    DAccount account;
    account.setDbusInterface("com.deepin.Calendar.Account");
    EXPECT_EQ(account.dbusInterface(), "com.deepin.Calendar.Account");
}

TEST(DAccountTest, AccountState_SetterGetter)
{
    DAccount account;
    DAccount::AccountStates state(DAccount::Account_Open | DAccount::Account_Calendar);
    account.setAccountState(state);
    EXPECT_EQ(account.accountState(), state);
}

TEST(DAccountTest, DtLastSync_SetterGetter)
{
    DAccount account;
    QDateTime dt(QDate(2026, 1, 2), QTime(3, 45));
    account.setDtLastSync(dt);
    EXPECT_EQ(account.dtLastSync(), dt);
}

// ============ toJsonString / fromJsonString ============

TEST(DAccountTest, ToJsonString_NullAccount_ReturnsFalse)
{
    DAccount::Ptr nullAccount;
    QString jsonStr;
    EXPECT_FALSE(DAccount::toJsonString(nullAccount, jsonStr));
}

TEST(DAccountTest, ToJsonString_ValidAccount)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_UnionID));
    account->setAccountID("acc-001");
    account->setDisplayName("Test Account");
    account->setAccountName("testacc");
    account->setSyncTag(5);
    account->setSyncFreq(DAccount::SyncFreq_30Mins);
    account->setIntervalTime(45);

    QString jsonStr;
    EXPECT_TRUE(DAccount::toJsonString(account, jsonStr));
    ASSERT_FALSE(jsonStr.isEmpty());

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj.value("accountID").toString(), "acc-001");
    EXPECT_EQ(obj.value("displayName").toString(), "Test Account");
    EXPECT_EQ(obj.value("accountName").toString(), "testacc");
    EXPECT_EQ(obj.value("syncTag").toInt(), 5);
    EXPECT_EQ(obj.value("type").toInt(), int(DAccount::Account_UnionID));
}

TEST(DAccountTest, FromJsonString_NullAccount_CreatesNew)
{
    DAccount::Ptr account;
    QString json = "{\"accountID\":\"new-id\",\"displayName\":\"Created\"}";
    EXPECT_TRUE(DAccount::fromJsonString(account, json));
    ASSERT_NE(account, nullptr);
    EXPECT_EQ(account->accountID(), "new-id");
    EXPECT_EQ(account->displayName(), "Created");
}

TEST(DAccountTest, FromJsonString_ExistingAccount_Updates)
{
    DAccount::Ptr account(new DAccount(DAccount::Account_Local));
    account->setDisplayName("Old");
    QString json = "{\"accountID\":\"updated-id\",\"displayName\":\"Updated\"}";
    EXPECT_TRUE(DAccount::fromJsonString(account, json));
    EXPECT_EQ(account->accountID(), "updated-id");
    EXPECT_EQ(account->displayName(), "Updated");
}

TEST(DAccountTest, FromJsonString_InvalidJSON_ReturnsFalse)
{
    DAccount::Ptr account;
    EXPECT_FALSE(DAccount::fromJsonString(account, "not json"));
}

TEST(DAccountTest, ToJsonFromJson_RoundTrip)
{
    DAccount::Ptr original(new DAccount(DAccount::Account_CalDav));
    original->setAccountID("rt-001");
    original->setDisplayName("RoundTrip");
    original->setAccountName("rtacc");
    original->setDbusPath("/rt/path");
    original->setDbusInterface("com.rt.Iface");
    original->setAvatar("/rt/avatar");
    original->setDescription("RT desc");
    original->setSyncTag(77);
    original->setSyncState(DAccount::Sync_StorageFull);
    original->setDbName("rt.db");
    original->setSyncFreq(DAccount::SyncFreq_24hour);
    original->setIntervalTime(120);

    QString jsonStr;
    ASSERT_TRUE(DAccount::toJsonString(original, jsonStr));

    DAccount::Ptr parsed;
    ASSERT_TRUE(DAccount::fromJsonString(parsed, jsonStr));
    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(parsed->accountID(), "rt-001");
    EXPECT_EQ(parsed->displayName(), "RoundTrip");
    EXPECT_EQ(parsed->accountName(), "rtacc");
    EXPECT_EQ(parsed->dbusPath(), "/rt/path");
    EXPECT_EQ(parsed->dbusInterface(), "com.rt.Iface");
    EXPECT_EQ(parsed->avatar(), "/rt/avatar");
    EXPECT_EQ(parsed->description(), "RT desc");
    EXPECT_EQ(parsed->syncTag(), 77);
    EXPECT_EQ(parsed->syncState(), DAccount::Sync_StorageFull);
    EXPECT_EQ(parsed->dbName(), "rt.db");
    EXPECT_EQ(parsed->syncFreq(), DAccount::SyncFreq_24hour);
    EXPECT_EQ(parsed->intervalTime(), 120);
    EXPECT_EQ(parsed->accountType(), DAccount::Account_CalDav);
}

// ============ toJsonListString / fromJsonListString ============

TEST(DAccountTest, ToJsonListString_EmptyList)
{
    DAccount::List list;
    QString jsonStr;
    EXPECT_TRUE(DAccount::toJsonListString(list, jsonStr));
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    EXPECT_TRUE(obj.contains("accounts"));
    EXPECT_EQ(obj.value("accounts").toArray().count(), 0);
}

TEST(DAccountTest, ToJsonListString_MultipleAccounts)
{
    DAccount::List list;
    DAccount::Ptr a1(new DAccount(DAccount::Account_Local));
    a1->setAccountID("a1");
    DAccount::Ptr a2(new DAccount(DAccount::Account_UnionID));
    a2->setAccountID("a2");
    list.append(a1);
    list.append(a2);

    QString jsonStr;
    EXPECT_TRUE(DAccount::toJsonListString(list, jsonStr));

    DAccount::List parsed;
    EXPECT_TRUE(DAccount::fromJsonListString(parsed, jsonStr));
    ASSERT_EQ(parsed.count(), 2);
    EXPECT_EQ(parsed[0]->accountID(), "a1");
    EXPECT_EQ(parsed[1]->accountID(), "a2");
}

TEST(DAccountTest, FromJsonListString_InvalidJSON_ReturnsFalse)
{
    DAccount::List list;
    EXPECT_FALSE(DAccount::fromJsonListString(list, "not json"));
}

TEST(DAccountTest, FromJsonListString_ValidJSON_NoAccountsKey)
{
    DAccount::List list;
    EXPECT_TRUE(DAccount::fromJsonListString(list, "{}"));
    EXPECT_EQ(list.count(), 0);
}

// ============ syncFreqToJsonString / syncFreqFromJsonString ============

TEST(DAccountTest, SyncFreqToJsonString)
{
    DAccount::Ptr account(new DAccount);
    account->setSyncFreq(DAccount::SyncFreq_30Mins);
    account->setIntervalTime(30);
    QString jsonStr = DAccount::syncFreqToJsonString(account);

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj.value("syncFreq").toInt(), int(DAccount::SyncFreq_30Mins));
    EXPECT_EQ(obj.value("m_intervalTime").toInt(), 30);
}

TEST(DAccountTest, SyncFreqFromJsonString_Valid)
{
    DAccount::Ptr account(new DAccount);
    account->setSyncFreq(DAccount::SyncFreq_Maunal);
    account->setIntervalTime(0);

    QString jsonStr = "{\"syncFreq\":3,\"m_intervalTime\":60}";
    DAccount::syncFreqFromJsonString(account, jsonStr);
    EXPECT_EQ(account->syncFreq(), DAccount::SyncFreq_1hour);
    EXPECT_EQ(account->intervalTime(), 60);
}

TEST(DAccountTest, SyncFreqFromJsonString_InvalidJSON_NoChange)
{
    DAccount::Ptr account(new DAccount);
    account->setSyncFreq(DAccount::SyncFreq_24hour);
    account->setIntervalTime(100);

    DAccount::syncFreqFromJsonString(account, "not json");
    // Should remain unchanged on parse error
    EXPECT_EQ(account->syncFreq(), DAccount::SyncFreq_24hour);
    EXPECT_EQ(account->intervalTime(), 100);
}

TEST(DAccountTest, SyncFreqRoundTrip)
{
    DAccount::Ptr original(new DAccount);
    original->setSyncFreq(DAccount::SyncFreq_1hour);
    original->setIntervalTime(90);

    QString jsonStr = DAccount::syncFreqToJsonString(original);

    DAccount::Ptr parsed(new DAccount);
    DAccount::syncFreqFromJsonString(parsed, jsonStr);
    EXPECT_EQ(parsed->syncFreq(), DAccount::SyncFreq_1hour);
    EXPECT_EQ(parsed->intervalTime(), 90);
}
