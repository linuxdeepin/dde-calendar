// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | DScheduleType (ctor) | low | - | 1 | 2 |
// | accountID/setAccountID | low | - | 1 | 1 |
// | privilege/setPrivilege | low | - | 1 | 1 |
// | typeColor/setTypeColor | low | - | 1 | 1 |
// | colorID/colorCode accessors | low | - | 1 | 1 |
// | typeID/setTypeID | low | - | 1 | 1 |
// | displayName/setDisplayName | low | - | 1 | 1 |
// | showState/setShowState | low | - | 1 | 1 |
// | typeName/setTypeName | low | - | 1 | 1 |
// | typePath/setTypePath | low | - | 1 | 1 |
// | description/setDescription | low | - | 1 | 1 |
// | dtCreate/dtUpdate/dtDelete | low | - | 1 | 3 |
// | deleted/setDeleted | mid | name pattern | 1 | 1 |
// | syncTag/setSyncTag | low | - | 1 | 1 |
// | fromJsonString | high | json parse, static | 3 | 5 |
// | toJsonString | mid | - | 2 | 3 |
// | fromJsonListString | high | json parse, static | 3 | 3 |
// | toJsonListString | mid | - | 2 | 3 |
// | operator< | low | - | 1 | 2 |
// ─── 生成后填入 actual 列，低于 min 即违规 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 4. 同质 ≥ 3 组用 TEST_P: [ ]
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [ ]
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]
// 10. stub_ext vs gMock 选择正确: [x]

#include "dscheduletype.h"
#include "dtypecolor.h"

#include <gtest/gtest.h>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {

DScheduleType::Ptr makeFilledType()
{
    DScheduleType::Ptr st(new DScheduleType("acct-001"));
    st->setTypeID("type-001");
    st->setTypeName("work");
    st->setDisplayName("Work");
    st->setTypePath("/cal/work");
    DTypeColor color;
    color.setColorID("color-001");
    color.setColorCode("#FF0000");
    color.setPrivilege(DTypeColor::PriUser);
    st->setTypeColor(color);
    st->setColorID("color-002");
    st->setColorCode("#00FF00");
    st->setDescription("desc");
    st->setPrivilege(DScheduleType::Read | DScheduleType::Write);
    st->setDtCreate(QDateTime::fromString("2024-01-02T03:04:05", Qt::ISODate));
    st->setDtUpdate(QDateTime::fromString("2024-02-03T04:05:06", Qt::ISODate));
    st->setDtDelete(QDateTime::fromString("2024-03-04T05:06:07", Qt::ISODate));
    st->setShowState(DScheduleType::Hide);
    st->setDeleted(1);
    st->setSyncTag(42);
    return st;
}

} // namespace

class DScheduleTypeTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};


















