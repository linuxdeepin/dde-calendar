// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "jsondata.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>
#include <QDate>
#include <QTime>

// Test fixture for JsonData
class TestJsonData : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_data = new JsonData();
    }

    void TearDown() override
    {
        delete m_data;
        m_data = nullptr;
    }

    // Helper: build a slots JSON object containing a single slot
    QJsonObject buildSingleSlotJson(const QString &name, const QJsonValue &value)
    {
        QJsonObject root;
        QJsonArray slotArray;
        QJsonObject slot;
        slot["name"] = name;
        slot["value"] = value;
        slotArray.append(slot);
        root["slots"] = slotArray;
        return root;
    }

    JsonData *m_data = nullptr;
};

// ---- Constructor / Destructor ----

TEST_F(TestJsonData, ConstructorCreatesValidObject)
{
    ASSERT_NE(m_data, nullptr);
}

TEST_F(TestJsonData, DestructorDoesNotCrash)
{
    JsonData *data = new JsonData();
    delete data;
    SUCCEED();
}

// ---- isVaild ----

TEST_F(TestJsonData, IsVaildOnDefaultObject)
{
    EXPECT_TRUE(m_data->isVaild());
}

TEST_F(TestJsonData, IsVaildFalseAfterSetTitle)
{
    m_data->setTitleName(QStringLiteral("Test"));
    EXPECT_FALSE(m_data->isVaild());
}

TEST_F(TestJsonData, IsVaildFalseAfterSetOffset)
{
    m_data->setOffset(5);
    EXPECT_FALSE(m_data->isVaild());
}

TEST_F(TestJsonData, IsVaildFalseAfterSetRepeatStatus)
{
    m_data->setRepeatStatus(JsonData::EVED);
    EXPECT_FALSE(m_data->isVaild());
}

TEST_F(TestJsonData, IsVaildFalseAfterSetPropertyStatus)
{
    m_data->setPropertyStatus(JsonData::NEXT);
    EXPECT_FALSE(m_data->isVaild());
}

TEST_F(TestJsonData, IsVaildFalseAfterSetDateTime)
{
    SemanticsDateTime sdt;
    SuggestDatetimeInfo info;
    info.hasTime = true;
    sdt.suggestDatetime.append(info);
    m_data->setDateTime(sdt);
    EXPECT_FALSE(m_data->isVaild());
}

// ---- TitleName / setTitleName ----

TEST_F(TestJsonData, TitleNameDefaultEmpty)
{
    EXPECT_TRUE(m_data->TitleName().isEmpty());
}

TEST_F(TestJsonData, SetTitleNameAndRetrieve)
{
    m_data->setTitleName(QStringLiteral("Meeting"));
    EXPECT_EQ(m_data->TitleName().toStdString(), "Meeting");
}

TEST_F(TestJsonData, SetTitleNameEmpty)
{
    m_data->setTitleName(QStringLiteral(""));
    EXPECT_TRUE(m_data->TitleName().isEmpty());
}

// ---- SuggestMsg / setSuggestMsg ----

TEST_F(TestJsonData, SuggestMsgDefaultEmpty)
{
    EXPECT_TRUE(m_data->SuggestMsg().isEmpty());
}

TEST_F(TestJsonData, SetSuggestMsgAndRetrieve)
{
    m_data->setSuggestMsg(QStringLiteral("Hello"));
    EXPECT_EQ(m_data->SuggestMsg().toStdString(), "Hello");
}

// ---- ShouldEndSession / setShouldEndSession ----

TEST_F(TestJsonData, ShouldEndSessionDefaultFalse)
{
    EXPECT_FALSE(m_data->ShouldEndSession());
}

TEST_F(TestJsonData, SetShouldEndSessionTrue)
{
    m_data->setShouldEndSession(true);
    EXPECT_TRUE(m_data->ShouldEndSession());
}

TEST_F(TestJsonData, SetShouldEndSessionFalse)
{
    m_data->setShouldEndSession(true);
    m_data->setShouldEndSession(false);
    EXPECT_FALSE(m_data->ShouldEndSession());
}

// ---- getRepeatStatus / setRepeatStatus ----

TEST_F(TestJsonData, RepeatStatusDefaultNone)
{
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::NONE);
}

