// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcaldavcolorallocator.h"
#include "daccountdatabase.h"
#include "daccount.h"
#include "dscheduletype.h"
#include "units.h"

#include <gtest/gtest.h>
#include <QSet>

namespace {

class MockAccountDatabase : public DAccountDataBase
{
public:
    explicit MockAccountDatabase(const DAccount::Ptr &account)
        : DAccountDataBase(account) {}
    void setScheduleTypes(const DScheduleType::List &types) { m_types = types; }
    DScheduleType::List getScheduleTypeList(const int isDeleted = 0) override { return m_types; }
private:
    DScheduleType::List m_types;
};

DScheduleType::Ptr makeType(const QString &colorCode, const QString &typePath)
{
    DScheduleType::Ptr type = DScheduleType::Ptr::create();
    type->setColorCode(colorCode);
    type->setTypePath(typePath);
    return type;
}

DAccount::Ptr makeAccount()
{
    DAccount::Ptr account = DAccount::Ptr::create();
    account->setAccountID(QStringLiteral("test-account-id"));
    return account;
}

} // namespace

TEST(DCalDavColorAllocatorTest, NextColor_NullDatabase)
{
    QString color = DCalDavColorAllocator::nextColor(nullptr);
    EXPECT_EQ(color, GCalDavTypeColors.first());
}

TEST(DCalDavColorAllocatorTest, NextColor_EmptyScheduleTypes)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    db.setScheduleTypes({});
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors.first());
}

TEST(DCalDavColorAllocatorTest, NextColor_FirstUsedReturnsSecond)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    db.setScheduleTypes({makeType(GCalDavTypeColors[0].toLower(), "/path/1")});
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors[1]);
}

TEST(DCalDavColorAllocatorTest, NextColor_SkipUsedColors)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    db.setScheduleTypes({
        makeType(GCalDavTypeColors[0], "/path/1"),
        makeType(GCalDavTypeColors[1], "/path/2"),
        makeType(GCalDavTypeColors[2], "/path/3"),
    });
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors[3]);
}

TEST(DCalDavColorAllocatorTest, NextColor_AllUsedWrapsAround)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    DScheduleType::List allTypes;
    for (const QString &c : GCalDavTypeColors) {
        allTypes.append(makeType(c.toLower(), QStringLiteral("/path/%1").arg(allTypes.size())));
    }
    db.setScheduleTypes(allTypes);
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors.at(allTypes.size() % GCalDavTypeColors.size()));
}

TEST(DCalDavColorAllocatorTest, NextColor_NullTypeEntriesSkipped)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    DScheduleType::List types;
    types.append(DScheduleType::Ptr(nullptr));
    types.append(makeType(GCalDavTypeColors[0], "/path/1"));
    db.setScheduleTypes(types);
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors[1]);
}

TEST(DCalDavColorAllocatorTest, NextColor_EmptyTypePathSkipped)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    db.setScheduleTypes({makeType(GCalDavTypeColors[0], "")});
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors.first());
}

TEST(DCalDavColorAllocatorTest, NextColor_CaseInsensitiveColorMatching)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    db.setScheduleTypes({makeType(QStringLiteral("#ff282a"), "/path/1")});
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors[1]);
}

TEST(DCalDavColorAllocatorTest, NextColor_TrimmedColorCode)
{
    DAccount::Ptr account = makeAccount();
    MockAccountDatabase db(account);
    db.setScheduleTypes({makeType(QStringLiteral("  #FF282A  "), "/path/1")});
    QString color = DCalDavColorAllocator::nextColor(&db);
    EXPECT_EQ(color, GCalDavTypeColors[1]);
}
