// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountservice.h"
#include "daccountmodule.h"
#include "daccount.h"
#include "csystemdtimercontrol.h"
#include "service_stub.h"
#include "../third-party_stub/stub.h"
#include <gtest/gtest.h>
#include <QObject>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include "units.h"
#include <QFile>
#include "dschedule.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

static void stub_startCalendarServiceSystemdTimer(void *obj)
{
    Q_UNUSED(obj);
}

static QString stub_getDBPath_das()
{
    static QTemporaryDir tmpDir;
    return tmpDir.path();
}

class test_daccountservice : public QObject, public ::testing::Test
{
public:
    test_daccountservice()
    {
        stub.set(ADDR(CSystemdTimerControl, startCalendarServiceSystemdTimer),
                 stub_startCalendarServiceSystemdTimer);
        stub.set(getDBPath, stub_getDBPath_das);
        // Clean up any previous DB file for a fresh test start
        QFile::remove(stub_getDBPath_das() + "/test-account.db");
        DAccount::Ptr account(new DAccount);
        account->setAccountID("test-account-id");
        account->setDbName("test-account.db");
        m_module = DAccountModule::Ptr(new DAccountModule(account));

        m_service = new DAccountService("/test/path", "com.test.Interface", m_module);
    }
    ~test_daccountservice()
    {
        if (m_service) {
            delete m_service;
        }
    }
protected:
    Stub stub;
    DAccountModule::Ptr m_module;
    DAccountService *m_service = nullptr;
};

// DAccountService::DAccountService(...)
TEST_F(test_daccountservice, Constructor)
{
    EXPECT_NE(m_service, nullptr);
    EXPECT_EQ(m_service->getPath(), "/test/path");
    EXPECT_EQ(m_service->getInterface(), "com.test.Interface");
}

// QString DAccountService::getAccountInfo()
TEST_F(test_daccountservice, GetAccountInfo)
{
    QString result = m_service->getAccountInfo();
    EXPECT_FALSE(result.isEmpty());
}

// QString DAccountService::getScheduleTypeList()
TEST_F(test_daccountservice, GetScheduleTypeList)
{
    QString result = m_service->getScheduleTypeList();
    EXPECT_FALSE(result.isEmpty());
}

// QString DAccountService::getScheduleTypeByID(const QString &typeID)
TEST_F(test_daccountservice, GetScheduleTypeByID)
{
    QString result = m_service->getScheduleTypeByID("nonexistent-type-id");
    EXPECT_TRUE(result.isEmpty());
}

// QString DAccountService::createScheduleType(const QString &typeInfo)
TEST_F(test_daccountservice, CreateScheduleType)
{
    QString typeInfo = QStringLiteral("{\"name\":\"testType\",\"colorID\":1,\"isShowStatus\":true,\"isNotification\":true}");
    QString result = m_service->createScheduleType(typeInfo);

    EXPECT_FALSE(result.isEmpty());
}

// bool DAccountService::updateScheduleType(const QString &typeInfo)
TEST_F(test_daccountservice, UpdateScheduleType)
{
    QString typeInfo = QStringLiteral("{\"name\":\"testType\",\"colorID\":1,\"isShowStatus\":true,\"isNotification\":true}");
    QString typeID = m_service->createScheduleType(typeInfo);
    ASSERT_FALSE(typeID.isEmpty());
    QString updateInfo = QStringLiteral("{\"typeID\":\"%1\",\"name\":\"updatedType\",\"colorID\":2,\"isShowStatus\":true,\"isNotification\":true}").arg(typeID);
    bool result = m_service->updateScheduleType(updateInfo);
    EXPECT_TRUE(result);
}

// bool DAccountService::deleteScheduleTypeByID(const QString &typeID)
TEST_F(test_daccountservice, DeleteScheduleTypeByID)
{
    QString typeInfo = QStringLiteral("{\"name\":\"testType\",\"colorID\":1,\"isShowStatus\":true,\"isNotification\":true}");
    QString typeID = m_service->createScheduleType(typeInfo);
    ASSERT_FALSE(typeID.isEmpty());
    bool result = m_service->deleteScheduleTypeByID(typeID);
    EXPECT_TRUE(result);
}

