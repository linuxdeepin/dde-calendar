// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calendarhuangli.h"
#include "dhuanglidatabase.h"
#include "ddatabase.h"
#include "units.h"
#include "../third-party_stub/stub.h"
#include "config.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>

static QString g_huangliTestConfigPath;

static QDir stub_getAppConfigDir()
{
    return QDir(g_huangliTestConfigPath);
}

static QDir stub_getAppCacheDir()
{
    return QDir(g_huangliTestConfigPath);
}

static QSharedPointer<QProcess> stub_huangliDownloadFile(QString url, QString filename)
{
    Q_UNUSED(url)
    Q_UNUSED(filename)
    return QSharedPointer<QProcess>(new QProcess);
}

test_calendarhuangli::test_calendarhuangli()
{
    g_huangliTestConfigPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_stub = new Stub();
    m_stub->set(getAppConfigDir, stub_getAppConfigDir);
    m_stub->set(getAppCacheDir, stub_getAppCacheDir);
    m_stub->set(DownloadFile, stub_huangliDownloadFile);
    calendarHuangLi = new CalendarHuangLi();

    // Redirect database to real huangli.db
    calendarHuangLi->m_database->m_database.close();
    calendarHuangLi->m_database->m_database.setDatabaseName(HL_DATABASE_DIR);
    calendarHuangLi->m_database->m_database.open();
}

test_calendarhuangli::~test_calendarhuangli()
{
    if (calendarHuangLi) {
        if (calendarHuangLi->m_database->m_database.isOpen()) {
            calendarHuangLi->m_database->m_database.close();
        }
        delete calendarHuangLi;
        calendarHuangLi = nullptr;
    }
    delete m_stub;
    m_stub = nullptr;
    QSqlDatabase::removeDatabase("HuangLi");
}

//QString CalendarHuangLi::GetFestivalMonth(quint32 year, quint32 month)
TEST_F(test_calendarhuangli, GetFestivalMonth)
{
    quint32 year = 2020;
    quint32 month = 12;
    QJsonArray fesMonth = calendarHuangLi->getFestivalMonth(year, month);
    for (const QJsonValue &val : fesMonth) {
        EXPECT_TRUE(val.isObject());
    }
}

//QString CalendarHuangLi::GetHuangLiDay(quint32 year, quint32 month, quint32 day)
TEST_F(test_calendarhuangli, GetHuangLiDay)
{
    //2020年12月13日黄历信息
    const QString huangli_20201213 = "{\"Avoid\":\"嫁娶.祈福.出火.移徙.入宅.\",\"GanZhiDay\":\"庚寅\",\"GanZhiMonth\":\"戊子\","
                                     "\"GanZhiYear\":\"庚子\",\"LunarDayName\":\"廿九\",\"LunarFestival\":\"\",\"LunarLeapMonth\":0,"
                                     "\"LunarMonthName\":\"十月\",\"SolarFestival\":\"南京大屠杀死难者国家公祭日\","
                                     "\"Suit\":\"纳财.开市.交易.立券.会亲友.进人口.经络.祭祀.祈福.安香.出火.求医.治病.修造.动土.拆卸.扫舍.安床."
                                     "栽种.牧养.开生坟.合寿木.入殓.安葬.启攒.\",\"Term\":\"\",\"Worktime\":0,\"Zodiac\":\"鼠\"}";
    quint32 year = 2020;
    quint32 month = 12;
    quint32 day = 13;
    QString gethuangli = calendarHuangLi->getHuangLiDay(year, month, day);
    EXPECT_EQ(huangli_20201213, gethuangli);

    //2020年12月14日黄历信息
    const QString huangli_20201214 = "{\"Avoid\":\"入宅.修造.动土.破土.安门.上梁.\",\"GanZhiDay\":\"辛卯\",\"GanZhiMonth\":\"戊子\","
                                     "\"GanZhiYear\":\"庚子\",\"LunarDayName\":\"三十\",\"LunarFestival\":\"\",\"LunarLeapMonth\":0,"
                                     "\"LunarMonthName\":\"十月\",\"SolarFestival\":\"\",\"Suit\":\"祭祀.入殓.移柩.余事勿取.\",\"Term\":"
                                     "\"\",\"Worktime\":0,\"Zodiac\":\"鼠\"}";
    day = 14;
    gethuangli = calendarHuangLi->getHuangLiDay(year, month, day);
    EXPECT_EQ(huangli_20201214, gethuangli);
}

