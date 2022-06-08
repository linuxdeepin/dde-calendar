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
#include "clocaldata.h"

CLocalData::CLocalData()
{
}

DSchedule::List CLocalData::scheduleInfoVector() const
{
    return m_scheduleInfoVector;
}

void CLocalData::setScheduleInfoVector(const DSchedule::List &scheduleInfoVector)
{
    m_scheduleInfoVector = scheduleInfoVector;
}

DSchedule::Ptr CLocalData::SelectInfo() const
{
    return m_SelectInfo;
}

void CLocalData::setSelectInfo(const DSchedule::Ptr &SelectInfo)
{
    m_SelectInfo = SelectInfo;
}

void CLocalData::setToTitleName(const QString &title)
{
    m_ToTitleName = title;
}

QString CLocalData::getToTitleName() const
{
    return m_ToTitleName;
}

void CLocalData::setToTime(const SemanticsDateTime &timeInfo)
{
    m_ToTime = timeInfo;
}

SemanticsDateTime CLocalData::getToTime() const
{
    return m_ToTime;
}

int CLocalData::getOffet() const
{
    return m_offset;
}

void CLocalData::setOffset(int offset)
{
    m_offset = offset;
}

void CLocalData::setNewInfo(const DSchedule::Ptr &newInfo)
{
    m_NewInfo = newInfo;
}

DSchedule::Ptr CLocalData::getNewInfo() const
{
    return m_NewInfo;
}
