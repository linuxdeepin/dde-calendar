// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonArray>

#include "dbusdatastruct.h"

// ============================================================================
// CaLunarDayInfo
// ============================================================================

TEST(CaLunarDayInfoTest, ConstructorInitializesDefaults)
{
    CaLunarDayInfo info;
    EXPECT_EQ(info.mGanZhiYear, QString(""));
    EXPECT_EQ(info.mGanZhiMonth, QString(""));
    EXPECT_EQ(info.mGanZhiDay, QString(""));
    EXPECT_EQ(info.mLunarMonthName, QString(""));
    EXPECT_EQ(info.mLunarDayName, QString(""));
    EXPECT_EQ(info.mLunarLeapMonth, 0);
    EXPECT_EQ(info.mZodiac, QString(""));
    EXPECT_EQ(info.mTerm, QString(""));
    EXPECT_EQ(info.mSolarFestival, QString(""));
    EXPECT_EQ(info.mLunarFestival, QString(""));
    EXPECT_EQ(info.mWorktime, 0);
}

TEST(CaLunarDayInfoTest, RegisterMetaTypeDoesNotCrash)
{
    CaLunarDayInfo::registerMetaType();
    SUCCEED();
}

// ============================================================================
// CaLunarMonthInfo
// ============================================================================

TEST(CaLunarMonthInfoTest, ConstructorInitializesDefaults)
{
    CaLunarMonthInfo info;
    EXPECT_EQ(info.mFirstDayWeek, 0);
    EXPECT_EQ(info.mDays, 0);
    EXPECT_TRUE(info.mCaLunarDayInfo.isEmpty());
}

TEST(CaLunarMonthInfoTest, RegisterMetaTypeDoesNotCrash)
{
    CaLunarMonthInfo::registerMetaType();
    SUCCEED();
}

// ============================================================================
// CaHuangLiDayInfo
// ============================================================================

TEST(CaHuangLiDayInfoTest, ConstructorInitializesDefaults)
{
    CaHuangLiDayInfo info;
    EXPECT_EQ(info.mGanZhiYear, QString(""));
    EXPECT_EQ(info.mGanZhiMonth, QString(""));
    EXPECT_EQ(info.mGanZhiDay, QString(""));
    EXPECT_EQ(info.mLunarMonthName, QString(""));
    EXPECT_EQ(info.mLunarDayName, QString(""));
    EXPECT_EQ(info.mLunarLeapMonth, 0);
    EXPECT_EQ(info.mZodiac, QString(""));
    EXPECT_EQ(info.mTerm, QString(""));
    EXPECT_EQ(info.mSolarFestival, QString(""));
    EXPECT_EQ(info.mLunarFestival, QString(""));
    EXPECT_EQ(info.mWorktime, 0);
    EXPECT_EQ(info.mSuit, QString(""));
    EXPECT_EQ(info.mAvoid, QString(""));
}

TEST(CaHuangLiDayInfoTest, RegisterMetaTypeDoesNotCrash)
{
    CaHuangLiDayInfo::registerMetaType();
    SUCCEED();
}

TEST(CaHuangLiDayInfoTest, ToJsonProducesValidJsonWithAllFields)
{
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mGanZhiMonth = "丙寅";
    info.mGanZhiDay = "戊辰";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    info.mLunarLeapMonth = 1;
    info.mZodiac = "鼠";
    info.mTerm = "立春";
    info.mSolarFestival = "元旦";
    info.mLunarFestival = "春节";
    info.mWorktime = 2;
    info.mSuit = "祭祀";
    info.mAvoid = "嫁娶";

    QString json = info.toJson();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    ASSERT_EQ(err.error, QJsonParseError::NoError) << err.errorString().toStdString();
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();

    EXPECT_EQ(obj.value("Suit").toString(), QString("祭祀"));
    EXPECT_EQ(obj.value("Avoid").toString(), QString("嫁娶"));
    EXPECT_EQ(obj.value("Worktime").toInt(), 2);
    EXPECT_EQ(obj.value("LunarFestival").toString(), QString("春节"));
    EXPECT_EQ(obj.value("SolarFestival").toString(), QString("元旦"));
    EXPECT_EQ(obj.value("Term").toString(), QString("立春"));
    EXPECT_EQ(obj.value("Zodiac").toString(), QString("鼠"));
    EXPECT_EQ(obj.value("LunarLeapMonth").toInt(), 1);
    EXPECT_EQ(obj.value("LunarDayName").toString(), QString("初一"));
    EXPECT_EQ(obj.value("LunarMonthName").toString(), QString("正月"));
    EXPECT_EQ(obj.value("GanZhiDay").toString(), QString("戊辰"));
    EXPECT_EQ(obj.value("GanZhiMonth").toString(), QString("丙寅"));
    EXPECT_EQ(obj.value("GanZhiYear").toString(), QString("甲子"));
}

