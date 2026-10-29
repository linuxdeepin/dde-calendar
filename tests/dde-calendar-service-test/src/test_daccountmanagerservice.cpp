// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountmanagerservice.h"
#include "syncoperation.h"
#include "csystemdtimercontrol.h"
#include "service_stub.h"
#include "../third-party_stub/stub.h"
#include <gtest/gtest.h>
#include <QObject>

static void stub_startCalendarServiceSystemdTimer_dams(void *obj)
{
    Q_UNUSED(obj);
}

static bool stub_hasAvailable_dams(void *obj)
{
    Q_UNUSED(obj);
    return false;
}

class test_daccountmanagerservice : public QObject, public ::testing::Test
{
public:
    test_daccountmanagerservice()
    {
        stub.set(ADDR(CSystemdTimerControl, startCalendarServiceSystemdTimer),
                 stub_startCalendarServiceSystemdTimer_dams);
        stub.set(ADDR(Syncoperation, hasAvailable), stub_hasAvailable_dams);
        m_service = new DAccountManagerService();

        m_service->getCalendarGeneralSettings();
    }
    ~test_daccountmanagerservice()
    {
        if (m_service) {
            delete m_service;
        }
    }
protected:
    Stub stub;
    DAccountManagerService *m_service = nullptr;
};

// DAccountManagerService::DAccountManagerService(QObject *parent)
TEST_F(test_daccountmanagerservice, Constructor)
{
    EXPECT_NE(m_service, nullptr);
}

// QString DAccountManagerService::getAccountList()
TEST_F(test_daccountmanagerservice, GetAccountList)
{
    QString result = m_service->getAccountList();
    EXPECT_NO_THROW(result);
}

// void DAccountManagerService::remindJob(const QString &accountID, const QString &alarmID)
TEST_F(test_daccountmanagerservice, RemindJob)
{
    EXPECT_NO_THROW(m_service->remindJob("nonexistent-account", "alarm-1"));
}

// void DAccountManagerService::updateRemindJob(bool isClear)
TEST_F(test_daccountmanagerservice, UpdateRemindJob)
{
    EXPECT_NO_THROW(m_service->updateRemindJob(false));
    EXPECT_NO_THROW(m_service->updateRemindJob(true));
}

// void DAccountManagerService::notifyMsgHanding(const QString &accountID, const QString &alarmID, const qint32 operationNum)
TEST_F(test_daccountmanagerservice, NotifyMsgHanding)
{
    EXPECT_NO_THROW(m_service->notifyMsgHanding("nonexistent-account", "alarm-1", 1));
}

// void DAccountManagerService::downloadByAccountID(const QString &accountID)
TEST_F(test_daccountmanagerservice, DownloadByAccountID)
{
    EXPECT_NO_THROW(m_service->downloadByAccountID("nonexistent-account"));
}

// bool DAccountManagerService::requestCalDavManualSync(const QString &accountID, const QString &requestID)
TEST_F(test_daccountmanagerservice, RequestCalDavManualSync)
{
    bool result = m_service->requestCalDavManualSync("nonexistent-account", "req-1");
    EXPECT_FALSE(result);
}

// void DAccountManagerService::uploadNetWorkAccountData()
TEST_F(test_daccountmanagerservice, UploadNetWorkAccountData)
{
    EXPECT_NO_THROW(m_service->uploadNetWorkAccountData());
}

// QString DAccountManagerService::getCalDavAccountStatusList()
TEST_F(test_daccountmanagerservice, GetCalDavAccountStatusList)
{
    QString result = m_service->getCalDavAccountStatusList();
    EXPECT_NO_THROW(result);
}

// QString DAccountManagerService::getCalDavAccountConfig(const QString &accountID)
TEST_F(test_daccountmanagerservice, GetCalDavAccountConfig)
{
    QString result = m_service->getCalDavAccountConfig("nonexistent-account");
    EXPECT_NO_THROW(result);
}

// QString DAccountManagerService::validateCalDavAccountForUpdate(...)
TEST_F(test_daccountmanagerservice, ValidateCalDavAccountForUpdate)
{
    QString result = m_service->validateCalDavAccountForUpdate(
        "nonexistent-account", 0, "http://localhost", "user", "cred");
    EXPECT_NO_THROW(result);
}

// QString DAccountManagerService::validateCalDavAccount(...)
TEST_F(test_daccountmanagerservice, ValidateCalDavAccount)
{
    QString result = m_service->validateCalDavAccount(0, "http://localhost", "user", "cred");
    EXPECT_NO_THROW(result);
}

// bool DAccountManagerService::updateCalDavCredentialReference(const QString &accountID, const QString &credentialRef)
TEST_F(test_daccountmanagerservice, UpdateCalDavCredentialReference)
{
    bool result = m_service->updateCalDavCredentialReference("nonexistent-account", "cred");
    EXPECT_NO_THROW(result);
}

// QString DAccountManagerService::createCalDavAccount(...)
TEST_F(test_daccountmanagerservice, CreateCalDavAccount)
{
    QString result = m_service->createCalDavAccount(0, "http://localhost", "user", "cred", "test");
    EXPECT_NO_THROW(result);
}

