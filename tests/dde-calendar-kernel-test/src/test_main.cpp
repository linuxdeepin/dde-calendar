// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QtGlobal>
#include <QByteArray>
#include <QDir>
#include <ctime>
#include <gtest/gtest.h>

TEST(KernelFixtures, HasIcsFiles)
{
    ASSERT_FALSE(QDir(FIXTURES_DIR).entryList({QStringLiteral("*.ics")}, QDir::Files).isEmpty());
}

int main(int argc, char **argv)
{
    // Some fixtures deliberately contain floating times. Freeze their local zone.
    qputenv("TZ", "Asia/Shanghai");
    tzset();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