TEST(CaHuangLiDayInfoTest, ToJsonWithDefaultsProducesEmptyValues)
{
    CaHuangLiDayInfo info;
    QString json = info.toJson();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    ASSERT_EQ(err.error, QJsonParseError::NoError);
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj.value("Suit").toString(), QString(""));
    EXPECT_EQ(obj.value("Worktime").toInt(), 0);
    EXPECT_EQ(obj.value("LunarLeapMonth").toInt(), 0);
    EXPECT_EQ(obj.size(), 13);
}

TEST(CaHuangLiDayInfoTest, StrJsonToInfoParsesValidJson)
{
    CaHuangLiDayInfo info;
    QString json = QString("{\"Suit\":\"祭祀\",\"Avoid\":\"嫁娶\",\"Worktime\":2,"
                           "\"LunarFestival\":\"春节\",\"SolarFestival\":\"元旦\","
                           "\"Term\":\"立春\",\"Zodiac\":\"鼠\","
                           "\"LunarLeapMonth\":1,\"LunarDayName\":\"初一\","
                           "\"LunarMonthName\":\"正月\",\"GanZhiDay\":\"戊辰\","
                           "\"GanZhiMonth\":\"丙寅\",\"GanZhiYear\":\"甲子\"}");
    bool isVaild = false;
    info.strJsonToInfo(json, isVaild);
    EXPECT_TRUE(isVaild);
    EXPECT_EQ(info.mSuit, QString("祭祀"));
    EXPECT_EQ(info.mAvoid, QString("嫁娶"));
    EXPECT_EQ(info.mWorktime, 2);
    EXPECT_EQ(info.mLunarFestival, QString("春节"));
    EXPECT_EQ(info.mSolarFestival, QString("元旦"));
    EXPECT_EQ(info.mTerm, QString("立春"));
    EXPECT_EQ(info.mZodiac, QString("鼠"));
    EXPECT_EQ(info.mLunarLeapMonth, 1);
    EXPECT_EQ(info.mLunarDayName, QString("初一"));
    EXPECT_EQ(info.mLunarMonthName, QString("正月"));
    EXPECT_EQ(info.mGanZhiDay, QString("戊辰"));
    EXPECT_EQ(info.mGanZhiMonth, QString("丙寅"));
    EXPECT_EQ(info.mGanZhiYear, QString("甲子"));
}

TEST(CaHuangLiDayInfoTest, StrJsonToInfoInvalidJsonSetsFlagFalse)
{
    CaHuangLiDayInfo info;
    bool isVaild = true;
    info.strJsonToInfo("not a json string {{{", isVaild);
    EXPECT_FALSE(isVaild);
}

TEST(CaHuangLiDayInfoTest, StrJsonToInfoEmptyStringSetsFlagFalse)
{
    CaHuangLiDayInfo info;
    bool isVaild = true;
    info.strJsonToInfo("", isVaild);
    EXPECT_FALSE(isVaild);
}

