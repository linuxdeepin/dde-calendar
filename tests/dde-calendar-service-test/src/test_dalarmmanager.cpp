// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dalarmmanager.h"
#include "dreminddata.h"
#include "dschedule.h"
#include "dbus/dbusnotify.h"

#include "gtest/gtest.h"
#include <QObject>
#include <QSignalSpy>
#include <QDateTime>
#include <QString>
#include <QVariantList>
#include "event.h"

#include "../third-party_stub/stub.h"
#include "service_stub.h"
#include <QDBusAbstractInterface>
#include <QProcess>

// ---------------------------------------------------------------------------
// Stub helpers
// ---------------------------------------------------------------------------

// Stub QProcess::start with openMode overload — prevents systemctl execution in tests
static void stub_QProcess_start2(const QString &program, const QStringList &arguments,
                                 QIODevice::OpenModeFlag mode)
{
    Q_UNUSED(program);
    Q_UNUSED(arguments);
    Q_UNUSED(mode);
}

// Custom stub: returns a DBus reply with a valid quint32 so Notify() returns >= 0
static QDBusMessage notify_callWithArgumentList_stub(QDBus::CallMode mode,
                                                      const QString &method,
                                                      const QList<QVariant> &args)
{
    Q_UNUSED(mode);
    Q_UNUSED(method);
    Q_UNUSED(args);
    QDBusMessage reply = QDBusMessage::createMethodCall(
        "org.test", "/test", "org.test", "test");
    reply << QVariant(static_cast<quint32>(42));
    return reply;
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------

// Forward declare stub from service_stub.cpp
QDBusMessage::MessageType type_Stub();

class test_dalarmmanager : public QObject, public ::testing::Test
{
public:
    test_dalarmmanager()
    {
        stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList), notify_callWithArgumentList_stub);
        stub.set(ADDR(QDBusMessage, type), type_Stub);
        stub.set(static_cast<void(QProcess::*)(const QString &, const QStringList &,
                                               QIODevice::OpenMode)>(&QProcess::start),
                 stub_QProcess_start2);
        manager = new DAlarmManager();
    }
    ~test_dalarmmanager() override
    {
        delete manager;
    }

protected:
    DAlarmManager *manager = nullptr;
    Stub stub;

    // Helper: create a DRemindData::Ptr with given values
    static DRemindData::Ptr makeRemindData(int remindCount = 1)
    {
        DRemindData::Ptr data(new DRemindData());
        data->setAccountID("acct-001");
        data->setAlarmID("alarm-001");
        data->setScheduleID("sched-001");
        data->setRemindCount(remindCount);
        data->setNotifyid(10);
        data->setDtRemind(QDateTime::currentDateTime().addSecs(3600));
        data->setDtStart(QDateTime::currentDateTime().addDays(1));
        data->setDtEnd(QDateTime::currentDateTime().addDays(1).addSecs(3600));
        data->setRecurrenceId(QDateTime::currentDateTime());
        return data;
    }

    // Helper: create a DSchedule::Ptr with an alarm of the given type
    static DSchedule::Ptr makeSchedule(DSchedule::AlarmType alarmType,
                                        const QDateTime &start,
                                        const QDateTime &end,
                                        bool allDay = false,
                                        const QString &summary = "Test Schedule")
    {
        KCalendarCore::Event event;
        event.setSummary(summary);
        event.setDtStart(start);
        event.setDtEnd(end);
        event.setAllDay(allDay);
        DSchedule::Ptr schedule(new DSchedule(event));
        schedule->setAlarmType(alarmType);
        return schedule;
    }

    // Helper: create a DSchedule::Ptr without alarms
    static DSchedule::Ptr makeScheduleNoAlarm(const QDateTime &start,
                                               const QDateTime &end,
                                               bool allDay = false)
    {
        KCalendarCore::Event event;
        event.setSummary("No Alarm Schedule");
        event.setDtStart(start);
        event.setDtEnd(end);
        event.setAllDay(allDay);
        DSchedule::Ptr schedule(new DSchedule(event));
        return schedule;
    }
};

// ---------------------------------------------------------------------------
// DAlarmManager::DAlarmManager (constructor)
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, Constructor_CreatesManager)
{
    // Manager already created in fixture; verify it exists and has dbus notify
    ASSERT_NE(manager, nullptr);
    EXPECT_NE(manager->getdbusnotify(), nullptr);
}

