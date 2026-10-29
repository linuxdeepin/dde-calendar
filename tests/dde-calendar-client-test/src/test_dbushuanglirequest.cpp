// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbushuanglirequest.h"
#include "cdbuspendingcallwatcher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QVariant>

#include <gtest/gtest.h>

namespace {

const char kHuangLiPath[] = "/com/deepin/dataserver/Calendar/HuangLi";
const char kHuangLiInterface[] = "com.deepin.dataserver.Calendar.HuangLi";

class MockHuangLiService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dataserver.Calendar.HuangLi")

public slots:
    QString getFestivalMonth(quint32 year, quint32 month)
    {
        Q_UNUSED(year)
        Q_UNUSED(month)
        QJsonArray rootArr;
        QJsonObject monthObj;
        QJsonArray listArr;

        QJsonObject day1;
        day1["status"] = 1;
        day1["date"] = "2026-01-01";
        listArr.append(day1);

        QJsonObject day2;
        day2["status"] = 2;
        day2["date"] = "2026-01-02";
        listArr.append(day2);

        monthObj["list"] = listArr;
        rootArr.append(monthObj);
        return QString::fromUtf8(QJsonDocument(rootArr).toJson());
    }

    QString getHuangLiDay(quint32 year, quint32 month, quint32 day)
    {
        Q_UNUSED(year)
        Q_UNUSED(month)
        Q_UNUSED(day)
        QJsonObject obj;
        obj["GanZhiYear"] = "丙午";
        obj["GanZhiMonth"] = "庚寅";
        obj["GanZhiDay"] = "甲子";
        obj["LunarMonthName"] = "正月";
        obj["LunarDayName"] = "初一";
        obj["LunarLeapMonth"] = 0;
        obj["Zodiac"] = "马";
        obj["Term"] = "";
        obj["SolarFestival"] = "元旦";
        obj["LunarFestival"] = "";
        obj["Worktime"] = 0;
        obj["Suit"] = "祭祀";
        obj["Avoid"] = "动土";
        return QString::fromUtf8(QJsonDocument(obj).toJson());
    }

    QString getHuangLiMonth(quint32 year, quint32 month, bool fill)
    {
        Q_UNUSED(year)
        Q_UNUSED(month)
        Q_UNUSED(fill)
        QJsonObject obj;
        obj["FirstDayWeek"] = 3;
        obj["Days"] = 31;
        return QString::fromUtf8(QJsonDocument(obj).toJson());
    }

    QString getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
    {
        Q_UNUSED(year)
        Q_UNUSED(month)
        Q_UNUSED(day)
        return QStringLiteral("{\"LunarMonthName\":\"正月\"}");
    }

    QString getLunarMonthCalendar(quint32 year, quint32 month, bool fill)
    {
        Q_UNUSED(year)
        Q_UNUSED(month)
        Q_UNUSED(fill)
        return QStringLiteral("{\"Days\":30}");
    }
};

} // namespace

TEST(DbusHuangLiRequestTest, GetFestivalMonth)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockHuangLiService service;
    ASSERT_TRUE(bus.registerObject(QString::fromLatin1(kHuangLiPath), &service,
                                   QDBusConnection::ExportAllSlots));

    {
        DbusHuangLiRequest req;
        ASSERT_TRUE(req.isValid());

        FestivalInfo festivalInfo;
        bool result = req.getFestivalMonth(2026, 1, festivalInfo);
        EXPECT_TRUE(result);
        EXPECT_EQ(festivalInfo.listHoliday.size(), 2);
        if (festivalInfo.listHoliday.size() >= 2) {
            EXPECT_EQ(festivalInfo.listHoliday.at(0).status, static_cast<uint>(1));
            EXPECT_EQ(festivalInfo.listHoliday.at(0).date, QDate(2026, 1, 1));
            EXPECT_EQ(festivalInfo.listHoliday.at(1).status, static_cast<uint>(2));
            EXPECT_EQ(festivalInfo.listHoliday.at(1).date, QDate(2026, 1, 2));
        }
    }

    bus.unregisterObject(QString::fromLatin1(kHuangLiPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusHuangLiRequestTest, GetHuangLiDay)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockHuangLiService service;
    ASSERT_TRUE(bus.registerObject(QString::fromLatin1(kHuangLiPath), &service,
                                   QDBusConnection::ExportAllSlots));

    {
        DbusHuangLiRequest req;
        ASSERT_TRUE(req.isValid());

        CaHuangLiDayInfo info;
        bool result = req.getHuangLiDay(2026, 1, 1, info);
        EXPECT_TRUE(result);
        EXPECT_EQ(info.mGanZhiYear, QStringLiteral("丙午"));
        EXPECT_EQ(info.mSuit, QStringLiteral("祭祀"));
        EXPECT_EQ(info.mAvoid, QStringLiteral("动土"));
    }

    bus.unregisterObject(QString::fromLatin1(kHuangLiPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusHuangLiRequestTest, GetHuangLiMonth)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockHuangLiService service;
    ASSERT_TRUE(bus.registerObject(QString::fromLatin1(kHuangLiPath), &service,
                                   QDBusConnection::ExportAllSlots));

    {
        DbusHuangLiRequest req;
        ASSERT_TRUE(req.isValid());

        CaHuangLiMonthInfo info;
        bool result = req.getHuangLiMonth(2026, 1, false, info);
        EXPECT_TRUE(result);
        EXPECT_EQ(info.mFirstDayWeek, 3);
        EXPECT_EQ(info.mDays, 31);
    }

    bus.unregisterObject(QString::fromLatin1(kHuangLiPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusHuangLiRequestTest, GetLunarInfoBySolar)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockHuangLiService service;
    ASSERT_TRUE(bus.registerObject(QString::fromLatin1(kHuangLiPath), &service,
                                   QDBusConnection::ExportAllSlots));

    {
        DbusHuangLiRequest req;
        ASSERT_TRUE(req.isValid());

        req.getLunarInfoBySolar(2026, 1, 1);

        QEventLoop loop;
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();
    }

    bus.unregisterObject(QString::fromLatin1(kHuangLiPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusHuangLiRequestTest, GetLunarMonthCalendar)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    MockHuangLiService service;
    ASSERT_TRUE(bus.registerObject(QString::fromLatin1(kHuangLiPath), &service,
                                   QDBusConnection::ExportAllSlots));

    {
        DbusHuangLiRequest req;
        ASSERT_TRUE(req.isValid());

        req.getLunarMonthCalendar(2026, 1, true);

        QEventLoop loop;
        QTimer::singleShot(3000, &loop, &QEventLoop::quit);
        loop.exec();
    }

    bus.unregisterObject(QString::fromLatin1(kHuangLiPath));
    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

TEST(DbusHuangLiRequestTest, GetFestivalMonthInvalidJson)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        GTEST_SKIP() << "Session DBus is unavailable";
    }

    bool serviceRegistered = bus.registerService(QStringLiteral("com.deepin.dataserver.Calendar"));
    if (!serviceRegistered) {
        GTEST_SKIP() << "Calendar DBus service is already owned";
    }

    {
        DbusHuangLiRequest req;
        FestivalInfo festivalInfo;
        bool result = req.getFestivalMonth(2026, 1, festivalInfo);
        EXPECT_FALSE(result);
    }

    bus.unregisterService(QStringLiteral("com.deepin.dataserver.Calendar"));
}

#include "test_dbushuanglirequest.moc"
