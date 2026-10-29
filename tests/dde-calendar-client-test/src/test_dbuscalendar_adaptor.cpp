// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbuscalendar_adaptor.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QWidget>

#include <gtest/gtest.h>

namespace {

// Mock parent that is a QWidget (needed for RaiseWindow) and provides
// the slots that CalendarAdaptor invokes via QMetaObject::invokeMethod.
class MockCalendarParent : public QWidget
{
    Q_OBJECT
public:
    int activeWindowCallCount = 0;
    QString lastOpenScheduleJob;

public slots:
    void ActiveWindow() { ++activeWindowCallCount; }
    void slotOpenSchedule(QString job) { lastOpenScheduleJob = job; }
};

// Pump the event loop briefly so queued invocations land.
inline void pumpEvents(int ms = 200)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

// Parse a JSON string returned by adaptor methods.
inline QJsonObject parseJson(const QString &s)
{
    QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
    return doc.object();
}

} // namespace

// ---- Constructor ----

TEST(CalendarAdaptorTest, Constructor_CreatesWithValidParent)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_EQ(adaptor.parent(), &parent);
}

// ---- Destructor ----

TEST(CalendarAdaptorTest, Destructor_DoesNotCrash)
{
    MockCalendarParent parent;
    {
        CalendarAdaptor *adaptor = new CalendarAdaptor(&parent);
        delete adaptor;
    }
    SUCCEED();
}

// ---- ActiveWindow ----

TEST(CalendarAdaptorTest, ActiveWindow_InvokesParentSlot)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    adaptor.ActiveWindow();
    pumpEvents();
    EXPECT_EQ(parent.activeWindowCallCount, 1);
}

TEST(CalendarAdaptorTest, ActiveWindow_MultipleCallsIncrementCounter)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    adaptor.ActiveWindow();
    adaptor.ActiveWindow();
    adaptor.ActiveWindow();
    pumpEvents();
    EXPECT_EQ(parent.activeWindowCallCount, 3);
}

// ---- RaiseWindow ----

TEST(CalendarAdaptorTest, RaiseWindow_UnminimizesWidget)
{
    MockCalendarParent parent;
    parent.setWindowState(Qt::WindowMinimized);
    ASSERT_TRUE(parent.windowState() & Qt::WindowMinimized);

    CalendarAdaptor adaptor(&parent);
    adaptor.RaiseWindow();

    EXPECT_FALSE(parent.windowState() & Qt::WindowMinimized);
}

TEST(CalendarAdaptorTest, RaiseWindow_PreservesOtherWindowState)
{
    MockCalendarParent parent;
    parent.setWindowState(Qt::WindowMinimized | Qt::WindowMaximized);

    CalendarAdaptor adaptor(&parent);
    adaptor.RaiseWindow();

    EXPECT_FALSE(parent.windowState() & Qt::WindowMinimized);
    EXPECT_TRUE(parent.windowState() & Qt::WindowMaximized);
}

// ---- OpenSchedule ----

TEST(CalendarAdaptorTest, OpenSchedule_InvokesSlotWithJob)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString job = QStringLiteral("schedule-123");
    adaptor.OpenSchedule(job);
    pumpEvents();
    EXPECT_EQ(parent.lastOpenScheduleJob, job);
}

TEST(CalendarAdaptorTest, OpenSchedule_EmptyJobStillInvokesSlot)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    adaptor.OpenSchedule(QString());
    pumpEvents();
    EXPECT_TRUE(parent.lastOpenScheduleJob.isEmpty());
}

// ---- QuerySchedules ----

TEST(CalendarAdaptorTest, QuerySchedules_Today_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.QuerySchedules(QStringLiteral("today"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("count").toInt(), 0);
    EXPECT_EQ(obj.value("type").toString(), QStringLiteral("single_day"));
}

TEST(CalendarAdaptorTest, QuerySchedules_Day_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.QuerySchedules(QStringLiteral("day"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("type").toString(), QStringLiteral("single_day"));
}

TEST(CalendarAdaptorTest, QuerySchedules_SpecificDate_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.QuerySchedules(QStringLiteral("2026-01-15"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("type").toString(), QStringLiteral("single_day"));
    EXPECT_EQ(obj.value("date").toString(), QStringLiteral("2026-01-15"));
}

TEST(CalendarAdaptorTest, QuerySchedules_DateRange_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.QuerySchedules(QStringLiteral("2026-01-01,2026-01-31"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("type").toString(), QStringLiteral("date_range"));
}

TEST(CalendarAdaptorTest, QuerySchedules_SearchKeyword_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.QuerySchedules(QStringLiteral("search:meeting"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("type").toString(), QStringLiteral("search"));
    EXPECT_EQ(obj.value("keyword").toString(), QStringLiteral("meeting"));
}

TEST(CalendarAdaptorTest, QuerySchedules_InvalidDate_ReturnsSuccessWithNoType)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.QuerySchedules(QStringLiteral("not-a-date"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("count").toInt(), 0);
}

// ---- CreateSchedule ----

TEST(CalendarAdaptorTest, CreateSchedule_InvalidJson_ReturnsEmpty)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.CreateSchedule(QStringLiteral("not json"));
    EXPECT_TRUE(result.isEmpty());
}

TEST(CalendarAdaptorTest, CreateSchedule_MissingTitle_ReturnsEmpty)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString json = QStringLiteral(R"({"startTime":"2026-01-15T10:00:00"})");
    QString result = adaptor.CreateSchedule(json);
    EXPECT_TRUE(result.isEmpty());
}

TEST(CalendarAdaptorTest, CreateSchedule_MissingStartTime_ReturnsEmpty)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString json = QStringLiteral(R"({"title":"Test Event"})");
    QString result = adaptor.CreateSchedule(json);
    EXPECT_TRUE(result.isEmpty());
}

TEST(CalendarAdaptorTest, CreateSchedule_ValidJson_NoAccount_ReturnsEmpty)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString json = QStringLiteral(R"({"title":"Test","startTime":"2026-01-15T10:00:00"})");
    QString result = adaptor.CreateSchedule(json);
    // Without a running backend, no account is available so creation fails.
    EXPECT_TRUE(result.isEmpty());
}

// ---- ModifySchedule ----

TEST(CalendarAdaptorTest, ModifySchedule_EmptyScheduleId_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_FALSE(adaptor.ModifySchedule(QString(), QStringLiteral("delete"), QString()));
}

TEST(CalendarAdaptorTest, ModifySchedule_EmptyOperation_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_FALSE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QString(), QString()));
}

