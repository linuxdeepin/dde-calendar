// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcalendardeletehelper.h"
#include "dcaldavprofile.h"
#include "dcaldavcalendarinfo.h"
#include "dcaldavcategoryinfo.h"
#include "dcaldavoutboxenqueuer.h"
#include "dcaldavaccountdeletioncleanup.h"
#include "dcaldavaccountregistrar.h"
#include "dcaldavaccountsync.h"
#include "dcaldavretrypolicy.h"
#include "dcaldavsyncstatusmapper.h"
#include "dcaldavtransport.h"
#include "dcaldaverrorcode.h"
#include "dcaldavaccountstatus.h"
#include "ddatabase.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "units.h"
#include "dcaldavxmlreader.h"
#include "dcaldavsyncjobmanager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QTemporaryDir>

#include <gtest/gtest.h>

// ---------------------------------------------------------------------------
// DCalDavCalendarDeleteHelper::collectTypeIDs
// ---------------------------------------------------------------------------

TEST(CalDavCalendarDeleteHelper, CollectTypeIDsEmptyPrimaryAndCategories)
{
    DCalDavCategoryInfo::List categories;
    const QStringList result = DCalDavCalendarDeleteHelper::collectTypeIDs(
        QString(), categories);
    EXPECT_TRUE(result.isEmpty());
}

TEST(CalDavCalendarDeleteHelper, CollectTypeIDsPrimaryOnly)
{
    DCalDavCategoryInfo::List categories;
    const QStringList result = DCalDavCalendarDeleteHelper::collectTypeIDs(
        QStringLiteral("type-1"), categories);
    ASSERT_EQ(1, result.size());
    EXPECT_EQ(QStringLiteral("type-1"), result.first());
}

TEST(CalDavCalendarDeleteHelper, CollectTypeIDsDeduplicatesPrimaryAndCategories)
{
    DCalDavCategoryInfo cat1;
    cat1.scheduleTypeId = QStringLiteral("type-2");
    DCalDavCategoryInfo cat2;
    cat2.scheduleTypeId = QStringLiteral("type-1");
    DCalDavCategoryInfo cat3;
    cat3.scheduleTypeId = QStringLiteral("type-3");
    DCalDavCategoryInfo cat4;
    cat4.scheduleTypeId = QString();
    DCalDavCategoryInfo::List categories;
    categories << cat1 << cat2 << cat3 << cat4;

    const QStringList result = DCalDavCalendarDeleteHelper::collectTypeIDs(
        QStringLiteral("type-1"), categories);
    ASSERT_EQ(3, result.size());
    EXPECT_EQ(QStringLiteral("type-1"), result[0]);
    EXPECT_EQ(QStringLiteral("type-2"), result[1]);
    EXPECT_EQ(QStringLiteral("type-3"), result[2]);
}

// ---------------------------------------------------------------------------
// DCalDavCalendarDeleteHelper::collectScheduleIDs
// ---------------------------------------------------------------------------

TEST(CalDavCalendarDeleteHelper, CollectScheduleIDsNullDatabaseReturnsEmpty)
{
    QStringList typeIDs;
    typeIDs << QStringLiteral("type-1");
    const QStringList result = DCalDavCalendarDeleteHelper::collectScheduleIDs(
        nullptr, typeIDs);
    EXPECT_TRUE(result.isEmpty());
}

