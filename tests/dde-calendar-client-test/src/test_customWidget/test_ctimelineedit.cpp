// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ctimelineedit.h"

#include <QSignalSpy>
#include <QLineEdit>
#include <QTest>

test_ctimelineedit::test_ctimelineedit()
{

}

// Constructor: verify object creation, id stored, default state
TEST_F(test_ctimelineedit, constructor)
{
    CTimeLineEdit *edit = new CTimeLineEdit(10);
    EXPECT_NE(edit, nullptr);
    EXPECT_EQ(edit->m_id, 10);
    EXPECT_EQ(edit->m_num, 0);
    delete edit;
}

// Constructor with default id
TEST_F(test_ctimelineedit, constructor_default_id)
{
    CTimeLineEdit *edit = new CTimeLineEdit();
    EXPECT_NE(edit, nullptr);
    EXPECT_EQ(edit->m_id, 0);
    delete edit;
}

// initView: called by constructor, verify embedded style enabled (no crash)
TEST_F(test_ctimelineedit, initView)
{
    // initView is called in constructor; verify widget is functional
    EXPECT_GE(mWidget->minimum(), 0);
    EXPECT_LE(mWidget->maximum(), 100);
}

// setRange: set custom range and verify
TEST_F(test_ctimelineedit, setRange)
{
    mWidget->setRange(1, 31);
    EXPECT_EQ(mWidget->minimum(), 1);
    EXPECT_EQ(mWidget->maximum(), 31);
}

// setNum(int): basic set within default range
TEST_F(test_ctimelineedit, setNum_basic)
{
    mWidget->setRange(0, 99);
    QSignalSpy spy(mWidget, &CTimeLineEdit::signalNumChange);
    mWidget->setNum(50);
    EXPECT_EQ(mWidget->m_num, 50);
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    EXPECT_EQ(args.at(0).toInt(), 5);  // m_id
    EXPECT_EQ(args.at(1).toInt(), 50); // m_num
}

// setNum(int): value below minimum gets clamped in m_num
TEST_F(test_ctimelineedit, setNum_below_minimum)
{
    mWidget->setRange(1, 31);
    QSignalSpy spy(mWidget, &CTimeLineEdit::signalNumChange);
    mWidget->setNum(-5);
    EXPECT_EQ(mWidget->m_num, 1); // clamped to minimum
    EXPECT_EQ(spy.count(), 1);
}

// setNum(int): value above maximum gets clamped in m_num
TEST_F(test_ctimelineedit, setNum_above_maximum)
{
    mWidget->setRange(1, 31);
    QSignalSpy spy(mWidget, &CTimeLineEdit::signalNumChange);
    mWidget->setNum(100);
    EXPECT_EQ(mWidget->m_num, 31); // clamped to maximum
    EXPECT_EQ(spy.count(), 1);
}

// setNum(int, bool): canCarry=false delegates to setNum(int)
TEST_F(test_ctimelineedit, setNum_carry_false)
{
    mWidget->setRange(0, 99);
    QSignalSpy spy(mWidget, &CTimeLineEdit::signalNumChange);
    mWidget->setNum(42, false);
    EXPECT_EQ(mWidget->m_num, 42);
    EXPECT_EQ(spy.count(), 1);
}

// setNum(int, bool): canCarry=true, in range
TEST_F(test_ctimelineedit, setNum_carry_true_in_range)
{
    mWidget->setRange(0, 99);
    mWidget->setNum(10);
    QSignalSpy spyNum(mWidget, &CTimeLineEdit::signalNumChange);
    QSignalSpy spyJump(mWidget, &CTimeLineEdit::signalDateJump);
    mWidget->setNum(50, true);
    EXPECT_EQ(mWidget->m_num, 50);
    EXPECT_EQ(spyNum.count(), 1);
    EXPECT_EQ(spyJump.count(), 0);
}

// setNum(int, bool): canCarry=true, above max triggers signalDateJump
TEST_F(test_ctimelineedit, setNum_carry_true_above_max)
{
    mWidget->setRange(0, 31);
    mWidget->setNum(15);
    QSignalSpy spyNum(mWidget, &CTimeLineEdit::signalNumChange);
    QSignalSpy spyJump(mWidget, &CTimeLineEdit::signalDateJump);
    mWidget->setNum(35, true);
    EXPECT_EQ(spyNum.count(), 0);
    EXPECT_EQ(spyJump.count(), 1);
    QList<QVariant> args = spyJump.takeFirst();
    EXPECT_EQ(args.at(0).toInt(), 5);     // m_id
    EXPECT_EQ(args.at(1).toInt(), 20);    // 35 - 15 = 20
}