// ---------------------------------------------------------------------------
// DAlarmManager::getdbusnotify
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, GetDbusNotify_ReturnsNonNull)
{
    DBusNotify *notify = manager->getdbusnotify();
    EXPECT_NE(notify, nullptr);
}

TEST_F(test_dalarmmanager, GetDbusNotify_ReturnsSameInstance)
{
    DBusNotify *first = manager->getdbusnotify();
    DBusNotify *second = manager->getdbusnotify();
    EXPECT_EQ(first, second);
}

// ---------------------------------------------------------------------------
// DAlarmManager::updateRemind
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, UpdateRemind_EmptyList)
{
    DRemindData::List emptyList;
    // Should early-return without crash
    manager->updateRemind(emptyList);
}

TEST_F(test_dalarmmanager, UpdateRemind_NonEmptyList)
{
    DRemindData::List list;
    list.append(makeRemindData(1));
    list.append(makeRemindData(2));
    // Should process without crash (systemd calls stubbed)
    manager->updateRemind(list);
}

// ---------------------------------------------------------------------------
// DAlarmManager::notifyJobsChanged
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, NotifyJobsChanged_EmptyList)
{
    DRemindData::List emptyList;
    // Should early-return without crash
    manager->notifyJobsChanged(emptyList);
}

TEST_F(test_dalarmmanager, NotifyJobsChanged_NonEmptyList)
{
    DRemindData::List list;
    list.append(makeRemindData(1));
    // Should process without crash (systemd calls stubbed)
    manager->notifyJobsChanged(list);
}

// ---------------------------------------------------------------------------
// DAlarmManager::notifyMsgHanding
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, NotifyMsgHanding_Case1_EmitsSignal)
{
    DRemindData::Ptr data = makeRemindData(1);
    QSignalSpy spy(manager, &DAlarmManager::signalCallOpenCalendarUI);

    manager->notifyMsgHanding(data, 1);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), data->alarmID());
}

TEST_F(test_dalarmmanager, NotifyMsgHanding_Case2_RemindLater)
{
    DRemindData::Ptr data = makeRemindData(1);
    // Should call remindLater without crash
    manager->notifyMsgHanding(data, 2);
}

TEST_F(test_dalarmmanager, NotifyMsgHanding_Case21_RemindLater)
{
    DRemindData::Ptr data = makeRemindData(1);
    manager->notifyMsgHanding(data, 21);
}

TEST_F(test_dalarmmanager, NotifyMsgHanding_Case22_RemindLater)
{
    DRemindData::Ptr data = makeRemindData(1);
    manager->notifyMsgHanding(data, 22);
}

TEST_F(test_dalarmmanager, NotifyMsgHanding_Case23_RemindLater)
{
    DRemindData::Ptr data = makeRemindData(1);
    manager->notifyMsgHanding(data, 23);
}

TEST_F(test_dalarmmanager, NotifyMsgHanding_Case3_RemindLater)
{
    DRemindData::Ptr data = makeRemindData(1);
    manager->notifyMsgHanding(data, 3);
}

TEST_F(test_dalarmmanager, NotifyMsgHanding_Case4_RemindLater)
{
    DRemindData::Ptr data = makeRemindData(1);
    manager->notifyMsgHanding(data, 4);
}

TEST_F(test_dalarmmanager, NotifyMsgHanding_DefaultCase_NoCrash)
{
    DRemindData::Ptr data = makeRemindData(1);
    // Unknown operationNum should hit default warning branch
    manager->notifyMsgHanding(data, 999);
}

// ---------------------------------------------------------------------------
// DAlarmManager::remindLater
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, RemindLater_OperationNum2)
{
    DRemindData::Ptr data = makeRemindData(3);
    // operationNum == 2: uses remindCount directly
    manager->remindLater(data, 2);
}

TEST_F(test_dalarmmanager, RemindLater_OperationNumNot2)
{
    DRemindData::Ptr data = makeRemindData(3);
    // operationNum != 2: uses remindCount + 1, then decrements
    manager->remindLater(data, 3);
}

// ---------------------------------------------------------------------------
// DAlarmManager::remindJob
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, RemindJob_NoAlarms_ReturnsZero)
{
    DRemindData::Ptr data = makeRemindData(1);
    DSchedule::Ptr schedule = makeScheduleNoAlarm(
        QDateTime::currentDateTime().addDays(1),
        QDateTime::currentDateTime().addDays(1).addSecs(3600));

    int result = manager->remindJob(data, schedule);
    EXPECT_EQ(result, 0);
}

