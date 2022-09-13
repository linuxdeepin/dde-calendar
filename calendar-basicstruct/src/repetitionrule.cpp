// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "repetitionrule.h"

#include <QDebug>

RepetitionRule::RepetitionRule()
    : m_ruleID(RRule_NONE)
    , m_ruleEndType(RRuleType_NEVER)
    , m_endDateTime()
    , m_endCount(0)
{

}

/**
 * @brief getRuleId     获取重复规则id
 * @return
 */
RepetitionRule::RRuleID RepetitionRule::getRuleId() const
{
    return m_ruleID;
}

/**
 * @brief setRuleId     设置重复规则id
 * @param ruleId
 */
void RepetitionRule::setRuleId(const RepetitionRule::RRuleID ruleId)
{
    m_ruleID = ruleId;
}

/**
 * @brief setRuleType   设置重复结束类型
 * @param endType
 */
void RepetitionRule::setRuleType(const RepetitionRule::RRuleEndType &endType)
{
    m_ruleEndType = endType;
}

/**
 * @brief getRuleType   获取重复结束类型
 * @return
 */
RepetitionRule::RRuleEndType RepetitionRule::getRuleType() const
{
    return  m_ruleEndType;
}

/**
 * @brief getEndDate    获取结束时间
 * @return
 */
QDateTime RepetitionRule::getEndDate() const
{
    return  m_endDateTime;
}

/**
 * @brief setEndDate    设置结束时间
 * @param endDate       结束时间
 */
void RepetitionRule::setEndDate(const QDateTime &endDate)
{
    m_endDateTime = endDate;
}

/**
 * @brief getEndCount   获取结束次数
 * @return
 */
int RepetitionRule::getEndCount() const
{
    return  m_endCount;
}

/**
 * @brief setEndCount   设置结束次数
 * @param endCount      结束次数
 */
void RepetitionRule::setEndCount(const int endCount)
{
    m_endCount = endCount;
}

void RepetitionRule::clear()
{
    m_ruleID = RRule_NONE;
    m_ruleEndType = RRuleType_NEVER;
}

/**
 * @brief RuleStringToRuleData  解析重复规则
 * @param ruleStr
 * @return
 */
RepetitionRule RepetitionRule::RuleStringToRuleData(const QString &ruleStr)
{
    RepetitionRule _resultRule{};

    if (ruleStr.isEmpty()) {
        return _resultRule;
    }
    QStringList rruleslist = ruleStr.split(";", QString::SkipEmptyParts);

    if (rruleslist.count() > 0) {
        if (rruleslist.contains("FREQ=DAILY") && rruleslist.contains("BYDAY=MO,TU,WE,TH,FR")) {
            _resultRule.setRuleId(RRule_WORKDAY);
        } else if (rruleslist.contains("FREQ=DAILY")) {
            _resultRule.setRuleId(RRule_EVEDAY);
        } else if (rruleslist.contains("FREQ=WEEKLY")) {
            _resultRule.setRuleId(RRule_EVEWEEK);
        } else if (rruleslist.contains("FREQ=MONTHLY")) {
            _resultRule.setRuleId(RRule_EVEMONTH);
        } else if (rruleslist.contains("FREQ=YEARLY")) {
            _resultRule.setRuleId(RRule_EVEYEAR);
        }

        for (int i = 0; i < rruleslist.count(); i++) {
            if (rruleslist.at(i).contains("COUNT=")) {
                QStringList liststr = rruleslist.at(i).split("=", QString::SkipEmptyParts);
                _resultRule.setRuleType(RRuleType_FREQ);
                _resultRule.setEndCount(liststr.at(1).toInt() - 1);
            }
            if (rruleslist.at(i).contains("UNTIL=")) {
                QStringList liststr = rruleslist.at(i).split("=", QString::SkipEmptyParts);
                if (liststr.size() > 1) {
                    _resultRule.setRuleType(RRuleType_DATE);
                    QDateTime _endDate = resolutionEndDate(liststr.at(1));
                    _resultRule.setEndDate(_endDate);
                } else {
                    qWarning() << "Recurrence Rule Err," << ruleStr;
                }

            }
        }
    }
    return _resultRule;
}

/**
 * @brief RuleDataToRuleString      将重复数据转换为string类型
 * @param rule                      重复规则
 * @return
 */
QString RepetitionRule::RuleDataToRuleString(const RepetitionRule &rule)
{
    QString _resultStr {""};
    switch (rule.getRuleId()) {
    case RRule_NONE:
        _resultStr += "";
        break;
    case RRule_EVEDAY:
        _resultStr += "FREQ=DAILY";
        break;
    case RRule_WORKDAY:
        _resultStr += "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR";
        break;
    case RRule_EVEWEEK:
        _resultStr += "FREQ=WEEKLY";
        break;
    case RRule_EVEMONTH:
        _resultStr += "FREQ=MONTHLY";
        break;
    case RRule_EVEYEAR:
        _resultStr += "FREQ=YEARLY";
        break;
    case RRule_CUSTOM:

        break;
    }
    switch (rule.getRuleType()) {
    case RRuleType_NEVER:

        break;
    case RRuleType_FREQ:
        _resultStr += QString(";COUNT=%1").arg(rule.getEndCount() + 1);
        break;
    case RRuleType_DATE:
        _resultStr += ";UNTIL=" + endDateToString(rule.getEndDate());
        break;
    }
    return _resultStr;
}

/**
 * @brief RepetitionRule::resolutionEndDate     解析结束时间
 * @param endDateStr
 * @return
 */
QDateTime RepetitionRule::resolutionEndDate(const QString &endDateStr)
{
    return QDateTime::fromString(endDateStr.left(endDateStr.count() - 1), "yyyyMMddThhmmss");
}

/**
 * @brief RepetitionRule::endDateToString       结束时间转换为string
 * @param endDate
 * @return
 */
QString RepetitionRule::endDateToString(const QDateTime &endDate)
{
    return  endDate.toString("yyyyMMddThhmmss") + "Z";
}