TEST_F(TestJsonData, SetRepeatStatusEVED)
{
    m_data->setRepeatStatus(JsonData::EVED);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVED);
}

TEST_F(TestJsonData, SetRepeatStatusEVEW)
{
    m_data->setRepeatStatus(JsonData::EVEW);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEW);
}

TEST_F(TestJsonData, SetRepeatStatusEVEM)
{
    m_data->setRepeatStatus(JsonData::EVEM);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEM);
}

TEST_F(TestJsonData, SetRepeatStatusEVEY)
{
    m_data->setRepeatStatus(JsonData::EVEY);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEY);
}

TEST_F(TestJsonData, SetRepeatStatusWORKD)
{
    m_data->setRepeatStatus(JsonData::WORKD);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::WORKD);
}

TEST_F(TestJsonData, SetRepeatStatusRESTD)
{
    m_data->setRepeatStatus(JsonData::RESTD);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::RESTD);
}

// ---- getRepeatNum / setRepeatNum ----

TEST_F(TestJsonData, RepeatNumDefaultEmpty)
{
    EXPECT_TRUE(m_data->getRepeatNum().isEmpty());
}

TEST_F(TestJsonData, SetRepeatNumAndRetrieve)
{
    QVector<int> nums = {1, 3, 5};
    m_data->setRepeatNum(nums);
    EXPECT_EQ(m_data->getRepeatNum(), nums);
}

TEST_F(TestJsonData, SetRepeatNumEmpty)
{
    QVector<int> nums = {1, 2};
    m_data->setRepeatNum(nums);
    m_data->setRepeatNum(QVector<int>());
    EXPECT_TRUE(m_data->getRepeatNum().isEmpty());
}

// ---- getDateTime / setDateTime ----

TEST_F(TestJsonData, DateTimeDefaultEmpty)
{
    EXPECT_EQ(m_data->getDateTime().suggestDatetime.size(), 0);
    EXPECT_EQ(m_data->getDateTime().dateTime.size(), 0);
}

TEST_F(TestJsonData, SetDateTimeAndRetrieve)
{
    SemanticsDateTime sdt;
    SuggestDatetimeInfo info;
    info.hasTime = true;
    sdt.suggestDatetime.append(info);
    m_data->setDateTime(sdt);
    EXPECT_EQ(m_data->getDateTime().suggestDatetime.size(), 1);
    EXPECT_TRUE(m_data->getDateTime().suggestDatetime.at(0).hasTime);
}

// ---- getPropertyStatus / setPropertyStatus ----

TEST_F(TestJsonData, PropertyStatusDefaultNone)
{
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::PRO_NONE);
}

TEST_F(TestJsonData, SetPropertyStatusNext)
{
    m_data->setPropertyStatus(JsonData::NEXT);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::NEXT);
}

TEST_F(TestJsonData, SetPropertyStatusLast)
{
    m_data->setPropertyStatus(JsonData::LAST);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::LAST);
}

TEST_F(TestJsonData, SetPropertyStatusAll)
{
    m_data->setPropertyStatus(JsonData::ALL);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::ALL);
}

TEST_F(TestJsonData, SetPropertyStatusThis)
{
    m_data->setPropertyStatus(JsonData::PRO_THIS);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::PRO_THIS);
}

// ---- offset / setOffset ----

TEST_F(TestJsonData, OffsetDefaultMinusOne)
{
    EXPECT_EQ(m_data->offset(), -1);
}

TEST_F(TestJsonData, SetOffsetAndRetrieve)
{
    m_data->setOffset(10);
    EXPECT_EQ(m_data->offset(), 10);
}

TEST_F(TestJsonData, SetOffsetZero)
{
    m_data->setOffset(0);
    EXPECT_EQ(m_data->offset(), 0);
}

TEST_F(TestJsonData, SetOffsetNegative)
{
    m_data->setOffset(-5);
    EXPECT_EQ(m_data->offset(), -5);
}

// ---- getDateTimeInvalid ----

TEST_F(TestJsonData, DateTimeInvalidDefaultFalse)
{
    EXPECT_FALSE(m_data->getDateTimeInvalid());
}

