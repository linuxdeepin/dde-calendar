// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedulequerypar.h"

#include "units.h"
#include "commondef.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace {

bool readJsonInt(const QJsonObject &object, const QString &name, int &value)
{
    const QJsonValue jsonValue = object.value(name);
    if (!jsonValue.isDouble()) {
        return false;
    }

    const double number = jsonValue.toDouble();
    const int integer = jsonValue.toInt(0);
    if (number != integer) {
        return false;
    }

    value = integer;
    return true;
}

bool readJsonDateTime(const QJsonObject &object, const QString &name, QDateTime &value)
{
    const QJsonValue jsonValue = object.value(name);
    if (!jsonValue.isString()) {
        return false;
    }

    value = dtFromString(jsonValue.toString());
    return value.isValid();
}

} // namespace

DScheduleQueryPar::DScheduleQueryPar()
    : m_key("")
    , m_queryTop(1)
    , m_rruleType(RRule_None)
    , m_queryType(Query_None)
{
    qCDebug(CommonLogger) << "DScheduleQueryPar constructor called.";
}

QDateTime DScheduleQueryPar::dtStart() const
{
    // qCDebug(CommonLogger) << "Getting dtStart:" << m_dtStart;
    return m_dtStart;
}

void DScheduleQueryPar::setDtStart(const QDateTime &dtStart)
{
    // qCDebug(CommonLogger) << "Setting dtStart to:" << dtStart;
    m_dtStart = dtStart;
}

QDateTime DScheduleQueryPar::dtEnd() const
{
    // qCDebug(CommonLogger) << "Getting dtEnd:" << m_dtEnd;
    return m_dtEnd;
}

void DScheduleQueryPar::setDtEnd(const QDateTime &dtEnd)
{
    // qCDebug(CommonLogger) << "Setting dtEnd to:" << dtEnd;
    m_dtEnd = dtEnd;
}

QString DScheduleQueryPar::key() const
{
    // qCDebug(CommonLogger) << "Getting key:" << m_key;
    return m_key;
}

void DScheduleQueryPar::setKey(const QString &key)
{
    // qCDebug(CommonLogger) << "Setting key to:" << key;
    m_key = key;
}

DScheduleQueryPar::Ptr DScheduleQueryPar::fromJsonString(const QString &queryStr)
{
    // qCDebug(CommonLogger) << "Parsing DScheduleQueryPar from JSON string:" << queryStr;
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(queryStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "Failed to parse query parameters JSON. Error:" << jsonError.errorString() 
                                 << "Query string:" << queryStr;
        return nullptr;
    }

    if (!jsonDoc.isObject()) {
        qCWarning(CommonLogger) << "Query parameters JSON root is not an object";
        return nullptr;
    }

    DScheduleQueryPar::Ptr queryPar = DScheduleQueryPar::Ptr(new DScheduleQueryPar);
    QJsonObject rootObj = jsonDoc.object();
    if (!rootObj.contains("key") || !rootObj.value("key").isString()) {
        qCWarning(CommonLogger) << "Query parameters are missing a string key";
        return nullptr;
    }
    queryPar->setKey(rootObj.value("key").toString());

    QDateTime dtStart;
    QDateTime dtEnd;
    if (!readJsonDateTime(rootObj, QStringLiteral("dtStart"), dtStart)
        || !readJsonDateTime(rootObj, QStringLiteral("dtEnd"), dtEnd)
        || dtStart > dtEnd) {
        qCWarning(CommonLogger) << "Query parameters contain an invalid date range";
        return nullptr;
    }

    queryPar->setDtStart(dtStart);
    queryPar->setDtEnd(dtEnd);

    int queryTypeValue = 0;
    if (!readJsonInt(rootObj, QStringLiteral("queryType"), queryTypeValue)
        || queryTypeValue < Query_None
        || queryTypeValue > Query_ScheduleID) {
        qCWarning(CommonLogger) << "Query parameters contain an invalid query type";
        return nullptr;
    }
    const QueryType qType = static_cast<QueryType>(queryTypeValue);
    queryPar->setQueryType(qType);

    switch (qType) {
    case Query_Top: {
        int queryTop = 0;
        if (!readJsonInt(rootObj, QStringLiteral("queryTop"), queryTop) || queryTop <= 0) {
            qCWarning(CommonLogger) << "Top query requires a positive queryTop";
            return nullptr;
        }
        queryPar->setQueryTop(queryTop);
    } break;
    case Query_RRule: {
        int rruleValue = 0;
        if (!readJsonInt(rootObj, QStringLiteral("queryRRule"), rruleValue)
            || rruleValue < RRule_Day
            || rruleValue > RRule_Year) {
            qCWarning(CommonLogger) << "RRule query contains an invalid queryRRule";
            return nullptr;
        }
        queryPar->setRruleType(static_cast<RRuleType>(rruleValue));
    } break;
    case Query_ScheduleID:
        if (queryPar->key().isEmpty()) {
            qCWarning(CommonLogger) << "Schedule ID query requires a non-empty key";
            return nullptr;
        }
        break;
    case Query_None:
        break;
    default:
        return nullptr;
    }
    // qCDebug(CommonLogger) << "Successfully parsed DScheduleQueryPar from JSON.";
    return queryPar;
}

QString DScheduleQueryPar::toJsonString(const DScheduleQueryPar::Ptr &queryPar)
{
    // qCDebug(CommonLogger) << "Converting DScheduleQueryPar to JSON string.";
    if (queryPar.isNull()) {
        qCWarning(CommonLogger) << "Cannot convert null query parameters to JSON";
        return QString();
    }

    QJsonObject jsonObj;
    jsonObj.insert("key", queryPar->key());
    jsonObj.insert("dtStart", dtToString(queryPar->dtStart()));
    jsonObj.insert("dtEnd", dtToString(queryPar->dtEnd()));
    jsonObj.insert("queryType", queryPar->queryType());
    switch (queryPar->queryType()) {
    case Query_Top:
        // qCDebug(CommonLogger) << "Querying top" << queryPar->queryTop();
        jsonObj.insert("queryTop", queryPar->queryTop());
        break;
    case Query_RRule:
        // qCDebug(CommonLogger) << "Querying rrule" << queryPar->rruleType();
        jsonObj.insert("queryRRule", queryPar->rruleType());
        break;
    default:
        break;
    }

    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObj);
    QString jsonString = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    // qCDebug(CommonLogger) << "Resulting DScheduleQueryPar JSON:" << jsonString;
    return jsonString;
}

DScheduleQueryPar::QueryType DScheduleQueryPar::queryType() const
{
    // qCDebug(CommonLogger) << "Getting queryType:" << m_queryType;
    return m_queryType;
}

void DScheduleQueryPar::setQueryType(const QueryType &queryType)
{
    // qCDebug(CommonLogger) << "Setting queryType to:" << queryType;
    m_queryType = queryType;
}

int DScheduleQueryPar::queryTop() const
{
    // qCDebug(CommonLogger) << "Getting queryTop:" << m_queryTop;
    return m_queryTop;
}

void DScheduleQueryPar::setQueryTop(int queryTop)
{
    // qCDebug(CommonLogger) << "Setting queryTop to:" << queryTop;
    m_queryTop = queryTop;
}

DScheduleQueryPar::RRuleType DScheduleQueryPar::rruleType() const
{
    // qCDebug(CommonLogger) << "Getting rruleType:" << m_rruleType;
    return m_rruleType;
}

void DScheduleQueryPar::setRruleType(const RRuleType &rruleType)
{
    // qCDebug(CommonLogger) << "Setting rruleType to:" << rruleType;
    m_rruleType = rruleType;
}