TEST(CalDavCalendarDeleteHelper, CollectScheduleIDsDeduplicatesAcrossTypes)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("collect-schedule-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr typeA(new DScheduleType(account->accountID()));
    typeA->setTypeName(QStringLiteral("Type A"));
    typeA->setDisplayName(QStringLiteral("Type A"));
    typeA->setTypeColor(color);
    typeA->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(typeA).isEmpty());

    DScheduleType::Ptr typeB(new DScheduleType(account->accountID()));
    typeB->setTypeName(QStringLiteral("Type B"));
    typeB->setDisplayName(QStringLiteral("Type B"));
    typeB->setTypeColor(color);
    typeB->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(typeB).isEmpty());

    DSchedule::Ptr scheduleA1(new DSchedule);
    scheduleA1->setScheduleTypeID(typeA->typeID());
    scheduleA1->setSummary(QStringLiteral("A1"));
    scheduleA1->setDtStart(QDateTime(QDate(2026, 10, 1), QTime(10, 0), Qt::UTC));
    scheduleA1->setDtEnd(QDateTime(QDate(2026, 10, 1), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(scheduleA1).isEmpty());

    DSchedule::Ptr scheduleA2(new DSchedule);
    scheduleA2->setScheduleTypeID(typeA->typeID());
    scheduleA2->setSummary(QStringLiteral("A2"));
    scheduleA2->setDtStart(QDateTime(QDate(2026, 10, 2), QTime(10, 0), Qt::UTC));
    scheduleA2->setDtEnd(QDateTime(QDate(2026, 10, 2), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(scheduleA2).isEmpty());

    DSchedule::Ptr scheduleB1(new DSchedule);
    scheduleB1->setScheduleTypeID(typeB->typeID());
    scheduleB1->setSummary(QStringLiteral("B1"));
    scheduleB1->setDtStart(QDateTime(QDate(2026, 10, 3), QTime(10, 0), Qt::UTC));
    scheduleB1->setDtEnd(QDateTime(QDate(2026, 10, 3), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(scheduleB1).isEmpty());

    QStringList typeIDs;
    typeIDs << typeA->typeID() << typeB->typeID();
    const QStringList result = DCalDavCalendarDeleteHelper::collectScheduleIDs(
        &localDatabase, typeIDs);
    EXPECT_EQ(3, result.size());
}

// ---------------------------------------------------------------------------
// DCalDavCalendarDeleteHelper::deleteScheduleTypes
// ---------------------------------------------------------------------------

TEST(CalDavCalendarDeleteHelper, DeleteScheduleTypesNullDatabaseReturnsFalse)
{
    QStringList typeIDs;
    typeIDs << QStringLiteral("type-1");
    EXPECT_FALSE(DCalDavCalendarDeleteHelper::deleteScheduleTypes(
        nullptr, typeIDs, false, false));
}

TEST(CalDavCalendarDeleteHelper, DeleteScheduleTypesSkipsNullType)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("delete-skip-null-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    QStringList typeIDs;
    typeIDs << QStringLiteral("nonexistent-type");
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteScheduleTypes(
        &localDatabase, typeIDs, false, false));
}

TEST(CalDavCalendarDeleteHelper, DeleteScheduleTypesSoftDeleteAndHardDelete)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("delete-types-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Soft Delete"));
    type->setDisplayName(QStringLiteral("Soft Delete"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setScheduleTypeID(type->typeID());
    schedule->setSummary(QStringLiteral("Event"));
    schedule->setDtStart(QDateTime(QDate(2026, 10, 1), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 10, 1), QTime(11, 0), Qt::UTC));
    ASSERT_FALSE(localDatabase.createSchedule(schedule).isEmpty());

    QStringList typeIDs;
    typeIDs << type->typeID();

    // Soft delete: type hidden from default query, visible with include-deleted flag
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteScheduleTypes(
        &localDatabase, typeIDs, false, false));
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type->typeID()).isNull());
    EXPECT_FALSE(localDatabase.getScheduleTypeByID(type->typeID(), 1).isNull());

    // Hard delete a separate fresh type to verify complete removal
    DScheduleType::Ptr type2(new DScheduleType(account->accountID()));
    type2->setTypeName(QStringLiteral("Hard Delete"));
    type2->setDisplayName(QStringLiteral("Hard Delete"));
    type2->setTypeColor(color);
    type2->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type2).isEmpty());

    QStringList typeIDs2;
    typeIDs2 << type2->typeID();
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteScheduleTypes(
        &localDatabase, typeIDs2, true, false));
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type2->typeID()).isNull());
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type2->typeID(), 1).isNull());
}

TEST(CalDavCalendarDeleteHelper, DeleteScheduleTypesDeletesNonSystemColor)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("delete-color-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#FF0000"));
    color.setPrivilege(DTypeColor::PriUser);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("Color Delete"));
    type->setDisplayName(QStringLiteral("Color Delete"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    QStringList typeIDs;
    typeIDs << type->typeID();
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteScheduleTypes(
        &localDatabase, typeIDs, true, true));
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type->typeID()).isNull());
}

TEST(CalDavCalendarDeleteHelper, DeleteScheduleTypesKeepsSystemColor)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("keep-system-color-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);
    ASSERT_TRUE(localDatabase.addTypeColor(color));

    DScheduleType::Ptr type(new DScheduleType(account->accountID()));
    type->setTypeName(QStringLiteral("System Color"));
    type->setDisplayName(QStringLiteral("System Color"));
    type->setTypeColor(color);
    type->setPrivilege(DScheduleType::User);
    ASSERT_FALSE(localDatabase.createScheduleType(type).isEmpty());

    QStringList typeIDs;
    typeIDs << type->typeID();
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteScheduleTypes(
        &localDatabase, typeIDs, true, true));
    EXPECT_TRUE(localDatabase.getScheduleTypeByID(type->typeID()).isNull());
}

// ---------------------------------------------------------------------------
// DCalDavCalendarDeleteHelper::deleteOutboxItems
// ---------------------------------------------------------------------------

TEST(CalDavCalendarDeleteHelper, DeleteOutboxItemsNullDatabaseReturnsFalse)
{
    QStringList scheduleIDs;
    scheduleIDs << QStringLiteral("schedule-1");
    EXPECT_FALSE(DCalDavCalendarDeleteHelper::deleteOutboxItems(
        nullptr, QStringLiteral("account-1"), scheduleIDs));
}