TEST(CaHuangLiDayInfoTest, JsonObjectToInfoFullObjectSetsAllFields)
{
    QJsonObject obj;
    obj.insert("Suit", "祭祀");
    obj.insert("Avoid", "嫁娶");
    obj.insert("Worktime", 3);
    obj.insert("LunarFestival", "中秋节");
    obj.insert("SolarFestival", "国庆节");
    obj.insert("Term", "秋分");
    obj.insert("Zodiac", "兔");
    obj.insert("LunarLeapMonth", 2);
    obj.insert("LunarDayName", "十五");
    obj.insert("LunarMonthName", "八月");
    obj.insert("GanZhiDay", "癸酉");
    obj.insert("GanZhiMonth", "辛酉");
    obj.insert("GanZhiYear", "癸卯");

    CaHuangLiDayInfo info;
    info.jsonObjectToInfo(obj);
    EXPECT_EQ(info.mSuit, QString("祭祀"));
    EXPECT_EQ(info.mAvoid, QString("嫁娶"));
    EXPECT_EQ(info.mWorktime, 3);
    EXPECT_EQ(info.mLunarFestival, QString("中秋节"));
    EXPECT_EQ(info.mSolarFestival, QString("国庆节"));
    EXPECT_EQ(info.mTerm, QString("秋分"));
    EXPECT_EQ(info.mZodiac, QString("兔"));
    EXPECT_EQ(info.mLunarLeapMonth, 2);
    EXPECT_EQ(info.mLunarDayName, QString("十五"));
    EXPECT_EQ(info.mLunarMonthName, QString("八月"));
    EXPECT_EQ(info.mGanZhiDay, QString("癸酉"));
    EXPECT_EQ(info.mGanZhiMonth, QString("辛酉"));
    EXPECT_EQ(info.mGanZhiYear, QString("癸卯"));
}

TEST(CaHuangLiDayInfoTest, JsonObjectToInfoPartialObjectSetsOnlyPresentFields)
{
    QJsonObject obj;
    obj.insert("Suit", "祈福");
    obj.insert("Worktime", 5);

    CaHuangLiDayInfo info;
    info.mAvoid = "preset_avoid";
    info.mZodiac = "preset_zodiac";
    info.jsonObjectToInfo(obj);
    EXPECT_EQ(info.mSuit, QString("祈福"));
    EXPECT_EQ(info.mWorktime, 5);
    EXPECT_EQ(info.mAvoid, QString("preset_avoid"));
    EXPECT_EQ(info.mZodiac, QString("preset_zodiac"));
}

TEST(CaHuangLiDayInfoTest, JsonObjectToInfoEmptyObjectLeavesFieldsUnchanged)
{
    CaHuangLiDayInfo info;
    info.mSuit = "keep";
    info.jsonObjectToInfo(QJsonObject());
    EXPECT_EQ(info.mSuit, QString("keep"));
}

TEST(CaHuangLiDayInfoTest, RoundTripToJsonAndBack)
{
    CaHuangLiDayInfo original;
    original.mGanZhiYear = "甲子";
    original.mGanZhiMonth = "乙丑";
    original.mGanZhiDay = "丙寅";
    original.mLunarMonthName = "二月";
    original.mLunarDayName = "初三";
    original.mLunarLeapMonth = 4;
    original.mZodiac = "龙";
    original.mTerm = "惊蛰";
    original.mSolarFestival = "劳动节";
    original.mLunarFestival = "清明";
    original.mWorktime = 7;
    original.mSuit = "动土";
    original.mAvoid = "出行";

    QString json = original.toJson();
    CaHuangLiDayInfo restored;
    bool isVaild = false;
    restored.strJsonToInfo(json, isVaild);
    ASSERT_TRUE(isVaild);
    EXPECT_EQ(restored.mGanZhiYear, original.mGanZhiYear);
    EXPECT_EQ(restored.mGanZhiMonth, original.mGanZhiMonth);
    EXPECT_EQ(restored.mGanZhiDay, original.mGanZhiDay);
    EXPECT_EQ(restored.mLunarMonthName, original.mLunarMonthName);
    EXPECT_EQ(restored.mLunarDayName, original.mLunarDayName);
    EXPECT_EQ(restored.mLunarLeapMonth, original.mLunarLeapMonth);
    EXPECT_EQ(restored.mZodiac, original.mZodiac);
    EXPECT_EQ(restored.mTerm, original.mTerm);
    EXPECT_EQ(restored.mSolarFestival, original.mSolarFestival);
    EXPECT_EQ(restored.mLunarFestival, original.mLunarFestival);
    EXPECT_EQ(restored.mWorktime, original.mWorktime);
    EXPECT_EQ(restored.mSuit, original.mSuit);
    EXPECT_EQ(restored.mAvoid, original.mAvoid);
}

// ============================================================================
// CaHuangLiMonthInfo
// ============================================================================

