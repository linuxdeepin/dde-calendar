// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "changejsondata.h"
#include "jsondata.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// Test fixture for changejsondata
class TestChangeJsonData : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_data = new changejsondata();
    }

    void TearDown() override
    {
        delete m_data;
        m_data = nullptr;
    }

    // Helper: build a slotArray JSON object for JosnResolve with a single slot
    QJsonObject buildSlotsJson(const QJsonObject &slot)
    {
        QJsonObject root;
        QJsonArray slotArray;
        slotArray.append(slot);
        root["slots"] = slotArray;
        return root;
    }

    // Helper: build a fromTime/toTime slot with normValue containing suggestDatetime
    QJsonObject buildDatetimeSlot(const QString &slotName, const QString &suggestDatetimeStr, const QString &datetimeStr)
    {
        QJsonObject normValue;
        normValue["suggestDatetime"] = suggestDatetimeStr;
        normValue["datetime"] = datetimeStr;
        QJsonDocument doc(normValue);
        QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        QJsonObject slot;
        slot["name"] = slotName;
        slot["normValue"] = normValueStr;
        return slot;
    }

    // Helper: build a toPlace slot
    QJsonObject buildToPlaceSlot(const QString &placeValue)
    {
        QJsonObject slot;
        slot["name"] = QStringLiteral("toPlace");
        slot["value"] = placeValue;
        return slot;
    }

    changejsondata *m_data = nullptr;
};

// Constructor creates a valid object with default state
TEST_F(TestChangeJsonData, ConstructorCreatesValidObject)
{
    ASSERT_NE(m_data, nullptr);
    EXPECT_TRUE(m_data->isVaild());
    EXPECT_TRUE(m_data->toPlaceStr().isEmpty());
    EXPECT_EQ(m_data->toDateTime().suggestDatetime.size(), 0);
    EXPECT_EQ(m_data->fromDateTime().suggestDatetime.size(), 0);
}

// toPlaceStr getter/setter round-trip
TEST_F(TestChangeJsonData, ToPlaceStrGetterSetter)
{
    QString testPlace = QStringLiteral("Conference Room A");
    m_data->setToPlaceStr(testPlace);
    EXPECT_EQ(m_data->toPlaceStr(), testPlace);
}

// toPlaceStr setter with empty string
TEST_F(TestChangeJsonData, ToPlaceStrSetEmpty)
{
    m_data->setToPlaceStr(QStringLiteral("Some Place"));
    m_data->setToPlaceStr(QStringLiteral(""));
    EXPECT_TRUE(m_data->toPlaceStr().isEmpty());
}

// toDateTime getter/setter round-trip
TEST_F(TestChangeJsonData, ToDateTimeGetterSetter)
{
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime(QDate(2025, 6, 15), QTime(10, 30));
    info.hasTime = true;
    info.strDateTime = QStringLiteral("2025-06-15T10:30:00");
    dt.suggestDatetime.append(info);
    m_data->setToDateTime(dt);
    EXPECT_EQ(m_data->toDateTime().suggestDatetime.size(), 1);
    EXPECT_EQ(m_data->toDateTime().suggestDatetime[0].datetime, info.datetime);
}

// fromDateTime getter/setter round-trip
TEST_F(TestChangeJsonData, FromDateTimeGetterSetter)
{
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime(QDate(2025, 7, 20), QTime(14, 0));
    info.hasTime = true;
    info.strDateTime = QStringLiteral("2025-07-20T14:00:00");
    dt.suggestDatetime.append(info);
    m_data->setFromDateTime(dt);
    EXPECT_EQ(m_data->fromDateTime().suggestDatetime.size(), 1);
    EXPECT_EQ(m_data->fromDateTime().suggestDatetime[0].datetime, info.datetime);
}

// isVaild returns false when toPlaceStr is set
TEST_F(TestChangeJsonData, IsVaildFalseWhenToPlaceStrSet)
{
    m_data->setToPlaceStr(QStringLiteral("Some Place"));
    EXPECT_FALSE(m_data->isVaild());
}

// isVaild returns false when toDateTime has suggestDatetime
TEST_F(TestChangeJsonData, IsVaildFalseWhenToDateTimeSet)
{
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    dt.suggestDatetime.append(info);
    m_data->setToDateTime(dt);
    EXPECT_FALSE(m_data->isVaild());
}

// isVaild returns false when fromDateTime has suggestDatetime
TEST_F(TestChangeJsonData, IsVaildFalseWhenFromDateTimeSet)
{
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    dt.suggestDatetime.append(info);
    m_data->setFromDateTime(dt);
    EXPECT_FALSE(m_data->isVaild());
}

// isVaild returns true on default object
TEST_F(TestChangeJsonData, IsVaildTrueOnDefault)
{
    EXPECT_TRUE(m_data->isVaild());
}

