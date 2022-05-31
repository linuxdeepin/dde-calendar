/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "duploadtaskdata.h"

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
    m_taskType = taskType;
}

DUploadTaskData::TaskObject DUploadTaskData::taskObject() const
{
    return m_taskObject;
}

void DUploadTaskData::setTaskObject(const TaskObject &taskObject)
{
    m_taskObject = taskObject;
}

QString DUploadTaskData::objectId() const
{
    return m_objectId;
}

void DUploadTaskData::setObjectId(const QString &objectId)
{
    m_objectId = objectId;
}

QString DUploadTaskData::taskID() const
{
    return m_taskID;
}

void DUploadTaskData::setTaskID(const QString &taskID)
{
    m_taskID = taskID;
}

QDateTime DUploadTaskData::dtCreate() const
{
    return m_dtCreate;
}

void DUploadTaskData::setDtCreate(const QDateTime &dtCreate)
{
    m_dtCreate = dtCreate;
}