TEST_F(TestJsonData, DateTimeInvalidSetTrueViaJosnResolve)
{
    QJsonObject root;
    QJsonArray slotArray;
    QJsonObject slot;
    slot["name"] = QStringLiteral("datetimeInvalid");
    slotArray.append(slot);
    root["slots"] = slotArray;

    m_data->JosnResolve(root);
    EXPECT_TRUE(m_data->getDateTimeInvalid());
}

// ---- setDefaultValue (via JosnResolve with empty slots) ----

TEST_F(TestJsonData, SetDefaultValueResetsAll)
{
    m_data->setTitleName(QStringLiteral("Title"));
    m_data->setOffset(5);
    m_data->setRepeatStatus(JsonData::EVED);
    m_data->setPropertyStatus(JsonData::NEXT);
    m_data->setSuggestMsg(QStringLiteral("msg"));

    QJsonObject root;
    QJsonArray emptySlots;
    root["slots"] = emptySlots;
    m_data->JosnResolve(root);

    EXPECT_TRUE(m_data->TitleName().isEmpty());
    EXPECT_EQ(m_data->offset(), -1);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::NONE);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::PRO_NONE);
    EXPECT_TRUE(m_data->isVaild());
}

// ---- JosnResolve dispatching ----

TEST_F(TestJsonData, JosnResolveContentSlot)
{
    auto root = buildSingleSlotJson(QStringLiteral("content"), QStringLiteral("Test Title"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->TitleName().toStdString(), "Test Title");
}

TEST_F(TestJsonData, JosnResolveRepeatSlotEveryYear)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("EVERYEAR"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEY);
}

TEST_F(TestJsonData, JosnResolveRepeatSlotEveryDay)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("EVERYDAY"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVED);
}

TEST_F(TestJsonData, JosnResolveRepeatSlotWorkDay)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("WORKDAY"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::WORKD);
}

TEST_F(TestJsonData, JosnResolveRepeatSlotRestDay)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("RESTDAY"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::RESTD);
}

TEST_F(TestJsonData, JosnResolveRepeatSlotWeekend)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("WEEKEND"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::RESTD);
}

TEST_F(TestJsonData, JosnResolveRepeatSlotWeeklyW3)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("W3"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEW);
    EXPECT_EQ(m_data->getRepeatNum().size(), 1);
    EXPECT_EQ(m_data->getRepeatNum().at(0), 3);
}

TEST_F(TestJsonData, JosnResolveRepeatSlotMonthlyM2)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("M2"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEM);
    EXPECT_EQ(m_data->getRepeatNum().size(), 1);
    EXPECT_EQ(m_data->getRepeatNum().at(0), 2);
}

TEST_F(TestJsonData, JosnResolveRepeatSlotWeeklyNoNum)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("W"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEW);
    EXPECT_TRUE(m_data->getRepeatNum().isEmpty());
}

TEST_F(TestJsonData, JosnResolveRepeatSlotMultipleMatches)
{
    auto root = buildSingleSlotJson(QStringLiteral("repeat"), QStringLiteral("W1M2"));
    m_data->JosnResolve(root);
    // Last match sets status — M comes last in "W1M2", so EVEM
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEM);
    EXPECT_EQ(m_data->getRepeatNum().size(), 2);
}

TEST_F(TestJsonData, JosnResolveDatetimeSlot)
{
    // Build a datetime slot with normValue containing a JSON object
    QJsonObject innerObj;
    innerObj["suggestDatetime"] = QStringLiteral("2026-01-15T10:00:00");
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    // datetime slot uses "normValue" field (not "value") per suggestDatetimeResolve
    QJsonObject root;
    QJsonArray slotArray;
    QJsonObject slot;
    slot["name"] = QStringLiteral("datetime");
    slot["normValue"] = normValueStr;
    slotArray.append(slot);
    root["slots"] = slotArray;
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->getDateTime().suggestDatetime.size(), 1);
    EXPECT_TRUE(m_data->getDateTime().suggestDatetime.at(0).hasTime);
}

