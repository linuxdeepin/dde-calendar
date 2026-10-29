// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "clocaldata.h"
#include "dschedule.h"

// Test fixture for CLocalData
class TestCLocalData : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_data = new CLocalData();
    }

    void TearDown() override
    {
        delete m_data;
        m_data = nullptr;
    }

    CLocalData *m_data = nullptr;
};

// Constructor creates a valid object with default values
TEST_F(TestCLocalData, ConstructorCreatesValidObject)
{
    ASSERT_NE(m_data, nullptr);
    EXPECT_EQ(m_data->getOffet(), -1);
    EXPECT_TRUE(m_data->getToTitleName().isEmpty());
    EXPECT_TRUE(m_data->scheduleInfoVector().isEmpty());
    EXPECT_EQ(m_data->getToTime().suggestDatetime.size(), 0);
}

// Constructor default for offset is -1
TEST_F(TestCLocalData, ConstructorDefaultOffsetIsMinusOne)
{
    EXPECT_EQ(m_data->getOffet(), -1);
}

// Constructor default for ToTitleName is empty string
TEST_F(TestCLocalData, ConstructorDefaultToTitleNameIsEmpty)
{
    EXPECT_EQ(m_data->getToTitleName().toStdString(), "");
}

// Constructor default for SelectInfo is null
TEST_F(TestCLocalData, ConstructorDefaultSelectInfoIsNull)
{
    EXPECT_TRUE(m_data->SelectInfo().isNull());
}

// Constructor default for NewInfo is null
TEST_F(TestCLocalData, ConstructorDefaultNewInfoIsNull)
{
    EXPECT_TRUE(m_data->getNewInfo().isNull());
}

// Constructor default for scheduleInfoVector is empty
TEST_F(TestCLocalData, ConstructorDefaultScheduleInfoVectorIsEmpty)
{
    EXPECT_TRUE(m_data->scheduleInfoVector().isEmpty());
}

// scheduleInfoVector getter/setter round-trip
TEST_F(TestCLocalData, ScheduleInfoVectorGetterSetter)
{
    DSchedule::List list;
    auto schedule = DSchedule::Ptr(new DSchedule());
    schedule->setUid(QStringLiteral("sch001"));
    list.append(schedule);
    m_data->setScheduleInfoVector(list);
    EXPECT_EQ(m_data->scheduleInfoVector().size(), 1);
    EXPECT_EQ(m_data->scheduleInfoVector()[0]->uid(), QStringLiteral("sch001"));
}

// scheduleInfoVector setter with empty list
TEST_F(TestCLocalData, ScheduleInfoVectorSetEmpty)
{
    DSchedule::List list;
    auto schedule = DSchedule::Ptr(new DSchedule());
    list.append(schedule);
    m_data->setScheduleInfoVector(list);
    EXPECT_EQ(m_data->scheduleInfoVector().size(), 1);

    DSchedule::List emptyList;
    m_data->setScheduleInfoVector(emptyList);
    EXPECT_TRUE(m_data->scheduleInfoVector().isEmpty());
}

// SelectInfo getter/setter round-trip
TEST_F(TestCLocalData, SelectInfoGetterSetter)
{
    auto schedule = DSchedule::Ptr(new DSchedule());
    schedule->setUid(QStringLiteral("select001"));
    m_data->setSelectInfo(schedule);
    ASSERT_FALSE(m_data->SelectInfo().isNull());
    EXPECT_EQ(m_data->SelectInfo()->uid(), QStringLiteral("select001"));
}

// SelectInfo setter with null
TEST_F(TestCLocalData, SelectInfoSetNull)
{
    auto schedule = DSchedule::Ptr(new DSchedule());
    m_data->setSelectInfo(schedule);
    EXPECT_FALSE(m_data->SelectInfo().isNull());

    m_data->setSelectInfo(nullptr);
    EXPECT_TRUE(m_data->SelectInfo().isNull());
}

// ToTitleName getter/setter round-trip
TEST_F(TestCLocalData, ToTitleNameGetterSetter)
{
    QString title = QStringLiteral("Important Meeting");
    m_data->setToTitleName(title);
    EXPECT_EQ(m_data->getToTitleName(), title);
}

// ToTitleName setter with empty string
TEST_F(TestCLocalData, ToTitleNameSetEmpty)
{
    m_data->setToTitleName(QStringLiteral("Some Title"));
    m_data->setToTitleName(QStringLiteral(""));
    EXPECT_TRUE(m_data->getToTitleName().isEmpty());
}

// ToTime getter/setter round-trip
TEST_F(TestCLocalData, ToTimeGetterSetter)
{
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime(QDate(2025, 9, 10), QTime(15, 30));
    info.hasTime = true;
    info.strDateTime = QStringLiteral("2025-09-10T15:30:00");
    dt.suggestDatetime.append(info);
    m_data->setToTime(dt);
    EXPECT_EQ(m_data->getToTime().suggestDatetime.size(), 1);
    EXPECT_EQ(m_data->getToTime().suggestDatetime[0].datetime, info.datetime);
}

// ToTime setter with empty SemanticsDateTime
TEST_F(TestCLocalData, ToTimeSetEmpty)
{
    SemanticsDateTime dt;
    SuggestDatetimeInfo info;
    info.datetime = QDateTime::currentDateTime();
    dt.suggestDatetime.append(info);
    m_data->setToTime(dt);
    EXPECT_EQ(m_data->getToTime().suggestDatetime.size(), 1);

    SemanticsDateTime emptyDt;
    m_data->setToTime(emptyDt);
    EXPECT_EQ(m_data->getToTime().suggestDatetime.size(), 0);
}

// Offset getter/setter round-trip
TEST_F(TestCLocalData, OffsetGetterSetter)
{
    m_data->setOffset(5);
    EXPECT_EQ(m_data->getOffet(), 5);
}

// Offset setter with negative value
TEST_F(TestCLocalData, OffsetSetNegative)
{
    m_data->setOffset(-10);
    EXPECT_EQ(m_data->getOffet(), -10);
}

// Offset setter with zero
TEST_F(TestCLocalData, OffsetSetZero)
{
    m_data->setOffset(0);
    EXPECT_EQ(m_data->getOffet(), 0);
}

// Offset setter resets to -1
TEST_F(TestCLocalData, OffsetSetBackToDefault)
{
    m_data->setOffset(42);
    EXPECT_EQ(m_data->getOffet(), 42);
    m_data->setOffset(-1);
    EXPECT_EQ(m_data->getOffet(), -1);
}

// NewInfo getter/setter round-trip
TEST_F(TestCLocalData, NewInfoGetterSetter)
{
    auto schedule = DSchedule::Ptr(new DSchedule());
    schedule->setUid(QStringLiteral("new001"));
    m_data->setNewInfo(schedule);
    ASSERT_FALSE(m_data->getNewInfo().isNull());
    EXPECT_EQ(m_data->getNewInfo()->uid(), QStringLiteral("new001"));
}

// NewInfo setter with null
TEST_F(TestCLocalData, NewInfoSetNull)
{
    auto schedule = DSchedule::Ptr(new DSchedule());
    m_data->setNewInfo(schedule);
    EXPECT_FALSE(m_data->getNewInfo().isNull());

    m_data->setNewInfo(nullptr);
    EXPECT_TRUE(m_data->getNewInfo().isNull());
}
