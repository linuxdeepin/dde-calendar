// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_colorpickerwidget.h"

#include <QKeyEvent>
#include <QEvent>
#include <QCoreApplication>

TEST_F(test_colorpickerwidget, constructor)
{
    EXPECT_NE(mWidget, nullptr);
    EXPECT_NE(mWidget->m_colorLabel, nullptr);
    EXPECT_NE(mWidget->m_colorSlider, nullptr);
    EXPECT_NE(mWidget->m_colHexLineEdit, nullptr);
    EXPECT_NE(mWidget->m_cancelBtn, nullptr);
    EXPECT_NE(mWidget->m_enterBtn, nullptr);
}

TEST_F(test_colorpickerwidget, getSelectedColor)
{
    mWidget->slotUpdateColor(QColor("#000000"));
    EXPECT_EQ(mWidget->getSelectedColor(), QColor("#000000"));
    mWidget->slotUpdateColor(QColor("#123456"));
    EXPECT_EQ(mWidget->getSelectedColor(), QColor("#123456"));
}

TEST_F(test_colorpickerwidget, setColorHexLineEdit)
{
    mWidget->setColorHexLineEdit();
    EXPECT_TRUE(mWidget->m_colHexLineEdit->text().isEmpty());
    EXPECT_FALSE(mWidget->m_enterBtn->isEnabled());
}

TEST_F(test_colorpickerwidget, slotUpdateColor_valid)
{
    mWidget->slotUpdateColor(QColor("#000000"));
    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "000000");
    mWidget->slotUpdateColor(QColor("#123456"));
    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "123456");
}

TEST_F(test_colorpickerwidget, slotUpdateColor_invalid)
{
    mWidget->m_colHexLineEdit->setText("abcdef");
    mWidget->slotUpdateColor(QColor());
    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "abcdef");
}

TEST_F(test_colorpickerwidget, slotHexLineEditChange_empty)
{
    mWidget->slotHexLineEditChange("");
    EXPECT_FALSE(mWidget->m_enterBtn->isEnabled());
}

TEST_F(test_colorpickerwidget, slotHexLineEditChange_uppercase_short)
{
    mWidget->slotHexLineEditChange("FFfF");
    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "ffff");
    EXPECT_FALSE(mWidget->m_enterBtn->isEnabled());
}

TEST_F(test_colorpickerwidget, slotHexLineEditChange_uppercase_valid)
{
    mWidget->slotHexLineEditChange("FFfFff");
    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "ffffff");
    EXPECT_TRUE(mWidget->m_enterBtn->isEnabled());
}

TEST_F(test_colorpickerwidget, slotHexLineEditChange_lowercase_valid)
{
    mWidget->slotHexLineEditChange("aabbcc");
    EXPECT_TRUE(mWidget->m_enterBtn->isEnabled());
}

TEST_F(test_colorpickerwidget, slotCancelBtnClicked)
{
    mWidget->slotCancelBtnClicked();
}

TEST_F(test_colorpickerwidget, slotEnterBtnClicked)
{
    mWidget->slotEnterBtnClicked();
}

TEST_F(test_colorpickerwidget, keyPressEvent_Enter)
{
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    mWidget->keyPressEvent(&e);
}

TEST_F(test_colorpickerwidget, keyPressEvent_Return)
{
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    mWidget->keyPressEvent(&e);
}

TEST_F(test_colorpickerwidget, keyPressEvent_Other)
{
    QKeyEvent e(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    mWidget->keyPressEvent(&e);
}

TEST_F(test_colorpickerwidget, changeEvent_FontChange)
{
    QEvent e(QEvent::FontChange);
    mWidget->changeEvent(&e);
}

TEST_F(test_colorpickerwidget, changeEvent_Other)
{
    QEvent e(QEvent::LanguageChange);
    mWidget->changeEvent(&e);
}

TEST_F(test_colorpickerwidget, setLabelText)
{
    mWidget->setLabelText();
}

TEST_F(test_colorpickerwidget, initUI)
{
    mWidget->initUI();
}

TEST_F(test_colorpickerwidget, sliderValueChangedLambda)
{
    int oldHue = mWidget->m_colorLabel->m_hue;
    mWidget->m_colorSlider->setValue(180);
    QCoreApplication::processEvents();
    EXPECT_EQ(mWidget->m_colorLabel->m_hue, 180);
    mWidget->m_colorSlider->setValue(60);
    QCoreApplication::processEvents();
    EXPECT_EQ(mWidget->m_colorLabel->m_hue, 60);
    (void)oldHue;
}