// jsonObjResolve dispatches to fromDatetimeJsonResolve via fromTime slot
TEST_F(TestChangeJsonData, JsonObjResolveFromDatetime)
{
    QJsonObject slot = buildDatetimeSlot(QStringLiteral("fromTime"),
                                         QStringLiteral("2025-06-15T10:30:00"),
                                         QStringLiteral("2025-06-15T10:30:00"));
    QJsonObject root = buildSlotsJson(slot);
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->fromDateTime().suggestDatetime.size(), 1);
    EXPECT_FALSE(m_data->isVaild());
}

// jsonObjResolve dispatches to toDatetimeJsonResolve via toTime slot
TEST_F(TestChangeJsonData, JsonObjResolveToDatetime)
{
    QJsonObject slot = buildDatetimeSlot(QStringLiteral("toTime"),
                                         QStringLiteral("2025-06-15T10:30:00"),
                                         QStringLiteral("2025-06-15T10:30:00"));
    QJsonObject root = buildSlotsJson(slot);
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->toDateTime().suggestDatetime.size(), 1);
    EXPECT_FALSE(m_data->isVaild());
}

// jsonObjResolve dispatches to toPlaceJsonResolve via toPlace slot
TEST_F(TestChangeJsonData, JsonObjResolveToPlace)
{
    QJsonObject slot = buildToPlaceSlot(QStringLiteral("Meeting Room B"));
    QJsonObject root = buildSlotsJson(slot);
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->toPlaceStr().toStdString(), "Meeting Room B");
    EXPECT_FALSE(m_data->isVaild());
}

// jsonObjResolve is a no-op when slot name does not match any known field
TEST_F(TestChangeJsonData, JsonObjResolveUnknownNameNoOp)
{
    QJsonObject slot;
    slot["name"] = QStringLiteral("unknownSlot");
    slot["value"] = QStringLiteral("whatever");
    QJsonObject root = buildSlotsJson(slot);
    m_data->JosnResolve(root);

    // Object should still be valid (no state changed)
    EXPECT_TRUE(m_data->isVaild());
    EXPECT_TRUE(m_data->toPlaceStr().isEmpty());
}

// setDefaultValue clears all subclass-specific members
TEST_F(TestChangeJsonData, SetDefaultValueClearsState)
{
    // Set some state first
    m_data->setToPlaceStr(QStringLiteral("Some Place"));
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    dt.suggestDatetime.append(info);
    m_data->setFromDateTime(dt);
    m_data->setToDateTime(dt);

    EXPECT_FALSE(m_data->isVaild());

    // Trigger setDefaultValue via JosnResolve with empty slotArray
    QJsonObject root;
    QJsonArray emptySlots;
    root["slots"] = emptySlots;
    m_data->JosnResolve(root);

    // All subclass state should be cleared
    EXPECT_TRUE(m_data->toPlaceStr().isEmpty());
    EXPECT_EQ(m_data->fromDateTime().suggestDatetime.size(), 0);
    EXPECT_EQ(m_data->toDateTime().suggestDatetime.size(), 0);
    EXPECT_TRUE(m_data->isVaild());
}

// setDefaultValue also clears base class state
TEST_F(TestChangeJsonData, SetDefaultValueClearsBaseState)
{
    m_data->setTitleName(QStringLiteral("Test Title"));
    m_data->setToPlaceStr(QStringLiteral("Some Place"));
    EXPECT_FALSE(m_data->isVaild());

    QJsonObject root;
    QJsonArray emptySlots;
    root["slots"] = emptySlots;
    m_data->JosnResolve(root);

    EXPECT_TRUE(m_data->TitleName().isEmpty());
    EXPECT_TRUE(m_data->isVaild());
}

// fromDatetimeJsonResolve sets fromDateTime via suggestDatetimeResolve
TEST_F(TestChangeJsonData, FromDatetimeJsonResolveDirect)
{
    QJsonObject slot = buildDatetimeSlot(QStringLiteral("fromTime"),
                                         QStringLiteral("2025-08-01T09:00:00"),
                                         QStringLiteral("2025-08-01T09:00:00"));
    QJsonObject root = buildSlotsJson(slot);
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->fromDateTime().suggestDatetime.size(), 1);
}

// toDatetimeJsonResolve sets toDateTime via suggestDatetimeResolve
TEST_F(TestChangeJsonData, ToDatetimeJsonResolveDirect)
{
    QJsonObject slot = buildDatetimeSlot(QStringLiteral("toTime"),
                                         QStringLiteral("2025-08-01T09:00:00"),
                                         QStringLiteral("2025-08-01T09:00:00"));
    QJsonObject root = buildSlotsJson(slot);
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->toDateTime().suggestDatetime.size(), 1);
}

// toPlaceJsonResolve extracts JSON value and sets toPlaceStr
TEST_F(TestChangeJsonData, ToPlaceJsonResolveDirect)
{
    QJsonObject slot = buildToPlaceSlot(QStringLiteral("Office 402"));
    QJsonObject root = buildSlotsJson(slot);
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->toPlaceStr().toStdString(), "Office 402");
}
