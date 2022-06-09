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
#ifndef DUPLOADTASKDATA_H
#define DUPLOADTASKDATA_H

#include <QString>
#include <QSharedPointer>
#include <QVector>
#include <QDateTime>

//上传任务相关数据
class DUploadTaskData
{
public:
    //任务类型
    enum TaskType {
        Create, //创建
        Modify, //修改
        Delete, //删除
    };
    //任务对象
    enum TaskObject {
        Task_ScheduleType, //日程类型
        Task_Schedule, //日程
        Task_Color, //类型颜色
    };

    static QString sql_table_name(int task_obj);
    static QString sql_table_primary_key(int task_obj);

    typedef QSharedPointer<DUploadTaskData> Ptr;
    typedef QVector<Ptr> List;
    DUploadTaskData();
    TaskType taskType() const;
    void setTaskType(const TaskType &taskType);

    TaskObject taskObject() const;
    void setTaskObject(const TaskObject &taskObject);

    QString objectId() const;
    void setObjectId(const QString &objectId);

    QString taskID() const;
    void setTaskID(const QString &taskID);

    QDateTime dtCreate() const;
    void setDtCreate(const QDateTime &dtCreate);

private:
    TaskType m_taskType;
    TaskObject m_taskObject;
    QString m_objectId;
    QString m_taskID;
    QDateTime m_dtCreate;
};

#endif // DUPLOADTASKDATA_H