TEST_F(TestJsonData, JosnResolvePropertySlotNext)
{
    auto root = buildSingleSlotJson(QStringLiteral("property"), QStringLiteral("next"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::NEXT);
}

TEST_F(TestJsonData, JosnResolvePropertySlotLast)
{
    auto root = buildSingleSlotJson(QStringLiteral("property"), QStringLiteral("last"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::LAST);
}

TEST_F(TestJsonData, JosnResolvePropertySlotAll)
{
    auto root = buildSingleSlotJson(QStringLiteral("property"), QStringLiteral("all"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::ALL);
}

TEST_F(TestJsonData, JosnResolvePropertySlotThis)
{
    auto root = buildSingleSlotJson(QStringLiteral("property"), QStringLiteral("this"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::PRO_THIS);
}

TEST_F(TestJsonData, JosnResolvePosRankOffsetSlot)
{
    auto root = buildSingleSlotJson(QStringLiteral("posRank.offset"), QStringLiteral("7"));
    m_data->JosnResolve(root);
    EXPECT_EQ(m_data->offset(), 7);
}

TEST_F(TestJsonData, JosnResolveUnknownSlotCallsJsonObjResolve)
{
    // Unknown slot name — jsonObjResolve is a no-op, should not crash
    auto root = buildSingleSlotJson(QStringLiteral("unknownSlot"), QStringLiteral("value"));
    m_data->JosnResolve(root);
    SUCCEED();
}

TEST_F(TestJsonData, JosnResolveSlotWithoutNameKey)
{
    QJsonObject root;
    QJsonArray slotArray;
    QJsonObject slot;
    slot["value"] = QStringLiteral("no name key");
    slotArray.append(slot);
    root["slots"] = slotArray;
    m_data->JosnResolve(root);
    SUCCEED();
}

TEST_F(TestJsonData, JosnResolveEmptyJson)
{
    QJsonObject root;
    m_data->JosnResolve(root);
    EXPECT_TRUE(m_data->isVaild());
}

TEST_F(TestJsonData, JosnResolveSlotsNotArray)
{
    QJsonObject root;
    root["slots"] = QStringLiteral("not an array");
    m_data->JosnResolve(root);
    SUCCEED();
}

TEST_F(TestJsonData, JosnResolveMultipleSlots)
{
    QJsonObject root;
    QJsonArray slotArray;

    QJsonObject contentSlot;
    contentSlot["name"] = QStringLiteral("content");
    contentSlot["value"] = QStringLiteral("Multi Title");
    slotArray.append(contentSlot);

    QJsonObject offsetSlot;
    offsetSlot["name"] = QStringLiteral("posRank.offset");
    offsetSlot["value"] = QStringLiteral("3");
    slotArray.append(offsetSlot);

    root["slots"] = slotArray;
    m_data->JosnResolve(root);

    EXPECT_EQ(m_data->TitleName().toStdString(), "Multi Title");
    EXPECT_EQ(m_data->offset(), 3);
}

// ---- contentJsonResolve (protected, accessible via -fno-access-control) ----

TEST_F(TestJsonData, ContentJsonResolveSetsTitle)
{
    QJsonObject jsobj;
    jsobj["value"] = QStringLiteral("Direct Content");
    m_data->contentJsonResolve(jsobj);
    EXPECT_EQ(m_data->TitleName().toStdString(), "Direct Content");
}

// ---- repeatJsonResolve (protected) ----

TEST_F(TestJsonData, RepeatJsonResolveEveryYear)
{
    QJsonObject jsobj;
    jsobj["value"] = QStringLiteral("EVERYEAR");
    m_data->repeatJsonResolve(jsobj);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEY);
}

TEST_F(TestJsonData, RepeatJsonResolveWeeklyWithNum)
{
    QJsonObject jsobj;
    jsobj["value"] = QStringLiteral("W5");
    m_data->repeatJsonResolve(jsobj);
    EXPECT_EQ(m_data->getRepeatStatus(), JsonData::EVEW);
    EXPECT_EQ(m_data->getRepeatNum().at(0), 5);
}

// ---- propertyJsonResolve (protected) ----

TEST_F(TestJsonData, PropertyJsonResolveNext)
{
    QJsonObject jsobj;
    jsobj["value"] = QStringLiteral("next");
    m_data->propertyJsonResolve(jsobj);
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::NEXT);
}

TEST_F(TestJsonData, PropertyJsonResolveUnknownValue)
{
    QJsonObject jsobj;
    jsobj["value"] = QStringLiteral("unknown");
    m_data->propertyJsonResolve(jsobj);
    // Property status should remain unchanged (default PRO_NONE)
    EXPECT_EQ(m_data->getPropertyStatus(), JsonData::PRO_NONE);
}

// ---- posRankOffsetResolve (protected) ----

TEST_F(TestJsonData, PosRankOffsetResolveSetsOffset)
{
    QJsonObject jsobj;
    jsobj["value"] = QStringLiteral("42");
    m_data->posRankOffsetResolve(jsobj);
    EXPECT_EQ(m_data->offset(), 42);
}

// ---- datetimeJsonResolve (protected) ----

TEST_F(TestJsonData, DatetimeJsonResolveSetsDateTime)
{
    QJsonObject innerObj;
    innerObj["suggestDatetime"] = QStringLiteral("2026-06-01T09:30:00");
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QJsonObject jsobj;
    jsobj["normValue"] = normValueStr;
    m_data->datetimeJsonResolve(jsobj);

    EXPECT_EQ(m_data->getDateTime().suggestDatetime.size(), 1);
}

// ---- suggestDatetimeResolve (protected) ----

TEST_F(TestJsonData, SuggestDatetimeResolveSingleSuggestDatetime)
{
    QJsonObject innerObj;
    innerObj["suggestDatetime"] = QStringLiteral("2026-03-20T14:00:00");
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QJsonObject jsobj;
    jsobj["normValue"] = normValueStr;

    SemanticsDateTime result = m_data->suggestDatetimeResolve(jsobj);
    EXPECT_EQ(result.suggestDatetime.size(), 1);
    EXPECT_TRUE(result.suggestDatetime.at(0).hasTime);
}

TEST_F(TestJsonData, SuggestDatetimeResolveMultipleSuggestDatetimeWithSlash)
{
    QJsonObject innerObj;
    innerObj["suggestDatetime"] = QStringLiteral("2026-03-20T14:00:00/2026-04-20T15:00:00");
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QJsonObject jsobj;
    jsobj["normValue"] = normValueStr;

    SemanticsDateTime result = m_data->suggestDatetimeResolve(jsobj);
    EXPECT_EQ(result.suggestDatetime.size(), 2);
}

TEST_F(TestJsonData, SuggestDatetimeResolveWithFuzzyDatetime)
{
    QJsonObject innerObj;
    innerObj["datetime"] = QStringLiteral("2026-03-20T14:00:00");
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QJsonObject jsobj;
    jsobj["normValue"] = normValueStr;

    SemanticsDateTime result = m_data->suggestDatetimeResolve(jsobj);
    EXPECT_EQ(result.dateTime.size(), 1);
    EXPECT_TRUE(result.dateTime.at(0).hasTime);
}

TEST_F(TestJsonData, SuggestDatetimeResolveInvalidJsonNormValue)
{
    QJsonObject jsobj;
    jsobj["normValue"] = QStringLiteral("not a valid json");
    SemanticsDateTime result = m_data->suggestDatetimeResolve(jsobj);
    EXPECT_EQ(result.suggestDatetime.size(), 0);
    EXPECT_EQ(result.dateTime.size(), 0);
}

TEST_F(TestJsonData, SuggestDatetimeResolveEmptyNormValue)
{
    QJsonObject innerObj;
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QJsonObject jsobj;
    jsobj["normValue"] = normValueStr;

    SemanticsDateTime result = m_data->suggestDatetimeResolve(jsobj);
    EXPECT_EQ(result.suggestDatetime.size(), 0);
}

TEST_F(TestJsonData, SuggestDatetimeResolveFuzzyDatetimeWithSlash)
{
    QJsonObject innerObj;
    innerObj["datetime"] = QStringLiteral("2026-03-20T14:00:00/2026-03-21T15:00:00");
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QJsonObject jsobj;
    jsobj["normValue"] = normValueStr;

    SemanticsDateTime result = m_data->suggestDatetimeResolve(jsobj);
    EXPECT_EQ(result.dateTime.size(), 2);
}

TEST_F(TestJsonData, SuggestDatetimeResolveSuggestDatetimeWithEmptyEntries)
{
    QJsonObject innerObj;
    innerObj["suggestDatetime"] = QStringLiteral("/2026-03-20T14:00:00/");
    QJsonDocument doc(innerObj);
    QString normValueStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    QJsonObject jsobj;
    jsobj["normValue"] = normValueStr;

    SemanticsDateTime result = m_data->suggestDatetimeResolve(jsobj);
    // Empty entries are skipped, only one valid entry
    EXPECT_EQ(result.suggestDatetime.size(), 1);
}

// ---- resolveNormValue (protected) ----

TEST_F(TestJsonData, ResolveNormValueWithTime)
{
    SuggestDatetimeInfo info = m_data->resolveNormValue(QStringLiteral("2026-01-15T10:00:00"));
    EXPECT_TRUE(info.hasTime);
    EXPECT_FALSE(info.strDateTime.isEmpty());
}

TEST_F(TestJsonData, ResolveNormValueWithoutTime)
{
    SuggestDatetimeInfo info = m_data->resolveNormValue(QStringLiteral("2026-01-15"));
    EXPECT_FALSE(info.hasTime);
}

// ---- resolveDateTimeValeu (protected) ----

TEST_F(TestJsonData, ResolveDateTimeValeuDateOnly)
{
    DateTimeInfo info = m_data->resolveDateTimeValeu(QStringLiteral("2026-01-15"));
    EXPECT_TRUE(info.hasDate);
    EXPECT_FALSE(info.hasTime);
}

TEST_F(TestJsonData, ResolveDateTimeValeuDateTime)
{
    DateTimeInfo info = m_data->resolveDateTimeValeu(QStringLiteral("2026-01-15T10:30:00"));
    EXPECT_TRUE(info.hasDate);
    EXPECT_TRUE(info.hasTime);
}

TEST_F(TestJsonData, ResolveDateTimeValeuTimeOnly)
{
    DateTimeInfo info = m_data->resolveDateTimeValeu(QStringLiteral("T10:30:00"));
    EXPECT_FALSE(info.hasDate);
    EXPECT_TRUE(info.hasTime);
}

// ---- strTransform (protected) ----

TEST_F(TestJsonData, StrTransformDateOnly)
{
    QString result = m_data->strTransform(QStringLiteral("2026-01-15"));
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains(QStringLiteral("年")));
}

TEST_F(TestJsonData, StrTransformDateTime)
{
    QString result = m_data->strTransform(QStringLiteral("2026-01-15T10:00:00"));
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains(QStringLiteral("10:00:00")));
}

