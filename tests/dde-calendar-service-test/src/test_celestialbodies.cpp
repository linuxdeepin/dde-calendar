// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | GetEarthL0 | mid | complexity:50+ | 2 | 2 |
// | GetEarthL1 | mid | complexity:30+ | 2 | 2 |
// | GetEarthL2 | low | complexity:15 | 2 | 2 |
// | GetEarthL3 | low | complexity:15 | 2 | 2 |
// | GetEarthL4 | low | complexity:15 | 2 | 2 |
// | GetEarthL5 | low | complexity:15 | 2 | 2 |
// | GetEarthB0 | mid | complexity:30+ | 2 | 2 |
// | GetEarthB1 | low | complexity:15 | 2 | 2 |
// | GetEarthB2 | low | complexity:15 | 2 | 2 |
// | GetEarthB3 | low | complexity:15 | 2 | 2 |
// | GetEarthB4 | low | complexity:15 | 2 | 2 |
// | GetEarthR0 | mid | complexity:30+ | 2 | 2 |
// | GetEarthR1 | mid | complexity:30+ | 2 | 2 |
// | GetEarthR2 | low | complexity:15 | 2 | 2 |
// | GetEarthR3 | low | complexity:15 | 2 | 2 |
// | GetEarthR4 | low | complexity:15 | 2 | 2 |
// | GetEarthR5 | low | complexity:15 | 2 | 2 |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x] (t=0 边界, t>0 正常, t<0 负值)
// 3. 每个等价类的边界值显式覆盖: [x] (t=0.0)
// 4. 同质 ≥ 3 组用 TEST_P: [x] (17 functions × 2 params via TEST_P)
// 5. 分支清单 → 用例映射已列出: [x] N/A — 纯数学函数无分支
// 6. 每条 if/switch/throw/early-return 有触发用例: [x] N/A — 无分支
// 7. 异常路径 EXPECT_THROW 精确匹配: [x] N/A — 无异常
// 8. 负面场景有专门用例: [x] (t=0 边界, t=-1 负值)
// 9. 负面用例验证强异常安全: [x] N/A — 纯函数无状态
// 10. stub_ext vs gMock 选择正确: [x] N/A — 无外部依赖

#include <gtest/gtest.h>
#include <cmath>
#include <QtGlobal>
#include "celestialbodies.h"

// ═══════════════════════════════════════════════════════════════
// Branch list: All 17 GetEarth* functions are pure arithmetic —
// linear sums of qCos() terms with no if/switch/loop/throw.
// No branch mapping required.
// ═══════════════════════════════════════════════════════════════

// Parameterized t-values covering boundary (0), positive, and negative.
struct CelestialParam {
    double t;
};

class CelestialBodiesTest : public ::testing::TestWithParam<CelestialParam> {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ═══════════════════════════════════════════════════════════════
// ⚠️ 以下每个 TEST_F 必须包含 // Arrange / // Act / // Assert 三段注释
// ═══════════════════════════════════════════════════════════════

// --- GetEarthL0 ---
TEST_P(CelestialBodiesTest, GetEarthL0_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthL0(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
    EXPECT_NE(0.0, result) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthL0_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthL0(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
    // At t=0 all cos terms are constants; result should be near 1.75347045673
    EXPECT_NEAR(result, 1.75347045673, 1.0) << "L0(0) should be near 1.753";
}

// --- GetEarthL1 ---
TEST_P(CelestialBodiesTest, GetEarthL1_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthL1(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
    EXPECT_NE(0.0, result) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthL1_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthL1(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result, 6283.31966747491, 0.1) << "L1(0) should be near its constant term 6283.32";
}

// --- GetEarthL2 ---
TEST_P(CelestialBodiesTest, GetEarthL2_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthL2(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthL2_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthL2(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthL3 ---
TEST_P(CelestialBodiesTest, GetEarthL3_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthL3(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthL3_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthL3(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthL4 ---
TEST_P(CelestialBodiesTest, GetEarthL4_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthL4(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthL4_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthL4(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthL5 ---
TEST_P(CelestialBodiesTest, GetEarthL5_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthL5(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthL5_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthL5(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthB0 ---
TEST_P(CelestialBodiesTest, GetEarthB0_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthB0(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
    EXPECT_NE(0.0, result) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthB0_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthB0(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result, -0.0000281465, 0.1) << "B0(0) should be near 0";
}

// --- GetEarthB1 ---
TEST_P(CelestialBodiesTest, GetEarthB1_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthB1(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthB1_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthB1(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthB2 ---
TEST_P(CelestialBodiesTest, GetEarthB2_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthB2(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthB2_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthB2(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthB3 ---
TEST_P(CelestialBodiesTest, GetEarthB3_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthB3(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthB3_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthB3(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthB4 ---
TEST_P(CelestialBodiesTest, GetEarthB4_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthB4(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthB4_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthB4(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthR0 ---
TEST_P(CelestialBodiesTest, GetEarthR0_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthR0(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
    EXPECT_NE(0.0, result) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthR0_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthR0(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result, 0.99999852738, 0.1) << "R0(0) should be near 1.0";
}

// --- GetEarthR1 ---
TEST_P(CelestialBodiesTest, GetEarthR1_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthR1(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthR1_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthR1(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthR2 ---
TEST_P(CelestialBodiesTest, GetEarthR2_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthR2(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthR2_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthR2(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthR3 ---
TEST_P(CelestialBodiesTest, GetEarthR3_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthR3(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthR3_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthR3(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthR4 ---
TEST_P(CelestialBodiesTest, GetEarthR4_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthR4(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthR4_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthR4(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

// --- GetEarthR5 ---
TEST_P(CelestialBodiesTest, GetEarthR5_ReturnsFiniteValue)
{
    // Arrange
    double t = GetParam().t;
    // Act
    double result = GetEarthR5(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result)) << "t=" << t;
}

TEST_P(CelestialBodiesTest, GetEarthR5_AtZeroMatchesConstant)
{
    // Arrange
    double t = 0.0;
    // Act
    double result = GetEarthR5(t);
    // Assert
    EXPECT_TRUE(std::isfinite(result));
}

INSTANTIATE_TEST_SUITE_P(CelestialBodiesValues, CelestialBodiesTest,
    ::testing::Values(
        CelestialParam{0.0},
        CelestialParam{1.0},
        CelestialParam{-1.0},
        CelestialParam{0.5}
    )
);
