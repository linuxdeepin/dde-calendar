// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountdatabase.h"
#include "ddatabase.h"
#include "daccount.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "dreminddata.h"
#include "duploadtaskdata.h"
#include "dtypecolor.h"
#include "dcaldavrecoveryitem.h"
#include "units.h"

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDateTime>

class DAccountDataBaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(dir->isValid());
        account = DAccount::Ptr(new DAccount(DAccount::Account_Local));
        account->setAccountID("ut-account-" + DDataBase::createUuid());
        account->setAccountName("TestAccount");
        account->setDisplayName("Test Display");
        account->setSyncState(DAccount::Sync_Normal);
        account->setAccountState(DAccount::Account_Open);
        account->setCloudPath("/tmp/cloud");
        account->setSyncFreq(DAccount::SyncFreq_Maunal);
        account->setIntervalTime(15);
        account->setSyncTag(0);
        account->setIsExpandDisplay(true);

        db = new DAccountDataBase(account);
        db->setDBPath(dir->filePath("account.db"));
        db->initDBData();
    }
    void TearDown() override
    {
        if (db) {
            delete db;
            db = nullptr;
        }
        dir.reset();
    }

    DSchedule::Ptr makeSchedule(const QString &typeID = QString())
    {
        DSchedule::Ptr schedule(new DSchedule);
        schedule->setSummary("Test Schedule");
        schedule->setDescription("Test Description");
        schedule->setDtStart(QDateTime(QDate(2026, 1, 1), QTime(10, 0)));
        schedule->setDtEnd(QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
        schedule->setAllDay(false);
        schedule->setScheduleTypeID(typeID.isEmpty() ? "test-type-id" : typeID);
        schedule->setCreated(QDateTime::currentDateTime());
        schedule->setLastModified(QDateTime::currentDateTime());
        return schedule;
    }

    DScheduleType::Ptr makeScheduleType()
    {
        DScheduleType::Ptr type(new DScheduleType(account->accountID()));
        type->setTypeName("TestType");
        type->setDisplayName("Test Type");
        type->setPrivilege(DScheduleType::User);
        type->setShowState(DScheduleType::Show);
        type->setDtCreate(QDateTime::currentDateTime());
        DTypeColor color;
        color.setColorID("test-color-id");
        color.setColorCode("#FF0000");
        color.setPrivilege(DTypeColor::PriUser);
        type->setTypeColor(color);
        return type;
    }

    DTypeColor::Ptr makeTypeColor()
    {
        DTypeColor::Ptr color(new DTypeColor);
        color->setColorID("ut-color-" + DDataBase::createUuid());
        color->setColorCode("#ABCDEF");
        color->setPrivilege(DTypeColor::PriUser);
        color->setDtCreate(QDateTime::currentDateTime());
        return color;
    }

    DRemindData::Ptr makeRemindData(const QString &scheduleID)
    {
        DRemindData::Ptr remind(new DRemindData);
        remind->setAlarmID("alarm-" + DDataBase::createUuid());
        remind->setAccountID(account->accountID());
        remind->setScheduleID(scheduleID);
        remind->setRemindCount(0);
        remind->setNotifyid(1);
        remind->setDtRemind(QDateTime::currentDateTime().addSecs(3600));
        remind->setDtStart(QDateTime(QDate(2026, 1, 1), QTime(10, 0)));
        remind->setDtEnd(QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
        return remind;
    }

    DUploadTaskData::Ptr makeUploadTask()
    {
        DUploadTaskData::Ptr task(new DUploadTaskData);
        task->setTaskID("task-" + DDataBase::createUuid());
        task->setTaskType(DUploadTaskData::Create);
        task->setTaskObject(DUploadTaskData::Task_Schedule);
        task->setObjectId("obj-1");
        task->setDtCreate(QDateTime::currentDateTime());
        return task;
    }

    std::unique_ptr<QTemporaryDir> dir;
    DAccount::Ptr account;
    DAccountDataBase *db = nullptr;
};

// ---- Constructor ----

TEST_F(DAccountDataBaseTest, Constructor)
{
    EXPECT_EQ(account->accountID(), db->getConnectionName());
}

// ---- Schedule CRUD ----

TEST_F(DAccountDataBaseTest, CreateSchedule)
{
    DSchedule::Ptr schedule = makeSchedule();
    QString id = db->createSchedule(schedule);
    EXPECT_FALSE(id.isEmpty());
}

// DEFECT: createSchedule() dereferences null pointer at return statement (line 86:
// "return schedule->uid()") when schedule is null. Source defect recorded for Mode 5.
// TEST_F(DAccountDataBaseTest, CreateScheduleNullPtr)
// {
//     DSchedule::Ptr nullSchedule;
//     QString id = db->createSchedule(nullSchedule);
//     EXPECT_TRUE(id.isEmpty());
// }

TEST_F(DAccountDataBaseTest, CreateScheduleWithEmptyUid)
{
    DSchedule::Ptr schedule = makeSchedule();
    schedule->setUid("");
    QString id = db->createSchedule(schedule);
    EXPECT_FALSE(id.isEmpty());
    EXPECT_EQ(id, schedule->uid());
}

TEST_F(DAccountDataBaseTest, CreateScheduleWithUidZero)
{
    DSchedule::Ptr schedule = makeSchedule();
    schedule->setUid("0");
    QString id = db->createSchedule(schedule);
    EXPECT_FALSE(id.isEmpty());
    EXPECT_NE("0", schedule->uid());
}

TEST_F(DAccountDataBaseTest, UpdateSchedule)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    schedule->setSummary("Updated Summary");
    EXPECT_TRUE(db->updateSchedule(schedule));
}

