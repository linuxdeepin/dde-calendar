// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "duploadtaskdata.h"

// Add logging category
Q_LOGGING_CATEGORY(uploadTaskLog, "calendar.upload.task")

QString DUploadTaskData::sql_table_name(int task_obj)
{
    QString tableName;
    switch(task_obj) {
    case DUploadTaskData::Task_ScheduleType:
        tableName = "scheduleType";
        break;
    case DUploadTaskData::Task_Schedule:
        tableName = "schedules";
        break;
    case DUploadTaskData::Task_Color:
        tableName = "typeColor";
        break;
    default:
        qCWarning(uploadTaskLog) << "Unknown task object type:" << task_obj;
        break;
    }
    qCDebug(uploadTaskLog) << "Getting table name for task object:" << task_obj << "=" << tableName;
    return tableName;
}

QString DUploadTaskData::sql_table_primary_key(int task_obj)
{
    QString primaryKey;
    switch(task_obj) {
    case DUploadTaskData::Task_ScheduleType:
        primaryKey = "typeID";
        break;
    case DUploadTaskData::Task_Schedule:
        primaryKey = "scheduleID";
        break;
    case DUploadTaskData::Task_Color:
        primaryKey = "colorID";
        break;
    default:
        qCWarning(uploadTaskLog) << "Unknown task object type for primary key:" << task_obj;
        break;
    }
    qCDebug(uploadTaskLog) << "Getting primary key for task object:" << task_obj << "=" << primaryKey;
    return primaryKey;
}

DUploadTaskData::DUploadTaskData()
    : m_taskType(Create)
    , m_taskObject(Task_ScheduleType)
    , m_objectId("")
    , m_taskID("")
{
}

DUploadTaskData::TaskType DUploadTaskData::taskType() const
{
    return m_taskType;
}

void DUploadTaskData::setTaskType(const TaskType &taskType)
{
    qCDebug(uploadTaskLog) << "Setting task type to:" << taskType;
    m_taskType = taskType;
}

DUploadTaskData::TaskObject DUploadTaskData::taskObject() const
{
    return m_taskObject;
}

void DUploadTaskData::setTaskObject(const TaskObject &taskObject)
{
    qCDebug(uploadTaskLog) << "Setting task object to:" << taskObject;
    m_taskObject = taskObject;
}

QString DUploadTaskData::objectId() const
{
    return m_objectId;
}

void DUploadTaskData::setObjectId(const QString &objectId)
{
    qCDebug(uploadTaskLog) << "Setting object ID to:" << objectId;
    m_objectId = objectId;
}

QString DUploadTaskData::taskID() const
{
    return m_taskID;
}

void DUploadTaskData::setTaskID(const QString &taskID)
{
    qCDebug(uploadTaskLog) << "Setting task ID to:" << taskID;
    m_taskID = taskID;
}

QDateTime DUploadTaskData::dtCreate() const
{
    return m_dtCreate;
}

void DUploadTaskData::setDtCreate(const QDateTime &dtCreate)
{
    qCDebug(uploadTaskLog) << "Setting creation datetime to:" << dtCreate;
    m_dtCreate = dtCreate;
}