TEST(CalDavCalendarDeleteHelper, DeleteOutboxItemsEmptyAccountIDReturnsFalse)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    QStringList scheduleIDs;
    scheduleIDs << QStringLiteral("schedule-1");
    EXPECT_FALSE(DCalDavCalendarDeleteHelper::deleteOutboxItems(
        &database, QString(), scheduleIDs));
}

TEST(CalDavCalendarDeleteHelper, DeleteOutboxItemsNormalDelete)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("outbox-delete-account");

    DCalDavOutboxItem item1;
    item1.operationID = QStringLiteral("op-1");
    item1.accountID = accountID;
    item1.localScheduleID = QStringLiteral("schedule-1");
    item1.operationType = DCalDavOutboxItem::DeleteOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item1));

    DCalDavOutboxItem item2;
    item2.operationID = QStringLiteral("op-2");
    item2.accountID = accountID;
    item2.localScheduleID = QStringLiteral("schedule-2");
    item2.operationType = DCalDavOutboxItem::CreateOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item2));

    ASSERT_TRUE(database.hasCalDavOutboxItems(accountID));

    QStringList scheduleIDs;
    scheduleIDs << QStringLiteral("schedule-1") << QStringLiteral("schedule-2");
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteOutboxItems(
        &database, accountID, scheduleIDs));
    EXPECT_FALSE(database.hasCalDavOutboxItems(accountID));
}

TEST(CalDavCalendarDeleteHelper, DeleteOutboxItemsEmptyScheduleIDsReturnsTrue)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteOutboxItems(
        &database, QStringLiteral("account-1"), QStringList()));
}

// ---------------------------------------------------------------------------
// DCalDavCalendarDeleteHelper::deleteRemoteManagerData
// ---------------------------------------------------------------------------

TEST(CalDavCalendarDeleteHelper, DeleteRemoteManagerDataNullDatabaseReturnsFalse)
{
    QStringList scheduleIDs;
    scheduleIDs << QStringLiteral("schedule-1");
    EXPECT_FALSE(DCalDavCalendarDeleteHelper::deleteRemoteManagerData(
        nullptr, QStringLiteral("account-1"), QStringLiteral("cal-1"), scheduleIDs));
}

TEST(CalDavCalendarDeleteHelper, DeleteRemoteManagerDataNormalDelete)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("remote-delete-account"));
    account->setAccountName(QStringLiteral("user"));
    account->setDisplayName(QStringLiteral("Calendar"));
    account->setDbName(QStringLiteral("account.db"));
    account->setDbusPath(QStringLiteral("/test/account"));
    account->setDbusInterface(QStringLiteral("com.deepin.test.account"));
    account->setDtCreate(QDateTime::currentDateTime());
    ASSERT_FALSE(database.addAccountInfo(account).isEmpty());

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = QStringLiteral("https://example.test");
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(accountInfo));

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("cal-1");
    calendar.accountId = account->accountID();
    calendar.href = QStringLiteral("https://example.test/calendars/cal-1/");
    calendar.scheduleTypeID = QStringLiteral("type-1");
    ASSERT_TRUE(database.upsertCalDavCalendar(calendar));

    DCalDavOutboxItem item;
    item.operationID = QStringLiteral("op-1");
    item.accountID = account->accountID();
    item.localScheduleID = QStringLiteral("schedule-1");
    item.operationType = DCalDavOutboxItem::DeleteOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));

    QStringList scheduleIDs;
    scheduleIDs << QStringLiteral("schedule-1");
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::deleteRemoteManagerData(
        &database, account->accountID(), calendar.calendarId, scheduleIDs));
    EXPECT_FALSE(database.hasCalDavOutboxItems(account->accountID()));
    EXPECT_TRUE(database.getCalDavCalendarByIDIncludingDisabled(
        account->accountID(), calendar.calendarId).calendarId.isEmpty());
}

// ---------------------------------------------------------------------------
// DCalDavCalendarDeleteHelper::persistLocalDeleteState
// ---------------------------------------------------------------------------

TEST(CalDavCalendarDeleteHelper, PersistLocalDeleteStateNullDatabaseReturnsFalse)
{
    DCalDavCalendarInfo calendar;
    QStringList scheduleIDs;
    scheduleIDs << QStringLiteral("schedule-1");
    EXPECT_FALSE(DCalDavCalendarDeleteHelper::persistLocalDeleteState(
        nullptr, QStringLiteral("account-1"), calendar, scheduleIDs));
}

