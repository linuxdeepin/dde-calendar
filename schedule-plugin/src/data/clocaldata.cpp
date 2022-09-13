// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clocaldata.h"

CLocalData::CLocalData()
{
}

QVector<ScheduleDtailInfo> CLocalData::scheduleInfoVector() const
{
    return m_scheduleInfoVector;
}

void CLocalData::setScheduleInfoVector(const QVector<ScheduleDtailInfo> &scheduleInfoVector)
{
    m_scheduleInfoVector = scheduleInfoVector;
}

ScheduleDtailInfo CLocalData::SelectInfo() const
{
    return m_SelectInfo;
}

void CLocalData::setSelectInfo(const ScheduleDtailInfo &SelectInfo)
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

CLocalData *CLocalData::getDataByPoint(const CLocalData *localData)
{
    if (this == localData || localData == nullptr)
        return this;
    this->m_offset = localData->m_offset;
    this->m_SelectInfo = localData->m_SelectInfo;
    this->m_ToTime = localData->m_ToTime;
    this->m_scheduleInfoVector = localData->m_scheduleInfoVector;
    this->m_ToTitleName = localData->m_ToTitleName;
    return this;
}

void CLocalData::setNewInfo(const ScheduleDtailInfo &newInfo)
{
    m_NewInfo = newInfo;
}

ScheduleDtailInfo CLocalData::getNewInfo() const
{
    return m_NewInfo;
}
