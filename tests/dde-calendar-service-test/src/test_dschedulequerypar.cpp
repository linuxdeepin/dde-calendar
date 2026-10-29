// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dschedulequerypar.h"

#include <gtest/gtest.h>

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimeZone>

namespace {

// Helper: produce a dtToString-compatible timestamp for a fixed local QDateTime.
// dtToString emits "yyyy-MM-ddThh:mm:ss+HH:MM" where the offset is the
// system's current UTC offset.  We generate it the same way the production
// code does so round-trip tests are timezone-agnostic.
QString makeTimestamp(const QDateTime &dt)
{
    const int offset = dt.timeZone().offsetFromUtc(dt);
    const QChar sign = offset < 0 ? QLatin1Char('-') : QLatin1Char('+');
    const int absSec = qAbs(offset);
    return QStringLiteral("%1%2%3:%4")
        .arg(dt.toString(QStringLiteral("yyyy-MM-ddThh:mm:ss")))
        .arg(sign)
        .arg(absSec / 3600, 2, 10, QLatin1Char('0'))
        .arg((absSec % 3600) / 60, 2, 10, QLatin1Char('0'));
}

TEST(DScheduleQueryPar, DefaultConstructorSetsDefaults)
{
    DScheduleQueryPar par;
    EXPECT_EQ(QString(), par.key());
    EXPECT_EQ(1, par.queryTop());
    EXPECT_EQ(DScheduleQueryPar::RRule_None, par.rruleType());
    EXPECT_EQ(DScheduleQueryPar::Query_None, par.queryType());
    EXPECT_FALSE(par.dtStart().isValid());
    EXPECT_FALSE(par.dtEnd().isValid());
}

TEST(DScheduleQueryPar, KeyGetterSetter)
{
    DScheduleQueryPar par;
    par.setKey(QStringLiteral("my-key"));
    EXPECT_EQ(QStringLiteral("my-key"), par.key());

    par.setKey(QString());
    EXPECT_TRUE(par.key().isEmpty());
}

TEST(DScheduleQueryPar, DtStartGetterSetter)
{
    DScheduleQueryPar par;
    const QDateTime dt(QDate(2025, 1, 15), QTime(10, 30, 0));
    par.setDtStart(dt);
    EXPECT_EQ(dt, par.dtStart());
}

TEST(DScheduleQueryPar, DtEndGetterSetter)
{
    DScheduleQueryPar par;
    const QDateTime dt(QDate(2025, 6, 20), QTime(18, 0, 0));
    par.setDtEnd(dt);
    EXPECT_EQ(dt, par.dtEnd());
}

TEST(DScheduleQueryPar, QueryTypeGetterSetter)
{
    DScheduleQueryPar par;
    par.setQueryType(DScheduleQueryPar::Query_RRule);
    EXPECT_EQ(DScheduleQueryPar::Query_RRule, par.queryType());

    par.setQueryType(DScheduleQueryPar::Query_Top);
    EXPECT_EQ(DScheduleQueryPar::Query_Top, par.queryType());

    par.setQueryType(DScheduleQueryPar::Query_ScheduleID);
    EXPECT_EQ(DScheduleQueryPar::Query_ScheduleID, par.queryType());

    par.setQueryType(DScheduleQueryPar::Query_None);
    EXPECT_EQ(DScheduleQueryPar::Query_None, par.queryType());
}

TEST(DScheduleQueryPar, QueryTopGetterSetter)
{
    DScheduleQueryPar par;
    par.setQueryTop(5);
    EXPECT_EQ(5, par.queryTop());

    par.setQueryTop(100);
    EXPECT_EQ(100, par.queryTop());
}

TEST(DScheduleQueryPar, RruleTypeGetterSetter)
{
    DScheduleQueryPar par;
    par.setRruleType(DScheduleQueryPar::RRule_Day);
    EXPECT_EQ(DScheduleQueryPar::RRule_Day, par.rruleType());

    par.setRruleType(DScheduleQueryPar::RRule_Week);
    EXPECT_EQ(DScheduleQueryPar::RRule_Week, par.rruleType());

    par.setRruleType(DScheduleQueryPar::RRule_Month);
    EXPECT_EQ(DScheduleQueryPar::RRule_Month, par.rruleType());

    par.setRruleType(DScheduleQueryPar::RRule_Year);
    EXPECT_EQ(DScheduleQueryPar::RRule_Year, par.rruleType());
}

TEST(DScheduleQueryPar, FromJsonStringParsesValidQueryNone)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "test-key");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", static_cast<int>(DScheduleQueryPar::Query_None));

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    ASSERT_NE(nullptr, par);
    EXPECT_EQ(QStringLiteral("test-key"), par->key());
    EXPECT_EQ(DScheduleQueryPar::Query_None, par->queryType());
}