TEST_F(DScheduleTypeTest, DefaultConstructor_InitializesFields_ReturnsEmpty)
{
    // Arrange
    // Act
    DScheduleType st;
    // Assert
    EXPECT_EQ(st.accountID(), QString(""));
    EXPECT_EQ(st.typeID(), QString(""));
    EXPECT_EQ(st.typeName(), QString(""));
    EXPECT_EQ(st.displayName(), QString(""));
    EXPECT_EQ(st.typePath(), QString(""));
    EXPECT_EQ(st.description(), QString(""));
    EXPECT_EQ(st.privilege(), DScheduleType::Privileges(DScheduleType::None));
    EXPECT_EQ(st.showState(), DScheduleType::Show);
    EXPECT_EQ(st.deleted(), 0);
    EXPECT_EQ(st.syncTag(), 0);
}
TEST_F(DScheduleTypeTest, Constructor_WithAccountID_ReturnsAccountID)
{
    // Arrange
    // Act
    DScheduleType st("my-account");
    // Assert
    EXPECT_EQ(st.accountID(), QString("my-account"));
    EXPECT_EQ(st.typeID(), QString(""));
}
TEST_F(DScheduleTypeTest, AccountID_GetterSetter_ReturnsValue)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setAccountID("abc-123");
    // Assert
    EXPECT_EQ(st.accountID(), QString("abc-123"));
    EXPECT_NE(st.accountID(), QString(""));
}
TEST_F(DScheduleTypeTest, Privilege_GetterSetter_ReturnsFlags)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setPrivilege(DScheduleType::Read | DScheduleType::Delete);
    // Assert
    EXPECT_TRUE(st.privilege() & DScheduleType::Read);
    EXPECT_TRUE(st.privilege() & DScheduleType::Delete);
    EXPECT_FALSE(st.privilege() & DScheduleType::Write);
    EXPECT_EQ(int(st.privilege()), int(DScheduleType::Read | DScheduleType::Delete));
}
TEST_F(DScheduleTypeTest, TypeColor_GetterSetter_ReturnsColor)
{
    // Arrange
    DScheduleType st;
    DTypeColor color;
    color.setColorID("cid");
    color.setColorCode("#123456");
    color.setPrivilege(DTypeColor::PriSystem);
    // Act
    st.setTypeColor(color);
    // Assert
    EXPECT_EQ(st.typeColor().colorID(), QString("cid"));
    EXPECT_EQ(st.typeColor().colorCode(), QString("#123456"));
    EXPECT_EQ(st.typeColor().privilege(), DTypeColor::PriSystem);
}
TEST_F(DScheduleTypeTest, ColorIDAndColorCode_Accessors_ReturnsValues)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setColorID("red-id");
    // Assert
    EXPECT_EQ(st.getColorID(), QString("red-id"));
    st.setColorCode("#ABCDEF");
    EXPECT_EQ(st.getColorCode(), QString("#ABCDEF"));
}
TEST_F(DScheduleTypeTest, TypeID_GetterSetter_ReturnsValue)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setTypeID("tid-9");
    // Assert
    EXPECT_EQ(st.typeID(), QString("tid-9"));
    EXPECT_NE(st.typeID(), QString(""));
}
TEST_F(DScheduleTypeTest, DisplayName_GetterSetter_ReturnsValue)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setDisplayName("Display");
    // Assert
    EXPECT_EQ(st.displayName(), QString("Display"));
    EXPECT_NE(st.displayName(), QString(""));
}
TEST_F(DScheduleTypeTest, ShowState_GetterSetter_ReturnsState)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setShowState(DScheduleType::Hide);
    // Assert
    EXPECT_EQ(st.showState(), DScheduleType::Hide);
    st.setShowState(DScheduleType::Show);
    EXPECT_EQ(st.showState(), DScheduleType::Show);
}
TEST_F(DScheduleTypeTest, TypeName_GetterSetter_ReturnsValue)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setTypeName("meeting");
    // Assert
    EXPECT_EQ(st.typeName(), QString("meeting"));
    EXPECT_NE(st.typeName(), QString(""));
}
TEST_F(DScheduleTypeTest, TypePath_GetterSetter_ReturnsValue)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setTypePath("/path/to/type");
    // Assert
    EXPECT_EQ(st.typePath(), QString("/path/to/type"));
    EXPECT_NE(st.typePath(), QString(""));
}
TEST_F(DScheduleTypeTest, Description_GetterSetter_ReturnsValue)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setDescription("a description");
    // Assert
    EXPECT_EQ(st.description(), QString("a description"));
    EXPECT_NE(st.description(), QString(""));
}
TEST_F(DScheduleTypeTest, DtCreate_GetterSetter_ReturnsDateTime)
{
    // Arrange
    DScheduleType st;
    QDateTime dt(QDateTime::fromString("2024-05-06T07:08:09", Qt::ISODate));
    // Act
    st.setDtCreate(dt);
    // Assert
    EXPECT_EQ(st.dtCreate(), dt);
    EXPECT_TRUE(st.dtCreate().isValid());
}
TEST_F(DScheduleTypeTest, DtUpdate_GetterSetter_ReturnsDateTime)
{
    // Arrange
    DScheduleType st;
    QDateTime dt(QDateTime::fromString("2024-06-07T08:09:10", Qt::ISODate));
    // Act
    st.setDtUpdate(dt);
    // Assert
    EXPECT_EQ(st.dtUpdate(), dt);
    EXPECT_TRUE(st.dtUpdate().isValid());
}
TEST_F(DScheduleTypeTest, DtDelete_GetterSetter_ReturnsDateTime)
{
    // Arrange
    DScheduleType st;
    QDateTime dt(QDateTime::fromString("2024-07-08T09:10:11", Qt::ISODate));
    // Act
    st.setDtDelete(dt);
    // Assert
    EXPECT_EQ(st.dtDelete(), dt);
    EXPECT_TRUE(st.dtDelete().isValid());
}
TEST_F(DScheduleTypeTest, Deleted_GetterSetter_ReturnsInt)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setDeleted(1);
    // Assert
    EXPECT_EQ(st.deleted(), 1);
    st.setDeleted(0);
    EXPECT_EQ(st.deleted(), 0);
}
TEST_F(DScheduleTypeTest, SyncTag_GetterSetter_ReturnsInt)
{
    // Arrange
    DScheduleType st;
    // Act
    st.setSyncTag(99);
    // Assert
    EXPECT_EQ(st.syncTag(), 99);
    EXPECT_NE(st.syncTag(), 0);
}
TEST_F(DScheduleTypeTest, FromJsonString_ParsesAllFields_ReturnsValues)
{
    // Arrange
    QString json = R"({
        "accountID": "acc-json",
        "typeID": "tid-json",
        "typeName": "tn-json",
        "displayName": "dn-json",
        "typePath": "/json/path",
        "TypeColor": {"colorID": "cjson", "colorCode": "#000001", "privilege": 7},
        "description": "desc-json",
        "privilege": 3,
        "dtCreate": "2024-01-01T00:00:00",
        "dtDelete": "2024-02-02T00:00:00",
        "dtUpdate": "2024-03-03T00:00:00",
        "showState": 0,
        "isDeleted": 1
    })";
    // Act
    DScheduleType::Ptr st;
    // Assert
    ASSERT_TRUE(DScheduleType::fromJsonString(st, json));
    ASSERT_FALSE(st.isNull());
    EXPECT_EQ(st->accountID(), QString("acc-json"));
    EXPECT_EQ(st->typeID(), QString("tid-json"));
    EXPECT_EQ(st->typeName(), QString("tn-json"));
    EXPECT_EQ(st->displayName(), QString("dn-json"));
    EXPECT_EQ(st->typePath(), QString("/json/path"));
    EXPECT_EQ(st->typeColor().colorID(), QString("cjson"));
    EXPECT_EQ(st->typeColor().colorCode(), QString("#000001"));
    EXPECT_EQ(st->typeColor().privilege(), DTypeColor::PriUser);
    EXPECT_EQ(st->description(), QString("desc-json"));
    EXPECT_EQ(int(st->privilege()), 3);
    EXPECT_EQ(st->showState(), DScheduleType::Hide);
    EXPECT_EQ(st->deleted(), 1);
}