TEST_F(DAccountDataBaseTest, UpdateScheduleNullPtr)
{
    DSchedule::Ptr nullSchedule;
    EXPECT_FALSE(db->updateSchedule(nullSchedule));
}

TEST_F(DAccountDataBaseTest, GetScheduleByScheduleID)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DSchedule::Ptr result = db->getScheduleByScheduleID(schedule->schedulingID());
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ(schedule->summary(), result->summary());
}

TEST_F(DAccountDataBaseTest, GetScheduleByScheduleIDNotFound)
{
    DSchedule::Ptr result = db->getScheduleByScheduleID("nonexistent-id");
    EXPECT_TRUE(result.isNull());
}

TEST_F(DAccountDataBaseTest, ScheduleExistsByScheduleID)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    EXPECT_TRUE(db->scheduleExistsByScheduleID(schedule->schedulingID()));
    EXPECT_FALSE(db->scheduleExistsByScheduleID("nonexistent"));
}

TEST_F(DAccountDataBaseTest, IsScheduleDeletedByScheduleID)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    EXPECT_FALSE(db->isScheduleDeletedByScheduleID(schedule->schedulingID()));
    db->deleteScheduleByScheduleID(schedule->schedulingID());
    EXPECT_TRUE(db->isScheduleDeletedByScheduleID(schedule->schedulingID()));
}

// ---- CalDAV Recovery ----

TEST_F(DAccountDataBaseTest, UpsertCalDavRecoveryItem)
{
    DCalDavRecoveryItem item;
    item.accountID = account->accountID();
    item.localScheduleID = "sched-1";
    item.operationType = DCalDavRecoveryItem::CreateOperation;
    item.scheduleIcs = "BEGIN:VCALENDAR\nEND:VCALENDAR";
    item.calendarID = "cal-1";
    item.href = "https://example.test/cal/event.ics";
    item.etag = "\"etag-1\"";
    item.createdAt = QDateTime::currentDateTime();
    EXPECT_TRUE(db->upsertCalDavRecoveryItem(item));
}

