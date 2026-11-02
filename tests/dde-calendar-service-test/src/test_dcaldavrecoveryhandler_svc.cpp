// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavrecoveryhandler.h"
#include "dcaldavrecoveryitem.h"
#include "ddatabase.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "dschedule.h"
#include "daccount.h"

#include <QTemporaryDir>

#include <gtest/gtest.h>

TEST(CalDavRecoveryHandlerSvc, NullDatabasesReturnWithoutCrash)
{
    DCalDavRecoveryHandler::recover(nullptr, nullptr, QStringLiteral("account-1"));
}

TEST(CalDavRecoveryHandlerSvc, NullLocalDatabaseReturnsWithoutCrash)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavRecoveryHandler::recover(nullptr, &managerDatabase, QStringLiteral("account-1"));
}

TEST(CalDavRecoveryHandlerSvc, NullManagerDatabaseReturnsWithoutCrash)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("null-manager-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DCalDavRecoveryHandler::recover(&localDatabase, nullptr, account->accountID());
}

TEST(CalDavRecoveryHandlerSvc, EmptyAccountIDReturnsWithoutCrash)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("empty-account-test"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, QString());
}

TEST(CalDavRecoveryHandlerSvc, NoRecoveryItemsIsNoOp)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("no-items-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, account->accountID());

    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
}

TEST(CalDavRecoveryHandlerSvc, RecoversLocalCreateOperationWhenLocalScheduleNeverCommitted)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    DAccount::Ptr account(new DAccount(DAccount::Account_CalDav));
    account->setAccountID(QStringLiteral("localcreate-rollback-account"));
    DAccountDataBase localDatabase(account);
    localDatabase.setDBPath(directory.filePath(QStringLiteral("local.db")));
    localDatabase.initDBData();

    DAccountManagerDataBase managerDatabase;
    managerDatabase.setDBPath(directory.filePath(QStringLiteral("manager.db")));
    managerDatabase.setLoaclDB(directory.filePath(QStringLiteral("local.db")));
    managerDatabase.initDBData();

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setUid(QStringLiteral("localcreate-rollback-schedule"));
    schedule->setSummary(QStringLiteral("Local create rollback"));
    schedule->setDtStart(QDateTime(QDate(2026, 10, 30), QTime(10, 0), Qt::UTC));
    schedule->setDtEnd(QDateTime(QDate(2026, 10, 30), QTime(11, 0), Qt::UTC));

    DCalDavRecoveryItem recovery;
    recovery.accountID = account->accountID();
    recovery.localScheduleID = schedule->uid();
    recovery.operationType = DCalDavRecoveryItem::LocalCreateOperation;
    recovery.scheduleIcs = DSchedule::toIcsString(schedule);
    ASSERT_TRUE(localDatabase.upsertCalDavRecoveryItem(recovery));

    DCalDavRecoveryHandler::recover(&localDatabase, &managerDatabase, account->accountID());

    EXPECT_TRUE(localDatabase.getCalDavRecoveryItems(account->accountID()).isEmpty());
}