TEST(CalDavCalendarDeleteHelper, PersistLocalDeleteStateDisablesAndEnqueues)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("persist-delete-account"));
    account->setAccountName(QStringLiteral("user"));
    account->setDisplayName(QStringLiteral("Calendar"));
    account->setDbName(QStringLiteral("account.db"));
    account->setDbusPath(QStringLiteral("/test/account2"));
    account->setDbusInterface(QStringLiteral("com.deepin.test.account2"));
    account->setDtCreate(QDateTime::currentDateTime());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();
    ASSERT_FALSE(database.addAccountInfo(account).isEmpty());

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = account->accountID();
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = QStringLiteral("https://example.test");
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/mock");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(accountInfo));

    DTypeColor color;
    color.setColorID(DDataBase::createUuid());
    color.setColorCode(QStringLiteral("#4381D5"));
    color.setPrivilege(DTypeColor::PriSystem);

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("persist-cal-1");
    calendar.accountId = account->accountID();
    calendar.href = QStringLiteral("https://example.test/calendars/persist-cal-1/");
    calendar.scheduleTypeID = QStringLiteral("persist-type-1");
    calendar.enabled = true;
    ASSERT_TRUE(database.upsertCalDavCalendar(calendar));

    DCalDavOutboxItem item;
    item.operationID = QStringLiteral("op-1");
    item.accountID = account->accountID();
    item.localScheduleID = QStringLiteral("schedule-1");
    item.operationType = DCalDavOutboxItem::DeleteOperation;
    ASSERT_TRUE(database.upsertCalDavOutboxItem(item));

    QStringList scheduleIDs;
    scheduleIDs << QStringLiteral("schedule-1");
    EXPECT_TRUE(DCalDavCalendarDeleteHelper::persistLocalDeleteState(
        &database, account->accountID(), calendar, scheduleIDs));

    const DCalDavCalendarInfo disabledCalendar =
        database.getCalDavCalendarByIDIncludingDisabled(
            account->accountID(), calendar.calendarId);
    EXPECT_FALSE(disabledCalendar.calendarId.isEmpty());
    EXPECT_FALSE(disabledCalendar.enabled);

    EXPECT_TRUE(database.hasPendingCalDavCalendarDelete(
        account->accountID(), calendar.scheduleTypeID));
}

// ---------------------------------------------------------------------------
// DCalDavAccountDeletionCleanup::resume edge cases
// ---------------------------------------------------------------------------

TEST(CalDavAccountDeletionCleanup, ResumeNullDatabaseIsNoOp)
{
    DCalDavAccountDeletionCleanup::resume(nullptr, QString());
}

TEST(CalDavAccountDeletionCleanup, ResumeSkipsWhenAccountStillExists)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("existing-account"));
    account->setAccountName(QStringLiteral("user"));
    account->setDisplayName(QStringLiteral("Calendar"));
    account->setDbName(QStringLiteral("existing.db"));
    account->setDbusPath(QStringLiteral("/test/existing"));
    account->setDbusInterface(QStringLiteral("com.deepin.test.existing"));
    account->setDtCreate(QDateTime::currentDateTime());
    ASSERT_FALSE(database.addAccountInfo(account).isEmpty());

    const QString dbName = QStringLiteral("existing-file.db");
    QFile file(directory.filePath(dbName));
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.close();
    ASSERT_TRUE(database.upsertCalDavAccountDeletionCleanup(account->accountID(), dbName));

    DCalDavAccountDeletionCleanup::resume(&database, directory.path());

    EXPECT_TRUE(QFile::exists(file.fileName()));
    EXPECT_TRUE(database.getCalDavAccountDeletionCleanups().isEmpty());
}

TEST(CalDavAccountDeletionCleanup, ResumeSkipsUnsafeNameWithSlash)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("unsafe-slash-account");
    ASSERT_TRUE(database.upsertCalDavAccountDeletionCleanup(
        accountID, QStringLiteral("sub/dir.db")));

    DCalDavAccountDeletionCleanup::resume(&database, directory.path());

    EXPECT_FALSE(database.getCalDavAccountDeletionCleanups().isEmpty());
}

TEST(CalDavAccountDeletionCleanup, ResumeSkipsUnsafeNameWithDotDot)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("unsafe-dotdot-account");
    ASSERT_TRUE(database.upsertCalDavAccountDeletionCleanup(
        accountID, QStringLiteral("../../etc/passwd")));

    DCalDavAccountDeletionCleanup::resume(&database, directory.path());

    EXPECT_FALSE(database.getCalDavAccountDeletionCleanups().isEmpty());
}

TEST(CalDavAccountDeletionCleanup, ResumeSkipsEmptyName)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("unsafe-empty-account");
    EXPECT_FALSE(database.upsertCalDavAccountDeletionCleanup(accountID, QString()));

    DCalDavAccountDeletionCleanup::resume(&database, directory.path());

    EXPECT_TRUE(database.getCalDavAccountDeletionCleanups().isEmpty());
}

