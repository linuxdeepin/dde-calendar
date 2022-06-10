/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//#include "test_colorpickerwidget.h"

//test_colorpickerwidget::test_colorpickerwidget(QObject *parent) : QObject(parent)
//{

//}

//TEST_F(test_colorpickerwidget, getSelectedColor_001)
//{
//    mWidget->slotUpdateColor(QColor("#000000"));
//    EXPECT_EQ(mWidget->getSelectedColor(), "#000000") << "text: #000000";
//    mWidget->slotUpdateColor(QColor("#123456"));
//    EXPECT_EQ(mWidget->getSelectedColor(), "#123456") << "text: #123456";
//}

//TEST_F(test_colorpickerwidget, setColorHexLineEdit_001)
//{
//    mWidget->setColorHexLineEdit();
//    EXPECT_TRUE(mWidget->m_colHexLineEdit->text().isEmpty());
//}

//TEST_F(test_colorpickerwidget, slotUpdateColor)
//{
//    mWidget->slotUpdateColor(QColor("#000000"));
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "000000") << "text: #000000";
//    mWidget->slotUpdateColor(QColor("#123456"));
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "123456") << "text: #123456";
//}

//TEST_F(test_colorpickerwidget, slotHexLineEditChange_001)
//{
//    mWidget->slotHexLineEditChange("");
//    EXPECT_FALSE(mWidget->m_enterBtn->isEnabled());
//}

//TEST_F(test_colorpickerwidget, slotHexLineEditChange_002)
//{
//    mWidget->slotHexLineEditChange("FFfF");
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "ffff") << "eq";
//    EXPECT_FALSE(mWidget->m_enterBtn->isEnabled()) << "false";
//}

//TEST_F(test_colorpickerwidget, slotHexLineEditChange_003)
//{
//    mWidget->slotHexLineEditChange("FFfFff");
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "ffffff") << "eq";
//    EXPECT_TRUE(mWidget->m_enterBtn->isEnabled()) << "true";
//}

//TEST_F(test_colorpickerwidget, slotCancelBtnClicked_001)
//{
//    mWidget->slotCancelBtnClicked();
//}

//TEST_F(test_colorpickerwidget, slotEnterBtnClicked_001)
//{
//    mWidget->slotEnterBtnClicked();
//}


