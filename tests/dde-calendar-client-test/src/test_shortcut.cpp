// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_shortcut.h"

#include "shortcut.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "gtest/gtest.h"

test_Shortcut::test_Shortcut(QObject *parent)
    : QObject(parent)
{
}

// 构造函数应构建恰好 3 个快捷键分组，每组都带非空的 groupItems，
// 且每个 item 都包含 name/value 字段；toStr() 应回吐合法 JSON。
TEST(shorcutTest, toStrReturnsValidJsonWithThreeShortcutGroups)
{
    Shortcut shortcut;
    QString str = shortcut.toStr();

    ASSERT_FALSE(str.isEmpty());

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError)
        << parseError.errorString().toStdString();
    ASSERT_TRUE(doc.isObject());

    QJsonObject obj = doc.object();
    ASSERT_TRUE(obj.contains("shortcut"));
    QJsonArray groups = obj.value("shortcut").toArray();
    // 构造函数恰好注册了 3 个快捷键分组
    ASSERT_EQ(groups.size(), 3);

    for (const QJsonValue &groupVal : groups) {
        ASSERT_TRUE(groupVal.isObject());
        QJsonObject group = groupVal.toObject();
        ASSERT_TRUE(group.contains("groupItems"));
        QJsonArray items = group.value("groupItems").toArray();
        ASSERT_GT(items.size(), 0);
        for (const QJsonValue &itemVal : items) {
            ASSERT_TRUE(itemVal.isObject());
            QJsonObject item = itemVal.toObject();
            ASSERT_TRUE(item.contains("name"));
            ASSERT_TRUE(item.contains("value"));
            ASSERT_FALSE(item.value("name").toString().isEmpty());
            ASSERT_FALSE(item.value("value").toString().isEmpty());
        }
    }
}

// 构造函数注册的已知快捷键值应能在序列化结果中被检索到。
TEST(shorcutTest, constructorRegistersKnownShortcutValues)
{
    Shortcut shortcut;
    QString str = shortcut.toStr();

    ASSERT_TRUE(str.contains(QStringLiteral("F1")));
    ASSERT_TRUE(str.contains(QStringLiteral("Ctrl+C")));
    ASSERT_TRUE(str.contains(QStringLiteral("Ctrl+X")));
    ASSERT_TRUE(str.contains(QStringLiteral("Ctrl+V")));
    ASSERT_TRUE(str.contains(QStringLiteral("Ctrl+A")));
    ASSERT_TRUE(str.contains(QStringLiteral("Delete")));
}

// 同一对象多次调用 toStr() 应返回稳定一致的结果。
TEST(shorcutTest, toStrIsStableAcrossCalls)
{
    Shortcut shortcut;
    QString first = shortcut.toStr();
    QString second = shortcut.toStr();
    ASSERT_EQ(first, second);
}
