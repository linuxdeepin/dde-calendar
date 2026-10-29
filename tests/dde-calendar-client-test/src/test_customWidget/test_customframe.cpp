// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_customframe.h"

#include <QPaintEvent>

test_customframe::test_customframe()
{
    mCustomFrame = new CustomFrame();
}

test_customframe::~test_customframe()
{
    delete mCustomFrame;
    mCustomFrame = nullptr;
}

//void CustomFrame::setBColor(QColor normalC)
TEST_F(test_customframe, setBColor)
{
    mCustomFrame->setBColor(Qt::red);
    EXPECT_EQ(mCustomFrame->m_bnormalColor, Qt::red);
    EXPECT_TRUE(mCustomFrame->m_bflag);
}

//void CustomFrame::setRoundState(bool lstate, bool tstate, bool rstate, bool bstate)
TEST_F(test_customframe, setRoundState)
{
    mCustomFrame->setRoundState(true, true, true, true);
    EXPECT_TRUE(mCustomFrame->m_lstate);
    EXPECT_TRUE(mCustomFrame->m_tstate);
    EXPECT_TRUE(mCustomFrame->m_rstate);
    EXPECT_TRUE(mCustomFrame->m_bstate);

    mCustomFrame->setRoundState(false, false, false, false);
    EXPECT_FALSE(mCustomFrame->m_lstate);
    EXPECT_FALSE(mCustomFrame->m_tstate);
    EXPECT_FALSE(mCustomFrame->m_rstate);
    EXPECT_FALSE(mCustomFrame->m_bstate);
}

//void CustomFrame::setTextStr(QFont font, QColor tc, QString strc, int flag)
TEST_F(test_customframe, setTextStr_font_color_str_flag)
{
    QFont font;
    font.setPixelSize(14);
    mCustomFrame->setTextStr(font, Qt::red, "hello", 1);
    EXPECT_EQ(mCustomFrame->m_font, font);
    EXPECT_EQ(mCustomFrame->m_tnormalColor, Qt::red);
    EXPECT_EQ(mCustomFrame->m_text, "hello");
    EXPECT_EQ(mCustomFrame->m_textflag, 1);
}

//void CustomFrame::setTextStr(QString strc)
TEST_F(test_customframe, setTextStr_str)
{
    mCustomFrame->setTextStr("hello");
    EXPECT_EQ(mCustomFrame->m_text, "hello");
}

//void CustomFrame::setTextColor(QColor tc)
TEST_F(test_customframe, setTextColor)
{
    mCustomFrame->setTextColor(Qt::red);
    EXPECT_EQ(mCustomFrame->m_tnormalColor, Qt::red);
}

//void CustomFrame::setTextFont(QFont font)
TEST_F(test_customframe, setTextFont)
{
    QFont font;
    font.setPixelSize(16);
    mCustomFrame->setTextFont(font);
    EXPECT_EQ(mCustomFrame->m_font, font);
}

//void CustomFrame::setTextAlign(int flag)
TEST_F(test_customframe, setTextAlign)
{
    mCustomFrame->setTextAlign(2);
    EXPECT_EQ(mCustomFrame->m_textflag, 2);
}

//void CustomFrame::setRadius(int radius)
TEST_F(test_customframe, setRadius)
{
    mCustomFrame->setRadius(4);
    EXPECT_EQ(mCustomFrame->m_radius, 4);
}

//void CustomFrame::setboreder(int framew)
TEST_F(test_customframe, setboreder)
{
    mCustomFrame->setboreder(4);
    EXPECT_EQ(mCustomFrame->m_borderframew, 4);
}

//void CustomFrame::setFixedSize(int w, int h)
TEST_F(test_customframe, setFixedSize)
{
    mCustomFrame->setFixedSize(40, 44);
    EXPECT_TRUE(mCustomFrame->m_fixsizeflag);
    EXPECT_EQ(mCustomFrame->size(), QSize(40, 44));
}

// getTextStr: verify returns m_text
TEST_F(test_customframe, getTextStr_default)
{
    EXPECT_EQ(mCustomFrame->getTextStr().toStdString(), "");
}

TEST_F(test_customframe, getTextStr_after_set)
{
    mCustomFrame->setTextStr("test text");
    EXPECT_EQ(mCustomFrame->getTextStr().toStdString(), "test text");
}

// paintEvent: with m_bflag=false (no background path)
TEST_F(test_customframe, paintEvent_no_bflag)
{
    mCustomFrame->resize(100, 50);
    mCustomFrame->setTextStr("test");
    QPaintEvent *e = new QPaintEvent(QRect(0, 0, 100, 50));
    mCustomFrame->paintEvent(e);
    delete e;
}

// paintEvent: with m_bflag=true, all corners rounded
TEST_F(test_customframe, paintEvent_bflag_all_round)
{
    mCustomFrame->resize(100, 50);
    mCustomFrame->setBColor(Qt::white);
    mCustomFrame->setRoundState(true, true, true, true);
    mCustomFrame->setTextStr("test");
    QPaintEvent *e = new QPaintEvent(QRect(0, 0, 100, 50));
    mCustomFrame->paintEvent(e);
    delete e;
}

// paintEvent: with m_bflag=true, no corners rounded
TEST_F(test_customframe, paintEvent_bflag_no_round)
{
    mCustomFrame->resize(100, 50);
    mCustomFrame->setBColor(Qt::blue);
    mCustomFrame->setRoundState(false, false, false, false);
    mCustomFrame->setTextStr("test");
    QPaintEvent *e = new QPaintEvent(QRect(0, 0, 100, 50));
    mCustomFrame->paintEvent(e);
    delete e;
}

// paintEvent: with m_bflag=true, mixed corner states
TEST_F(test_customframe, paintEvent_bflag_mixed)
{
    mCustomFrame->resize(100, 50);
    mCustomFrame->setBColor(Qt::green);
    mCustomFrame->setRoundState(true, false, true, false);
    QPaintEvent *e = new QPaintEvent(QRect(0, 0, 100, 50));
    mCustomFrame->paintEvent(e);
    delete e;
}

// paintEvent: with text but no bflag
TEST_F(test_customframe, paintEvent_text_only)
{
    mCustomFrame->resize(100, 50);
    mCustomFrame->setTextStr("hello world");
    mCustomFrame->setTextColor(Qt::black);
    QPaintEvent *e = new QPaintEvent(QRect(0, 0, 100, 50));
    mCustomFrame->paintEvent(e);
    delete e;
}
