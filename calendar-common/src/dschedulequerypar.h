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
#ifndef DSCHEDULEQUERYPAR_H
#define DSCHEDULEQUERYPAR_H

#include <QDateTime>
#include <QString>
#include <QSharedPointer>

//查询日程相关参数
class DScheduleQueryPar
{
public:
    typedef QSharedPointer<DScheduleQueryPar> Ptr;

    enum QueryType {
        Query_None,
        Query_RRule, //查询重复规则
        Query_Top //查询前多少个
    };

    enum RRuleType {
        RRule_None = 0, //无
        RRule_Day = 4, //每天
        RRule_Week = 5, //每周
        RRule_Month = 6, //每月
        RRule_Year = 7, //每年
        RRule_Work = 8, //每个工作日
    };

    DScheduleQueryPar();

    QDateTime dtStart() const;
    void setDtStart(const QDateTime &dtStart);

    QDateTime dtEnd() const;
    void setDtEnd(const QDateTime &dtEnd);

    QString key() const;
    void setKey(const QString &key);

    static DScheduleQueryPar::Ptr fromJsonString(const QString &queryStr);
    static QString toJsonString(const DScheduleQueryPar::Ptr &queryPar);

    QueryType queryType() const;
    void setQueryType(const QueryType &queryType);

    int queryTop() const;
    void setQueryTop(int queryTop);

    RRuleType rruleType() const;
    void setRruleType(const RRuleType &rruleType);

private:
    QString m_key; //查询关键字
    int m_queryTop; //查询范围内前多少个日程
    RRuleType m_rruleType; //查询对应重复规则的日程
    QueryType m_queryType; //查询的类型
    QDateTime m_dtStart; //查询的起始时间
    QDateTime m_dtEnd; //查询的截止时间
};

#endif // DSCHEDULEQUERYPAR_H
