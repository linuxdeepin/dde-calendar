// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REMINDDATA_H
#define REMINDDATA_H

#include <QTime>
/**
 * @brief The RemindData class
 * 提醒规则
 */
class RemindData
{
public:
    explicit RemindData();
    RemindData(const int remindNum, const QTime &remindTime);
    //设置提前多久提醒
    void setRemindNum(const int num);
    //获取提前多久提醒
    int getRemindNum() const;
    //设置提醒时间
    void setRemindTime(const QTime &remindTime);
    //获取提醒时间
    QTime getRemindTime() const;
    //将sting转换为提醒数据
    static RemindData RemindStringToRemindData(const QString &remindStr);
    //将提醒数据转换为string
    static QString RemindDataToRemindString(const RemindData &remindData, const bool isAllDay);
private:
    /**
     * @brief m_RemindNum   提前多久提醒
     * -1 永不提醒
     * 若日程为全天 0～N 表示提前多少天提醒
     * 若日程为非全天 0～N 表示提前多少分钟提醒
     */
    int     m_RemindNum;
    /**
     * @brief m_RemindTime  提醒时间
     * 只有日程为全天 才有效
     */
    QTime   m_RemindTime;
};

#endif // REMINDDATA_H
