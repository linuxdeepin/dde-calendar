// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "queryjsondata.h"
#include "jsondata.h"

#include <QJsonObject>
#include <QJsonArray>

// Test fixture for QueryJsonData
class TestQueryJsonData : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_data = new QueryJsonData();
    }

    void TearDown() override
    {
        delete m_data;
        m_data = nullptr;
    }

    QueryJsonData *m_data = nullptr;
};

// Constructor creates a valid object inheriting JsonData
TEST_F(TestQueryJsonData, ConstructorCreatesValidObject)
{
    ASSERT_NE(m_data, nullptr);
}

// Destructor does not crash — covered by SetUp/TearDown lifecycle
TEST_F(TestQueryJsonData, DestructorDoesNotCrash)
{
    QueryJsonData *data = new QueryJsonData();
    delete data;
    SUCCEED();
}

// QueryJsonData inherits JsonData::isVaild — default object is valid
TEST_F(TestQueryJsonData, IsVaildOnDefaultObject)
{
    EXPECT_TRUE(m_data->isVaild());
}

// QueryJsonData inherits JsonData::JosnResolve — verify content slot parsing
TEST_F(TestQueryJsonData, JosnResolveContentSlot)
{
    QJsonObject root;
    QJsonArray slotArray;
    QJsonObject contentSlot;
    contentSlot["name"] = QStringLiteral("content");
    contentSlot["value"] = QStringLiteral("Query Title");
    slotArray.append(contentSlot);
    root["slots"] = slotArray;

    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->TitleName().toStdString(), "Query Title");
    EXPECT_FALSE(m_data->isVaild());
}

// QueryJsonData inherits setDefaultValue — resets state via JosnResolve
TEST_F(TestQueryJsonData, SetDefaultValueResetsState)
{
    m_data->setTitleName(QStringLiteral("Some Title"));
    EXPECT_FALSE(m_data->isVaild());

    QJsonObject root;
    QJsonArray emptySlots;
    root["slots"] = emptySlots;
    m_data->JosnResolve(root);

    EXPECT_TRUE(m_data->TitleName().isEmpty());
    EXPECT_TRUE(m_data->isVaild());
}