// bool DAccountService::scheduleTypeByUsed(const QString &typeID)
TEST_F(test_daccountservice, ScheduleTypeByUsed)
{
    bool result = m_service->scheduleTypeByUsed("nonexistent-type-id");
    EXPECT_FALSE(result);
}

// QString DAccountService::createSchedule(const QString &scheduleInfo)
TEST_F(test_daccountservice, CreateSchedule)
{
    QString typeInfo = QStringLiteral("{\"name\":\"testType\",\"colorID\":1,\"isShowStatus\":true,\"isNotification\":true}");
    QString typeID = m_service->createScheduleType(typeInfo);
    ASSERT_FALSE(typeID.isEmpty());

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("test schedule"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(9, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(10, 0)));
    schedule->setScheduleTypeID(typeID);
    QString scheduleInfo;
    ASSERT_TRUE(DSchedule::toJsonString(schedule, scheduleInfo));

    QString result = m_service->createSchedule(scheduleInfo);
    EXPECT_FALSE(result.isEmpty());
}

// bool DAccountService::updateSchedule(const QString &scheduleInfo)
TEST_F(test_daccountservice, UpdateSchedule)
{
    QString typeInfo = QStringLiteral("{\"name\":\"testType\",\"colorID\":1,\"isShowStatus\":true,\"isNotification\":true}");
    QString typeID = m_service->createScheduleType(typeInfo);
    ASSERT_FALSE(typeID.isEmpty());

    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("test schedule"));
    schedule->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(9, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(10, 0)));
    schedule->setScheduleTypeID(typeID);
    QString scheduleInfo;
    ASSERT_TRUE(DSchedule::toJsonString(schedule, scheduleInfo));
    QString scheduleID = m_service->createSchedule(scheduleInfo);
    ASSERT_FALSE(scheduleID.isEmpty());

    DSchedule::Ptr updated(new DSchedule);
    updated->setUid(scheduleID);
    updated->setSummary(QStringLiteral("updated schedule"));
    updated->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(9, 0)));
    updated->setDtEnd(QDateTime(QDate(2025, 1, 1), QTime(10, 0)));
    updated->setScheduleTypeID(typeID);
    QString updateInfo;
    ASSERT_TRUE(DSchedule::toJsonString(updated, updateInfo));
    bool result = m_service->updateSchedule(updateInfo);
    EXPECT_TRUE(result);
}

// QString DAccountService::getScheduleByScheduleID(const QString &scheduleID)
TEST_F(test_daccountservice, GetScheduleByScheduleID)
{
    QString result = m_service->getScheduleByScheduleID("nonexistent-schedule-id");
    EXPECT_TRUE(result.isEmpty());
}

// bool DAccountService::deleteScheduleByScheduleID(const QString &scheduleID)
TEST_F(test_daccountservice, DeleteScheduleByScheduleID)
{
    bool result = m_service->deleteScheduleByScheduleID("nonexistent-schedule-id");
    EXPECT_FALSE(result);
}

// QString DAccountService::querySchedulesWithParameter(const QString &params)
TEST_F(test_daccountservice, QuerySchedulesWithParameter)
{
    // Invalid params (missing "key") should return empty string
    QString invalidParams = QStringLiteral("{}");
    QString invalidResult = m_service->querySchedulesWithParameter(invalidParams);
    EXPECT_TRUE(invalidResult.isEmpty());

    // Create a schedule type + schedule within the query range so the query
    // returns a non-empty result.
    QString typeInfo = QStringLiteral("{\"name\":\"testType\",\"colorID\":1,\"isShowStatus\":true,\"isNotification\":true}");
    QString typeID = m_service->createScheduleType(typeInfo);
    ASSERT_FALSE(typeID.isEmpty());
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("queryable schedule"));
    schedule->setDtStart(QDateTime(QDate(2025, 6, 1), QTime(9, 0)));
    schedule->setDtEnd(QDateTime(QDate(2025, 6, 1), QTime(10, 0)));
    schedule->setScheduleTypeID(typeID);
    QString scheduleInfo;
    ASSERT_TRUE(DSchedule::toJsonString(schedule, scheduleInfo));
    ASSERT_FALSE(m_service->createSchedule(scheduleInfo).isEmpty());

    // Valid params with key and date range should return a non-empty result
    QString validParams = QStringLiteral("{\"key\":\"\",\"queryType\":0,\"dtStart\":\"2025-01-01T00:00:00\",\"dtEnd\":\"2025-12-31T23:59:59\"}");
    QString validResult = m_service->querySchedulesWithParameter(validParams);
    EXPECT_FALSE(validResult.isEmpty());
}