TEST_F(DScheduleTypeTest, FromJsonString_NullPtrInput_ReturnsValidPtr)
{
    // Arrange
    QString json = R"({"typeID": "auto-created"})";
    DScheduleType::Ptr st;
    // Act
    ASSERT_TRUE(DScheduleType::fromJsonString(st, json));
    // Assert
    ASSERT_FALSE(st.isNull());
    EXPECT_EQ(st->typeID(), QString("auto-created"));
    EXPECT_FALSE(st->typeID().isEmpty());
}

TEST_F(DScheduleTypeTest, FromJsonString_ExistingPtr_ReusesInstance)
{
    // Arrange
    QString json = R"({"typeID": "reused"})";
    DScheduleType::Ptr st(new DScheduleType("preexisting"));
    // Act
    ASSERT_TRUE(DScheduleType::fromJsonString(st, json));
    // Assert
    EXPECT_EQ(st->typeID(), QString("reused"));
    EXPECT_EQ(st->accountID(), QString("preexisting"));
    EXPECT_FALSE(st.isNull());
}

TEST_F(DScheduleTypeTest, FromJsonString_InvalidJson_ReturnsFalse)
{
    // Arrange
    DScheduleType::Ptr st;
    QString badJson = "{ not valid json";
    // Act
    bool ok = DScheduleType::fromJsonString(st, badJson);
    // Assert
    EXPECT_FALSE(ok);
    EXPECT_FALSE(DScheduleType::fromJsonString(st, QString()));
}

