// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cdateedit.h"

#include <QTest>
#include <QFocusEvent>
#include <QLineEdit>
#include <QInputMethodEvent>
#include <QCalendarWidget>

//CDateEdit::CDateEdit(QWidget *parent) — constructor exercises 5 signal connections
TEST_F(test_cdateedit, constructor)
{
    CDateEdit *edit = new CDateEdit();
    EXPECT_NE(edit, nullptr);
    delete edit;
}

//void CDateEdit::setDate(QDate date)
TEST_F(test_cdateedit, setDate_001)
{
    QDate date = QDate(2022, 4, 20);
    mWidget->setDate(date);
    EXPECT_EQ(mWidget->date().toString(), date.toString());
}

TEST_F(test_cdateedit, setDate_withLunar)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    QDate date = QDate(2022, 4, 20);
    mWidget->setDate(date);
    SUCCEED();
}

//void CDateEdit::setDisplayFormat(QString format)
TEST_F(test_cdateedit, setDisplayFormat_001)
{
    mWidget->setDisplayFormat("yyyy/MM/dd");
    EXPECT_EQ(mWidget->m_format, "yyyy/MM/dd");
    mWidget->setDisplayFormat("yyyy-MM-dd");
    EXPECT_EQ(mWidget->m_format, "yyyy-MM-dd");
}

//QString CDateEdit::displayFormat()
TEST_F(test_cdateedit, displayFormat_001)
{
    mWidget->setDisplayFormat("yyyy/MM/dd");
    EXPECT_EQ(mWidget->displayFormat(), "yyyy/MM/dd");
    mWidget->setDisplayFormat("yyyy-MM-dd");
    EXPECT_EQ(mWidget->displayFormat(), "yyyy-MM-dd");
}

//void CDateEdit::setLunarCalendarStatus(bool status)
TEST_F(test_cdateedit, setLunarCalendarStatus_001)
{
    mWidget->setLunarCalendarStatus(true);
    EXPECT_TRUE(mWidget->m_showLunarCalendar);
    mWidget->setLunarCalendarStatus(false);
    EXPECT_FALSE(mWidget->m_showLunarCalendar);
}

//void CDateEdit::setLunarTextFormat(QTextCharFormat format)
TEST_F(test_cdateedit, setLunarTextFormat_001)
{
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    mWidget->setLunarTextFormat(format);
    EXPECT_EQ(mWidget->m_lunarTextFormat, format);
}

//QTextCharFormat CDateEdit::getsetLunarTextFormat()
TEST_F(test_cdateedit, getsetLunarTextFormat_001)
{
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    mWidget->setLunarTextFormat(format);
    EXPECT_EQ(mWidget->getsetLunarTextFormat(), format);
}

//void CDateEdit::setCalendarPopup(bool enable)
TEST_F(test_cdateedit, setCalendarPopup_001)
{
    mWidget->setCalendarPopup(true);
    EXPECT_TRUE(mWidget->calendarPopup());
    mWidget->setCalendarPopup(false);
    EXPECT_FALSE(mWidget->calendarPopup());
}

//void CDateEdit::setEditCursorPos(int pos)
TEST_F(test_cdateedit, setEditCursorPos_001)
{
    mWidget->setEditCursorPos(0);
    EXPECT_EQ(mWidget->lineEdit()->cursorPosition(), 0);
    mWidget->setEditCursorPos(2);
    EXPECT_EQ(mWidget->lineEdit()->cursorPosition(), 2);
}

//void CDateEdit::slotDateEidtInfo(const QDate &date)
TEST_F(test_cdateedit, slotDateEidtInfo_001)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->slotDateEidtInfo(QDate(2022, 4, 20));
    SUCCEED();
}

TEST_F(test_cdateedit, slotDateEidtInfo_002)
{
    mWidget->setLunarCalendarStatus(false);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->slotDateEidtInfo(QDate(2022, 4, 20));
    SUCCEED();
}

//void CDateEdit::slotRefreshLineEditTextFormat(const QString &text)
TEST_F(test_cdateedit, slotRefreshLineEditTextFormat_001)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->slotRefreshLineEditTextFormat("2022/04/20");
    SUCCEED();
}

TEST_F(test_cdateedit, slotRefreshLineEditTextFormat_noLunar)
{
    mWidget->setLunarCalendarStatus(false);
    mWidget->slotRefreshLineEditTextFormat("2022/04/20");
    SUCCEED();
}