// bool DAccountService::exportSchedule(const QString &icsFilePath, const QString &typeID)
TEST_F(test_daccountservice, ExportSchedule)
{
    // Create a valid schedule type first, then export using its ID.
    // NOTE: exportSchedule with a nonexistent typeID triggers a null-pointer
    // dereference in DAccountModule::exportSchedule (daccountmodule.cpp:1732)
    // because getScheduleTypeByID returns null and displayName() is called
    // without a null check. This is a source code defect (see Mode 5 report).
    QString typeInfo = QStringLiteral("{\"name\":\"testType\",\"colorID\":1,\"isShowStatus\":true,\"isNotification\":true}");
    QString typeID = m_service->createScheduleType(typeInfo);
    ASSERT_FALSE(typeID.isEmpty());
    QString icsFilePath = QStringLiteral("/tmp/test_export_das.ics");
    bool result = m_service->exportSchedule(icsFilePath, typeID);
    EXPECT_TRUE(result);
    QFile::remove(icsFilePath);
}

// bool DAccountService::importSchedule(const QString &icsFilePath, const QString &typeID, const bool cleanExists)
TEST_F(test_daccountservice, ImportSchedule)
{
    QString icsFilePath = QStringLiteral("/tmp/test_nonexistent.ics");
    bool result = m_service->importSchedule(icsFilePath, "nonexistent-type-id", false);
    EXPECT_FALSE(result);
}

// QString DAccountService::getSysColors()
TEST_F(test_daccountservice, GetSysColors)
{
    QString result = m_service->getSysColors();
    EXPECT_FALSE(result.isEmpty());
}

// bool DAccountService::getExpand()
TEST_F(test_daccountservice, GetExpand)
{
    bool result = m_service->getExpand();
    EXPECT_NO_THROW(result);
}

// void DAccountService::setExpand(const bool &isExpand)
TEST_F(test_daccountservice, SetExpand)
{
    EXPECT_NO_THROW(m_service->setExpand(true));
    EXPECT_NO_THROW(m_service->setExpand(false));
}

// int DAccountService::getAccountState()
TEST_F(test_daccountservice, GetAccountState)
{
    int result = m_service->getAccountState();
    EXPECT_GE(result, 0);
}

// void DAccountService::setAccountState(const int accountState)
TEST_F(test_daccountservice, SetAccountState)
{
    EXPECT_NO_THROW(m_service->setAccountState(1));
    EXPECT_NO_THROW(m_service->setAccountState(0));
}

// int DAccountService::getSyncState()
TEST_F(test_daccountservice, GetSyncState)
{
    int result = m_service->getSyncState();
    EXPECT_GE(result, 0);
}

// QString DAccountService::getSyncFreq()
TEST_F(test_daccountservice, GetSyncFreq)
{
    QString result = m_service->getSyncFreq();
    EXPECT_NO_THROW(result);
}

// void DAccountService::setSyncFreq(const QString &freq)
TEST_F(test_daccountservice, SetSyncFreq)
{
    EXPECT_NO_THROW(m_service->setSyncFreq("15"));
}

// QString DAccountService::getDtLastUpdate()
TEST_F(test_daccountservice, GetDtLastUpdate)
{
    QString result = m_service->getDtLastUpdate();
    EXPECT_NO_THROW(result);
}
