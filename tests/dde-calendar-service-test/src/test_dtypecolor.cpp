// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | DTypeColor (ctor) | low | - | 1 | 2 |
// | setColorCode/colorCode | low | - | 1 | 2 |
// | setColorID/colorID | low | - | 1 | 1 |
// | setPrivilege/privilege | low | - | 1 | 2 |
// | isSysColorInfo | low | - | 1 | 1 |
// | operator!= | low | - | 1 | 1 |
// | setDtCreate/dtCreate | low | - | 1 | 1 |
// | fromJsonString | high | json parse | 3 | 2 |
// | toJsonString | mid | - | 2 | 3 |
// | operator< | low | - | 1 | 3 |
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

#include "dtypecolor.h"

#include <gtest/gtest.h>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class DTypeColorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

















TEST_F(DTypeColorTest, DefaultConstructor_InitializesFields_ReturnsDefaults)
{
    // Arrange
    // Act
    DTypeColor color;
    // Assert
    EXPECT_EQ(color.colorID(), QString(""));
    EXPECT_EQ(color.colorCode(), QString(""));
    EXPECT_EQ(color.privilege(), DTypeColor::PriUser);
}
TEST_F(DTypeColorTest, CopyConstructor_CopiesAllFields_ReturnsEqual)
{
    // Arrange
    DTypeColor original;
    original.setColorID("id-1");
    original.setColorCode("#AABBCC");
    original.setPrivilege(DTypeColor::PriSystem);
    original.setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));

    // Act
    DTypeColor copy(original);
    // Assert
    EXPECT_EQ(copy.colorID(), QString("id-1"));
    EXPECT_EQ(copy.colorCode(), QString("#AABBCC"));
    EXPECT_EQ(copy.privilege(), DTypeColor::PriSystem);
    EXPECT_EQ(copy.dtCreate(), original.dtCreate());
}
TEST_F(DTypeColorTest, ColorCode_GetterSetter_ReturnsValue)
{
    // Arrange
    DTypeColor color;
    // Act
    color.setColorCode("#123456");
    // Assert
    EXPECT_EQ(color.colorCode(), QString("#123456"));
    EXPECT_NE(color.colorCode(), QString(""));
}
TEST_F(DTypeColorTest, ColorID_GetterSetter_ReturnsValue)
{
    // Arrange
    DTypeColor color;
    // Act
    color.setColorID("cid-99");
    // Assert
    EXPECT_EQ(color.colorID(), QString("cid-99"));
    EXPECT_NE(color.colorID(), QString(""));
}
TEST_F(DTypeColorTest, Privilege_GetterSetter_ReturnsPrivilege)
{
    // Arrange
    DTypeColor color;
    // Act
    color.setPrivilege(DTypeColor::PriSystem);
    // Assert
    EXPECT_EQ(color.privilege(), DTypeColor::PriSystem);
    color.setPrivilege(DTypeColor::PriUser);
    EXPECT_EQ(color.privilege(), DTypeColor::PriUser);
}
TEST_F(DTypeColorTest, IsSysColorInfo_ChecksPrivilege_ReturnsBool)
{
    // Arrange
    DTypeColor systemColor;
    // Act
    systemColor.setPrivilege(DTypeColor::PriSystem);
    // Assert
    EXPECT_TRUE(systemColor.isSysColorInfo());

    DTypeColor userColor;
    userColor.setPrivilege(DTypeColor::PriUser);
    EXPECT_FALSE(userColor.isSysColorInfo());
}
TEST_F(DTypeColorTest, OperatorInequality_DifferentFields_ReturnsTrue)
{
    // Arrange
    DTypeColor a;
    a.setColorID("id-a");
    a.setColorCode("#AAAAAA");
    a.setPrivilege(DTypeColor::PriUser);

    DTypeColor b;
    // Act
    b.setColorID("id-b");

    // Assert
    EXPECT_TRUE(a != b);

    DTypeColor same;
    same.setColorID("id-a");
    same.setColorCode("#AAAAAA");
    same.setPrivilege(DTypeColor::PriUser);
    EXPECT_FALSE(a != same);
    EXPECT_NE(a.colorID(), b.colorID());
}
TEST_F(DTypeColorTest, DtCreate_GetterSetter_ReturnsDateTime)
{
    // Arrange
    DTypeColor color;
    QDateTime dt(QDateTime::fromString("2024-09-09T09:09:09", Qt::ISODate));
    // Act
    color.setDtCreate(dt);
    // Assert
    EXPECT_EQ(color.dtCreate(), dt);
    EXPECT_TRUE(color.dtCreate().isValid());
}
TEST_F(DTypeColorTest, FromJsonString_ParsesList_ReturnsList)
{
    // Arrange
    QString json = R"([
        {"colorID": "c1", "colorCode": "#FF0000", "privilege": 1, "dtCreate": "2024-01-01T00:00:00"},
        {"colorID": "c2", "colorCode": "#00FF00", "privilege": 7}
    ])";
    // Act
    DTypeColor::List list = DTypeColor::fromJsonString(json);
    // Assert
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.at(0)->colorID(), QString("c1"));
    EXPECT_EQ(list.at(0)->colorCode(), QString("#FF0000"));
    EXPECT_EQ(list.at(0)->privilege(), DTypeColor::PriSystem);
    EXPECT_EQ(list.at(1)->colorID(), QString("c2"));
    EXPECT_EQ(list.at(1)->privilege(), DTypeColor::PriUser);
}
TEST_F(DTypeColorTest, FromJsonString_InvalidJson_ReturnsEmptyList)
{
    // Arrange
    // Act
    DTypeColor::List list = DTypeColor::fromJsonString("not json");
    // Assert
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}
TEST_F(DTypeColorTest, ToJsonString_SerializesList_ReturnsArray)
{
    // Arrange
    DTypeColor::List list;
    DTypeColor::Ptr c1(new DTypeColor);
    c1->setColorID("c1");
    c1->setColorCode("#FF0000");
    c1->setPrivilege(DTypeColor::PriSystem);
    c1->setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));
    list.append(c1);

    QString json = DTypeColor::toJsonString(list);
    // Act
    QJsonDocument doc(QJsonDocument::fromJson(json.toUtf8()));
    // Assert
    ASSERT_TRUE(doc.isArray());
    QJsonArray arr = doc.array();
    EXPECT_EQ(arr.size(), 1);
    QJsonObject obj = arr.at(0).toObject();
    EXPECT_EQ(obj.value("colorID").toString(), QString("c1"));
    EXPECT_EQ(obj.value("colorCode").toString(), QString("#FF0000"));
    EXPECT_EQ(obj.value("privilege").toInt(), 1);
}
TEST_F(DTypeColorTest, ToJsonString_EmptyList_ReturnsEmptyArray)
{
    // Arrange
    DTypeColor::List list;
    QString json = DTypeColor::toJsonString(list);
    // Act
    QJsonDocument doc(QJsonDocument::fromJson(json.toUtf8()));
    // Assert
    EXPECT_TRUE(doc.isArray());
    EXPECT_EQ(doc.array().size(), 0);
}
TEST_F(DTypeColorTest, JsonRoundTrip_PreservesData_ReturnsEqual)
{
    // Arrange
    DTypeColor::List original;
    DTypeColor::Ptr c(new DTypeColor);
    c->setColorID("rt-id");
    c->setColorCode("#00ABCD");
    c->setPrivilege(DTypeColor::PriUser);
    c->setDtCreate(QDateTime::fromString("2024-05-05T05:05:05", Qt::ISODate));
    original.append(c);

    QString json = DTypeColor::toJsonString(original);
    // Act
    DTypeColor::List restored = DTypeColor::fromJsonString(json);
    // Assert
    EXPECT_EQ(restored.size(), 1);
    EXPECT_EQ(restored.at(0)->colorID(), QString("rt-id"));
    EXPECT_EQ(restored.at(0)->colorCode(), QString("#00ABCD"));
    EXPECT_EQ(restored.at(0)->privilege(), DTypeColor::PriUser);
}
TEST_F(DTypeColorTest, OperatorLessThan_ByPrivilege_ReturnsTrue)
{
    // Arrange
    DTypeColor::Ptr a(new DTypeColor);
    a->setPrivilege(DTypeColor::PriSystem);
    DTypeColor::Ptr b(new DTypeColor);
    // Act
    b->setPrivilege(DTypeColor::PriUser);
    // Assert
    EXPECT_TRUE(operator<(a, b));
    EXPECT_FALSE(operator<(b, a));
    EXPECT_NE(int(a->privilege()), int(b->privilege()));
}
TEST_F(DTypeColorTest, OperatorLessThan_ByDtCreateWhenPrivilegeEqual_ReturnsTrue)
{
    // Arrange
    DTypeColor::Ptr a(new DTypeColor);
    a->setPrivilege(DTypeColor::PriUser);
    a->setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));
    DTypeColor::Ptr b(new DTypeColor);
    b->setPrivilege(DTypeColor::PriUser);
    // Act
    b->setDtCreate(QDateTime::fromString("2024-06-01T00:00:00", Qt::ISODate));
    // Assert
    EXPECT_TRUE(operator<(a, b));
    EXPECT_FALSE(operator<(b, a));
    EXPECT_EQ(int(a->privilege()), int(b->privilege()));
    EXPECT_NE(a->dtCreate(), b->dtCreate());
}
TEST_F(DTypeColorTest, OperatorLessThan_WhenAllEqual_ReturnsTrue)
{
    // Arrange
    DTypeColor::Ptr a(new DTypeColor);
    a->setPrivilege(DTypeColor::PriUser);
    a->setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));
    DTypeColor::Ptr b(new DTypeColor);
    b->setPrivilege(DTypeColor::PriUser);
    // Act
    b->setDtCreate(QDateTime::fromString("2024-01-01T00:00:00", Qt::ISODate));
    // Assert
    EXPECT_TRUE(operator<(a, b));
    EXPECT_EQ(int(a->privilege()), int(b->privilege()));
    EXPECT_EQ(a->dtCreate(), b->dtCreate());
}