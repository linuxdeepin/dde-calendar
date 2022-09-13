// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "reminddata.h"

RemindData::RemindData()
    : m_RemindNum(-1)
    , m_RemindTime(9, 0, 0)
{

}

RemindData::RemindData(const int remindNum, const QTime &remindTime)
    : m_RemindNum(remindNum)
    , m_RemindTime(remindTime)
{

}

/**
 * @brief setRemindNum  设置提前多久提醒
 * @param num
 */
void RemindData::setRemindNum(const int num)
{
    m_RemindNum = num;
}

/**
 * @brief getRemindNum  获取提前多久提醒
 * @return
 */
int RemindData::getRemindNum() const
{
    return m_RemindNum;
}

/**
 * @brief setRemindTime 设置提醒时间
 * @param remindTime
 */
void RemindData::setRemindTime(const QTime &remindTime)
{
    m_RemindTime = remindTime;
}

/**
 * @brief getRemindTime 获取提醒时间
 * @return
 */
QTime RemindData::getRemindTime() const
{
    return  m_RemindTime;
}

/**
 * @brief RemindStringToRemindData      将sting转换为提醒数据
 * @param remindStr                     提醒的string数据
 * @return
 */
RemindData RemindData::RemindStringToRemindData(const QString &remindStr)
{
    RemindData _resultRemindData;
    if (!remindStr.isEmpty()) {
        QStringList liststr = remindStr.split(";", QString::SkipEmptyParts);
        _resultRemindData.setRemindNum(liststr.at(0).toInt());
        if (liststr.size() > 1) {
            _resultRemindData.setRemindTime(QTime::fromString(liststr.at(1), "hh:mm"));
        }
    }
    return _resultRemindData;
}

/**
 * @brief RemindDataToRemindString      将提醒数据转换为string
 * @param remindData                    提醒数据
 * @param isAllDay                      是否为全天
 * @return
 */
QString RemindData::RemindDataToRemindString(const RemindData &remindData, const bool isAllDay)
{
    QString _resultStr {""};
    if (remindData.getRemindNum() != -1) {
        _resultStr += QString::number(remindData.getRemindNum());
        if (isAllDay) {
            _resultStr += QString(";%1").arg(remindData.getRemindTime().toString("hh:mm"));
        }
    }
    return _resultStr;
}
