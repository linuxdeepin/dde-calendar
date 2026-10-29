// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcalendargeneralsettings.h"

#include <gtest/gtest.h>

#include <QJsonDocument>
#include <QJsonObject>

namespace {

TEST(DCalendarGeneralSettings, DefaultConstructorSetsDefaults)
{
    DCalendarGeneralSettings settings;
    EXPECT_EQ(Qt::Sunday, settings.firstDayOfWeek());
    EXPECT_EQ(DCalendarGeneralSettings::TwentyFour, settings.timeShowType());
}

TEST(DCalendarGeneralSettings, CopyConstructorCopiesValues)
{
    DCalendarGeneralSettings original;
    original.setFirstDayOfWeek(Qt::Wednesday);
    original.setTimeShowType(DCalendarGeneralSettings::Twelve);

    DCalendarGeneralSettings copy(original);
    EXPECT_EQ(Qt::Wednesday, copy.firstDayOfWeek());
    EXPECT_EQ(DCalendarGeneralSettings::Twelve, copy.timeShowType());
}

TEST(DCalendarGeneralSettings, CloneProducesEqualCopy)
{
    DCalendarGeneralSettings original;
    original.setFirstDayOfWeek(Qt::Friday);
    original.setTimeShowType(DCalendarGeneralSettings::Twelve);

    std::unique_ptr<DCalendarGeneralSettings> cloned(original.clone());
    ASSERT_NE(nullptr, cloned.get());
    EXPECT_EQ(Qt::Friday, cloned->firstDayOfWeek());
    EXPECT_EQ(DCalendarGeneralSettings::Twelve, cloned->timeShowType());
}

TEST(DCalendarGeneralSettings, FirstDayOfWeekGetterSetter)
{
    DCalendarGeneralSettings settings;
    settings.setFirstDayOfWeek(Qt::Monday);
    EXPECT_EQ(Qt::Monday, settings.firstDayOfWeek());

    settings.setFirstDayOfWeek(Qt::Saturday);
    EXPECT_EQ(Qt::Saturday, settings.firstDayOfWeek());
}

TEST(DCalendarGeneralSettings, TimeShowTypeGetterSetter)
{
    DCalendarGeneralSettings settings;
    settings.setTimeShowType(DCalendarGeneralSettings::Twelve);
    EXPECT_EQ(DCalendarGeneralSettings::Twelve, settings.timeShowType());

    settings.setTimeShowType(DCalendarGeneralSettings::TwentyFour);
    EXPECT_EQ(DCalendarGeneralSettings::TwentyFour, settings.timeShowType());
}

TEST(DCalendarGeneralSettings, FromJsonStringParsesValidJson)
{
    DCalendarGeneralSettings::Ptr ptr(new DCalendarGeneralSettings);
    const QString json = QStringLiteral(
        R"({"firstDayOfWeek": 3, "TimeShowType": 1})");
    ASSERT_TRUE(DCalendarGeneralSettings::fromJsonString(ptr, json));
    EXPECT_EQ(Qt::Wednesday, ptr->firstDayOfWeek());
    EXPECT_EQ(DCalendarGeneralSettings::Twelve, ptr->timeShowType());
}

TEST(DCalendarGeneralSettings, FromJsonStringHandlesInvalidJson)
{
    DCalendarGeneralSettings::Ptr ptr(new DCalendarGeneralSettings);
    const QString invalidJson = QStringLiteral("not a json string");
    EXPECT_FALSE(DCalendarGeneralSettings::fromJsonString(ptr, invalidJson));
}

TEST(DCalendarGeneralSettings, FromJsonStringWithMissingKeysKeepsDefaults)
{
    DCalendarGeneralSettings::Ptr ptr(new DCalendarGeneralSettings);
    const QString json = QStringLiteral(R"({})");
    ASSERT_TRUE(DCalendarGeneralSettings::fromJsonString(ptr, json));
    EXPECT_EQ(Qt::Sunday, ptr->firstDayOfWeek());
    EXPECT_EQ(DCalendarGeneralSettings::TwentyFour, ptr->timeShowType());
}

TEST(DCalendarGeneralSettings, ToJsonStringSerializesCorrectly)
{
    DCalendarGeneralSettings::Ptr ptr(new DCalendarGeneralSettings);
    ptr->setFirstDayOfWeek(Qt::Tuesday);
    ptr->setTimeShowType(DCalendarGeneralSettings::Twelve);

    QString jsonStr;
    DCalendarGeneralSettings::toJsonString(ptr, jsonStr);

    const QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    ASSERT_TRUE(doc.isObject());
    const QJsonObject obj = doc.object();
    EXPECT_EQ(2, obj.value(QStringLiteral("firstDayOfWeek")).toInt());
    EXPECT_EQ(1, obj.value(QStringLiteral("TimeShowType")).toInt());
}

TEST(DCalendarGeneralSettings, JsonRoundTripPreservesValues)
{
    DCalendarGeneralSettings::Ptr original(new DCalendarGeneralSettings);
    original->setFirstDayOfWeek(Qt::Thursday);
    original->setTimeShowType(DCalendarGeneralSettings::TwentyFour);

    QString jsonStr;
    DCalendarGeneralSettings::toJsonString(original, jsonStr);

    DCalendarGeneralSettings::Ptr restored(new DCalendarGeneralSettings);
    ASSERT_TRUE(DCalendarGeneralSettings::fromJsonString(restored, jsonStr));
    EXPECT_EQ(original->firstDayOfWeek(), restored->firstDayOfWeek());
    EXPECT_EQ(original->timeShowType(), restored->timeShowType());
}

} // namespace
