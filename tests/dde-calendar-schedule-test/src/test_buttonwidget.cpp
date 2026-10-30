// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "buttonwidget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <DVerticalLine>

#include <QSignalSpy>

class ButtonWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Constructor
TEST_F(ButtonWidgetTest, Constructor_InitializesLayoutAndDefaults)
{
    buttonwidget bw;
    EXPECT_EQ(bw.buttonCount(), 0);
    EXPECT_NE(bw.layout(), nullptr);
    auto *hbox = qobject_cast<QHBoxLayout *>(bw.layout());
    EXPECT_NE(hbox, nullptr);
    EXPECT_EQ(hbox->contentsMargins().left(), 0);
    EXPECT_EQ(hbox->contentsMargins().top(), 0);
    EXPECT_EQ(hbox->contentsMargins().right(), 0);
    EXPECT_EQ(hbox->contentsMargins().bottom(), 0);
}

// buttonCount
TEST_F(ButtonWidgetTest, ButtonCount_InitiallyZero)
{
    buttonwidget bw;
    EXPECT_EQ(bw.buttonCount(), 0);
}

// addbutton with ButtonNormal
TEST_F(ButtonWidgetTest, AddButton_ButtonNormal_ReturnsIndex)
{
    buttonwidget bw;
    int idx = bw.addbutton("OK", true, buttonwidget::ButtonNormal);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// addbutton with ButtonWarning
TEST_F(ButtonWidgetTest, AddButton_ButtonWarning_ReturnsIndex)
{
    buttonwidget bw;
    int idx = bw.addbutton("Warn", true, buttonwidget::ButtonWarning);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// addbutton with ButtonRecommend
TEST_F(ButtonWidgetTest, AddButton_ButtonRecommend_ReturnsIndex)
{
    buttonwidget bw;
    int idx = bw.addbutton("Rec", true, buttonwidget::ButtonRecommend);
    EXPECT_EQ(idx, 0);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// addbutton multiple buttons returns correct indices
TEST_F(ButtonWidgetTest, AddButton_MultipleButtons_ReturnsSequentialIndices)
{
    buttonwidget bw;
    EXPECT_EQ(bw.addbutton("A"), 0);
    EXPECT_EQ(bw.addbutton("B"), 1);
    EXPECT_EQ(bw.addbutton("C"), 2);
    EXPECT_EQ(bw.buttonCount(), 3);
}

// insertButton with text - ButtonNormal
TEST_F(ButtonWidgetTest, InsertButton_Text_ButtonNormal)
{
    buttonwidget bw;
    bw.insertButton(0, "Test", true, buttonwidget::ButtonNormal);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// insertButton with text - ButtonWarning
TEST_F(ButtonWidgetTest, InsertButton_Text_ButtonWarning)
{
    buttonwidget bw;
    bw.insertButton(0, "Test", true, buttonwidget::ButtonWarning);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// insertButton with text - ButtonRecommend
TEST_F(ButtonWidgetTest, InsertButton_Text_ButtonRecommend)
{
    buttonwidget bw;
    bw.insertButton(0, "Test", true, buttonwidget::ButtonRecommend);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// insertButton with text - not default
TEST_F(ButtonWidgetTest, InsertButton_Text_NotDefault)
{
    buttonwidget bw;
    bw.insertButton(0, "Test", false, buttonwidget::ButtonNormal);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// insertButton with QAbstractButton* overload
TEST_F(ButtonWidgetTest, InsertButton_ButtonOverload)
{
    buttonwidget bw;
    QPushButton *btn = new QPushButton("Custom");
    bw.insertButton(0, btn, true);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// insertButton with QAbstractButton* - not default
TEST_F(ButtonWidgetTest, InsertButton_ButtonOverload_NotDefault)
{
    buttonwidget bw;
    QPushButton *btn = new QPushButton("Custom");
    bw.insertButton(0, btn, false);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// onButtonClicked - test signal emission
TEST_F(ButtonWidgetTest, OnButtonClicked_EmitsSignal)
{
    buttonwidget bw;
    bw.addbutton("Button1");
    bw.addbutton("Button2");

    QSignalSpy spy(&bw, &buttonwidget::buttonClicked);

    // Click the first button
    auto *btn = bw.findChild<QPushButton *>();
    ASSERT_NE(btn, nullptr);
    btn->click();

    ASSERT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toInt(), 0);
    EXPECT_EQ(args.at(1).toString(), "Button1");
}

// CJK 2-char text spacing - Han characters should get Nbsp inserted
TEST_F(ButtonWidgetTest, InsertButton_CJK_TwoCharText_InsertsNbsp)
{
    buttonwidget bw;
    bw.addbutton(QString::fromUtf8("确定"));

    auto *btn = qobject_cast<QPushButton *>(bw.findChild<QPushButton *>());
    ASSERT_NE(btn, nullptr);
    QString text = btn->text();
    EXPECT_EQ(text.count(), 3); // char + Nbsp + char
    EXPECT_EQ(text.at(1), QChar::Nbsp);
}

// CJK 2-char text spacing - Katakana characters
TEST_F(ButtonWidgetTest, InsertButton_CJK_Katakana_TwoChar_InsertsNbsp)
{
    buttonwidget bw;
    // Two Katakana characters
    QString katakana = QString::fromUtf8("アイ");
    bw.addbutton(katakana);

    auto *btn = qobject_cast<QPushButton *>(bw.findChild<QPushButton *>());
    ASSERT_NE(btn, nullptr);
    EXPECT_EQ(btn->text().count(), 3);
}

// 2-char non-CJK text should NOT get Nbsp inserted
TEST_F(ButtonWidgetTest, InsertButton_NonCJK_TwoCharText_NoNbsp)
{
    buttonwidget bw;
    bw.addbutton("OK");

    auto *btn = qobject_cast<QPushButton *>(bw.findChild<QPushButton *>());
    ASSERT_NE(btn, nullptr);
    EXPECT_EQ(btn->text(), QString("OK"));
}

// 1-char text should not get spacing
TEST_F(ButtonWidgetTest, InsertButton_SingleChar_NoSpacing)
{
    buttonwidget bw;
    bw.addbutton("A");

    auto *btn = qobject_cast<QPushButton *>(bw.findChild<QPushButton *>());
    ASSERT_NE(btn, nullptr);
    EXPECT_EQ(btn->text(), QString("A"));
}

// 3-char text should not get spacing
TEST_F(ButtonWidgetTest, InsertButton_ThreeChar_NoSpacing)
{
    buttonwidget bw;
    bw.addbutton("ABC");

    auto *btn = qobject_cast<QPushButton *>(bw.findChild<QPushButton *>());
    ASSERT_NE(btn, nullptr);
    EXPECT_EQ(btn->text(), QString("ABC"));
}

// Button object name set to "ActionButton"
TEST_F(ButtonWidgetTest, InsertButton_SetsObjectName)
{
    buttonwidget bw;
    bw.addbutton("Test");

    auto *btn = bw.findChild<QAbstractButton *>("ActionButton");
    EXPECT_NE(btn, nullptr);
}

// onButtonClicked with second button
TEST_F(ButtonWidgetTest, OnButtonClicked_SecondButton_EmitsCorrectIndex)
{
    buttonwidget bw;
    bw.addbutton("First");
    bw.addbutton("Second");

    QSignalSpy spy(&bw, &buttonwidget::buttonClicked);

    auto buttons = bw.findChildren<QPushButton *>();
    ASSERT_GE(buttons.size(), 2);
    buttons[1]->click();

    ASSERT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toInt(), 1);
}

// Verify button type creates correct class
TEST_F(ButtonWidgetTest, AddButton_ButtonWarning_CreatesButton)
{
    buttonwidget bw;
    bw.addbutton("Warn", false, buttonwidget::ButtonWarning);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// Verify button type creates correct class
TEST_F(ButtonWidgetTest, AddButton_ButtonRecommend_CreatesButton)
{
    buttonwidget bw;
    bw.addbutton("Rec", false, buttonwidget::ButtonRecommend);
    EXPECT_EQ(bw.buttonCount(), 1);
}

// insertButton at non-zero index
TEST_F(ButtonWidgetTest, InsertButton_AtIndex_One)
{
    buttonwidget bw;
    bw.addbutton("A");
    bw.insertButton(1, "B");
    EXPECT_EQ(bw.buttonCount(), 2);
}

// DVerticalLine object name check
TEST_F(ButtonWidgetTest, InsertButton_CreatesVerticalLine)
{
    buttonwidget bw;
    bw.addbutton("Test");

    auto *line = bw.findChild<DVerticalLine *>("VLine");
    EXPECT_NE(line, nullptr);
}
