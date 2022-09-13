// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef REPETITIONRULE_H
#define REPETITIONRULE_H

#include <QObject>
#include <QDateTime>
/**
 * @brief The RepetitionRule class
 * 重复规则
 */

class RepetitionRule
{
public:
    enum RRuleID {
        RRule_NONE = 0,        //不重复
        RRule_EVEDAY,       //每天
        RRule_WORKDAY,      //每个工作日
        RRule_EVEWEEK,      //每周
        RRule_EVEMONTH,     //每月
        RRule_EVEYEAR,      //每年
        RRule_CUSTOM        //用户自定义
    };
    enum RRuleEndType {
        RRuleType_NEVER = 0,   //永不
        RRuleType_FREQ,     //结束于次数
        RRuleType_DATE      //结束与日期
    };
public:
    explicit RepetitionRule();
    //获取重复规则id
    RRuleID getRuleId() const;
    //设置重复规则id
    void setRuleId(const RRuleID ruleId);
    //设置重复结束类型
    void setRuleType(const RRuleEndType &endType);
    //获取重复结束类型
    RRuleEndType getRuleType() const;
    //获取结束时间
    QDateTime getEndDate() const;
    //设置结束时间
    void setEndDate(const QDateTime &endDate);
    //获取结束次数
    int getEndCount() const;
    //设置结束次数
    void setEndCount(const int endCount);
    void clear();
    //解析重复规则
    static RepetitionRule RuleStringToRuleData(const QString &ruleStr);
    //将重复数据转换为string类型
    static QString RuleDataToRuleString(const RepetitionRule &rule);
private:
    //解析结束时间
    static QDateTime resolutionEndDate(const QString &endDateStr);
    //结束时间转换为string
    static QString  endDateToString(const QDateTime &endDate);
private:
    /**
     * @brief m_ruleID
     * 重复规则编号
     */
    RRuleID   m_ruleID;
    /**
     * @brief m_endType
     * 结束类型
     */
    RRuleEndType    m_ruleEndType;
    /**
     * @brief m_endDateTime
     *  重复结束时间
     */
    QDateTime   m_endDateTime;
    /**
     * @brief m_endCount
     * 重复结束次数
     */
    int     m_endCount;
};

#endif // REPETITIONRULE_H
