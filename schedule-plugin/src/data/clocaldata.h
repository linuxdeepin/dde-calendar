/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#ifndef CLOCALDATA_H
#define CLOCALDATA_H

#include "dschedule.h"
#include "timedatastruct.h"

#include <QSharedPointer>

class CLocalData
{
public:
    typedef QSharedPointer<CLocalData> Ptr;
    CLocalData();
    DSchedule::List scheduleInfoVector() const;
    void setScheduleInfoVector(const DSchedule::List &scheduleInfoVector);
    DSchedule::Ptr SelectInfo() const;
    void setSelectInfo(const DSchedule::Ptr &SelectInfo);
    void setToTitleName(const QString &title);
    QString getToTitleName() const;
    /**
     * @brief setToTime     设置修改的日程时间
     * @param timeInfo      日程时间
     */
    void setToTime(const SemanticsDateTime &timeInfo);
    /**
     * @brief getToTime     获取修改的日程时间
     * @return
     */
    SemanticsDateTime getToTime() const;
    int getOffet() const;
    void setOffset(int offset);
    void setNewInfo(const DSchedule::Ptr &newInfo);
    DSchedule::Ptr getNewInfo() const;

private:
    DSchedule::List m_scheduleInfoVector {};
    DSchedule::Ptr m_SelectInfo {};
    DSchedule::Ptr m_NewInfo {};
    QString m_ToTitleName {""};
    SemanticsDateTime m_ToTime {};
    int m_offset {-1};
};

#endif // CLOCALDATA_H
