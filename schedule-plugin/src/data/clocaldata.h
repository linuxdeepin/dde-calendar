// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CLOCALDATA_H
#define CLOCALDATA_H

#include "schedulestructs.h"
#include "timedatastruct.h"
class CLocalData
{
public:
    CLocalData();
    QVector<ScheduleDtailInfo> scheduleInfoVector() const;
    void setScheduleInfoVector(const QVector<ScheduleDtailInfo> &scheduleInfoVector);
    ScheduleDtailInfo SelectInfo() const;
    void setSelectInfo(const ScheduleDtailInfo &SelectInfo);
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
    CLocalData *getDataByPoint(const CLocalData *localData);
    void setNewInfo(const ScheduleDtailInfo &newInfo);
    ScheduleDtailInfo getNewInfo() const;

private:
    QVector<ScheduleDtailInfo> m_scheduleInfoVector {};
    ScheduleDtailInfo m_SelectInfo {};
    ScheduleDtailInfo m_NewInfo {};
    QString m_ToTitleName {""};
    SemanticsDateTime m_ToTime {};
    int m_offset {-1};
};

#endif // CLOCALDATA_H