TEST_F(test_dalarmmanager, RemindJob_NDaysGte3_RemindCount1_FutureStart)
{
    // nDays >= 3 and remindCount == 1: Alarm_1Week_Front (offset -7 days, nDays=7)
    DRemindData::Ptr data = makeRemindData(1);
    QDateTime start = QDateTime::currentDateTime().addDays(10);
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_1Week_Front,
                                            start,
                                            start.addSecs(3600));
    int result = manager->remindJob(data, schedule);
    // Notify is stubbed, should return some value (not 0)
    // The exact return depends on stub behavior
    EXPECT_GE(result, 0);
}

TEST_F(test_dalarmmanager, RemindJob_NDaysGte3_RemindCount1_FarFutureStart)
{
    // nDays >= 3, remindCount == 1, but schedule start is more than 1 day away
    // Tests the additional argMake(4, ...) branch
    DRemindData::Ptr data = makeRemindData(1);
    QDateTime start = QDateTime::currentDateTime().addDays(10);
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_1Week_Front,
                                            start,
                                            start.addSecs(3600));
    int result = manager->remindJob(data, schedule);
    EXPECT_GE(result, 0);
}

TEST_F(test_dalarmmanager, RemindJob_NDays1to2_BmaxTrue)
{
    // nDays == 1 or 2, and bmax == true (remindCount >= 11)
    // Alarm_1Day_Front (offset -86400s = -1 day, nDays=1), remindCount=11 → bmax=true
    DRemindData::Ptr data = makeRemindData(11);
    QDateTime start = QDateTime::currentDateTime().addDays(2);
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_1Day_Front,
                                            start,
                                            start.addSecs(3600));
    int result = manager->remindJob(data, schedule);
    EXPECT_GE(result, 0);
}

TEST_F(test_dalarmmanager, RemindJob_DefaultBranch_FutureStart)
{
    // nDays == 0 (Alarm_Begin), falls to default branch
    DRemindData::Ptr data = makeRemindData(1);
    QDateTime start = QDateTime::currentDateTime().addDays(1);
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_Begin,
                                            start,
                                            start.addSecs(3600));
    int result = manager->remindJob(data, schedule);
    EXPECT_GE(result, 0);
}

TEST_F(test_dalarmmanager, RemindJob_ScheduleAlreadyStarted)
{
    // tm >= schedule->dtStart(): basic actions only
    DRemindData::Ptr data = makeRemindData(1);
    QDateTime start = QDateTime::currentDateTime().addDays(-1);
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_Begin,
                                            start,
                                            start.addSecs(3600));
    int result = manager->remindJob(data, schedule);
    EXPECT_GE(result, 0);
}

// ---------------------------------------------------------------------------
// DAlarmManager::getRemindLaterDuration (private, accessible via -fno-access-control)
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, GetRemindLaterDuration_Count1_NotCapped)
{
    qint64 duration = 0;
    bool bmax = manager->getRemindLaterDuration(1, duration);
    // duration = (10 + ((1-1)*5)) * 60000 = 10 * 60000 = 600000 ms (10 min)
    EXPECT_FALSE(bmax);
    EXPECT_EQ(duration, 600000);
}

TEST_F(test_dalarmmanager, GetRemindLaterDuration_Count3_NotCapped)
{
    qint64 duration = 0;
    bool bmax = manager->getRemindLaterDuration(3, duration);
    // duration = (10 + ((3-1)*5)) * 60000 = 20 * 60000 = 1200000 ms (20 min)
    EXPECT_FALSE(bmax);
    EXPECT_EQ(duration, 1200000);
}

TEST_F(test_dalarmmanager, GetRemindLaterDuration_Count11_Capped)
{
    qint64 duration = 0;
    bool bmax = manager->getRemindLaterDuration(11, duration);
    // duration = (10 + ((11-1)*5)) * 60000 = 60 * 60000 = 3600000 ms = Hour
    EXPECT_TRUE(bmax);
    EXPECT_EQ(duration, 3600000);
}

TEST_F(test_dalarmmanager, GetRemindLaterDuration_Count20_Capped)
{
    qint64 duration = 0;
    bool bmax = manager->getRemindLaterDuration(20, duration);
    // duration = (10 + ((20-1)*5)) * 60000 = 105 * 60000 = 6300000 > Hour → capped
    EXPECT_TRUE(bmax);
    EXPECT_EQ(duration, 3600000);
}