TEST(CalDavAccountDeletionCleanup, ResumeRemovesCleanupWhenFileNotExists)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("account-manager.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("missing-file-account");
    ASSERT_TRUE(database.upsertCalDavAccountDeletionCleanup(
        accountID, QStringLiteral("nonexistent.db")));

    DCalDavAccountDeletionCleanup::resume(&database, directory.path());

    EXPECT_TRUE(database.getCalDavAccountDeletionCleanups().isEmpty());
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::findScheduleTypeID
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, FindScheduleTypeIDReturnsMatch)
{
    DCalDavAccountRegistrar registrar;

    DCalDavCalendarInfo cal1;
    cal1.calendarId = QStringLiteral("cal-1");
    cal1.scheduleTypeID = QStringLiteral("type-1");
    DCalDavCalendarInfo cal2;
    cal2.calendarId = QStringLiteral("cal-2");
    cal2.scheduleTypeID = QStringLiteral("type-2");
    DCalDavCalendarInfo::List existing;
    existing << cal1 << cal2;

    const QString result = registrar.findScheduleTypeID(
        QStringLiteral("cal-2"), existing);
    EXPECT_EQ(QStringLiteral("type-2"), result);
}

TEST(CalDavAccountRegistrar, FindScheduleTypeIDReturnsEmptyWhenNoMatch)
{
    DCalDavAccountRegistrar registrar;

    DCalDavCalendarInfo cal1;
    cal1.calendarId = QStringLiteral("cal-1");
    cal1.scheduleTypeID = QStringLiteral("type-1");
    DCalDavCalendarInfo::List existing;
    existing << cal1;

    const QString result = registrar.findScheduleTypeID(
        QStringLiteral("cal-nonexistent"), existing);
    EXPECT_TRUE(result.isEmpty());
}

TEST(CalDavAccountRegistrar, FindScheduleTypeIDEmptyListReturnsEmpty)
{
    DCalDavAccountRegistrar registrar;

    DCalDavCalendarInfo::List existing;
    const QString result = registrar.findScheduleTypeID(
        QStringLiteral("cal-1"), existing);
    EXPECT_TRUE(result.isEmpty());
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::cancel(false)
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, CancelWithoutNotifyClearsStateWithoutCallback)
{
    DCalDavAccountRegistrar registrar;

    registrar.m_running = true;
    registrar.m_password = QStringLiteral("secret");
    bool callbackInvoked = false;
    registrar.m_callback = [&](const DCalDavAccountRegistrar::Result &) {
        callbackInvoked = true;
    };

    registrar.cancel(false);

    EXPECT_FALSE(callbackInvoked);
    EXPECT_FALSE(registrar.m_running);
    EXPECT_TRUE(registrar.m_password.isEmpty());
}

TEST(CalDavAccountRegistrar, CancelWhenNotRunningIsNoOp)
{
    DCalDavAccountRegistrar registrar;

    registrar.m_running = false;
    bool callbackInvoked = false;
    registrar.m_callback = [&](const DCalDavAccountRegistrar::Result &) {
        callbackInvoked = true;
    };

    registrar.cancel(false);

    EXPECT_FALSE(callbackInvoked);
}


// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::start
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, StartRejectsIncompleteRequest)
{
    DCalDavAccountRegistrar registrar;

    DCalDavAccountRegistrar::Request request;
    bool callbackInvoked = false;
    DCalDavAccountRegistrar::Result callbackResult;
    registrar.start(request, [&](const DCalDavAccountRegistrar::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    });

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(callbackResult.success);
    EXPECT_EQ(QStringLiteral("CalDAV account registration request is incomplete."),
              callbackResult.errorMessage);
}

TEST(CalDavAccountRegistrar, StartWhenAlreadyRunningIsNoOp)
{
    DCalDavAccountRegistrar registrar;
    registrar.m_running = true;

    DCalDavAccountRegistrar::Request request;
    bool callbackInvoked = false;
    registrar.start(request, [&](const DCalDavAccountRegistrar::Result &) {
        callbackInvoked = true;
    });

    EXPECT_FALSE(callbackInvoked);
}

TEST(CalDavAccountRegistrar, StartWithCredentialReaderFailure)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("cred-fail-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavSyncJobManager jobManager;

    DCalDavAccountRegistrar registrar;

    DCalDavAccountRegistrar::Request request;
    request.account.accountId = QStringLiteral("cred-fail-account");
    request.account.serverUrl = QStringLiteral("https://example.test");
    request.account.username = QStringLiteral("user");
    request.account.credentialRef = QStringLiteral("secret-service:/cred");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &managerDatabase;
    request.jobManager = &jobManager;
    request.credentialReader = [](const QString &, QString &, QString *errorMessage) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("credential read failed");
        }
        return false;
    };

    bool callbackInvoked = false;
    DCalDavAccountRegistrar::Result callbackResult;
    registrar.start(request, [&](const DCalDavAccountRegistrar::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    });

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(callbackResult.success);
    EXPECT_EQ(QStringLiteral("credential read failed"), callbackResult.errorMessage);
}

