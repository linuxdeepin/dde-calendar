// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_LUNARMANAGER_H
#define TEST_LUNARMANAGER_H

#include <gtest/gtest.h>

#include "lunarmanager.h"

// V-6584 未随附件上传该头文件，按 cpp 用法还原的最小 fixture
class test_lunarmanager : public testing::Test
{
public:
    test_lunarmanager();
    ~test_lunarmanager() override = default;
};

#endif // TEST_LUNARMANAGER_H
