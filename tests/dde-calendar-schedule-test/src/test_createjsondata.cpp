// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "createjsondata.h"
#include "jsondata.h"

#include <QJsonObject>
#include <QJsonArray>

// Test fixture for CreateJsonData
class TestCreateJsonData : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_data = new CreateJsonData();
    }

    void TearDown() override
    {
        delete m_data;
        m_data = nullptr;
    }

    CreateJsonData *m_data = nullptr;
};

// Constructor creates a valid object inheriting JsonData
TEST_F(TestCreateJsonData, ConstructorCreatesValidObject)
{
    ASSERT_NE(m_data, nullptr);
}

// Destructor does not crash — covered by SetUp/TearDown lifecycle
TEST_F(TestCreateJsonData, DestructorDoesNotCrash)
{
    CreateJsonData *data = new CreateJsonData();
    delete data;
    SUCCEED();
}

// CreateJsonData inherits JsonData::isVaild — default object is valid
TEST_F(TestCreateJsonData, IsVaildOnDefaultObject)
{
    EXPECT_TRUE(m_data->isVaild());
}

// CreateJsonData inherits JsonData::JosnResolve — verify content slot parsing
TEST_F(TestCreateJsonData, JosnResolveContentSlot)
{
    QJsonObject root;
    QJsonArray slotArray;
    QJsonObject contentSlot;
    contentSlot["name"] = QStringLiteral("content");
    contentSlot["value"] = QStringLiteral("Create Title");
    slotArray.append(contentSlot);
    root["slots"] = slotArray;

    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->TitleName().toStdString(), "Create Title");
    EXPECT_FALSE(m_data->isVaild());
}

// CreateJsonData inherits setDefaultValue — resets state via JosnResolve
TEST_F(TestCreateJsonData, SetDefaultValueResetsState)
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
