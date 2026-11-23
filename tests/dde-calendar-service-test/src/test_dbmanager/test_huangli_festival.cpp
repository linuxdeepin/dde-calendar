// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbmanager/dhuanglidatabase.h"
#include "units.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <gtest/gtest.h>

class TestHuangLiFestival : public ::testing::Test
{
protected:
    void SetUp() override
    {
        QDir().mkpath(getAppCacheDir().path());
        m_cacheFile = getAppCacheDir().filePath(QStringLiteral("2020.json"));
        QFile::remove(m_cacheFile);
    }

    void TearDown() override
    {
        QFile::remove(m_cacheFile);
    }

    static bool containsDate(const QJsonArray &result, const QString &date)
    {
        for (const auto &item : result) {
            if (item.toObject().value("date").toString() == date) {
                return true;
            }
        }
        return false;
    }

    QString m_cacheFile;
};

TEST_F(TestHuangLiFestival, QueryFestivalList_NoCache_FallsBackToBuiltin)
{
    DHuangLiDataBase db;
    const QJsonArray result = db.queryFestivalList(2020, 10);
    EXPECT_TRUE(containsDate(result, QStringLiteral("2020-10-01")));
}

TEST_F(TestHuangLiFestival, QueryFestivalList_ValidCache_IsPreferred)
{
    QJsonObject day;
    day.insert("name", QStringLiteral("TESTCACHE"));
    day.insert("date", QStringLiteral("2020-10-01"));
    day.insert("isOffDay", true);
    QJsonObject root;
    root.insert("year", 2020);
    root.insert("days", QJsonArray{day});

    QFile file(m_cacheFile);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(QJsonDocument(root).toJson());
    file.close();

    DHuangLiDataBase db;
    const QJsonArray result = db.queryFestivalList(2020, 10);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result.first().toObject().value("name").toString(), QStringLiteral("TESTCACHE"));
}

TEST_F(TestHuangLiFestival, QueryFestivalList_InvalidCache_FallsBackToBuiltin)
{
    QFile file(m_cacheFile);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("{not-json");
    file.close();

    DHuangLiDataBase db;
    const QJsonArray result = db.queryFestivalList(2020, 10);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(containsDate(result, QStringLiteral("2020-10-01")));
}
