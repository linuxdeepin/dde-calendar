// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_timeedit.h"

#include <QPixmap>
#include <QFocusEvent>
#include <QApplication>
#include <QTest>

test_timeedit::test_timeedit()
{
    mTimeEdit = new CTimeEdit();
}

test_timeedit::~test_timeedit()
{
    delete mTimeEdit;
    mTimeEdit = nullptr;
}

//void CTimeEdit::setTime(QTime time)
TEST_F(test_timeedit, setTime)
{
    mTimeEdit->setTime(QTime(23, 59, 59));
}

//QTime CTimeEdit::getTime()
TEST_F(test_timeedit, getTime)
{
    mTimeEdit->getTime();
}

//setTimeFormat
TEST_F(test_timeedit, setTimeFormat)
{
    mTimeEdit->setTimeFormat(1);
    mTimeEdit->setTimeFormat(0);
}

//slotFocusDraw
TEST_F(test_timeedit, slotFocusDraw)
{
    mTimeEdit->slotFocusDraw(false);
}

//getPixmap
TEST_F(test_timeedit, getPixmap)
{
    mTimeEdit->slotFocusDraw(true);
    mTimeEdit->setFixedSize(200, 50);
    QPixmap pixmap(mTimeEdit->size());
    mTimeEdit->render(&pixmap);
}

TEST_F(test_timeedit,focusInEvent)
{
    QFocusEvent focusEvent_in( QEvent::FocusIn,Qt::FocusReason::TabFocusReason);
    QApplication::sendEvent(mTimeEdit,&focusEvent_in);
}

TEST_F(test_timeedit, slotActivated_01)
{
    mTimeEdit->slotActivated("");
}

TEST_F(test_timeedit, slotEditingFinished_01)
{
    mTimeEdit->slotEditingFinished();
}

TEST_F(test_timeedit, initUI_01)
{
    mTimeEdit->initUI();
}

TEST_F(test_timeedit, initConnection_01)
{
    mTimeEdit->initConnection();
}

TEST_F(test_timeedit, showPopup_01)
{
    mTimeEdit->updateListItem(true);
    mTimeEdit->setFixedSize(200, 50);
    mTimeEdit->show();
    QTest::qWait(50);
    mTimeEdit->showPopup();
    QTest::qWait(50);
    mTimeEdit->close();
}

#include <QResizeEvent>
#include <QPaintEvent>
#include <QTest>

//void CTimeEdit::setMineTime(const QTime &mineTime)
TEST_F(test_timeedit, setMineTime)
{
    mTimeEdit->setMineTime(QTime(8, 0, 0));
}

//void CTimeEdit::setSelectItem(const QTime &time)
TEST_F(test_timeedit, setSelectItem)
{
    mTimeEdit->updateListItem(true);
    mTimeEdit->setSelectItem(QTime(8, 0, 0));
    mTimeEdit->setSelectItem(QTime(12, 30, 0));
    mTimeEdit->setSelectItem(QTime(0, 0, 0));
}

//void CTimeEdit::slotSetPlainText(const QString &arg)
TEST_F(test_timeedit, slotSetPlainText)
{
    mTimeEdit->updateListItem(true);
    mTimeEdit->setCurrentIndex(0);
    mTimeEdit->slotSetPlainText("08:00");
}

TEST_F(test_timeedit, slotSetPlainText_empty)
{
    mTimeEdit->slotSetPlainText("test");
}

//void CTimeEdit::updateListItem(bool isShowTimeInterval)
TEST_F(test_timeedit, updateListItem)
{
    mTimeEdit->updateListItem(true);
    mTimeEdit->updateListItem(false);
}

//void CTimeEdit::paintEvent(QPaintEvent *e)
TEST_F(test_timeedit, paintEvent_01)
{
    QPaintEvent e(QRect(0, 0, 100, 50));
    mTimeEdit->paintEvent(&e);
}

TEST_F(test_timeedit, paintEvent_02)
{
    mTimeEdit->m_hasFocus = true;
    QPaintEvent e(QRect(0, 0, 100, 50));
    mTimeEdit->paintEvent(&e);
}

//void CTimeEdit::resizeEvent(QResizeEvent *e)
TEST_F(test_timeedit, resizeEvent)
{
    mTimeEdit->setFixedSize(200, 50);
    QResizeEvent e(QSize(200, 50), QSize(100, 30));
    mTimeEdit->resizeEvent(&e);
}
