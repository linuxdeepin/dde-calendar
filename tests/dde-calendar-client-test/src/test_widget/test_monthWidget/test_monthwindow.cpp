/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
//#include "test_monthwindow.h"

//#include "../third-party_stub/stub.h"
//#include "monthWidget/monthview.h"

//test_monthWindow::test_monthWindow()
//{
//}

//void test_monthWindow::SetUp()
//{
//    m_monthWindow = new CMonthWindow(nullptr);
//}

//void test_monthWindow::TearDown()
//{
//    delete m_monthWindow;
//    m_monthWindow = nullptr;
//}

//TEST_F(test_monthWindow, setTheMe)
//{
//    m_monthWindow->setTheMe(0);
//    m_monthWindow->setTheMe(1);
//    m_monthWindow->setTheMe(2);
//}

//namespace MonthWindow {
//void deleteselectschedule_Stub(void *obj)
//{
//    Q_UNUSED(obj)
//}
//} // namespace MonthWindow

//TEST_F(test_monthWindow, deleteselectSchedule)
//{
//    Stub stub;
//    stub.set(ADDR(CMonthView, deleteSelectSchedule), MonthWindow::deleteselectschedule_Stub);
//    m_monthWindow->deleteselectSchedule();
//}

//TEST_F(test_monthWindow, previousMonth)
//{
//    QDate currentDate = QDate::currentDate();
//    m_monthWindow->setSelectDate(currentDate);
//    m_monthWindow->previousMonth();
//    ASSERT_EQ(m_monthWindow->getSelectDate(), currentDate.addMonths(1));
//}

//TEST_F(test_monthWindow, nextMonth)
//{
//    QDate currentDate = QDate::currentDate();
//    m_monthWindow->setSelectDate(currentDate);
//    m_monthWindow->nextMonth();
//    ASSERT_EQ(m_monthWindow->getSelectDate(), currentDate.addMonths(-1));
//}

//TEST_F(test_monthWindow, slotViewSelectDate)
//{
//    QDate currentDate = QDate::currentDate();
//    m_monthWindow->slotViewSelectDate(currentDate);
//}

//TEST_F(test_monthWindow, resizeEvent)
//{
//    m_monthWindow->setFixedSize(500, 300);
//    m_monthWindow->setFixedSize(600, 400);
//}

//TEST_F(test_monthWindow, slottoday)
//{
//    m_monthWindow->slottoday();
//}