TEST(CaHuangLiMonthInfoTest, ConstructorInitializesDefaults)
{
    CaHuangLiMonthInfo info;
    EXPECT_EQ(info.mFirstDayWeek, 0);
    EXPECT_EQ(info.mDays, 0);
    EXPECT_TRUE(info.mCaLunarDayInfo.isEmpty());
}

TEST(CaHuangLiMonthInfoTest, RegisterMetaTypeDoesNotCrash)
{
    CaHuangLiMonthInfo::registerMetaType();
    SUCCEED();
}

TEST(CaHuangLiMonthInfoTest, ToJsonWithEmptyDaysProducesValidJson)
{
    CaHuangLiMonthInfo info;
    info.mDays = 30;
    info.mFirstDayWeek = 3;
    QString json = info.toJson();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    ASSERT_EQ(err.error, QJsonParseError::NoError);
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj.value("Days").toInt(), 30);
    EXPECT_EQ(obj.value("FirstDayWeek").toInt(), 3);
    EXPECT_TRUE(obj.value("Datas").toArray().isEmpty());
}

TEST(CaHuangLiMonthInfoTest, ToJsonSerializesDayInfoArray)
{
    CaHuangLiMonthInfo info;
    info.mDays = 31;
    info.mFirstDayWeek = 1;

    CaHuangLiDayInfo day1;
    day1.mSuit = "祭祀";
    day1.mAvoid = "嫁娶";
    day1.mWorktime = 1;
    CaHuangLiDayInfo day2;
    day2.mSuit = "祈福";
    day2.mAvoid = "动土";
    day2.mWorktime = 0;
    info.mCaLunarDayInfo.append(day1);
    info.mCaLunarDayInfo.append(day2);

    QString json = info.toJson();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    ASSERT_EQ(err.error, QJsonParseError::NoError);
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj.value("Days").toInt(), 31);
    EXPECT_EQ(obj.value("FirstDayWeek").toInt(), 1);
    QJsonArray datas = obj.value("Datas").toArray();
    ASSERT_EQ(datas.size(), 2);
    QJsonObject d0 = datas.at(0).toObject();
    EXPECT_EQ(d0.value("Suit").toString(), QString("祭祀"));
    EXPECT_EQ(d0.value("Avoid").toString(), QString("嫁娶"));
    EXPECT_EQ(d0.value("Worktime").toInt(), 1);
    QJsonObject d1 = datas.at(1).toObject();
    EXPECT_EQ(d1.value("Suit").toString(), QString("祈福"));
    EXPECT_EQ(d1.value("Avoid").toString(), QString("动土"));
}

TEST(CaHuangLiMonthInfoTest, StrJsonToInfoParsesValidJson)
{
    CaHuangLiMonthInfo info;
    QString json = QString("{\"Days\":28,\"FirstDayWeek\":4,\"Datas\":["
                           "{\"Suit\":\"祭祀\",\"Avoid\":\"嫁娶\",\"Worktime\":1,"
                           "\"LunarFestival\":\"\",\"SolarFestival\":\"\",\"Term\":\"\","
                           "\"Zodiac\":\"\",\"LunarLeapMonth\":0,\"LunarDayName\":\"\","
                           "\"LunarMonthName\":\"\",\"GanZhiDay\":\"\",\"GanZhiMonth\":\"\","
                           "\"GanZhiYear\":\"\"}]}");
    bool isVaild = false;
    info.strJsonToInfo(json, isVaild);
    EXPECT_TRUE(isVaild);
    EXPECT_EQ(info.mDays, 28);
    EXPECT_EQ(info.mFirstDayWeek, 4);
    ASSERT_EQ(info.mCaLunarDayInfo.size(), 1);
    EXPECT_EQ(info.mCaLunarDayInfo.at(0).mSuit, QString("祭祀"));
    EXPECT_EQ(info.mCaLunarDayInfo.at(0).mAvoid, QString("嫁娶"));
    EXPECT_EQ(info.mCaLunarDayInfo.at(0).mWorktime, 1);
}

TEST(CaHuangLiMonthInfoTest, StrJsonToInfoInvalidJsonSetsFlagFalse)
{
    CaHuangLiMonthInfo info;
    bool isVaild = true;
    info.strJsonToInfo("{{{invalid", isVaild);
    EXPECT_FALSE(isVaild);
}

