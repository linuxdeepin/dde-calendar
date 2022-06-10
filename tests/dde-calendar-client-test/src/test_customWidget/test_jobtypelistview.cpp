/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
//#include "test_jobtypelistview.h"
//#include "cscheduleoperation.h"
//#include "../dialog_stub.h"

//static int jobtypelistview_stub_true(int)
//{
//    return 1;
//}

//test_jobtypelistview::test_jobtypelistview()
//{

//}

//TEST_F(test_jobtypelistview, canAdd_001)
//{
//    mWidget->canAdd();
//}

//TEST_F(test_jobtypelistview, viewportEvent_001)
//{
//    QHoverEvent e(QEvent::HoverLeave, QPointF(0, 0), QPointF(0, 0));
//    mWidget->viewportEvent(&e);
//}

//TEST_F(test_jobtypelistview, viewportEvent_002)
//{
//    QHoverEvent e(QEvent::HoverLeave, QPointF(0, 0), QPointF(0, 0));
//    mWidget->m_iIndexCurrentHover = 0;
//    mWidget->viewportEvent(&e);
//}

//TEST_F(test_jobtypelistview, viewportEvent_003)
//{
//    QHoverEvent e(QEvent::HoverMove, QPointF(1, 1), QPointF(1, 1));
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->viewportEvent(&e);
//}

//TEST_F(test_jobtypelistview, initUI_001)
//{
//    mWidget->initUI();
//}

//TEST_F(test_jobtypelistview, addJobTypeItem_001)
//{
//    mWidget->addJobTypeItem(JobTypeInfo());
//}

//TEST_F(test_jobtypelistview, slotUpdateJobType_001)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    mWidget->slotUpdateJobType();
//}

//TEST_F(test_jobtypelistview, slotDeleteJobType_001)
//{
//    mWidget->slotDeleteJobType();
//}

//TEST_F(test_jobtypelistview, slotDeleteJobType_002)
//{
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->slotDeleteJobType();
//}

//TEST_F(test_jobtypelistview, slotDeleteJobType_003)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    stub.set(ADDR(CScheduleOperation, isJobTypeUsed), jobtypelistview_stub_true);
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->slotDeleteJobType();
//}

//TEST_F(test_jobtypelistview, updateJobType_003)
//{
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->updateJobType();
//    EXPECT_EQ(mWidget->m_iIndexCurrentHover, -1);
//}