//void CDateEdit::slotCursorPositionChanged(int oldPos, int newPos)
TEST_F(test_cdateedit, slotCursorPositionChanged_001)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->slotCursorPositionChanged(0, 2);
    SUCCEED();
}

TEST_F(test_cdateedit, slotCursorPositionChanged_002)
{
    mWidget->setLunarCalendarStatus(false);
    mWidget->slotCursorPositionChanged(0, 2);
    SUCCEED();
}

TEST_F(test_cdateedit, slotCursorPositionChanged_003)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->selectAll();
    mWidget->slotCursorPositionChanged(2, 4);
    SUCCEED();
}

//void CDateEdit::slotSelectionChanged()
TEST_F(test_cdateedit, slotSelectionChanged_001)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->selectAll();
    mWidget->slotSelectionChanged();
    SUCCEED();
}

TEST_F(test_cdateedit, slotSelectionChanged_002)
{
    mWidget->setLunarCalendarStatus(false);
    mWidget->selectAll();
    mWidget->slotSelectionChanged();
    SUCCEED();
}

//QString CDateEdit::getLunarName(const QDate &date)
TEST_F(test_cdateedit, getLunarName_001)
{
    mWidget->setLunarCalendarStatus(false);
    mWidget->getLunarName(QDate(2022, 4, 20));
    SUCCEED();
}

TEST_F(test_cdateedit, getLunarName_002)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->getLunarName(QDate(2022, 4, 20));
    SUCCEED();
}

//void CDateEdit::setLineEditTextFormat(QLineEdit *lineEdit, const QList<QTextLayout::FormatRange> &formats)
TEST_F(test_cdateedit, setLineEditTextFormat_001)
{
    mWidget->setLineEditTextFormat(mWidget->lineEdit(), QList<QTextLayout::FormatRange>());
    SUCCEED();
}

TEST_F(test_cdateedit, setLineEditTextFormat_nullLineEdit)
{
    mWidget->setLineEditTextFormat(nullptr, QList<QTextLayout::FormatRange>());
    SUCCEED();
}

TEST_F(test_cdateedit, setLineEditTextFormat_withFormats)
{
    QList<QTextLayout::FormatRange> formats;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = 4;
    range.format.setFontWeight(QFont::Bold);
    formats.append(range);
    mWidget->setLineEditTextFormat(mWidget->lineEdit(), formats);
    SUCCEED();
}

//void CDateEdit::changeEvent(QEvent *e)
TEST_F(test_cdateedit, changeEvent_fontChange_withLunar)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    QEvent event(QEvent::FontChange);
    QApplication::sendEvent(mWidget, &event);
    SUCCEED();
}

TEST_F(test_cdateedit, changeEvent_fontChange_noLunar)
{
    mWidget->setLunarCalendarStatus(false);
    QEvent event(QEvent::FontChange);
    QApplication::sendEvent(mWidget, &event);
    SUCCEED();
}

TEST_F(test_cdateedit, changeEvent_otherEvent)
{
    QEvent event(QEvent::Resize);
    QApplication::sendEvent(mWidget, &event);
    SUCCEED();
}

//bool CDateEdit::showGongli() — private, called via slotDateEidtInfo when lunar enabled
TEST_F(test_cdateedit, showGongli_viaSetDate)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->setFixedSize(200, 30);
    mWidget->setDate(QDate(2022, 4, 20));
    SUCCEED();
}

//void CDateEdit::updateCalendarWidget()
TEST_F(test_cdateedit, updateCalendarWidget_001)
{
    mWidget->setCalendarPopup(true);
    mWidget->setLunarCalendarStatus(true);
    SUCCEED();
}

TEST_F(test_cdateedit, updateCalendarWidget_002)
{
    mWidget->setCalendarPopup(true);
    mWidget->setLunarCalendarStatus(false);
    SUCCEED();
}

TEST_F(test_cdateedit, updateCalendarWidget_003)
{
    mWidget->setCalendarPopup(false);
    mWidget->setLunarCalendarStatus(true);
    SUCCEED();
}

// CDateEdit constructor lambda: editingFinished signal → slotRefreshLineEditTextFormat
TEST_F(test_cdateedit, editingFinished_lambda)
{
    mWidget->setLunarCalendarStatus(true);
    mWidget->setDisplayFormat("yyyy/MM/dd");
    mWidget->setFixedSize(200, 30);
    mWidget->setDate(QDate(2022, 4, 20));
    emit mWidget->lineEdit()->editingFinished();
    SUCCEED();
}
