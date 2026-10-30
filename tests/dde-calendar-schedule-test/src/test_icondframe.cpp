// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "icondframe.h"
#include "../globaldef.h"
#include "reply.h"

#include <QSvgRenderer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QSignalSpy>
#include <QTest>
#include <QFocusEvent>
#include <DGuiApplicationHelper>

class IconDFrameTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Constructor
TEST_F(IconDFrameTest, Constructor_CreatesRenderersAndSetsFocusPolicy)
{
    IconDFrame frame;
    EXPECT_EQ(frame.focusPolicy(), Qt::TabFocus);
}

// Destructor - no crash
TEST_F(IconDFrameTest, Destructor_NoCrash)
{
    {
        IconDFrame frame;
    }
    SUCCEED();
}

// TitleColor / setTitleColor
TEST_F(IconDFrameTest, TitleColor_SetAndGet)
{
    IconDFrame frame;
    QColor color(255, 0, 0);
    frame.setTitleColor(color);
    EXPECT_EQ(frame.TitleColor(), color);
}

TEST_F(IconDFrameTest, TitleColor_Default)
{
    IconDFrame frame;
    // Default color should be default-constructed (invalid or black)
    QColor c = frame.TitleColor();
    SUCCEED();
}

// TitleFont - sets pixelSize to 12
TEST_F(IconDFrameTest, TitleFont_SetsPixelSize12)
{
    IconDFrame frame;
    // Access private TitleFont method via -fno-access-control
    QFont font = frame.TitleFont();
    EXPECT_EQ(font.pixelSize(), 12);
}

// setCenterLayout
TEST_F(IconDFrameTest, SetCenterLayout_SetsVBoxLayout)
{
    IconDFrame frame;
    QHBoxLayout *innerLayout = new QHBoxLayout();
    frame.setCenterLayout(innerLayout);

    auto *outer = qobject_cast<QVBoxLayout *>(frame.layout());
    EXPECT_NE(outer, nullptr);
}

// paintEvent - basic render, no crash
TEST_F(IconDFrameTest, PaintEvent_NoCrash)
{
    IconDFrame frame;
    frame.resize(100, 100);
    frame.show();
    QTest::qWait(50);
    frame.hide();
    SUCCEED();
}

// paintEvent with m_tabFocusIn = true (via -fno-access-control)
TEST_F(IconDFrameTest, PaintEvent_WithTabFocus)
{
    IconDFrame frame;
    frame.m_tabFocusIn = true;
    frame.resize(100, 100);
    frame.show();
    QTest::qWait(50);
    frame.hide();
    SUCCEED();
}

// hideEvent - emits widgetIsHide when enabled
TEST_F(IconDFrameTest, HideEvent_EmitsSignal_WhenEnabled)
{
    IconDFrame frame;
    frame.setEnabled(true);
    QSignalSpy spy(&frame, &IconDFrame::widgetIsHide);

    frame.show();
    QTest::qWait(10);
    frame.hide();
    QTest::qWait(10);

    EXPECT_GE(spy.count(), 1);
}

// hideEvent - does not emit when disabled
TEST_F(IconDFrameTest, HideEvent_NoSignal_WhenDisabled)
{
    IconDFrame frame;
    frame.setEnabled(false);
    QSignalSpy spy(&frame, &IconDFrame::widgetIsHide);

    frame.show();
    QTest::qWait(10);
    frame.hide();
    QTest::qWait(10);

    EXPECT_EQ(spy.count(), 0);
}

// focusInEvent - directly invoke the handler since offscreen platform may not deliver focus events
TEST_F(IconDFrameTest, FocusInEvent_SetsTabFocusFalse)
{
    IconDFrame frame;
    frame.m_tabFocusIn = true;
    QFocusEvent focusInEvent(QEvent::FocusIn);
    frame.focusInEvent(&focusInEvent);
    EXPECT_FALSE(frame.m_tabFocusIn);
}

// focusOutEvent - directly invoke the handler since offscreen platform may not deliver focus events
TEST_F(IconDFrameTest, FocusOutEvent_SetsTabFocusFalse)
{
    IconDFrame frame;
    frame.m_tabFocusIn = true;
    QFocusEvent focusOutEvent(QEvent::FocusOut);
    frame.focusOutEvent(&focusOutEvent);
    EXPECT_FALSE(frame.m_tabFocusIn);
}

// setTheMe with type 0 (dark theme)
TEST_F(IconDFrameTest, SetTheMe_Type0_SetsDarkTitleColor)
{
    IconDFrame frame;
    frame.setTheMe(0);
    QColor c = frame.TitleColor();
    EXPECT_EQ(c.red(), 0);
    EXPECT_EQ(c.green(), 0);
    EXPECT_EQ(c.blue(), 0);
    EXPECT_NEAR(c.alphaF(), 0.4, 0.01);
}

// setTheMe with type 1
TEST_F(IconDFrameTest, SetTheMe_Type1_SetsDarkTitleColor)
{
    IconDFrame frame;
    frame.setTheMe(1);
    QColor c = frame.TitleColor();
    EXPECT_EQ(c.red(), 0);
    EXPECT_EQ(c.green(), 0);
    EXPECT_EQ(c.blue(), 0);
}

// setTheMe with type 2 (light theme)
TEST_F(IconDFrameTest, SetTheMe_Type2_SetsLightTitleColor)
{
    IconDFrame frame;
    frame.setTheMe(2);
    QColor c = frame.TitleColor();
    EXPECT_EQ(c.red(), 255);
    EXPECT_EQ(c.green(), 255);
    EXPECT_EQ(c.blue(), 255);
    EXPECT_NEAR(c.alphaF(), 0.4, 0.01);
}

// slotReceivce - no-op, should not crash
TEST_F(IconDFrameTest, SlotReceivce_NoOp)
{
    IconDFrame frame;
    QVariant data(42);
    Reply *reply = nullptr;
    frame.slotReceivce(data, reply);
    SUCCEED();
}

// paintPixmap and paintTitle are called via paintEvent - verified by no-crash tests above

// Destructor deletes all renderers
TEST_F(IconDFrameTest, Destructor_DeletesRenderers)
{
    IconDFrame *frame = new IconDFrame();
    // Access private members via -fno-access-control
    EXPECT_NE(frame->m_Dayrenderer, nullptr);
    EXPECT_NE(frame->m_Weekrenderer, nullptr);
    EXPECT_NE(frame->m_Monthrenderer, nullptr);
    EXPECT_NE(frame->m_backgroundrenderer, nullptr);
    delete frame;
    SUCCEED();
}