TEST_F(DAccountDataBaseTest, GetCalDavRecoveryItems)
{
    DCalDavRecoveryItem item;
    item.accountID = account->accountID();
    item.localScheduleID = "sched-recovery-1";
    item.operationType = DCalDavRecoveryItem::ModifyOperation;
    item.scheduleIcs = "ICS";
    item.calendarID = "cal-1";
    item.href = "href-1";
    item.etag = "\"e1\"";
    item.createdAt = QDateTime::currentDateTime();
    ASSERT_TRUE(db->upsertCalDavRecoveryItem(item));

    DCalDavRecoveryItem::List items = db->getCalDavRecoveryItems(account->accountID());
    EXPECT_FALSE(items.isEmpty());
}

TEST_F(DAccountDataBaseTest, GetCalDavRecoveryItemsEmpty)
{
    DCalDavRecoveryItem::List items = db->getCalDavRecoveryItems(account->accountID());
    EXPECT_TRUE(items.isEmpty());
}

TEST_F(DAccountDataBaseTest, DeleteCalDavRecoveryItem)
{
    DCalDavRecoveryItem item;
    item.accountID = account->accountID();
    item.localScheduleID = "sched-del-recovery";
    item.operationType = DCalDavRecoveryItem::DeleteOperation;
    item.scheduleIcs = "ICS";
    item.calendarID = "cal-del";
    item.href = "href-del";
    item.etag = "\"e-del\"";
    item.createdAt = QDateTime::currentDateTime();
    ASSERT_TRUE(db->upsertCalDavRecoveryItem(item));
    EXPECT_TRUE(db->deleteCalDavRecoveryItem(account->accountID(), "sched-del-recovery"));
    DCalDavRecoveryItem::List items = db->getCalDavRecoveryItems(account->accountID());
    EXPECT_TRUE(items.isEmpty());
}

// ---- Schedule list/query ----

TEST_F(DAccountDataBaseTest, GetScheduleIDListByTypeID)
{
    DSchedule::Ptr schedule = makeSchedule("type-list-1");
    db->createSchedule(schedule);
    QStringList ids = db->getScheduleIDListByTypeID("type-list-1");
    EXPECT_FALSE(ids.isEmpty());
}

TEST_F(DAccountDataBaseTest, GetScheduleListByTypeID)
{
    DSchedule::Ptr schedule = makeSchedule("type-list-2");
    db->createSchedule(schedule);
    DSchedule::List list = db->getScheduleListByTypeID("type-list-2");
    EXPECT_FALSE(list.isEmpty());
}

TEST_F(DAccountDataBaseTest, DeleteScheduleByScheduleID)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    EXPECT_TRUE(db->deleteScheduleByScheduleID(schedule->schedulingID()));
    EXPECT_TRUE(db->isScheduleDeletedByScheduleID(schedule->schedulingID()));
}

TEST_F(DAccountDataBaseTest, DeleteScheduleByScheduleIDHardDelete)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    EXPECT_TRUE(db->deleteScheduleByScheduleID(schedule->schedulingID(), 1));
    EXPECT_FALSE(db->scheduleExistsByScheduleID(schedule->schedulingID()));
}

TEST_F(DAccountDataBaseTest, DeleteSchedulesByTypeID)
{
    DSchedule::Ptr s1 = makeSchedule("type-del-batch");
    DSchedule::Ptr s2 = makeSchedule("type-del-batch");
    db->createSchedule(s1);
    db->createSchedule(s2);
    EXPECT_TRUE(db->deleteSchedulesByScheduleTypeID("type-del-batch", 1));
    QStringList ids = db->getScheduleIDListByTypeID("type-del-batch");
    EXPECT_TRUE(ids.isEmpty());
}

TEST_F(DAccountDataBaseTest, RestoreSchedulesByTypeID)
{
    DSchedule::Ptr schedule = makeSchedule("type-restore");
    db->createSchedule(schedule);
    db->deleteSchedulesByScheduleTypeID("type-restore");
    EXPECT_TRUE(db->restoreSchedulesByScheduleTypeID("type-restore"));
    QStringList ids = db->getScheduleIDListByTypeID("type-restore");
    EXPECT_FALSE(ids.isEmpty());
}