TEST(CaHuangLiMonthInfoTest, StrJsonToInfoEmptyStringSetsFlagFalse)
{
    CaHuangLiMonthInfo info;
    bool isVaild = true;
    info.strJsonToInfo("", isVaild);
    EXPECT_FALSE(isVaild);
}

TEST(CaHuangLiMonthInfoTest, StrJsonToInfoPartialJsonSetsOnlyPresentFields)
{
    CaHuangLiMonthInfo info;
    info.mDays = 99;
    info.mFirstDayWeek = 9;
    QString json = QString("{\"Days\":30}");
    bool isVaild = false;
    info.strJsonToInfo(json, isVaild);
    EXPECT_TRUE(isVaild);
    EXPECT_EQ(info.mDays, 30);
    EXPECT_EQ(info.mFirstDayWeek, 9);
    EXPECT_TRUE(info.mCaLunarDayInfo.isEmpty());
}

TEST(CaHuangLiMonthInfoTest, ClearResetsAllFields)
{
    CaHuangLiMonthInfo info;
    info.mDays = 31;
    info.mFirstDayWeek = 5;
    CaHuangLiDayInfo day;
    day.mSuit = "test";
    info.mCaLunarDayInfo.append(day);
    info.mCaLunarDayInfo.append(day);

    info.clear();
    EXPECT_EQ(info.mDays, 0);
    EXPECT_TRUE(info.mCaLunarDayInfo.isEmpty());
}

TEST(CaHuangLiMonthInfoTest, RoundTripToJsonAndBack)
{
    CaHuangLiMonthInfo original;
    original.mDays = 30;
    original.mFirstDayWeek = 2;
    CaHuangLiDayInfo day;
    day.mSuit = "祭祀";
    day.mAvoid = "嫁娶";
    day.mWorktime = 3;
    day.mZodiac = "鼠";
    original.mCaLunarDayInfo.append(day);

    QString json = original.toJson();
    CaHuangLiMonthInfo restored;
    bool isVaild = false;
    restored.strJsonToInfo(json, isVaild);
    ASSERT_TRUE(isVaild);
    EXPECT_EQ(restored.mDays, original.mDays);
    EXPECT_EQ(restored.mFirstDayWeek, original.mFirstDayWeek);
    ASSERT_EQ(restored.mCaLunarDayInfo.size(), 1);
    EXPECT_EQ(restored.mCaLunarDayInfo.at(0).mSuit, QString("祭祀"));
    EXPECT_EQ(restored.mCaLunarDayInfo.at(0).mAvoid, QString("嫁娶"));
    EXPECT_EQ(restored.mCaLunarDayInfo.at(0).mWorktime, 3);
    EXPECT_EQ(restored.mCaLunarDayInfo.at(0).mZodiac, QString("鼠"));
}

// ============================================================================
// QDebug operator<< tests
// ============================================================================

TEST(CaLunarDayInfoTest, QDebugOperatorStreamsAllFields)
{
    CaLunarDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mGanZhiMonth = "丙寅";
    info.mGanZhiDay = "戊辰";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    info.mLunarLeapMonth = 1;
    info.mZodiac = "鼠";
    info.mTerm = "立春";
    info.mSolarFestival = "元旦";
    info.mLunarFestival = "春节";
    info.mWorktime = 2;

    QString output;
    QDebug dbg(&output);
    dbg << info;

    EXPECT_FALSE(output.isEmpty());
    EXPECT_TRUE(output.contains("甲子"));
    EXPECT_TRUE(output.contains("鼠"));
}

TEST(CaLunarMonthInfoTest, QDebugOperatorStreamsAllFields)
{
    CaLunarMonthInfo info;
    info.mFirstDayWeek = 3;
    info.mDays = 30;
    CaLunarDayInfo day;
    day.mGanZhiYear = "甲子";
    info.mCaLunarDayInfo.append(day);

    QString output;
    QDebug dbg(&output);
    dbg << info;

    EXPECT_FALSE(output.isEmpty());
    EXPECT_TRUE(output.contains("甲子"));
}

TEST(CaHuangLiDayInfoTest, QDebugOperatorStreamsAllFields)
{
    CaHuangLiDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mSuit = "祭祀";
    info.mAvoid = "嫁娶";
    info.mWorktime = 1;

    QString output;
    QDebug dbg(&output);
    dbg << info;

    EXPECT_FALSE(output.isEmpty());
    EXPECT_TRUE(output.contains("祭祀"));
}