// ---------------------------------------------------------------------------
// DAlarmManager::getBodyTimePart (private, accessible via -fno-access-control)
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, GetBodyTimePart_AllDay_Today)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime jobtime = now; // diff == 0
    QString result = manager->getBodyTimePart(now, jobtime, true, true);
    EXPECT_EQ(result.toStdString(), "Today");
}

TEST_F(test_dalarmmanager, GetBodyTimePart_AllDay_Tomorrow)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime jobtime = now.addDays(1); // diff == 1
    QString result = manager->getBodyTimePart(now, jobtime, true, true);
    EXPECT_EQ(result.toStdString(), "Tomorrow");
}

TEST_F(test_dalarmmanager, GetBodyTimePart_AllDay_FarFuture)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime jobtime = now.addDays(5); // diff > 1
    QString result = manager->getBodyTimePart(now, jobtime, true, true);
    // Should contain a date string, not "Today" or "Tomorrow"
    EXPECT_NE(result.toStdString(), "Today");
    EXPECT_NE(result.toStdString(), "Tomorrow");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(test_dalarmmanager, GetBodyTimePart_NonAllDay_Today)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime jobtime = now; // diff == 0
    QString result = manager->getBodyTimePart(now, jobtime, false, true);
    // Should start with "Today " followed by time
    EXPECT_TRUE(result.startsWith("Today"));
    EXPECT_TRUE(result.contains(":"));
}

TEST_F(test_dalarmmanager, GetBodyTimePart_NonAllDay_Tomorrow)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime jobtime = now.addDays(1); // diff == 1
    QString result = manager->getBodyTimePart(now, jobtime, false, true);
    EXPECT_TRUE(result.startsWith("Tomorrow"));
    EXPECT_TRUE(result.contains(":"));
}

TEST_F(test_dalarmmanager, GetBodyTimePart_NonAllDay_FarFuture)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime jobtime = now.addDays(5); // diff > 1
    QString result = manager->getBodyTimePart(now, jobtime, false, true);
    // Should contain "yyyy/MM/dd HH:mm" format
    EXPECT_NE(result.toStdString(), "Today");
    EXPECT_NE(result.toStdString(), "Tomorrow");
    EXPECT_TRUE(result.contains("/"));
    EXPECT_TRUE(result.contains(":"));
}

// ---------------------------------------------------------------------------
// DAlarmManager::getRemindBody (private, accessible via -fno-access-control)
// ---------------------------------------------------------------------------

TEST_F(test_dalarmmanager, GetRemindBody_AllDaySingleDay)
{
    QDateTime start = QDateTime::currentDateTime().addDays(1);
    QDateTime end = start; // same day
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_Begin, start, end, true, "AllDay Single");
    QString result = manager->getRemindBody(schedule);
    EXPECT_TRUE(result.contains("AllDay Single"));
}

TEST_F(test_dalarmmanager, GetRemindBody_AllDayMultiDay)
{
    QDateTime start = QDateTime::currentDateTime().addDays(1);
    QDateTime end = start.addDays(2); // different day
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_Begin, start, end, true, "AllDay Multi");
    QString result = manager->getRemindBody(schedule);
    EXPECT_TRUE(result.contains("AllDay Multi"));
    EXPECT_TRUE(result.contains(" to "));
}

TEST_F(test_dalarmmanager, GetRemindBody_NonAllDaySingleDay)
{
    QDateTime start = QDateTime::currentDateTime().addDays(1);
    QDateTime end = start.addSecs(3600); // same day, 1 hour later
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_Begin, start, end, false, "NonAllDay Single");
    QString result = manager->getRemindBody(schedule);
    EXPECT_TRUE(result.contains("NonAllDay Single"));
}

TEST_F(test_dalarmmanager, GetRemindBody_NonAllDayMultiDay)
{
    QDateTime start = QDateTime::currentDateTime().addDays(1);
    QDateTime end = start.addDays(2); // different day
    DSchedule::Ptr schedule = makeSchedule(DSchedule::Alarm_Begin, start, end, false, "NonAllDay Multi");
    QString result = manager->getRemindBody(schedule);
    EXPECT_TRUE(result.contains("NonAllDay Multi"));
}
