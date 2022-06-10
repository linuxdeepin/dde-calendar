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
#include "test_calldayscheduleitem.h"

#include "../testscheduledata.h"

#include <QPainter>

//test_CAlldayscheduleitem::test_CAlldayscheduleitem()
//{
//}

//void test_CAlldayscheduleitem::SetUp()
//{
//    m_rectF.setRect(0, 0, 200, 50);
//    m_allItem = new CAllDayScheduleItem(m_rectF);
//}

//void test_CAlldayscheduleitem::TearDown()
//{
//    delete m_allItem;
//    m_allItem = nullptr;
//}

////paintBackground
//TEST_F(test_CAlldayscheduleitem, paintBackground)
//{
//    m_allItem->setData(TestDataInfo::getScheduleItemDInfo().first());
//    QPixmap pixmap(m_rectF.toRect().size());
//    pixmap.fill(Qt::transparent);
//    QPainter painter(&pixmap);
//    m_allItem->setPressSchedule(TestDataInfo::getScheduleItemDInfo().first());
//    m_allItem->m_vSelectflag = true;
//    m_allItem->paintBackground(&painter, pixmap.rect(), true);
//    m_allItem->m_vHoverflag = true;
//    m_allItem->m_vSelectflag = false;
//    m_allItem->paintBackground(&painter, pixmap.rect(), true);
//}

////hasSelectSchedule
//TEST_F(test_CAlldayscheduleitem, hasSelectSchedule)
//{
//    m_allItem->setData(TestDataInfo::getScheduleItemDInfo().first());

//    bool hasSelectInfo = m_allItem->hasSelectSchedule(TestDataInfo::getScheduleItemDInfo().first());
//    ASSERT_TRUE(hasSelectInfo);
//}
