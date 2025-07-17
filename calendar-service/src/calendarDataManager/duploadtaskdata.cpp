// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "duploadtaskdata.h"
#include "commondef.h"

QString DUploadTaskData::sql_table_name(int task_obj)
{
    qCDebug(ServiceLogger) << "Getting SQL table name for task object:" << task_obj;
    switch(task_obj) {
    case DUploadTaskData::Task_ScheduleType:
        qCDebug(ServiceLogger) << "Returning table name for ScheduleType";
        return "scheduleType";
    case DUploadTaskData::Task_Schedule:
        qCDebug(ServiceLogger) << "Returning table name for Schedule";
        return "schedules";
    case DUploadTaskData::Task_Color:
        qCDebug(ServiceLogger) << "Returning table name for Color";
        return "typeColor";
    }
    qCDebug(ServiceLogger) << "Unknown task object, returning empty string";
    return "";
}

QString DUploadTaskData::sql_table_primary_key(int task_obj)
{
    qCDebug(ServiceLogger) << "Getting SQL table primary key for task object:" << task_obj;
    switch(task_obj) {
    case DUploadTaskData::Task_ScheduleType:
        qCDebug(ServiceLogger) << "Returning primary key for ScheduleType";
        return "typeID";
    case DUploadTaskData::Task_Schedule:
        qCDebug(ServiceLogger) << "Returning primary key for Schedule";
        return "scheduleID";
    case DUploadTaskData::Task_Color:
        qCDebug(ServiceLogger) << "Returning primary key for Color";
        return "colorID";
    }
    qCDebug(ServiceLogger) << "Unknown task object, returning empty primary key";
    return "";
}

DUploadTaskData::DUploadTaskData()
    : m_taskType(Create)
    , m_taskObject(Task_ScheduleType)
    , m_objectId("")
    , m_taskID("")
{
    qCDebug(ServiceLogger) << "Creating DUploadTaskData instance with default values";
}

DUploadTaskData::TaskType DUploadTaskData::taskType() const
{
    // qCDebug(ServiceLogger) << "Getting task type:" << m_taskType;
    return m_taskType;
}

void DUploadTaskData::setTaskType(const TaskType &taskType)
{
    // qCDebug(ServiceLogger) << "Setting task type to:" << taskType;
    m_taskType = taskType;
}

DUploadTaskData::TaskObject DUploadTaskData::taskObject() const
{
    // qCDebug(ServiceLogger) << "Getting task object:" << m_taskObject;
    return m_taskObject;
}

void DUploadTaskData::setTaskObject(const TaskObject &taskObject)
{
    // qCDebug(ServiceLogger) << "Setting task object to:" << taskObject;
    m_taskObject = taskObject;
}

QString DUploadTaskData::objectId() const
{
    // qCDebug(ServiceLogger) << "Getting object ID:" << m_objectId;
    return m_objectId;
}

void DUploadTaskData::setObjectId(const QString &objectId)
{
    // qCDebug(ServiceLogger) << "Setting object ID to:" << objectId;
    m_objectId = objectId;
}

QString DUploadTaskData::taskID() const
{
    // qCDebug(ServiceLogger) << "Getting task ID:" << m_taskID;
    return m_taskID;
}

void DUploadTaskData::setTaskID(const QString &taskID)
{
    // qCDebug(ServiceLogger) << "Setting task ID to:" << taskID;
    m_taskID = taskID;
}

QDateTime DUploadTaskData::dtCreate() const
{
    // qCDebug(ServiceLogger) << "Getting creation datetime:" << m_dtCreate;
    return m_dtCreate;
}

void DUploadTaskData::setDtCreate(const QDateTime &dtCreate)
{
    // qCDebug(ServiceLogger) << "Setting creation datetime to:" << dtCreate;
    m_dtCreate = dtCreate;
}