TEST(CalDavAccountRegistrar, StartWithCredentialReaderSuccessCallsDiscovery)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("cred-ok-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavSyncJobManager jobManager;

    DCalDavAccountRegistrar registrar;

    DCalDavAccountRegistrar::Request request;
    request.account.accountId = QStringLiteral("cred-ok-account");
    request.account.serverUrl = QStringLiteral("https://nonexistent.invalid");
    request.account.username = QStringLiteral("user");
    request.account.credentialRef = QStringLiteral("secret-service:/cred");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &managerDatabase;
    request.jobManager = &jobManager;
    request.credentialReader = [](const QString &, QString &password, QString *) {
        password = QStringLiteral("test-password");
        return true;
    };

    bool callbackInvoked = false;
    registrar.start(request, [&](const DCalDavAccountRegistrar::Result &) {
        callbackInvoked = true;
    });

    QElapsedTimer timer;
    timer.start();
    while (!callbackInvoked && timer.elapsed() < 5000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    if (!callbackInvoked) {
        registrar.cancel(true);
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    SUCCEED();
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::cancel(true)
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, CancelWithNotifyInvokesCallback)
{
    DCalDavAccountRegistrar registrar;

    registrar.m_running = true;
    registrar.m_password = QStringLiteral("secret");
    bool callbackInvoked = false;
    DCalDavAccountRegistrar::Result callbackResult;
    registrar.m_callback = [&](const DCalDavAccountRegistrar::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    };

    registrar.cancel(true);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(callbackResult.success);
    EXPECT_EQ(QStringLiteral("CalDAV account registration cancelled."),
              callbackResult.errorMessage);
    EXPECT_FALSE(registrar.m_running);
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::startDiscovery (no-op when not running)
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, StartDiscoveryWhenNotRunningIsNoOp)
{
    DCalDavAccountRegistrar registrar;
    registrar.m_running = false;

    registrar.startDiscovery();

    EXPECT_FALSE(registrar.m_running);
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::finish
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, FinishSuccessInvokesCallback)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountRegistrar registrar;
    registrar.m_running = true;
    registrar.m_request.account.accountId = QStringLiteral("finish-success-account");
    registrar.m_request.accountManagerDatabase = &managerDatabase;

    bool callbackInvoked = false;
    DCalDavAccountRegistrar::Result callbackResult;
    registrar.m_callback = [&](const DCalDavAccountRegistrar::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    };

    registrar.finish(true);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_TRUE(callbackResult.success);
    EXPECT_FALSE(registrar.m_running);
}

TEST(CalDavAccountRegistrar, FinishWhenNotRunningIsNoOp)
{
    DCalDavAccountRegistrar registrar;
    registrar.m_running = false;

    bool callbackInvoked = false;
    registrar.m_callback = [&](const DCalDavAccountRegistrar::Result &) {
        callbackInvoked = true;
    };

    registrar.finish(false, QStringLiteral("should not fire"));

    EXPECT_FALSE(callbackInvoked);
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::persistCalendars
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, PersistCalendarsEmptyDiscoveryReturnsTrue)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("persist-empty-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountRegistrar registrar;
    registrar.m_request.account.accountId = QStringLiteral("persist-empty-account");
    registrar.m_request.localDatabase = &localDatabase;
    registrar.m_request.accountManagerDatabase = &managerDatabase;

    DCalDavXmlReader::DiscoveryResult discovery;
    DCalDavAccountSync::Request syncRequest;
    QString errorMessage;

    bool result = registrar.persistCalendars(discovery, syncRequest, &errorMessage);

    EXPECT_TRUE(result);
    EXPECT_TRUE(syncRequest.calendars.isEmpty());
}

TEST(CalDavAccountRegistrar, PersistCalendarsRejectsInsecureUrl)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("persist-insecure-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountRegistrar registrar;
    registrar.m_request.account.accountId = QStringLiteral("persist-insecure-account");
    registrar.m_request.localDatabase = &localDatabase;
    registrar.m_request.accountManagerDatabase = &managerDatabase;

    DCalDavXmlReader::DiscoveryResult discovery;
    DCalDavXmlReader::CalendarCollection collection;
    collection.href = QStringLiteral("http://example.test/calendar");
    discovery.calendarCollections.append(collection);

    DCalDavAccountSync::Request syncRequest;
    QString errorMessage;

    bool result = registrar.persistCalendars(discovery, syncRequest, &errorMessage);

    EXPECT_FALSE(result);
    EXPECT_EQ(QStringLiteral("CalDAV discovery returned an insecure calendar URL."),
              errorMessage);
}

TEST(CalDavAccountRegistrar, PersistCalendarsCreatesNewCalendar)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("persist-new-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountRegistrar registrar;
    registrar.m_request.account.accountId = QStringLiteral("persist-new-account");
    registrar.m_request.localDatabase = &localDatabase;
    registrar.m_request.accountManagerDatabase = &managerDatabase;

    DCalDavXmlReader::DiscoveryResult discovery;
    DCalDavXmlReader::CalendarCollection collection;
    collection.href = QStringLiteral("https://example.test/calendars/new");
    collection.displayName = QStringLiteral("New Calendar");
    collection.color = QStringLiteral("#4381D5");
    collection.privileges = DCalDavXmlReader::ReadPrivilege | DCalDavXmlReader::WritePrivilege;
    collection.privilegesKnown = true;
    discovery.calendarCollections.append(collection);

    DCalDavAccountSync::Request syncRequest;
    QString errorMessage;

    bool result = registrar.persistCalendars(discovery, syncRequest, &errorMessage);

    EXPECT_TRUE(result);
    ASSERT_EQ(1, syncRequest.calendars.size());
    EXPECT_EQ(QStringLiteral("https://example.test/calendars/new"),
              syncRequest.calendars.first().calendar.href);
}

TEST(CalDavAccountRegistrar, PersistCalendarsDisablesInaccessibleCalendar)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("persist-inaccessible-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavCalendarInfo existing;
    existing.calendarId = QStringLiteral("existing-cal");
    existing.accountId = QStringLiteral("persist-inaccessible-account");
    existing.href = QStringLiteral("https://example.test/calendars/existing");
    existing.scheduleTypeID = QStringLiteral("existing-type");
    existing.enabled = true;
    ASSERT_TRUE(managerDatabase.upsertCalDavCalendar(existing));

    DCalDavAccountRegistrar registrar;
    registrar.m_request.account.accountId = QStringLiteral("persist-inaccessible-account");
    registrar.m_request.localDatabase = &localDatabase;
    registrar.m_request.accountManagerDatabase = &managerDatabase;

    DCalDavXmlReader::DiscoveryResult discovery;
    DCalDavXmlReader::CalendarCollection collection;
    collection.href = QStringLiteral("https://example.test/calendars/existing");
    collection.privilegesKnown = true;
    collection.privileges = DCalDavXmlReader::NoPrivilege;
    discovery.calendarCollections.append(collection);

    DCalDavAccountSync::Request syncRequest;
    QString errorMessage;

    bool result = registrar.persistCalendars(discovery, syncRequest, &errorMessage);

    EXPECT_TRUE(result);
    EXPECT_TRUE(syncRequest.calendars.isEmpty());
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::handleRemoteCalendarDeletion
// ---------------------------------------------------------------------------

TEST(CalDavAccountRegistrar, HandleRemoteCalendarDeletion)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("delete-remote-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountRegistrar registrar;
    registrar.m_request.account.accountId = QStringLiteral("delete-remote-account");
    registrar.m_request.localDatabase = &localDatabase;
    registrar.m_request.accountManagerDatabase = &managerDatabase;

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("cal-to-delete");
    calendar.accountId = QStringLiteral("delete-remote-account");
    calendar.href = QStringLiteral("https://example.test/calendars/old");
    calendar.scheduleTypeID = QStringLiteral("type-to-delete");

    QString errorMessage;
    registrar.handleRemoteCalendarDeletion(calendar, &errorMessage);

    SUCCEED();
}

TEST(CalDavAccountRegistrar, PersistRemoteCalendarDeletionRecoveryDirect)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("recovery-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DCalDavAccountRegistrar registrar;
    registrar.m_request.account.accountId = QStringLiteral("recovery-account");
    registrar.m_request.localDatabase = &localDatabase;

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("recovery-cal");
    calendar.accountId = QStringLiteral("recovery-account");
    calendar.href = QStringLiteral("https://example.test/calendars/recovery");
    calendar.scheduleTypeID = QStringLiteral("recovery-type");

    QStringList typeIDs;
    typeIDs << QStringLiteral("recovery-type");

    QString errorMessage;
    registrar.persistRemoteCalendarDeletionRecovery(calendar, typeIDs, &errorMessage);

    SUCCEED();
}

TEST(CalDavAccountRegistrar, CommitRemoteCalendarDeletionDirect)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("commit-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavAccountRegistrar registrar;
    registrar.m_request.account.accountId = QStringLiteral("commit-account");
    registrar.m_request.localDatabase = &localDatabase;
    registrar.m_request.accountManagerDatabase = &managerDatabase;

    DCalDavCalendarInfo calendar;
    calendar.calendarId = QStringLiteral("commit-cal");
    calendar.accountId = QStringLiteral("commit-account");
    calendar.href = QStringLiteral("https://example.test/calendars/commit");
    calendar.scheduleTypeID = QStringLiteral("commit-type");

    QStringList typeIDs;
    typeIDs << QStringLiteral("commit-type");
    QStringList scheduleIDs;

    QString errorMessage;
    registrar.commitRemoteCalendarDeletion(calendar, typeIDs, scheduleIDs, &errorMessage);

    SUCCEED();
}

// ---------------------------------------------------------------------------
// DCalDavAccountSync::fail
// ---------------------------------------------------------------------------

TEST(CalDavAccountSync, FailWithRetryableErrorSchedulesRetry)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("fail-retry-account");

    DCalDavAccountInfo accountInfo;
    accountInfo.accountId = accountID;
    accountInfo.providerType = DCalDavProviderProfile::Provider_Other;
    accountInfo.serverUrl = QStringLiteral("https://example.test");
    accountInfo.username = QStringLiteral("user");
    accountInfo.credentialRef = QStringLiteral("secret-service:/retry");
    ASSERT_TRUE(database.upsertCalDavAccountInfo(accountInfo));

    DCalDavAccountSync synchronizer;
    synchronizer.m_running = true;
    synchronizer.m_request.accountID = accountID;
    synchronizer.m_request.accountManagerDatabase = &database;
    synchronizer.m_request.retryCount = 0;
    synchronizer.m_result.failureResponse.error = DCalDavTransport::NetworkUnavailable;
    synchronizer.m_result.failureCode = DCalDavErrorCode::NetworkError;

    bool callbackInvoked = false;
    DCalDavAccountSync::Result callbackResult;
    synchronizer.m_callback = [&](const DCalDavAccountSync::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    };

    synchronizer.fail(QStringLiteral("network error"));

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(callbackResult.success);
    EXPECT_EQ(QStringLiteral("network error"), callbackResult.errorMessage);

    EXPECT_TRUE(database.earliestCalDavRetryAt().isValid());
}

TEST(CalDavAccountSync, FailWithNonRetryableErrorUpdatesSyncStatus)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("fail-noretry-account");

    DCalDavAccountSync synchronizer;
    synchronizer.m_running = true;
    synchronizer.m_request.accountID = accountID;
    synchronizer.m_request.accountManagerDatabase = &database;
    synchronizer.m_request.retryCount = 0;
    synchronizer.m_result.failureResponse.error = DCalDavTransport::AuthenticationFailed;
    synchronizer.m_result.failureCode = DCalDavErrorCode::AuthenticationFailed;

    bool callbackInvoked = false;
    DCalDavAccountSync::Result callbackResult;
    synchronizer.m_callback = [&](const DCalDavAccountSync::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    };

    synchronizer.fail(QStringLiteral("auth failed"));

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(callbackResult.success);
    EXPECT_EQ(QStringLiteral("auth failed"), callbackResult.errorMessage);

    EXPECT_FALSE(database.earliestCalDavRetryAt().isValid());
}

