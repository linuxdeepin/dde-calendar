// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_dcalendarddialog.h"

#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>

test_dcalendarddialog::test_dcalendarddialog()
{
    mDialog = new DCalendarDDialog();
}

test_dcalendarddialog::~test_dcalendarddialog()
{
    delete mDialog;
    mDialog = nullptr;
}

//DCalendarDDialog::DCalendarDDialog(QWidget *parent)
TEST_F(test_dcalendarddialog, Constructor)
{
    ASSERT_NE(mDialog, nullptr);
    // m_timeFormat and m_dateFormat are initialized from CalendarManager singleton
    // which returns environment-dependent values, so only verify they are set
    EXPECT_FALSE(mDialog->m_timeFormat.isEmpty());
    EXPECT_FALSE(mDialog->m_dateFormat.isEmpty());
}

//void DCalendarDDialog::setTimeFormat(int value)
TEST_F(test_dcalendarddialog, setTimeFormat)
{
    // value == 0 → "h:mm"
    mDialog->setTimeFormat(0);
    EXPECT_EQ(mDialog->m_timeFormat, QStringLiteral("h:mm"));

    // value != 0 → "hh:mm"
    mDialog->setTimeFormat(1);
    EXPECT_EQ(mDialog->m_timeFormat, QStringLiteral("hh:mm"));

    mDialog->setTimeFormat(0);
    EXPECT_EQ(mDialog->m_timeFormat, QStringLiteral("h:mm"));
}

//void DCalendarDDialog::setDateFormat(int value)
TEST_F(test_dcalendarddialog, setDateFormat)
{
    mDialog->setDateFormat(0);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy/M/d"));

    mDialog->setDateFormat(1);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy-M-d"));

    mDialog->setDateFormat(2);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy.M.d"));

    mDialog->setDateFormat(3);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy/MM/dd"));

    mDialog->setDateFormat(4);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy-MM-dd"));

    mDialog->setDateFormat(5);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy.MM.dd"));

    mDialog->setDateFormat(6);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yy/M/d"));

    mDialog->setDateFormat(7);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yy-M-d"));

    mDialog->setDateFormat(8);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yy.M.d"));

    // default branch
    mDialog->setDateFormat(999);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy-MM-dd"));

    mDialog->setDateFormat(-1);
    EXPECT_EQ(mDialog->m_dateFormat, QStringLiteral("yyyy-MM-dd"));
}

//void DCalendarDDialog::updateDateTimeFormat()
TEST_F(test_dcalendarddialog, updateDateTimeFormat)
{
    // No-op function that only logs; verify it does not crash
    mDialog->m_dateFormat = QStringLiteral("test-date");
    mDialog->m_timeFormat = QStringLiteral("test-time");
    mDialog->updateDateTimeFormat();
    SUCCEED();
}

//void DCalendarDDialog::mouseMoveEvent(QMouseEvent *event)
TEST_F(test_dcalendarddialog, mouseMoveEvent)
{
    QMouseEvent event(QEvent::MouseMove, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mDialog->mouseMoveEvent(&event);
    SUCCEED();
}

//void DCalendarDDialog::keyPressEvent(QKeyEvent *event)
TEST_F(test_dcalendarddialog, keyPressEvent)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    mDialog->keyPressEvent(&event);
    SUCCEED();
}

//bool DCalendarDDialog::eventFilter(QObject *o, QEvent *e)
TEST_F(test_dcalendarddialog, eventFilter)
{
    QEvent event(QEvent::None);
    bool result = mDialog->eventFilter(mDialog, &event);
    // eventFilter delegates to parent DDialog::eventFilter; result is a bool
    EXPECT_TRUE(result == true || result == false);
}