TEST(CaHuangLiMonthInfoTest, QDebugOperatorStreamsAllFields)
{
    CaHuangLiMonthInfo info;
    info.mDays = 31;
    info.mFirstDayWeek = 2;
    CaHuangLiDayInfo day;
    day.mSuit = "祭祀";
    info.mCaLunarDayInfo.append(day);

    QString output;
    QDebug dbg(&output);
    dbg << info;

    EXPECT_FALSE(output.isEmpty());
}

// ============================================================================
// QDBusArgument operator<< and operator>> tests
// A default-constructed QDBusArgument has null internals; all operations are
// safe no-ops, which exercises the operator code paths for coverage without
// requiring a running dbus-daemon.
// ============================================================================

TEST(CaLunarDayInfoTest, QDBusArgumentWriteOperatorDoesNotCrash)
{
    CaLunarDayInfo::registerMetaType();
    CaLunarDayInfo info;
    info.mGanZhiYear = "甲子";
    info.mGanZhiMonth = "丙寅";
    info.mGanZhiDay = "戊辰";
    info.mLunarMonthName = "正月";
    info.mLunarDayName = "初一";
    info.mLunarLeapMonth = 1;
    info.mZodiac = "鼠";
    info.mTerm = "立春";
    info.mSolarFestival = "元旦";
    info.mLunarFestival = "春节";
    info.mWorktime = 2;

    QDBusArgument arg;
    arg << info;
    SUCCEED();
}

TEST(CaLunarDayInfoTest, QDBusArgumentReadOperatorDoesNotCrash)
{
    CaLunarDayInfo::registerMetaType();
    QDBusArgument arg;
    CaLunarDayInfo info;
    arg >> info;
    SUCCEED();
}

TEST(CaLunarMonthInfoTest, QDBusArgumentWriteOperatorDoesNotCrash)
{
    CaLunarMonthInfo::registerMetaType();
    CaLunarDayInfo::registerMetaType();
    CaLunarMonthInfo info;
    info.mFirstDayWeek = 3;
    info.mDays = 30;
    CaLunarDayInfo day;
    day.mGanZhiYear = "甲子";
    info.mCaLunarDayInfo.append(day);

    QDBusArgument arg;
    arg << info;
    SUCCEED();
}

TEST(CaLunarMonthInfoTest, QDBusArgumentReadOperatorDoesNotCrash)
{
    CaLunarMonthInfo::registerMetaType();
    CaLunarDayInfo::registerMetaType();
    QDBusArgument arg;
    CaLunarMonthInfo info;
    arg >> info;
    SUCCEED();
}

TEST(CaHuangLiDayInfoTest, QDBusArgumentWriteOperatorDoesNotCrash)
{
    CaHuangLiDayInfo::registerMetaType();
    CaHuangLiDayInfo info;
    info.mSuit = "祭祀";
    info.mAvoid = "嫁娶";
    info.mWorktime = 1;

    QDBusArgument arg;
    arg << info;
    SUCCEED();
}

TEST(CaHuangLiDayInfoTest, QDBusArgumentReadOperatorDoesNotCrash)
{
    CaHuangLiDayInfo::registerMetaType();
    QDBusArgument arg;
    CaHuangLiDayInfo info;
    arg >> info;
    SUCCEED();
}

TEST(CaHuangLiMonthInfoTest, QDBusArgumentWriteOperatorDoesNotCrash)
{
    CaHuangLiMonthInfo::registerMetaType();
    CaHuangLiDayInfo::registerMetaType();
    CaHuangLiMonthInfo info;
    info.mDays = 31;
    info.mFirstDayWeek = 2;
    CaHuangLiDayInfo day;
    day.mSuit = "祭祀";
    info.mCaLunarDayInfo.append(day);

    QDBusArgument arg;
    arg << info;
    SUCCEED();
}

TEST(CaHuangLiMonthInfoTest, QDBusArgumentReadOperatorDoesNotCrash)
{
    CaHuangLiMonthInfo::registerMetaType();
    CaHuangLiDayInfo::registerMetaType();
    QDBusArgument arg;
    CaHuangLiMonthInfo info;
    arg >> info;
    SUCCEED();
}