// setNum(int, bool): canCarry=true, below min triggers signalDateJump
TEST_F(test_ctimelineedit, setNum_carry_true_below_min)
{
    mWidget->setRange(1, 31);
    mWidget->setNum(10);
    QSignalSpy spyNum(mWidget, &CTimeLineEdit::signalNumChange);
    QSignalSpy spyJump(mWidget, &CTimeLineEdit::signalDateJump);
    mWidget->setNum(-5, true);
    EXPECT_EQ(spyNum.count(), 0);
    EXPECT_EQ(spyJump.count(), 1);
    QList<QVariant> args = spyJump.takeFirst();
    EXPECT_EQ(args.at(0).toInt(), 5);      // m_id
    EXPECT_EQ(args.at(1).toInt(), -15);    // -5 - 10 = -15
}

// setStepEnabled / stepEnabled: set and verify
TEST_F(test_ctimelineedit, setStepEnabled)
{
    mWidget->setStepEnabled(CTimeLineEdit::StepUpEnabled);
    EXPECT_EQ(mWidget->stepEnabled(), CTimeLineEdit::StepUpEnabled);

    mWidget->setStepEnabled(CTimeLineEdit::StepDownEnabled);
    EXPECT_EQ(mWidget->stepEnabled(), CTimeLineEdit::StepDownEnabled);

    mWidget->setStepEnabled(CTimeLineEdit::StepUpEnabled | CTimeLineEdit::StepDownEnabled);
    EXPECT_EQ(mWidget->stepEnabled(), CTimeLineEdit::StepUpEnabled | CTimeLineEdit::StepDownEnabled);
}

// slotEditingFinished: set value then call slot
TEST_F(test_ctimelineedit, slotEditingFinished)
{
    mWidget->setRange(0, 99);
    QSignalSpy spy(mWidget, &CTimeLineEdit::signalNumChange);
    mWidget->setValue(30);
    mWidget->slotEditingFinished();
    EXPECT_EQ(mWidget->m_num, 30);
    EXPECT_EQ(spy.count(), 1);
}

// slotTextEdited: digits only
TEST_F(test_ctimelineedit, slotTextEdited_digits)
{
    mWidget->lineEdit()->setText("123");
    mWidget->slotTextEdited("123");
    EXPECT_EQ(mWidget->lineEdit()->text().toStdString(), "123");
}

// slotTextEdited: non-digit chars filtered out
TEST_F(test_ctimelineedit, slotTextEdited_non_digits)
{
    mWidget->lineEdit()->setText("12");
    mWidget->slotTextEdited("a1b2c3");
    EXPECT_EQ(mWidget->lineEdit()->text().toStdString(), "123");
}

// slotTextEdited: value zero results in empty string
TEST_F(test_ctimelineedit, slotTextEdited_zero)
{
    mWidget->lineEdit()->setText("0");
    mWidget->slotTextEdited("0");
    EXPECT_EQ(mWidget->lineEdit()->text().toStdString(), "");
}

// slotTextEdited: leading zeros stripped
TEST_F(test_ctimelineedit, slotTextEdited_leading_zeros)
{
    mWidget->lineEdit()->setText("5");
    mWidget->slotTextEdited("005");
    EXPECT_EQ(mWidget->lineEdit()->text().toStdString(), "5");
}

// slotTextEdited: all non-digit results in zero -> empty
TEST_F(test_ctimelineedit, slotTextEdited_all_non_digit)
{
    mWidget->lineEdit()->setText("");
    mWidget->slotTextEdited("abc");
    EXPECT_EQ(mWidget->lineEdit()->text().toStdString(), "");
}

// stepBy: positive steps
TEST_F(test_ctimelineedit, stepBy_positive)
{
    mWidget->setRange(0, 99);
    mWidget->setValue(10);
    QSignalSpy spy(mWidget, &CTimeLineEdit::signalNumChange);
    mWidget->stepBy(5);
    EXPECT_EQ(mWidget->m_num, 15);
    EXPECT_GE(spy.count(), 1);
}

// stepBy: negative steps
TEST_F(test_ctimelineedit, stepBy_negative)
{
    mWidget->setRange(0, 99);
    mWidget->setValue(20);
    QSignalSpy spy(mWidget, &CTimeLineEdit::signalNumChange);
    mWidget->stepBy(-5);
    EXPECT_EQ(mWidget->m_num, 15);
    EXPECT_GE(spy.count(), 1);
}

// stepBy: steps that exceed range with carry
TEST_F(test_ctimelineedit, stepBy_carry)
{
    mWidget->setRange(0, 31);
    mWidget->setValue(28);
    QSignalSpy spyJump(mWidget, &CTimeLineEdit::signalDateJump);
    mWidget->stepBy(10);
    EXPECT_EQ(spyJump.count(), 1);
}
