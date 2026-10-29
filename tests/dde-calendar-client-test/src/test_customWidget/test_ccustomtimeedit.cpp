// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_ccustomtimeedit.h"

#include <QTest>
#include <QFocusEvent>
#include <QLineEdit>
#include <QKeyEvent>

void test_CCustomTimeEdit::SetUp()
{
    m_timeEdit = new CCustomTimeEdit();
}

void test_CCustomTimeEdit::TearDown()
{
    delete m_timeEdit;
    m_timeEdit = nullptr;
}

//CCustomTimeEdit::CCustomTimeEdit(QWidget *parent) — constructor
TEST_F(test_CCustomTimeEdit, constructor)
{
    CCustomTimeEdit *edit = new CCustomTimeEdit();
    EXPECT_NE(edit, nullptr);
    EXPECT_LE(edit->maximumWidth(), 80);
    delete edit;
}

//QLineEdit *CCustomTimeEdit::getLineEdit()
TEST_F(test_CCustomTimeEdit, getLineEdit)
{
    QLineEdit *edit = m_timeEdit->getLineEdit();
    EXPECT_NE(edit, nullptr);
    EXPECT_EQ(edit, m_timeEdit->lineEdit());
}

//mousePressEvent + mouseDoubleClickEvent
TEST_F(test_CCustomTimeEdit, mouseEventTest)
{
    QWidget *widget = new QWidget();
    m_timeEdit->setParent(widget);
    QTest::mousePress(m_timeEdit, Qt::LeftButton);
    QTest::mouseRelease(m_timeEdit, Qt::LeftButton);
    QTest::mouseDClick(m_timeEdit, Qt::LeftButton);
    QTest::mouseRelease(m_timeEdit, Qt::LeftButton);

    m_timeEdit->setParent(nullptr);
    delete widget;
}

//focusInEvent — emits signalUpdateFocus(true)
TEST_F(test_CCustomTimeEdit, focusInEvent)
{
    QFocusEvent focusEvent_in(QEvent::FocusIn, Qt::FocusReason::ActiveWindowFocusReason);
    QApplication::sendEvent(m_timeEdit, &focusEvent_in);
}

//focusOutEvent — emits signalUpdateFocus(false)
TEST_F(test_CCustomTimeEdit, focusOutEvent)
{
    QFocusEvent focusEvent_out(QEvent::FocusOut, Qt::FocusReason::TabFocusReason);
    QApplication::sendEvent(m_timeEdit, &focusEvent_out);
}

//keyPressEvent — Left/Right keys
TEST_F(test_CCustomTimeEdit, keyPressEvent_Left)
{
    QWidget *widget = new QWidget();
    m_timeEdit->setParent(widget);
    QKeyEvent keyevent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QApplication::sendEvent(m_timeEdit, &keyevent);
    m_timeEdit->setParent(nullptr);
    delete widget;
}

TEST_F(test_CCustomTimeEdit, keyPressEvent_Right)
{
    QWidget *widget = new QWidget();
    m_timeEdit->setParent(widget);
    QKeyEvent keyevent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(m_timeEdit, &keyevent);
    m_timeEdit->setParent(nullptr);
    delete widget;
}

TEST_F(test_CCustomTimeEdit, keyPressEvent_Other)
{
    QKeyEvent keyevent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QApplication::sendEvent(m_timeEdit, &keyevent);
}