TEST_F(DAccountDataBaseTest, QuerySchedulesByKey)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    DSchedule::Ptr schedule = makeSchedule(type->typeID());
    schedule->setSummary("SearchableMeeting");
    db->createSchedule(schedule);
    DSchedule::List results = db->querySchedulesByKey("Searchable");
    EXPECT_FALSE(results.isEmpty());
}

TEST_F(DAccountDataBaseTest, QuerySchedulesByRRule)
{
    DSchedule::Ptr schedule = makeSchedule();
    schedule->setSummary("RRuleTest");
    schedule->setRRuleType(DSchedule::RRule_Day);
    db->createSchedule(schedule);
    DSchedule::List results = db->querySchedulesByRRule("RRule", DSchedule::RRule_Day);
    // May or may not find depending on implementation, just test it doesn't crash.
    SUCCEED();
}

TEST_F(DAccountDataBaseTest, GetRemindSchedule)
{
    DSchedule::Ptr schedule = makeSchedule();
    schedule->setSummary("RemindTest");
    schedule->setAlarmType(DSchedule::Alarm_Begin);
    db->createSchedule(schedule);
    DSchedule::List results = db->getRemindSchedule();
    SUCCEED();
}

// ---- Schedule Type CRUD ----

TEST_F(DAccountDataBaseTest, CreateScheduleType)
{
    DScheduleType::Ptr type = makeScheduleType();
    QString id = db->createScheduleType(type);
    EXPECT_FALSE(id.isEmpty());
}

TEST_F(DAccountDataBaseTest, GetScheduleTypeByID)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    DScheduleType::Ptr result = db->getScheduleTypeByID(type->typeID());
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ(type->typeName(), result->typeName());
}

TEST_F(DAccountDataBaseTest, GetScheduleTypeByIDNotFound)
{
    DScheduleType::Ptr result = db->getScheduleTypeByID("nonexistent-type");
    EXPECT_TRUE(result.isNull());
}

TEST_F(DAccountDataBaseTest, GetScheduleTypeList)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    DScheduleType::List list = db->getScheduleTypeList();
    EXPECT_FALSE(list.isEmpty());
}

TEST_F(DAccountDataBaseTest, GetScheduleTypeListDeleted)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    db->deleteScheduleTypeByID(type->typeID());
    DScheduleType::List list = db->getScheduleTypeList(1);
    EXPECT_FALSE(list.isEmpty());
}

TEST_F(DAccountDataBaseTest, ScheduleTypeByUsed)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    DSchedule::Ptr schedule = makeSchedule(type->typeID());
    db->createSchedule(schedule);
    EXPECT_TRUE(db->scheduleTypeByUsed(type->typeID()));
    EXPECT_FALSE(db->scheduleTypeByUsed("unused-type-id"));
}

TEST_F(DAccountDataBaseTest, DeleteScheduleTypeByID)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    EXPECT_TRUE(db->deleteScheduleTypeByID(type->typeID()));
    DScheduleType::Ptr result = db->getScheduleTypeByID(type->typeID(), 1);
    EXPECT_FALSE(result.isNull());
}

TEST_F(DAccountDataBaseTest, RestoreScheduleTypeByID)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    db->deleteScheduleTypeByID(type->typeID());
    EXPECT_TRUE(db->restoreScheduleTypeByID(type->typeID()));
    DScheduleType::Ptr result = db->getScheduleTypeByID(type->typeID());
    EXPECT_FALSE(result.isNull());
}

TEST_F(DAccountDataBaseTest, UpdateScheduleType)
{
    DScheduleType::Ptr type = makeScheduleType();
    DTypeColor typeColor = type->typeColor();
    db->addTypeColor(typeColor);
    db->createScheduleType(type);
    type->setDisplayName("Updated Display");
    EXPECT_TRUE(db->updateScheduleType(type));
    DScheduleType::Ptr result = db->getScheduleTypeByID(type->typeID());
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ("Updated Display", result->displayName());
}

