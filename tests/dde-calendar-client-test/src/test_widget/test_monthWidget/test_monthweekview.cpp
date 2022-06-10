/*
* Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     zhengxiaokang <zhengxiaokang@uniontech.com>
*
* Maintainer: zhengxiaokang <zhengxiaokang@uniontech.com>
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
#include "test_monthweekview.h"
#include <QDate>
#include <QPaintEvent>

//test_monthweekview::test_monthweekview()
//{
//    mMonthWeekView = new CMonthWeekView();
//    mWeekRect = new WeekRect();
//}

//test_monthweekview::~test_monthweekview()
//{
//    delete mMonthWeekView;
//    mMonthWeekView = nullptr;
//    delete mWeekRect;
//    mWeekRect = nullptr;
//}

////void CMonthWeekView::setFirstDay(const Qt::DayOfWeek weekday)
//TEST_F(test_monthweekview, setFirstDay)
//{
//    mMonthWeekView->setFirstDay(static_cast<Qt::DayOfWeek>(QDate::currentDate().dayOfWeek()));
//    mMonthWeekView->setFirstDay(static_cast<Qt::DayOfWeek>(QDate::currentDate().addDays(1).dayOfWeek()));
//}

////void CMonthWeekView::setTheMe(int type)
//TEST_F(test_monthweekview, setTheMe)
//{
//    mMonthWeekView->setTheMe(1);
//    mMonthWeekView->setTheMe(2);
//}

////void CMonthWeekView::setCurrentDate(const QDate &currentDate)
//TEST_F(test_monthweekview, setCurrentDate)
//{
//    mMonthWeekView->setCurrentDate(QDate::currentDate());
//}

//TEST_F(test_monthweekview, paintEvent_01)
//{
//    QPaintEvent e(QRect(0, 0, 1, 1));
//    mMonthWeekView->paintEvent(&e);
//}

////void WeekRect::setWeek(const Qt::DayOfWeek &showWeek, const bool &showLine)
//TEST_F(test_monthweekview, setWeek)
//{
//    mWeekRect->setWeek(Qt::Sunday, false);
//}

////void WeekRect::setRect(const QRectF &rectF)
//TEST_F(test_monthweekview, setRect)
//{
//    mWeekRect->setRect(QRectF());
//}

////void WeekRect::setTheMe(int type)
//TEST_F(test_monthweekview, setRectTheMe_01)
//{
//    mWeekRect->setTheMe(1);
//    EXPECT_EQ(mWeekRect->m_testColor, QColor("#6F6F6F"));
//}

////void WeekRect::setTheMe(int type)
//TEST_F(test_monthweekview, setRectTheMe_02)
//{
//    mWeekRect->setTheMe(3);
//    EXPECT_EQ(mWeekRect->m_testColor, QColor("#C0C6D4"));
//}
