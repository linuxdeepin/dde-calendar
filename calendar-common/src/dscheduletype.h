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
#ifndef DSCHEDULEYPE_H
#define DSCHEDULEYPE_H

#include "dtypecolor.h"

#include <QFlag>
#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QSharedPointer>

class DAccount;
//日程类型
class DScheduleType
{
public:
    enum Privilege {
        None = 0x0, //不可读不可写
        Read = 0x1, //可读
        Write = 0x2, //可编辑
    };

    enum ShowState {
        Show,
        Hide
    };

    Q_DECLARE_FLAGS(Privileges, Privilege)

    typedef QSharedPointer<DScheduleType> Ptr;

    DScheduleType();
    explicit DScheduleType(DAccount *parent);
    DAccount *account() const;

    Privilege privilege() const;
    void setPrivilege(const Privilege &privilege);

    DTypeColor typeColor() const;
    void setTypeColor(const DTypeColor &typeColor);

    QString typeID() const;
    void setTypeID(const QString &typeID);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    ShowState showState() const;
    void setShowState(const ShowState &showState);

    QString typeName() const;
    void setTypeName(const QString &typeName);

    QString typePath() const;
    void setTypePath(const QString &typePath);

    QString description() const;
    void setDescription(const QString &description);

    QDateTime dtCreate() const;
    void setDtCreate(const QDateTime &dtCreate);

    QDateTime dtUpdate() const;
    void setDtUpdate(const QDateTime &dtUpdate);

    QDateTime dtDelete() const;
    void setDtDelete(const QDateTime &dtDelete);

    int deleted() const;
    void setDeleted(int deleted);

    static bool fromJsonString(DScheduleType::Ptr &scheduleType, const QString &jsonStr);
    static bool toJsonString(const DScheduleType::Ptr &scheduleType, QString &jsonStr);

private:
    DAccount *m_account;
    QString m_typeID;
    QString m_typeName; //类型名称
    QString m_displayName; //类型显示名称
    QString m_typePath; //类型云端路径
    DTypeColor m_typeColor; //相关颜色信息
    QString m_description; //类型相关描述
    Privilege m_privilege; //类型权限
    QDateTime m_dtCreate; //创建时间
    QDateTime m_dtUpdate; //更新时间
    QDateTime m_dtDelete; //删除时间
    ShowState m_showState; //类型下日程显示状态
    int m_deleted; //是否被删除
};

Q_DECLARE_METATYPE(DScheduleType)

#endif // DSCHEDULEYPE_H