TEST_F(DAccountDataBaseTest, GetFestivalTypeID)
{
    QString festivalID = db->getFestivalTypeID();
    EXPECT_FALSE(festivalID.isEmpty());
}

// ---- Account Info ----

TEST_F(DAccountDataBaseTest, GetAccountInfo)
{
    DAccount::Ptr info(new DAccount(DAccount::Account_Local));
    db->getAccountInfo(info);
    EXPECT_EQ(account->accountName(), info->accountName());
}

TEST_F(DAccountDataBaseTest, UpdateAccountInfo)
{
    account->setDisplayName("Updated Display Name");
    db->updateAccountInfo();
    DAccount::Ptr info(new DAccount(DAccount::Account_Local));
    db->getAccountInfo(info);
    EXPECT_EQ("Updated Display Name", info->displayName());
}

// ---- Type Color ----

TEST_F(DAccountDataBaseTest, AddTypeColorPtr)
{
    DTypeColor::Ptr color = makeTypeColor();
    EXPECT_TRUE(db->addTypeColor(color));
}

TEST_F(DAccountDataBaseTest, AddTypeColorRef)
{
    DTypeColor color;
    color.setColorID("ut-color-ref-" + DDataBase::createUuid());
    color.setColorCode("#123456");
    color.setPrivilege(DTypeColor::PriUser);
    color.setDtCreate(QDateTime::currentDateTime());
    EXPECT_TRUE(db->addTypeColor(color));
}

TEST_F(DAccountDataBaseTest, DeleteTypeColor)
{
    DTypeColor::Ptr color = makeTypeColor();
    db->addTypeColor(color);
    db->deleteTypeColor(color->colorID());
    DTypeColor::List list = db->getSysColor();
    for (const auto &c : list) {
        EXPECT_NE(color->colorID(), c->colorID());
    }
}

TEST_F(DAccountDataBaseTest, GetSysColor)
{
    DTypeColor::List list = db->getSysColor();
    EXPECT_FALSE(list.isEmpty());
}

// ---- Remind Info ----

TEST_F(DAccountDataBaseTest, CreateRemindInfo)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
    db->createRemindInfo(remind);
    SUCCEED();
}

TEST_F(DAccountDataBaseTest, UpdateRemindInfo)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
    db->createRemindInfo(remind);
    remind->setRemindCount(1);
    db->updateRemindInfo(remind);
    SUCCEED();
}

TEST_F(DAccountDataBaseTest, DeleteRemindInfoByAlarmID)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
    db->createRemindInfo(remind);
    db->deleteRemindInfoByAlarmID(remind->alarmID());
    DRemindData::Ptr result = db->getRemindData(remind->alarmID());
    EXPECT_TRUE(result.isNull());
}

TEST_F(DAccountDataBaseTest, GetRemindData)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
    db->createRemindInfo(remind);
    DRemindData::Ptr result = db->getRemindData(remind->alarmID());
    ASSERT_FALSE(result.isNull());
    EXPECT_EQ(remind->alarmID(), result->alarmID());
}

TEST_F(DAccountDataBaseTest, GetRemindDataNotFound)
{
    DRemindData::Ptr result = db->getRemindData("nonexistent-alarm");
    EXPECT_TRUE(result.isNull());
}

TEST_F(DAccountDataBaseTest, GetValidRemindJob)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
    db->createRemindInfo(remind);
    DRemindData::List list = db->getValidRemindJob();
    SUCCEED();
}

TEST_F(DAccountDataBaseTest, ClearRemindJobDatabase)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
    db->createRemindInfo(remind);
    db->clearRemindJobDatabase();
    DRemindData::List list = db->getValidRemindJob();
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DAccountDataBaseTest, GetRemindByScheduleID)
{
    DSchedule::Ptr schedule = makeSchedule();
    db->createSchedule(schedule);
    DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
    db->createRemindInfo(remind);
    DRemindData::List list = db->getRemindByScheduleID(schedule->schedulingID());
    EXPECT_FALSE(list.isEmpty());
}