TEST_F(TestJsonData, StrTransformYearOnly)
{
    QString result = m_data->strTransform(QStringLiteral("2026"));
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.contains(QStringLiteral("年")));
}

// ---- strDateTransform (protected) ----

TEST_F(TestJsonData, StrDateTransformYearOnly)
{
    QString result = m_data->strDateTransform(QStringLiteral("2026"));
    EXPECT_EQ(result.toStdString(), "2026年");
}

TEST_F(TestJsonData, StrDateTransformYearMonth)
{
    QString result = m_data->strDateTransform(QStringLiteral("2026-03"));
    EXPECT_EQ(result.toStdString(), "2026年03月");
}

TEST_F(TestJsonData, StrDateTransformYearMonthDay)
{
    QString result = m_data->strDateTransform(QStringLiteral("2026-03-20"));
    EXPECT_EQ(result.toStdString(), "2026年03月20日");
}

TEST_F(TestJsonData, StrDateTransformEmpty)
{
    // Empty string splits into single empty element → returns "" + "年"
    QString result = m_data->strDateTransform(QStringLiteral(""));
    EXPECT_EQ(result.toStdString(), "年");
}

// ---- jsonObjResolve (protected, no-op) ----

TEST_F(TestJsonData, JsonObjResolveIsNoOp)
{
    QJsonObject jsobj;
    jsobj["anything"] = QStringLiteral("value");
    m_data->jsonObjResolve(jsobj);
    // Should not crash and should not change state
    EXPECT_TRUE(m_data->isVaild());
}

// ---- setDateTimeStatus (protected, no getter — verify through setDefaultValue) ----

TEST_F(TestJsonData, SetDateTimeStatusViaProtectedAccess)
{
    m_data->setDateTimeStatus(JsonData::ALLHAS);
    SUCCEED();
}
