// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_daywindow.h"
#include <QTest>

test_daywindow::test_daywindow()
{
}

void test_daywindow::SetUp()
{
    m_dayWindow = new CDayWindow();
    QDateTime currentTime = QDateTime::currentDateTime();
    m_dayWindow->setCurrentDateTime(currentTime);
    m_dayWindow->setFixedSize(1200, 600);
}

void test_daywindow::TearDown()
{
    delete m_dayWindow;
    m_dayWindow = nullptr;
}

//CDayWindow::CDayWindow(QWidget *parent)
TEST_F(test_daywindow, constructor)
{
    CDayWindow *window = new CDayWindow();
    ASSERT_NE(window, nullptr);
    delete window;
}

//void CDayWindow::setTheMe(int type)
TEST_F(test_daywindow, setTheMe)
{
    m_dayWindow->setTheMe(0);
    m_dayWindow->setTheMe(1);
    m_dayWindow->setTheMe(2);
}

//void CDayWindow::setSearchWFlag(bool flag)
TEST_F(test_daywindow, setSearchWFlag)
{
    m_dayWindow->setSearchWFlag(true);
    m_dayWindow->setSearchWFlag(false);
}

//void CDayWindow::setLunarVisible(bool state)
TEST_F(test_daywindow, setLunarVisible)
{
    m_dayWindow->setLunarVisible(true);
    m_dayWindow->setLunarVisible(false);
}

//void CDayWindow::setTime(const QTime time)
TEST_F(test_daywindow, setTime)
{
    m_dayWindow->setTime(QTime(10, 30, 0));
    m_dayWindow->setTime(QTime());
}

//void CDayWindow::updateHeight()
TEST_F(test_daywindow, updateHeight)
{
    m_dayWindow->updateHeight();
}

//void CDayWindow::setCurrentDateTime(const QDateTime &currentDate)
TEST_F(test_daywindow, setCurrentDateTime)
{
    m_dayWindow->setCurrentDateTime(QDateTime(QDate(2025, 6, 15), QTime(14, 30)));
}

//void CDayWindow::setYearData()
TEST_F(test_daywindow, setYearData)
{
    m_dayWindow->setYearData();
}

//void CDayWindow::updateShowDate(const bool isUpdateBar)
TEST_F(test_daywindow, updateShowDate)
{
    m_dayWindow->updateShowDate(true);
    m_dayWindow->updateShowDate(false);
}

//void CDayWindow::updateShowSchedule()
TEST_F(test_daywindow, updateShowSchedule)
{
    m_dayWindow->updateShowSchedule();
}

//void CDayWindow::updateShowLunar()
TEST_F(test_daywindow, updateShowLunar)
{
    m_dayWindow->updateShowLunar();
}

//void CDayWindow::updateSearchScheduleInfo()
TEST_F(test_daywindow, updateSearchScheduleInfo)
{
    m_dayWindow->updateSearchScheduleInfo();
}

//void CDayWindow::setSelectSearchScheduleInfo(const DSchedule::Ptr &info)
//void CDayWindow::deleteselectSchedule()
TEST_F(test_daywindow, deleteselectSchedule)
{
    m_dayWindow->deleteselectSchedule();
}

//void CDayWindow::setSelectSearchScheduleInfo(const DSchedule::Ptr &info)
TEST_F(test_daywindow, setSelectSearchScheduleInfo)
{
    DSchedule::Ptr info(new DSchedule());
    info->setDtStart(QDateTime(QDate::currentDate(), QTime(9, 0)));
    info->setDtEnd(QDateTime(QDate::currentDate(), QTime(10, 0)));
    info->setSummary("Test Schedule");
    m_dayWindow->setSelectSearchScheduleInfo(info);
}

//void CDayWindow::slotScheduleHide()
TEST_F(test_daywindow, slotScheduleHide)
{
    m_dayWindow->slotScheduleHide();
}

//void CDayWindow::slotChangeSelectDate(const QDate &date)
TEST_F(test_daywindow, slotChangeSelectDate)
{
    m_dayWindow->slotChangeSelectDate(QDate::currentDate().addDays(1));
}

//void CDayWindow::slotIsDragging(bool &isDragging)
TEST_F(test_daywindow, slotIsDragging)
{
    bool isDragging = false;
    m_dayWindow->slotIsDragging(isDragging);
}

//void CDayWindow::slotSwitchPrePage()
TEST_F(test_daywindow, slotSwitchPrePage)
{
    m_dayWindow->slotSwitchPrePage();
}

//void CDayWindow::slotSwitchNextPage()
TEST_F(test_daywindow, slotSwitchNextPage)
{
    m_dayWindow->slotSwitchNextPage();
}