TEST(DScheduleQueryPar, FromJsonStringParsesValidQueryTop)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "top-query");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", static_cast<int>(DScheduleQueryPar::Query_Top));
    obj.insert("queryTop", 10);

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    ASSERT_NE(nullptr, par);
    EXPECT_EQ(DScheduleQueryPar::Query_Top, par->queryType());
    EXPECT_EQ(10, par->queryTop());
}

TEST(DScheduleQueryPar, FromJsonStringParsesValidQueryRRule)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "rrule-query");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", static_cast<int>(DScheduleQueryPar::Query_RRule));
    obj.insert("queryRRule", static_cast<int>(DScheduleQueryPar::RRule_Week));

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    ASSERT_NE(nullptr, par);
    EXPECT_EQ(DScheduleQueryPar::Query_RRule, par->queryType());
    EXPECT_EQ(DScheduleQueryPar::RRule_Week, par->rruleType());
}

TEST(DScheduleQueryPar, FromJsonStringParsesValidQueryScheduleID)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "schedule-id-123");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", static_cast<int>(DScheduleQueryPar::Query_ScheduleID));

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    ASSERT_NE(nullptr, par);
    EXPECT_EQ(DScheduleQueryPar::Query_ScheduleID, par->queryType());
    EXPECT_EQ(QStringLiteral("schedule-id-123"), par->key());
}

TEST(DScheduleQueryPar, FromJsonStringReturnsNullptrOnInvalidJson)
{
    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString("not json");
    EXPECT_EQ(nullptr, par);
}

TEST(DScheduleQueryPar, FromJsonStringReturnsNullptrOnMissingKey)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", 0);

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    EXPECT_EQ(nullptr, par);
}

TEST(DScheduleQueryPar, FromJsonStringReturnsNullptrOnInvalidDateRange)
{
    // dtStart > dtEnd should fail
    const QDateTime dtStart(QDate(2025, 12, 31), QTime(23, 59, 59));
    const QDateTime dtEnd(QDate(2025, 1, 1), QTime(0, 0, 0));

    QJsonObject obj;
    obj.insert("key", "test");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", 0);

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    EXPECT_EQ(nullptr, par);
}

TEST(DScheduleQueryPar, FromJsonStringReturnsNullptrOnInvalidQueryType)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "test");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", 99); // out of range

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    EXPECT_EQ(nullptr, par);
}

TEST(DScheduleQueryPar, FromJsonStringReturnsNullptrOnZeroQueryTop)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "test");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", static_cast<int>(DScheduleQueryPar::Query_Top));
    obj.insert("queryTop", 0); // must be > 0

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    EXPECT_EQ(nullptr, par);
}

TEST(DScheduleQueryPar, FromJsonStringReturnsNullptrOnInvalidRRule)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "test");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", static_cast<int>(DScheduleQueryPar::Query_RRule));
    obj.insert("queryRRule", 99); // out of range

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    EXPECT_EQ(nullptr, par);
}

TEST(DScheduleQueryPar, FromJsonStringReturnsNullptrOnEmptyKeyForScheduleID)
{
    const QDateTime dtStart(QDate(2025, 1, 1), QTime(0, 0, 0));
    const QDateTime dtEnd(QDate(2025, 12, 31), QTime(23, 59, 59));

    QJsonObject obj;
    obj.insert("key", "");
    obj.insert("dtStart", makeTimestamp(dtStart));
    obj.insert("dtEnd", makeTimestamp(dtEnd));
    obj.insert("queryType", static_cast<int>(DScheduleQueryPar::Query_ScheduleID));

    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));

    DScheduleQueryPar::Ptr par = DScheduleQueryPar::fromJsonString(json);
    EXPECT_EQ(nullptr, par);
}

