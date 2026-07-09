// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_timeedit.h"
#include "calendarmanage.h"

#include <QPixmap>
#include <QFocusEvent>
#include <QApplication>

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

TEST_F(test_timeedit, usesTimeShowTypeForInitialTimeFormat)
{
    const QString oldTimeFormat = CalendarManager::getInstance()->getTimeFormat();
    const int oldTimeShowType = CalendarManager::getInstance()->getTimeShowType();
    CalendarManager::getInstance()->setTimeFormatChanged(0);
    CalendarManager::getInstance()->setTimeShowType(1, false);
    CTimeEdit timeEdit;

    timeEdit.setTime(QTime(18, 0));

    EXPECT_EQ(timeEdit.lineEdit()->text(), QTime(18, 0).toString("AP h:mm"));

    CalendarManager::getInstance()->setTimeFormatChanged(oldTimeFormat == "hh:mm" ? 1 : 0);
    CalendarManager::getInstance()->setTimeShowType(oldTimeShowType, false);
}

TEST_F(test_timeedit, keepsTimeShowTypeWhenShortTimeFormatChanges)
{
    const QString oldTimeFormat = CalendarManager::getInstance()->getTimeFormat();
    const int oldTimeShowType = CalendarManager::getInstance()->getTimeShowType();
    CalendarManager::getInstance()->setTimeShowType(1, false);
    CTimeEdit timeEdit;
    timeEdit.setTime(QTime(18, 0));

    timeEdit.setTimeFormat(0);

    EXPECT_EQ(timeEdit.lineEdit()->text(), QTime(18, 0).toString("AP h:mm"));

    CalendarManager::getInstance()->setTimeFormatChanged(oldTimeFormat == "hh:mm" ? 1 : 0);
    CalendarManager::getInstance()->setTimeShowType(oldTimeShowType, false);
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
    mTimeEdit->showPopup();
}
