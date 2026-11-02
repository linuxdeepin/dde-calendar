// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "gtest/gtest.h"
#include "duploadtaskdata.h"

#include <QDateTime>
#include <QString>

// Test fixture for DUploadTaskData
class TestDUploadTaskData : public ::testing::Test
{
protected:
    void SetUp() override
    {
        data = new DUploadTaskData();
    }
    void TearDown() override
    {
        delete data;
        data = nullptr;
    }
    DUploadTaskData *data = nullptr;
};

// Constructor sets default values
TEST_F(TestDUploadTaskData, Constructor_DefaultValues)
{
    EXPECT_EQ(data->taskType(), DUploadTaskData::Create);
    EXPECT_EQ(data->taskObject(), DUploadTaskData::Task_ScheduleType);
    EXPECT_EQ(data->objectId(), QString(""));
    EXPECT_EQ(data->taskID(), QString(""));
}

// taskType / setTaskType
TEST_F(TestDUploadTaskData, TaskType_SetAndGet)
{
    data->setTaskType(DUploadTaskData::Modify);
    EXPECT_EQ(data->taskType(), DUploadTaskData::Modify);

    data->setTaskType(DUploadTaskData::Delete);
    EXPECT_EQ(data->taskType(), DUploadTaskData::Delete);

    data->setTaskType(DUploadTaskData::Create);
    EXPECT_EQ(data->taskType(), DUploadTaskData::Create);
}

// taskObject / setTaskObject
TEST_F(TestDUploadTaskData, TaskObject_SetAndGet)
{
    data->setTaskObject(DUploadTaskData::Task_Schedule);
    EXPECT_EQ(data->taskObject(), DUploadTaskData::Task_Schedule);

    data->setTaskObject(DUploadTaskData::Task_Color);
    EXPECT_EQ(data->taskObject(), DUploadTaskData::Task_Color);

    data->setTaskObject(DUploadTaskData::Task_ScheduleType);
    EXPECT_EQ(data->taskObject(), DUploadTaskData::Task_ScheduleType);
}

// objectId / setObjectId
TEST_F(TestDUploadTaskData, ObjectId_SetAndGet)
{
    data->setObjectId("obj123");
    EXPECT_EQ(data->objectId(), QString("obj123"));

    data->setObjectId("");
    EXPECT_EQ(data->objectId(), QString(""));

    data->setObjectId("special chars: !@#$%^&*()");
    EXPECT_EQ(data->objectId(), QString("special chars: !@#$%^&*()"));
}

// taskID / setTaskID
TEST_F(TestDUploadTaskData, TaskID_SetAndGet)
{
    data->setTaskID("task456");
    EXPECT_EQ(data->taskID(), QString("task456"));

    data->setTaskID("");
    EXPECT_EQ(data->taskID(), QString(""));

    data->setTaskID("中文ID");
    EXPECT_EQ(data->taskID(), QString("中文ID"));
}

// dtCreate / setDtCreate
TEST_F(TestDUploadTaskData, DtCreate_SetAndGet)
{
    QDateTime dt(QDate(2026, 1, 15), QTime(10, 30, 0));
    data->setDtCreate(dt);
    EXPECT_EQ(data->dtCreate(), dt);

    QDateTime dt2 = QDateTime::currentDateTime();
    data->setDtCreate(dt2);
    EXPECT_EQ(data->dtCreate(), dt2);

    data->setDtCreate(QDateTime());
    EXPECT_EQ(data->dtCreate(), QDateTime());
}

// sql_table_name
TEST_F(TestDUploadTaskData, SqlTableName_TaskScheduleType)
{
    EXPECT_EQ(DUploadTaskData::sql_table_name(DUploadTaskData::Task_ScheduleType), QString("scheduleType"));
}

TEST_F(TestDUploadTaskData, SqlTableName_TaskSchedule)
{
    EXPECT_EQ(DUploadTaskData::sql_table_name(DUploadTaskData::Task_Schedule), QString("schedules"));
}

TEST_F(TestDUploadTaskData, SqlTableName_TaskColor)
{
    EXPECT_EQ(DUploadTaskData::sql_table_name(DUploadTaskData::Task_Color), QString("typeColor"));
}

TEST_F(TestDUploadTaskData, SqlTableName_Unknown)
{
    EXPECT_EQ(DUploadTaskData::sql_table_name(999), QString(""));
}

// sql_table_primary_key
TEST_F(TestDUploadTaskData, SqlTablePrimaryKey_TaskScheduleType)
{
    EXPECT_EQ(DUploadTaskData::sql_table_primary_key(DUploadTaskData::Task_ScheduleType), QString("typeID"));
}

TEST_F(TestDUploadTaskData, SqlTablePrimaryKey_TaskSchedule)
{
    EXPECT_EQ(DUploadTaskData::sql_table_primary_key(DUploadTaskData::Task_Schedule), QString("scheduleID"));
}

TEST_F(TestDUploadTaskData, SqlTablePrimaryKey_TaskColor)
{
    EXPECT_EQ(DUploadTaskData::sql_table_primary_key(DUploadTaskData::Task_Color), QString("colorID"));
}

TEST_F(TestDUploadTaskData, SqlTablePrimaryKey_Unknown)
{
    EXPECT_EQ(DUploadTaskData::sql_table_primary_key(999), QString(""));
}