//QString CalendarHuangLi::GetHuangLiMonth(quint32 year, quint32 month, bool fill)
TEST_F(test_calendarhuangli, GetHuangLiMonth)
{
    quint32 year = 2020;
    quint32 month = 12;

    bool fill = false;
    QString result = calendarHuangLi->getHuangLiMonth(year, month, fill);
    EXPECT_FALSE(result.isEmpty());
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    EXPECT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    EXPECT_TRUE(obj.contains("FirstDayWeek"));
    EXPECT_TRUE(obj.contains("Days"));
    EXPECT_TRUE(obj.contains("Datas"));

    fill = true;
    result = calendarHuangLi->getHuangLiMonth(year, month, fill);
    EXPECT_FALSE(result.isEmpty());
    doc = QJsonDocument::fromJson(result.toUtf8());
    EXPECT_TRUE(doc.isObject());
    obj = doc.object();
    EXPECT_TRUE(obj.contains("FirstDayWeek"));
    EXPECT_TRUE(obj.contains("Days"));
    EXPECT_TRUE(obj.contains("Datas"));
}

//CaLunarDayInfo CalendarHuangLi::GetLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
TEST_F(test_calendarhuangli, GetLunarInfoBySolar)
{
    quint32 year = 2020;
    quint32 month = 12;
    quint32 day = 13;
    CaLunarDayInfo huangliDayInfo = calendarHuangLi->getLunarInfoBySolar(year, month, day);
    EXPECT_EQ(QString("庚子"), huangliDayInfo.mGanZhiYear);
    EXPECT_EQ(QString("戊子"), huangliDayInfo.mGanZhiMonth);
    EXPECT_EQ(QString("庚寅"), huangliDayInfo.mGanZhiDay);
    EXPECT_EQ(QString("十月"), huangliDayInfo.mLunarMonthName);
    EXPECT_EQ(QString("廿九"), huangliDayInfo.mLunarDayName);
    EXPECT_EQ(QString("鼠"), huangliDayInfo.mZodiac);
    EXPECT_EQ(QString("南京大屠杀死难者国家公祭日"), huangliDayInfo.mSolarFestival);
}

//CaLunarMonthInfo CalendarHuangLi::GetLunarCalendarMonth(quint32 year, quint32 month, bool fill)
TEST_F(test_calendarhuangli, GetLunarCalendarMonth)
{
    quint32 year = 2020;
    quint32 month = 12;

    bool fill = false;
    CaLunarMonthInfo monthInfo = calendarHuangLi->getLunarCalendarMonth(year, month, fill);
    EXPECT_GT(monthInfo.mDays, 0);
    EXPECT_GE(monthInfo.mFirstDayWeek, 0);
    EXPECT_LE(monthInfo.mFirstDayWeek, 6);

    fill = true;
    monthInfo = calendarHuangLi->getLunarCalendarMonth(year, month, fill);
    EXPECT_GT(monthInfo.mDays, 0);
    EXPECT_GE(monthInfo.mFirstDayWeek, 0);
    EXPECT_LE(monthInfo.mFirstDayWeek, 6);
}

//Constructor test
TEST_F(test_calendarhuangli, Constructor)
{
    EXPECT_NE(nullptr, calendarHuangLi);
    EXPECT_NE(nullptr, calendarHuangLi->m_database);
}