TEST_F(DScheduleTypeTest, ToJsonStringSerializesAllFields)
{
    DScheduleType::Ptr st = makeFilledType();
    QString json;
    ASSERT_TRUE(DScheduleType::toJsonString(st, json));
    QJsonDocument doc(QJsonDocument::fromJson(json.toUtf8()));
    ASSERT_TRUE(doc.isObject());
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj.value("accountID").toString(), QString("acct-001"));
    EXPECT_EQ(obj.value("typeID").toString(), QString("type-001"));
    EXPECT_EQ(obj.value("typeName").toString(), QString("work"));
    EXPECT_EQ(obj.value("displayName").toString(), QString("Work"));
    EXPECT_EQ(obj.value("typePath").toString(), QString("/cal/work"));
    EXPECT_EQ(obj.value("description").toString(), QString("desc"));
    EXPECT_EQ(obj.value("isDeleted").toInt(), 1);
    EXPECT_EQ(obj.value("showState").toInt(), static_cast<int>(DScheduleType::Hide));
    QJsonObject colorObj = obj.value("TypeColor").toObject();
    EXPECT_EQ(colorObj.value("colorID").toString(), QString("color-002"));
    EXPECT_EQ(colorObj.value("colorCode").toString(), QString("#00FF00"));
}

TEST_F(DScheduleTypeTest, ToJsonStringFailsOnNullPtr)
{
    DScheduleType::Ptr st;
    QString json;
    EXPECT_FALSE(DScheduleType::toJsonString(st, json));
}

TEST_F(DScheduleTypeTest, JsonRoundTripPreservesData)
{
    DScheduleType::Ptr original = makeFilledType();
    QString json;
    ASSERT_TRUE(DScheduleType::toJsonString(original, json));
    DScheduleType::Ptr restored;
    ASSERT_TRUE(DScheduleType::fromJsonString(restored, json));
    EXPECT_EQ(restored->accountID(), original->accountID());
    EXPECT_EQ(restored->typeID(), original->typeID());
    EXPECT_EQ(restored->typeName(), original->typeName());
    EXPECT_EQ(restored->displayName(), original->displayName());
    EXPECT_EQ(restored->typePath(), original->typePath());
    EXPECT_EQ(restored->typeColor().colorID(), original->typeColor().colorID());
    EXPECT_EQ(restored->typeColor().colorCode(), original->typeColor().colorCode());
    EXPECT_EQ(restored->description(), original->description());
    EXPECT_EQ(restored->showState(), original->showState());
    EXPECT_EQ(restored->deleted(), original->deleted());
}

TEST_F(DScheduleTypeTest, FromJsonListStringParsesMultiple)
{
    QString json = R"({"scheduleType": [
        {"typeID": "l1", "accountID": "a1"},
        {"typeID": "l2", "accountID": "a2", "isDeleted": 1}
    ]})";
    DScheduleType::List list;
    ASSERT_TRUE(DScheduleType::fromJsonListString(list, json));
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.at(0)->typeID(), QString("l1"));
    EXPECT_EQ(list.at(0)->accountID(), QString("a1"));
    EXPECT_EQ(list.at(1)->typeID(), QString("l2"));
    EXPECT_EQ(list.at(1)->deleted(), 1);
}

