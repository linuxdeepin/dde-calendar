// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedulequerypar.h"

#include "units.h"
#include "commondef.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

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

    DScheduleQueryPar::Ptr queryPar = DScheduleQueryPar::Ptr(new DScheduleQueryPar);
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("key")) {
        queryPar->setKey(rootObj.value("key").toString());
    }
    if (rootObj.contains("dtStart")) {
        queryPar->setDtStart(dtFromString(rootObj.value("dtStart").toString()));
    }
    if (rootObj.contains("dtEnd")) {
        queryPar->setDtEnd(dtFromString(rootObj.value("dtEnd").toString()));
    }
    QueryType qType = Query_None;
    if (rootObj.contains("queryType")) {
        qType = static_cast<QueryType>(rootObj.value("queryType").toInt());
        queryPar->setQueryType(qType);
    }
    switch (qType) {
    case Query_Top: {
        if (rootObj.contains("queryTop")) {
            queryPar->setQueryTop(rootObj.value("queryTop").toInt());
        }
    } break;
    case Query_RRule: {
        if (rootObj.contains("queryRRule")) {
            queryPar->setRruleType(static_cast<RRuleType>(rootObj.value("queryRRule").toInt()));
        }
    }
    default:
        break;
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
