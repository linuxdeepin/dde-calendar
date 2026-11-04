// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_timejumpdialog.h"

test_timejumpdialog::test_timejumpdialog()
{
    mDialog = new TimeJumpDialog(DArrowRectangle::ArrowDirection::ArrowTop);
}

test_timejumpdialog::~test_timejumpdialog()
{
    delete mDialog;
    mDialog = nullptr;
}

// TimeJumpDialog(ArrowDirection direction, QWidget *parent = nullptr)
TEST_F(test_timejumpdialog, Constructor)
{
    TimeJumpDialog dialog(DArrowRectangle::ArrowDirection::ArrowTop);
}

// ~TimeJumpDialog()
TEST_F(test_timejumpdialog, Destructor)
{
    TimeJumpDialog *dialog = new TimeJumpDialog(DArrowRectangle::ArrowDirection::ArrowTop);
    delete dialog;
}

// void TimeJumpDialog::initView()
TEST_F(test_timejumpdialog, initView)
{
    mDialog->initView();
}

// void TimeJumpDialog::initConnect()
TEST_F(test_timejumpdialog, initConnect)
{
    mDialog->initConnect();
}

// void TimeJumpDialog::initData()
TEST_F(test_timejumpdialog, initData)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
}

// void TimeJumpDialog::showPopup(const QDate &date, int x, int y)
TEST_F(test_timejumpdialog, showPopup)
{
    mDialog->showPopup(QDate(2024, 6, 15), 100, 100);
}

// int TimeJumpDialog::getMaxDayNum()
TEST_F(test_timejumpdialog, getMaxDayNum_January)
{
    mDialog->m_date = QDate(2024, 1, 15);
    EXPECT_EQ(mDialog->getMaxDayNum(), 31);
}

TEST_F(test_timejumpdialog, getMaxDayNum_FebruaryNonLeap)
{
    mDialog->m_date = QDate(2023, 2, 15);
    EXPECT_EQ(mDialog->getMaxDayNum(), 28);
}

TEST_F(test_timejumpdialog, getMaxDayNum_FebruaryLeap)
{
    mDialog->m_date = QDate(2024, 2, 15);
    EXPECT_EQ(mDialog->getMaxDayNum(), 29);
}

TEST_F(test_timejumpdialog, getMaxDayNum_April)
{
    mDialog->m_date = QDate(2024, 4, 15);
    EXPECT_EQ(mDialog->getMaxDayNum(), 30);
}

TEST_F(test_timejumpdialog, getMaxDayNum_December)
{
    mDialog->m_date = QDate(2024, 12, 15);
    EXPECT_EQ(mDialog->getMaxDayNum(), 31);
}

// void TimeJumpDialog::resetEdietStepEnable()
TEST_F(test_timejumpdialog, resetEdietStepEnable_Normal)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
    mDialog->resetEdietStepEnable();
}

TEST_F(test_timejumpdialog, resetEdietStepEnable_MinBoundary)
{
    mDialog->m_date = QDate(1900, 1, 1);
    mDialog->initData();
    mDialog->resetEdietStepEnable();
}

TEST_F(test_timejumpdialog, resetEdietStepEnable_MaxBoundary)
{
    mDialog->m_date = QDate(2100, 12, 31);
    mDialog->initData();
    mDialog->resetEdietStepEnable();
}

// void TimeJumpDialog::slotEditNumChange(int id, int num)
TEST_F(test_timejumpdialog, slotEditNumChange_Year)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
    mDialog->slotEditNumChange(1, 2025);
    EXPECT_EQ(mDialog->m_date.year(), 2025);
}

TEST_F(test_timejumpdialog, slotEditNumChange_Month)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
    mDialog->slotEditNumChange(2, 7);
    EXPECT_EQ(mDialog->m_date.month(), 7);
}

TEST_F(test_timejumpdialog, slotEditNumChange_Day)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
    mDialog->slotEditNumChange(3, 20);
    EXPECT_EQ(mDialog->m_date.day(), 20);
}

// void TimeJumpDialog::slotDateJump(int id, int num)
TEST_F(test_timejumpdialog, slotDateJump_Year)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
    mDialog->slotDateJump(1, 1);
    EXPECT_EQ(mDialog->m_date.year(), 2025);
}

TEST_F(test_timejumpdialog, slotDateJump_Month)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
    mDialog->slotDateJump(2, 1);
    EXPECT_EQ(mDialog->m_date.month(), 7);
}

TEST_F(test_timejumpdialog, slotDateJump_Day)
{
    mDialog->m_date = QDate(2024, 6, 15);
    mDialog->initData();
    mDialog->slotDateJump(3, 1);
    EXPECT_EQ(mDialog->m_date.day(), 16);
}

// void TimeJumpDialog::slotJumpButtonClicked()
TEST_F(test_timejumpdialog, slotJumpButtonClicked)
{
    GTEST_SKIP() << "Requires CScheduleBaseWidget::setSelectDate which needs global state";
}

// void TimeJumpDialog::showPopup(const QDate &date, const QPoint &pos)
TEST_F(test_timejumpdialog, showPopup_QPoint)
{
    mDialog->showPopup(QDate(2024, 6, 15), QPoint(100, 100));
    SUCCEED();
}