// bool DAccountManagerService::updateCalDavAccount(...)
TEST_F(test_daccountmanagerservice, UpdateCalDavAccount)
{
    bool result = m_service->updateCalDavAccount(
        "nonexistent-account", 0, "http://localhost", "user", "cred", "test");
    EXPECT_NO_THROW(result);
}

// bool DAccountManagerService::deleteCalDavAccount(const QString &accountID)
TEST_F(test_daccountmanagerservice, DeleteCalDavAccount)
{
    bool result = m_service->deleteCalDavAccount("nonexistent-account");
    EXPECT_NO_THROW(result);
}

// bool DAccountManagerService::deleteCalDavAccountWithLocalDataOption(const QString &accountID, bool deleteLocalData)
TEST_F(test_daccountmanagerservice, DeleteCalDavAccountWithLocalDataOption)
{
    bool result = m_service->deleteCalDavAccountWithLocalDataOption("nonexistent-account", true);
    EXPECT_NO_THROW(result);
}

// bool DAccountManagerService::resolveCalDavConflict(const QString &accountID, const QString &localScheduleID, bool keepLocal)
TEST_F(test_daccountmanagerservice, ResolveCalDavConflict)
{
    bool result = m_service->resolveCalDavConflict("nonexistent-account", "sched-1", true);
    EXPECT_NO_THROW(result);
}

// bool DAccountManagerService::resolveAllCalDavConflicts(const QString &accountID, bool keepLocal)
TEST_F(test_daccountmanagerservice, ResolveAllCalDavConflicts)
{
    bool result = m_service->resolveAllCalDavConflicts("nonexistent-account", true);
    EXPECT_NO_THROW(result);
}

// QString DAccountManagerService::getCalendarGeneralSettings()
TEST_F(test_daccountmanagerservice, GetCalendarGeneralSettings)
{
    QString result = m_service->getCalendarGeneralSettings();
    EXPECT_FALSE(result.isEmpty());
}

// void DAccountManagerService::setCalendarGeneralSettings(const QString &cgSet)
TEST_F(test_daccountmanagerservice, SetCalendarGeneralSettings)
{
    QString cgSet = m_service->getCalendarGeneralSettings();
    EXPECT_NO_THROW(m_service->setCalendarGeneralSettings(cgSet));
}

// void DAccountManagerService::calendarIsShow(const bool &isShow)
TEST_F(test_daccountmanagerservice, CalendarIsShow)
{
    EXPECT_NO_THROW(m_service->calendarIsShow(true));
    EXPECT_NO_THROW(m_service->calendarIsShow(false));
}

// void DAccountManagerService::login()
TEST_F(test_daccountmanagerservice, Login)
{
    EXPECT_NO_THROW(m_service->login());
}

// void DAccountManagerService::logout()
TEST_F(test_daccountmanagerservice, Logout)
{
    EXPECT_NO_THROW(m_service->logout());
}

// bool DAccountManagerService::isSupportUid()
TEST_F(test_daccountmanagerservice, IsSupportUid)
{
    bool result = m_service->isSupportUid();
    EXPECT_FALSE(result);
}

// int DAccountManagerService::getfirstDayOfWeek() const
TEST_F(test_daccountmanagerservice, GetFirstDayOfWeek)
{
    int result = m_service->getfirstDayOfWeek();
    EXPECT_GE(result, 0);
}

// void DAccountManagerService::setFirstDayOfWeek(const int firstday)
TEST_F(test_daccountmanagerservice, SetFirstDayOfWeek)
{
    EXPECT_NO_THROW(m_service->setFirstDayOfWeek(1));
}

// int DAccountManagerService::getTimeFormatType() const
TEST_F(test_daccountmanagerservice, GetTimeFormatType)
{
    int result = m_service->getTimeFormatType();
    EXPECT_GE(result, 0);
}

// void DAccountManagerService::setTimeFormatType(const int timeType)
TEST_F(test_daccountmanagerservice, SetTimeFormatType)
{
    EXPECT_NO_THROW(m_service->setTimeFormatType(0));
}

// int DAccountManagerService::getFirstDayOfWeekSource()
TEST_F(test_daccountmanagerservice, GetFirstDayOfWeekSource)
{
    int result = m_service->getFirstDayOfWeekSource();
    EXPECT_GE(result, 0);
}

// void DAccountManagerService::setFirstDayOfWeekSource(const int source)
TEST_F(test_daccountmanagerservice, SetFirstDayOfWeekSource)
{
    EXPECT_NO_THROW(m_service->setFirstDayOfWeekSource(0));
    EXPECT_NO_THROW(m_service->setFirstDayOfWeekSource(1));
    EXPECT_NO_THROW(m_service->setFirstDayOfWeekSource(99));
}

// int DAccountManagerService::getTimeFormatTypeSource()
TEST_F(test_daccountmanagerservice, GetTimeFormatTypeSource)
{
    int result = m_service->getTimeFormatTypeSource();
    EXPECT_GE(result, 0);
}

// void DAccountManagerService::setTimeFormatTypeSource(const int source)
TEST_F(test_daccountmanagerservice, SetTimeFormatTypeSource)
{
    EXPECT_NO_THROW(m_service->setTimeFormatTypeSource(0));
    EXPECT_NO_THROW(m_service->setTimeFormatTypeSource(1));
    EXPECT_NO_THROW(m_service->setTimeFormatTypeSource(99));
}
