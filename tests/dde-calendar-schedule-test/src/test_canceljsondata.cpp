// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "canceljsondata.h"
#include "jsondata.h"

#include <QJsonObject>
#include <QJsonArray>

// Test fixture for cancelJsonData
class TestCancelJsonData : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_data = new cancelJsonData();
    }

    void TearDown() override
    {
        delete m_data;
        m_data = nullptr;
    }

    cancelJsonData *m_data = nullptr;
};

// Constructor initializes object and inherits JsonData behavior
TEST_F(TestCancelJsonData, ConstructorCreatesValidObject)
{
    ASSERT_NE(m_data, nullptr);
    // A freshly constructed cancelJsonData should be valid (default state)
    EXPECT_TRUE(m_data->isVaild());
}

// Destructor does not crash — covered by SetUp/TearDown lifecycle
TEST_F(TestCancelJsonData, DestructorDoesNotCrash)
{
    cancelJsonData *data = new cancelJsonData();
    delete data;
    SUCCEED();
}

// cancelJsonData inherits JsonData::isVaild() — default object is valid
TEST_F(TestCancelJsonData, IsVaildOnDefaultObject)
{
    EXPECT_TRUE(m_data->isVaild());
}

// cancelJsonData inherits JsonData::JosnResolve — verify content slot parsing
TEST_F(TestCancelJsonData, JosnResolveContentSlot)
{
    QJsonObject root;
    QJsonArray slotArray;
    QJsonObject contentSlot;
    contentSlot["name"] = QStringLiteral("content");
    contentSlot["value"] = QStringLiteral("Test Title");
    slotArray.append(contentSlot);
    root["slots"] = slotArray;

    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->TitleName().toStdString(), "Test Title");
    // After setting a title, isVaild should return false
    EXPECT_FALSE(m_data->isVaild());
}

// cancelJsonData inherits setDefaultValue — resets all state
TEST_F(TestCancelJsonData, SetDefaultValueResetsState)
{
    m_data->setTitleName(QStringLiteral("Some Title"));
    EXPECT_FALSE(m_data->isVaild());

    // Trigger setDefaultValue via JosnResolve with empty slots
    QJsonObject root;
    QJsonArray emptySlots;
    root["slots"] = emptySlots;
    m_data->JosnResolve(root);

    EXPECT_TRUE(m_data->TitleName().isEmpty());
    EXPECT_TRUE(m_data->isVaild());
}
