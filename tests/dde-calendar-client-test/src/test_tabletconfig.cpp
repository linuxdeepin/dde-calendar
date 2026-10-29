// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_tabletconfig.h"

test_TabletConfig::test_TabletConfig()
{
}

// 静态成员默认初始化为 false（非平板模式）。
TEST(istableTest, defaultIsTabletStateIsFalse)
{
    TabletConfig::setIsTablet(false);
    EXPECT_FALSE(TabletConfig::isTablet());
}

// setIsTablet 应能正确切换静态状态。
TEST(istableTest, setIsTabletTogglesState)
{
    TabletConfig::setIsTablet(true);
    ASSERT_TRUE(TabletConfig::isTablet());

    TabletConfig::setIsTablet(false);
    ASSERT_FALSE(TabletConfig::isTablet());

    TabletConfig::setIsTablet(true);
    ASSERT_TRUE(TabletConfig::isTablet());
}

// 构造实例不应改变静态平板状态。
TEST(istableTest, instanceConstructionDoesNotResetStaticState)
{
    TabletConfig::setIsTablet(false);
    bool before = TabletConfig::isTablet();

    TabletConfig config;
    EXPECT_EQ(TabletConfig::isTablet(), before);

    TabletConfig *heap = new TabletConfig();
    EXPECT_EQ(TabletConfig::isTablet(), before);
    delete heap;

    // 恢复静态状态，避免污染后续用例
    TabletConfig::setIsTablet(false);
}