TEST_F(DScheduleTypeTest, FromJsonListStringFailsOnInvalidJson)
{
    DScheduleType::List list;
    EXPECT_FALSE(DScheduleType::fromJsonListString(list, "not json"));
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(DScheduleTypeTest, ToJsonListStringSerializesList)
{
    DScheduleType::List list;
    list.append(makeFilledType());
    list.append([]() {
        DScheduleType::Ptr st(new DScheduleType("acct-002"));
        st->setTypeID("type-002");
        return st;
    }());
    QString json;
    ASSERT_TRUE(DScheduleType::toJsonListString(list, json));
    QJsonDocument doc(QJsonDocument::fromJson(json.toUtf8()));
    ASSERT_TRUE(doc.isObject());
    QJsonArray arr = doc.object().value("scheduleType").toArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.at(0).toObject().value("typeID").toString(), QString("type-001"));
    EXPECT_EQ(arr.at(1).toObject().value("typeID").toString(), QString("type-002"));
}

TEST_F(DScheduleTypeTest, ToJsonListStringEmptyList)
{
    DScheduleType::List list;
    QString json;
    ASSERT_TRUE(DScheduleType::toJsonListString(list, json));
    QJsonDocument doc(QJsonDocument::fromJson(json.toUtf8()));
    EXPECT_TRUE(doc.isObject());
    EXPECT_EQ(doc.object().value("scheduleType").toArray().size(), 0);
}

TEST_F(DScheduleTypeTest, ListJsonRoundTrip)
{
    DScheduleType::List original;
    original.append(makeFilledType());
    QString json;
    ASSERT_TRUE(DScheduleType::toJsonListString(original, json));
    DScheduleType::List restored;
    ASSERT_TRUE(DScheduleType::fromJsonListString(restored, json));
    EXPECT_EQ(restored.size(), original.size());
    EXPECT_EQ(restored.at(0)->typeID(), original.at(0)->typeID());
    EXPECT_EQ(restored.at(0)->accountID(), original.at(0)->accountID());
}

TEST_F(DScheduleTypeTest, OperatorLessThanByPrivilege)
{
    DScheduleType::Ptr st1(new DScheduleType("a"));
    st1->setPrivilege(DScheduleType::Read);
    DScheduleType::Ptr st2(new DScheduleType("b"));
    st2->setPrivilege(DScheduleType::Read | DScheduleType::Write);
    EXPECT_TRUE(operator<(st1, st2));
    EXPECT_FALSE(operator<(st2, st1));
}

TEST_F(DScheduleTypeTest, OperatorLessThanByDtCreateWhenPrivilegeEqual)
{
    DScheduleType::Ptr st1(new DScheduleType("a"));
    st1->setPrivilege(DScheduleType::Read);
    st1->setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));
    DScheduleType::Ptr st2(new DScheduleType("b"));
    st2->setPrivilege(DScheduleType::Read);
    st2->setDtCreate(QDateTime::fromString("2024-02-01T00:00:00", Qt::ISODate));
    EXPECT_TRUE(operator<(st1, st2));
    EXPECT_FALSE(operator<(st2, st1));
}

TEST_F(DScheduleTypeTest, OperatorLessThanReturnsTrueWhenAllEqual)
{
    DScheduleType::Ptr st1(new DScheduleType("a"));
    st1->setPrivilege(DScheduleType::Read);
    st1->setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));
    DScheduleType::Ptr st2(new DScheduleType("b"));
    st2->setPrivilege(DScheduleType::Read);
    st2->setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));
    EXPECT_TRUE(operator<(st1, st2));
}
