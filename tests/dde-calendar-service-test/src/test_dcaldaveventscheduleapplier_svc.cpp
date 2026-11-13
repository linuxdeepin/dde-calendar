// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldaveventscheduleapplier.h"
#include "dcaldaveventmappinginfo.h"
#include "daccountdatabase.h"
#include "daccountmanagerdatabase.h"
#include "daccount.h"
#include "dschedule.h"
#include "dscheduletype.h"
#include "dtypecolor.h"

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>

static const char *kValidIcs =
    "BEGIN:VCALENDAR\r\n"
    "VERSION:2.0\r\n"
    "PRODID:-//Test//Test//EN\r\n"
    "BEGIN:VEVENT\r\n"
    "UID:test-uid-123\r\n"
    "DTSTAMP:20240101T000000Z\r\n"
    "DTSTART:20240101T090000Z\r\n"
    "DTEND:20240101T100000Z\r\n"
    "SUMMARY:Applier Event\r\n"
    "END:VEVENT\r\n"
    "END:VCALENDAR\r\n";

static const char *kBrokenIcs = "NOT-A-CALENDAR";

/**
 * DCalDavEventScheduleApplier::applyUpdate / applyDelete 直测。
 * 两个方法为 private static，工程已加 -fno-access-control 可直接调用。
 */
class DCalDavEventScheduleApplierTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(dir->isValid());

        account = DAccount::Ptr(new DAccount(DAccount::Account_Local));
        account->setAccountID("ut-applier-account");
        account->setDbName("ut-applier.db");

        localDB = new DAccountDataBase(account);
        localDB->setDBPath(dir->filePath("account.db"));
        localDB->initDBData();

        mgrDB = new DAccountManagerDataBase;
        mgrDB->setDBPath(dir->filePath("manager.db"));
        mgrDB->setLoaclDB(dir->filePath("local.db"));
        mgrDB->initDBData();

        DScheduleType::Ptr type(new DScheduleType(account->accountID()));
        type->setTypeName("ApplierType");
        type->setDisplayName("Applier");
        type->setPrivilege(DScheduleType::User);
        type->setShowState(DScheduleType::Show);
        type->setDtCreate(QDateTime::currentDateTime());
        DTypeColor::Ptr color(new DTypeColor);
        color->setColorID("ut-applier-color");
        color->setColorCode("#00FF00");
        color->setPrivilege(DTypeColor::PriUser);
        // getScheduleTypeByID 对 typeColor 做 INNER JOIN，必须先插入颜色行
        ASSERT_TRUE(localDB->addTypeColor(color));
        type->setTypeColor(*color.data());
        typeID = localDB->createScheduleType(type);
        ASSERT_FALSE(typeID.isEmpty());

        scheduleID = localDB->createSchedule(makeLocalSchedule(typeID));
        ASSERT_FALSE(scheduleID.isEmpty());
    }

    void TearDown() override
    {
        if (localDB) {
            delete localDB;
            localDB = nullptr;
        }
        if (mgrDB) {
            delete mgrDB;
            mgrDB = nullptr;
        }
        // DAccountDataBase 以 accountID 作为 Qt SQL 连接名（固定 "ut-applier-account"），
        // 主动释放连接避免用例间复用旧连接/残留状态
        if (QSqlDatabase::contains("ut-applier-account"))
            QSqlDatabase::removeDatabase("ut-applier-account");
        dir.reset();
    }

    DSchedule::Ptr makeLocalSchedule(const QString &type)
    {
        DSchedule::Ptr s(new DSchedule);
        s->setSummary("Local Schedule");
        s->setDescription("Applier UT");
        s->setDtStart(QDateTime(QDate(2026, 1, 1), QTime(10, 0)));
        s->setDtEnd(QDateTime(QDate(2026, 1, 1), QTime(11, 0)));
        s->setAllDay(false);
        s->setScheduleTypeID(type);
        s->setCreated(QDateTime::currentDateTime());
        s->setLastModified(QDateTime::currentDateTime());
        return s;
    }

    DCalDavEventScheduleApplier::Request makeRequest()
    {
        DCalDavEventScheduleApplier::Request req;
        req.accountID = account->accountID();
        req.calendarID = "ut-calendar";
        req.scheduleTypeID = typeID;
        req.calendarPrivileges = 0;   // 无写权限 → 类型会被降为 Read 并更新
        req.localDatabase = localDB;
        req.accountManagerDatabase = mgrDB;
        return req;
    }

    DCalDavEventReconciler::Action makeAction(const char *ics, const QString &href)
    {
        DCalDavEventReconciler::Action action;
        action.type = DCalDavEventReconciler::UpdateAction;
        action.remoteEvent.href = href;
        action.remoteEvent.uid = "test-uid-123";
        action.remoteEvent.etag = "\"updated-etag\"";
        action.remoteEvent.calendarData = QString::fromLatin1(ics);
        action.existingMapping.localScheduleID = scheduleID;
        action.existingMapping.accountID = account->accountID();
        action.existingMapping.calendarID = "ut-calendar";
        action.existingMapping.uid = "test-uid-123";
        action.existingMapping.href = "/cal/old.ics";
        action.existingMapping.etag = "\"old-etag\"";
        return action;
    }

    // 预置一条旧映射，保证 mapSchedule 中对旧 href 的 delete 返回 true
    void upsertOldMapping(const DCalDavEventReconciler::Action &action)
    {
        ASSERT_TRUE(mgrDB->upsertCalDavEventMapping(action.existingMapping));
    }

    std::unique_ptr<QTemporaryDir> dir;
    DAccount::Ptr account;
    DAccountDataBase *localDB = nullptr;
    DAccountManagerDataBase *mgrDB = nullptr;
    QString typeID;
    QString scheduleID;
};