TEST(CalendarAdaptorTest, ModifySchedule_UnknownOperation_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_FALSE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QStringLiteral("unknown"), QString()));
}

TEST(CalendarAdaptorTest, ModifySchedule_SnoozePositiveMinutes_ReturnsTrue)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_TRUE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QStringLiteral("snooze"), QStringLiteral("5")));
}

TEST(CalendarAdaptorTest, ModifySchedule_SnoozeZeroMinutes_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_FALSE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QStringLiteral("snooze"), QStringLiteral("0")));
}

TEST(CalendarAdaptorTest, ModifySchedule_SnoozeNegativeMinutes_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_FALSE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QStringLiteral("snooze"), QStringLiteral("-10")));
}

TEST(CalendarAdaptorTest, ModifySchedule_UpdateInvalidJson_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_FALSE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QStringLiteral("update"), QStringLiteral("bad json")));
}

TEST(CalendarAdaptorTest, ModifySchedule_Delete_NoAccount_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    EXPECT_FALSE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QStringLiteral("delete"), QString()));
}

TEST(CalendarAdaptorTest, ModifySchedule_Update_NoAccount_ReturnsFalse)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString json = QStringLiteral(R"({"title":"Updated"})");
    EXPECT_FALSE(adaptor.ModifySchedule(QStringLiteral("sched-1"), QStringLiteral("update"), json));
}

// ---- GetCalendarView ----

TEST(CalendarAdaptorTest, GetCalendarView_DayView_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetCalendarView(QStringLiteral("day"), QStringLiteral("2026-01-15"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("viewType").toString(), QStringLiteral("day"));
}

TEST(CalendarAdaptorTest, GetCalendarView_WeekView_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetCalendarView(QStringLiteral("week"), QStringLiteral("2026-01-15"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("viewType").toString(), QStringLiteral("week"));
    EXPECT_TRUE(obj.contains("days"));
}

TEST(CalendarAdaptorTest, GetCalendarView_MonthView_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetCalendarView(QStringLiteral("month"), QStringLiteral("2026-01-15"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("viewType").toString(), QStringLiteral("month"));
    EXPECT_EQ(obj.value("year").toInt(), 2026);
    EXPECT_EQ(obj.value("month").toInt(), 1);
}

TEST(CalendarAdaptorTest, GetCalendarView_InvalidDate_DefaultsToCurrentDate)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetCalendarView(QStringLiteral("day"), QStringLiteral("invalid"));
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("viewType").toString(), QStringLiteral("day"));
    EXPECT_EQ(obj.value("date").toString(), QDate::currentDate().toString("yyyy-MM-dd"));
}

// ---- GetLunarInfo ----

TEST(CalendarAdaptorTest, GetLunarInfo_InvalidDate_ReturnsError)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetLunarInfo(QStringLiteral("not-a-date"));
    QJsonObject obj = parseJson(result);
    EXPECT_FALSE(obj.value("success").toBool());
    EXPECT_TRUE(obj.contains("error"));
}

TEST(CalendarAdaptorTest, GetLunarInfo_ImpossibleDate_ReturnsError)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetLunarInfo(QStringLiteral("2026-02-30"));
    QJsonObject obj = parseJson(result);
    EXPECT_FALSE(obj.value("success").toBool());
    EXPECT_TRUE(obj.contains("error"));
}

TEST(CalendarAdaptorTest, GetLunarInfo_ValidDate_ReturnsSolarDate)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetLunarInfo(QStringLiteral("2026-01-15"));
    QJsonObject obj = parseJson(result);
    // Even if the lunar service is unavailable, solarDate should be set.
    EXPECT_EQ(obj.value("solarDate").toString(), QStringLiteral("2026-01-15"));
}

// ---- GetReminders ----

TEST(CalendarAdaptorTest, GetReminders_DefaultHours_ReturnsSuccess)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetReminders(24);
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("count").toInt(), 0);
}

TEST(CalendarAdaptorTest, GetReminders_ZeroHours_DefaultsToTwentyFour)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetReminders(0);
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("timeRange").toInt(), 24);
}

TEST(CalendarAdaptorTest, GetReminders_NegativeHours_DefaultsToTwentyFour)
{
    MockCalendarParent parent;
    CalendarAdaptor adaptor(&parent);
    QString result = adaptor.GetReminders(-5);
    QJsonObject obj = parseJson(result);
    EXPECT_TRUE(obj.value("success").toBool());
    EXPECT_EQ(obj.value("timeRange").toInt(), 24);
}

#include "test_dbuscalendar_adaptor.moc"