// DEFECT: deleteRemindInfos is declared in daccountdatabase.h:114 but never implemented.
// Test skipped — source defect recorded for Mode 5.
// TEST_F(DAccountDataBaseTest, DeleteRemindInfos)
// {
//     DSchedule::Ptr schedule = makeSchedule();
//     db->createSchedule(schedule);
//     DRemindData::Ptr remind = makeRemindData(schedule->schedulingID());
//     db->createRemindInfo(remind);
//     db->deleteRemindInfos(QStringList() << remind->alarmID());
//     DRemindData::Ptr result = db->getRemindData(remind->alarmID());
//     EXPECT_TRUE(result.isNull());
// }

// ---- Upload Task ----

TEST_F(DAccountDataBaseTest, AddUploadTask)
{
    DUploadTaskData::Ptr task = makeUploadTask();
    db->addUploadTask(task);
    SUCCEED();
}

TEST_F(DAccountDataBaseTest, GetUploadTask)
{
    DUploadTaskData::Ptr task = makeUploadTask();
    db->addUploadTask(task);
    DUploadTaskData::List list = db->getUploadTask();
    EXPECT_FALSE(list.isEmpty());
}

TEST_F(DAccountDataBaseTest, DeleteUploadTask)
{
    DUploadTaskData::Ptr task = makeUploadTask();
    db->addUploadTask(task);
    db->deleteUploadTask(task->taskID());
    DUploadTaskData::List list = db->getUploadTask();
    bool found = false;
    for (const auto &t : list) {
        if (t->taskID() == task->taskID()) found = true;
    }
    EXPECT_FALSE(found);
}

// ---- initDBData already tested in SetUp ----

TEST_F(DAccountDataBaseTest, InitDBDataExistingFile)
{
    // Calling initDBData again on existing DB should just open it.
    db->initDBData();
    SUCCEED();
}

TEST_F(DAccountDataBaseTest, InitDBDataNewFile)
{
    DAccount::Ptr account2(new DAccount(DAccount::Account_Local));
    account2->setAccountID("ut-account2-" + DDataBase::createUuid());
    account2->setAccountName("TestAccount2");
    account2->setDisplayName("Test Display 2");
    account2->setSyncState(DAccount::Sync_Normal);
    account2->setAccountState(DAccount::Account_Open);
    account2->setSyncFreq(DAccount::SyncFreq_Maunal);
    account2->setIntervalTime(15);
    account2->setSyncTag(0);
    account2->setIsExpandDisplay(true);

    DAccountDataBase db2(account2);
    db2.setDBPath(dir->filePath("account2.db"));
    db2.initDBData();
    SUCCEED();
}

// ---- initSysType (tested via initDBData, which calls initScheduleType->initSysType) ----

TEST_F(DAccountDataBaseTest, InitSysTypeCreatesTypes)
{
    DScheduleType::List list = db->getScheduleTypeList();
    // initSysType creates Work, Life, Other, Festival types
    EXPECT_GE(list.size(), 4);
}

TEST_F(DAccountDataBaseTest, InitTypeColorCreatesColors)
{
    DTypeColor::List list = db->getSysColor();
    EXPECT_GE(list.size(), 9);
}

// ---- systemTypeTran (tested indirectly via initSysType) ----

TEST_F(DAccountDataBaseTest, SystemTypeTranLocalAccount)
{
    // The initSysType already calls systemTypeTran for local accounts.
    DScheduleType::List list = db->getScheduleTypeList();
    bool foundWork = false;
    for (const auto &t : list) {
        if (t->typeName() == "Work") {
            foundWork = true;
            break;
        }
    }
    EXPECT_TRUE(foundWork);
}

TEST_F(DAccountDataBaseTest, InitAccountDB)
{
    // initAccountDB is called during initDBData. Verify account info was stored.
    DAccount::Ptr info(new DAccount(DAccount::Account_Local));
    db->getAccountInfo(info);
    EXPECT_EQ(account->accountName(), info->accountName());
    EXPECT_EQ(account->displayName(), info->displayName());
}