// ---------------------------------------------------------------------------
// applyUpdate
// ---------------------------------------------------------------------------

TEST_F(DCalDavEventScheduleApplierTest, ApplyUpdate_EmptyMappingID_ReturnsFalse)
{
    auto request = makeRequest();
    auto action = makeAction(kValidIcs, "/cal/e1.ics");
    action.existingMapping.localScheduleID.clear();

    DCalDavEventScheduleApplier::Result result;
    EXPECT_FALSE(DCalDavEventScheduleApplier::applyUpdate(request, action, result));
    EXPECT_FALSE(result.errorMessage.isEmpty());
}

TEST_F(DCalDavEventScheduleApplierTest, ApplyUpdate_LocalScheduleMissing_ReturnsFalse)
{
    auto request = makeRequest();
    auto action = makeAction(kValidIcs, "/cal/e1.ics");
    action.existingMapping.localScheduleID = "nonexistent-schedule-id";

    DCalDavEventScheduleApplier::Result result;
    EXPECT_FALSE(DCalDavEventScheduleApplier::applyUpdate(request, action, result));
    EXPECT_TRUE(result.errorMessage.contains("not found"));
}

TEST_F(DCalDavEventScheduleApplierTest, ApplyUpdate_InvalidIcs_ReturnsFalse)
{
    auto request = makeRequest();
    auto action = makeAction(kBrokenIcs, "/cal/e1.ics");
    upsertOldMapping(action);

    DCalDavEventScheduleApplier::Result result;
    EXPECT_FALSE(DCalDavEventScheduleApplier::applyUpdate(request, action, result));
    EXPECT_EQ(result.updatedCount, 0);
}

TEST_F(DCalDavEventScheduleApplierTest, ApplyUpdate_Success_UpdatesScheduleAndMapping)
{
    auto request = makeRequest();
    auto action = makeAction(kValidIcs, "/cal/e1.ics");
    upsertOldMapping(action);

    DCalDavEventScheduleApplier::Result result;
    EXPECT_TRUE(DCalDavEventScheduleApplier::applyUpdate(request, action, result))
        << "err=" << result.errorMessage.toStdString();
    EXPECT_EQ(result.updatedCount, 1);

    // 本地日程应已更新为新摘要
    DSchedule::Ptr updated = localDB->getScheduleByScheduleID(scheduleID);
    ASSERT_FALSE(updated.isNull());
    EXPECT_EQ(updated->summary(), QStringLiteral("Applier Event"));
    // 类型权限应被降级为 Read
    DScheduleType::Ptr type = localDB->getScheduleTypeByID(typeID);
    ASSERT_FALSE(type.isNull());
    EXPECT_EQ(type->privilege(), DScheduleType::Read);
    // 新映射应已写入
    DCalDavEventMappingInfo mapping = mgrDB->getCalDavEventMapping(
        account->accountID(), QStringLiteral("/cal/e1.ics"));
    EXPECT_EQ(mapping.localScheduleID, scheduleID);
}

// ---------------------------------------------------------------------------
// applyDelete
// ---------------------------------------------------------------------------

TEST_F(DCalDavEventScheduleApplierTest, ApplyDelete_EmptyMappingID_ReturnsFalse)
{
    auto request = makeRequest();
    auto action = makeAction(kValidIcs, "/cal/e1.ics");
    action.existingMapping.localScheduleID.clear();

    DCalDavEventScheduleApplier::Result result;
    EXPECT_FALSE(DCalDavEventScheduleApplier::applyDelete(request, action, result));
    EXPECT_FALSE(result.errorMessage.isEmpty());
}

TEST_F(DCalDavEventScheduleApplierTest, ApplyDelete_MappingDeleteFails_ReturnsFalse)
{
    auto request = makeRequest();
    auto action = makeAction(kValidIcs, "/cal/missing.ics");
    // 不预置映射 → deleteCalDavEventMapping 无行可删，走失败分支

    DCalDavEventScheduleApplier::Result result;
    EXPECT_FALSE(DCalDavEventScheduleApplier::applyDelete(request, action, result));
    EXPECT_EQ(result.deletedCount, 0);
}

TEST_F(DCalDavEventScheduleApplierTest, ApplyDelete_Success_RemovesScheduleAndMapping)
{
    auto request = makeRequest();
    auto action = makeAction(kValidIcs, "/cal/e1.ics");
    action.existingMapping.href = "/cal/e1.ics";
    upsertOldMapping(action);

    DCalDavEventScheduleApplier::Result result;
    EXPECT_TRUE(DCalDavEventScheduleApplier::applyDelete(request, action, result))
        << "err=" << result.errorMessage.toStdString();
    EXPECT_EQ(result.deletedCount, 1);

    DSchedule::Ptr gone = localDB->getScheduleByScheduleID(scheduleID);
    ASSERT_FALSE(gone.isNull());
    // deleteScheduleByScheduleID 默认为软删除（isDeleted=1），行仍在
    QSqlDatabase conn = QSqlDatabase::database(account->accountID());
    ASSERT_TRUE(conn.isValid());
    QSqlQuery query(conn);
    ASSERT_TRUE(query.exec(QString("SELECT isDeleted FROM schedules WHERE scheduleID = '%1'")
                               .arg(scheduleID)));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}