TEST(DScheduleQueryPar, ToJsonStringSerializesQueryNone)
{
    DScheduleQueryPar::Ptr par(new DScheduleQueryPar);
    par->setKey(QStringLiteral("serialize-key"));
    par->setDtStart(QDateTime(QDate(2025, 3, 1), QTime(8, 0, 0)));
    par->setDtEnd(QDateTime(QDate(2025, 3, 31), QTime(20, 0, 0)));
    par->setQueryType(DScheduleQueryPar::Query_None);

    const QString json = DScheduleQueryPar::toJsonString(par);
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    ASSERT_TRUE(doc.isObject());
    const QJsonObject obj = doc.object();
    EXPECT_EQ(QStringLiteral("serialize-key"), obj.value("key").toString());
    EXPECT_EQ(static_cast<int>(DScheduleQueryPar::Query_None),
              obj.value("queryType").toInt());
    // Query_None should not include queryTop or queryRRule
    EXPECT_FALSE(obj.contains("queryTop"));
    EXPECT_FALSE(obj.contains("queryRRule"));
}

TEST(DScheduleQueryPar, ToJsonStringSerializesQueryTop)
{
    DScheduleQueryPar::Ptr par(new DScheduleQueryPar);
    par->setKey(QStringLiteral("top"));
    par->setDtStart(QDateTime(QDate(2025, 3, 1), QTime(8, 0, 0)));
    par->setDtEnd(QDateTime(QDate(2025, 3, 31), QTime(20, 0, 0)));
    par->setQueryType(DScheduleQueryPar::Query_Top);
    par->setQueryTop(7);

    const QString json = DScheduleQueryPar::toJsonString(par);
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    ASSERT_TRUE(doc.isObject());
    const QJsonObject obj = doc.object();
    EXPECT_EQ(static_cast<int>(DScheduleQueryPar::Query_Top),
              obj.value("queryType").toInt());
    EXPECT_EQ(7, obj.value("queryTop").toInt());
    EXPECT_FALSE(obj.contains("queryRRule"));
}

TEST(DScheduleQueryPar, ToJsonStringSerializesQueryRRule)
{
    DScheduleQueryPar::Ptr par(new DScheduleQueryPar);
    par->setKey(QStringLiteral("rrule"));
    par->setDtStart(QDateTime(QDate(2025, 3, 1), QTime(8, 0, 0)));
    par->setDtEnd(QDateTime(QDate(2025, 3, 31), QTime(20, 0, 0)));
    par->setQueryType(DScheduleQueryPar::Query_RRule);
    par->setRruleType(DScheduleQueryPar::RRule_Month);

    const QString json = DScheduleQueryPar::toJsonString(par);
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    ASSERT_TRUE(doc.isObject());
    const QJsonObject obj = doc.object();
    EXPECT_EQ(static_cast<int>(DScheduleQueryPar::Query_RRule),
              obj.value("queryType").toInt());
    EXPECT_EQ(static_cast<int>(DScheduleQueryPar::RRule_Month),
              obj.value("queryRRule").toInt());
    EXPECT_FALSE(obj.contains("queryTop"));
}

TEST(DScheduleQueryPar, ToJsonStringReturnsEmptyForNullPtr)
{
    DScheduleQueryPar::Ptr nullPtr;
    const QString json = DScheduleQueryPar::toJsonString(nullPtr);
    EXPECT_TRUE(json.isEmpty());
}

TEST(DScheduleQueryPar, JsonRoundTripPreservesValues)
{
    DScheduleQueryPar::Ptr original(new DScheduleQueryPar);
    original->setKey(QStringLiteral("round-trip"));
    original->setDtStart(QDateTime(QDate(2025, 1, 1), QTime(9, 0, 0)));
    original->setDtEnd(QDateTime(QDate(2025, 6, 30), QTime(17, 0, 0)));
    original->setQueryType(DScheduleQueryPar::Query_Top);
    original->setQueryTop(3);

    const QString json = DScheduleQueryPar::toJsonString(original);
    DScheduleQueryPar::Ptr restored = DScheduleQueryPar::fromJsonString(json);
    ASSERT_NE(nullptr, restored);
    EXPECT_EQ(original->key(), restored->key());
    EXPECT_EQ(original->queryType(), restored->queryType());
    EXPECT_EQ(original->queryTop(), restored->queryTop());
    EXPECT_EQ(original->dtStart(), restored->dtStart());
    EXPECT_EQ(original->dtEnd(), restored->dtEnd());
}

} // namespace