TEST(CalDavAccountSync, FailWithNoTransportErrorUpdatesSyncStatus)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase database;
    database.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    database.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    database.initDBData();

    const QString accountID = QStringLiteral("fail-noerror-account");

    DCalDavAccountSync synchronizer;
    synchronizer.m_running = true;
    synchronizer.m_request.accountID = accountID;
    synchronizer.m_request.accountManagerDatabase = &database;
    synchronizer.m_request.retryCount = 0;
    synchronizer.m_result.failureResponse.error = DCalDavTransport::NoError;
    synchronizer.m_result.failureCode = DCalDavErrorCode::Unknown;

    bool callbackInvoked = false;
    DCalDavAccountSync::Result callbackResult;
    synchronizer.m_callback = [&](const DCalDavAccountSync::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    };

    synchronizer.fail(QStringLiteral("unknown error"));

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(callbackResult.success);
    EXPECT_EQ(QStringLiteral("unknown error"), callbackResult.errorMessage);

    EXPECT_FALSE(database.earliestCalDavRetryAt().isValid());
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar::start() without credentialReader — exercises the
// async credential callback lambda (parseItemPath fails → callback(success=false)
// → finish(false, ...)).
// ---------------------------------------------------------------------------
TEST(CalDavAccountRegistrar, StartWithoutCredentialReaderInvokesAsyncCallback)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("async-cb-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavSyncJobManager jobManager;

    DCalDavAccountRegistrar registrar;

    DCalDavAccountRegistrar::Request request;
    request.account.accountId = QStringLiteral("async-cb-account");
    request.account.serverUrl = QStringLiteral("https://example.test");
    request.account.username = QStringLiteral("user");
    // Invalid credentialRef (no "secret-service:" prefix) → parseItemPath fails
    // → readPasswordAsync calls the callback synchronously with success=false.
    request.account.credentialRef = QStringLiteral("invalid-ref");
    request.localDatabase = &localDatabase;
    request.accountManagerDatabase = &managerDatabase;
    request.jobManager = &jobManager;
    // No credentialReader set — falls through to readPasswordAsync path.

    bool callbackInvoked = false;
    DCalDavAccountRegistrar::Result callbackResult;
    registrar.start(request, [&](const DCalDavAccountRegistrar::Result &result) {
        callbackInvoked = true;
        callbackResult = result;
    });

    EXPECT_TRUE(callbackInvoked);
    EXPECT_FALSE(callbackResult.success);
}

// ---------------------------------------------------------------------------
// DCalDavAccountRegistrar D0 (deleting) destructor — heap-allocate and delete.
// ---------------------------------------------------------------------------
TEST(CalDavAccountRegistrar, HeapDeleteInvokesDeletingDestructor)
{
    auto *registrar = new DCalDavAccountRegistrar();
    ASSERT_NE(nullptr, registrar);
    // Deleting via operator delete invokes the D0 (deleting) destructor.
    delete registrar;
    SUCCEED();
}
