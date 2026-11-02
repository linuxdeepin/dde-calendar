// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_schedulesearchview.h"
#include "../third-party_stub/stub.h"
#include "../third-party_stub/addr_pri.h"
#include "../dialog_stub.h"
#include "dschedule.h"

#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QEnterEvent>
#include <QFont>
#include <QColor>
#include <QPixmap>
#include <QTest>

static DSchedule::Ptr createTestSchedule()
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary("test schedule");
    schedule->setDtStart(QDateTime::currentDateTime());
    schedule->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    schedule->setScheduleTypeID("1");
    return schedule;
}

test_schedulesearchview::test_schedulesearchview()
{
}

test_schedulesearchview::~test_schedulesearchview()
{
}

void test_schedulesearchview::SetUp()
{
    mScheduleSearchItem = new CScheduleSearchItem();
    mScheduleSearchDateItem = new CScheduleSearchDateItem();
    mScheduleListWidget = new CScheduleListWidget();
    mScheduleSearchView = new CScheduleSearchView();
}

void test_schedulesearchview::TearDown()
{
    delete mScheduleSearchView;
    mScheduleSearchView = nullptr;
    delete mScheduleListWidget;
    mScheduleListWidget = nullptr;
    delete mScheduleSearchDateItem;
    mScheduleSearchDateItem = nullptr;
    delete mScheduleSearchItem;
    mScheduleSearchItem = nullptr;
}

// ==================== CScheduleSearchItem tests ====================

// Constructor
TEST_F(test_schedulesearchview, CScheduleSearchItem_Constructor)
{
    CScheduleSearchItem *item = new CScheduleSearchItem();
    delete item;
}

// void setBackgroundColor(QColor color1)
TEST_F(test_schedulesearchview, CScheduleSearchItem_setBackgroundColor)
{
    mScheduleSearchItem->setBackgroundColor(QColor(255, 0, 0));
}

// void setText(QColor tColor, QFont font)
TEST_F(test_schedulesearchview, CScheduleSearchItem_setText)
{
    QFont font;
    mScheduleSearchItem->setText(QColor(0, 0, 255), font);
}

// void setTimeC(QColor tColor, QFont font)
TEST_F(test_schedulesearchview, CScheduleSearchItem_setTimeC)
{
    QFont font;
    mScheduleSearchItem->setTimeC(QColor(0, 255, 0), font);
}

// void setData(DSchedule::Ptr vScheduleInfo, QDate date)
TEST_F(test_schedulesearchview, CScheduleSearchItem_setData)
{
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());
    EXPECT_EQ(schedule, mScheduleSearchItem->getData());
}

// const DSchedule::Ptr &getData() const
TEST_F(test_schedulesearchview, CScheduleSearchItem_getData)
{
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());
    const DSchedule::Ptr &result = mScheduleSearchItem->getData();
    EXPECT_EQ(result, schedule);
}

// void setRoundtype(int rtype)
TEST_F(test_schedulesearchview, CScheduleSearchItem_setRoundtype)
{
    mScheduleSearchItem->setRoundtype(2);
}

// void setTheMe(int type)
TEST_F(test_schedulesearchview, CScheduleSearchItem_setTheMe)
{
    mScheduleSearchItem->setTheMe(0);
    mScheduleSearchItem->setTheMe(2);
}

// void setDurationSize(QFont font)
TEST_F(test_schedulesearchview, CScheduleSearchItem_setDurationSize)
{
    QFont font;
    mScheduleSearchItem->setDurationSize(font);
}

// void slotEdit()
TEST_F(test_schedulesearchview, CScheduleSearchItem_slotEdit)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());
    mScheduleSearchItem->slotEdit();
}

// void slotDelete()
TEST_F(test_schedulesearchview, CScheduleSearchItem_slotDelete)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    QWidget parentWidget;
    CScheduleSearchItem *item = new CScheduleSearchItem(&parentWidget);
    DSchedule::Ptr schedule = createTestSchedule();
    item->setData(schedule, QDate::currentDate());
    item->slotDelete();
    delete item;
}

// void slotTimeFormatChanged(int value)
TEST_F(test_schedulesearchview, CScheduleSearchItem_slotTimeFormatChanged)
{
    mScheduleSearchItem->slotTimeFormatChanged(0);
    mScheduleSearchItem->slotTimeFormatChanged(1);
}

// void slotSchotCutClicked()
TEST_F(test_schedulesearchview, CScheduleSearchItem_slotSchotCutClicked)
{
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());
    mScheduleSearchItem->slotSchotCutClicked();
}

// void paintEvent(QPaintEvent *e)
TEST_F(test_schedulesearchview, CScheduleSearchItem_paintEvent)
{
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());
    mScheduleSearchItem->resize(200, 50);
    mScheduleSearchItem->show();
    QTest::qWait(50);
    mScheduleSearchItem->hide();
}

// void contextMenuEvent(QContextMenuEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchItem_contextMenuEvent)
{
    Stub stub;
    calendarQMenuExecStub(stub);
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(10, 10));
    mScheduleSearchItem->contextMenuEvent(&event);
}

// void mouseDoubleClickEvent(QMouseEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchItem_mouseDoubleClickEvent)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());
    QMouseEvent event(QEvent::MouseButtonDblClick, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mScheduleSearchItem->mouseDoubleClickEvent(&event);
}

// void mousePressEvent(QMouseEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchItem_mousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    mScheduleSearchItem->mousePressEvent(&event);
}

// void mouseReleaseEvent(QMouseEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchItem_mouseReleaseEvent)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    mScheduleSearchItem->mouseReleaseEvent(&event);
}

// void enterEvent(QEvent *event) / enterEvent(QEnterEvent*)
TEST_F(test_schedulesearchview, CScheduleSearchItem_enterEvent)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QEnterEvent event(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
    mScheduleSearchItem->enterEvent(&event);
#else
    QEvent event(QEvent::Enter);
    mScheduleSearchItem->enterEvent(&event);
#endif
}

// void leaveEvent(QEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchItem_leaveEvent)
{
    QEvent event(QEvent::Leave);
    mScheduleSearchItem->leaveEvent(&event);
}

// bool eventFilter(QObject *o, QEvent *e)
TEST_F(test_schedulesearchview, CScheduleSearchItem_eventFilter)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mScheduleSearchItem->eventFilter(mScheduleSearchItem, &event);

    QEvent otherEvent(QEvent::None);
    mScheduleSearchItem->eventFilter(mScheduleSearchItem, &otherEvent);
}

// void focusInEvent(QFocusEvent *e)
TEST_F(test_schedulesearchview, CScheduleSearchItem_focusInEvent)
{
    QFocusEvent event(QEvent::FocusIn, Qt::TabFocusReason);
    mScheduleSearchItem->focusInEvent(&event);
}

// void focusOutEvent(QFocusEvent *e)
TEST_F(test_schedulesearchview, CScheduleSearchItem_focusOutEvent)
{
    QFocusEvent event(QEvent::FocusOut);
    mScheduleSearchItem->focusOutEvent(&event);
}

// void keyPressEvent(QKeyEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchItem_keyPressEvent)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchItem->setData(schedule, QDate::currentDate());

    QKeyEvent returnEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    mScheduleSearchItem->keyPressEvent(&returnEvent);

    QKeyEvent altMEvent(QEvent::KeyPress, Qt::Key_M, Qt::AltModifier);
    mScheduleSearchItem->keyPressEvent(&altMEvent);
}

// ==================== CScheduleSearchView tests ====================

// Constructor
TEST_F(test_schedulesearchview, CScheduleSearchView_Constructor)
{
    CScheduleSearchView *view = new CScheduleSearchView();
    delete view;
}

// void setTheMe(int type)
TEST_F(test_schedulesearchview, CScheduleSearchView_setTheMe)
{
    mScheduleSearchView->setTheMe(0);
    mScheduleSearchView->setTheMe(2);
}

// void clearSearch()
TEST_F(test_schedulesearchview, CScheduleSearchView_clearSearch)
{
    mScheduleSearchView->clearSearch();
}

// void setMaxWidth(const int w)
TEST_F(test_schedulesearchview, CScheduleSearchView_setMaxWidth)
{
    mScheduleSearchView->setMaxWidth(300);
}

// bool getHasScheduleShow()
TEST_F(test_schedulesearchview, CScheduleSearchView_getHasScheduleShow)
{
    mScheduleSearchView->getHasScheduleShow();
}

// bool getScheduleStatus()
TEST_F(test_schedulesearchview, CScheduleSearchView_getScheduleStatus)
{
    mScheduleSearchView->getScheduleStatus();
}

// void deleteSchedule()
TEST_F(test_schedulesearchview, CScheduleSearchView_deleteSchedule)
{
    mScheduleSearchView->deleteSchedule();
}

// void slotsetSearch(QString str)
TEST_F(test_schedulesearchview, CScheduleSearchView_slotsetSearch)
{
    mScheduleSearchView->slotsetSearch("test");
}

// void slotSelectSchedule(const DSchedule::Ptr &scheduleInfo)
TEST_F(test_schedulesearchview, CScheduleSearchView_slotSelectSchedule)
{
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleSearchView->slotSelectSchedule(schedule);
}

// void updateSearch()
TEST_F(test_schedulesearchview, CScheduleSearchView_updateSearch)
{
    mScheduleSearchView->updateSearch();
}

// void slotSelectCurrentItem(CScheduleSearchItem *item, bool itemFocusOut)
TEST_F(test_schedulesearchview, CScheduleSearchView_slotSelectCurrentItem)
{
    mScheduleSearchView->slotSelectCurrentItem(nullptr, false);
}

// void slotListWidgetClicked()
TEST_F(test_schedulesearchview, CScheduleSearchView_slotListWidgetClicked)
{
    mScheduleSearchView->slotListWidgetClicked();
}

// void slotScearedScheduleUpdate()
TEST_F(test_schedulesearchview, CScheduleSearchView_slotScearedScheduleUpdate)
{
    mScheduleSearchView->slotScearedScheduleUpdate();
}

// void resizeEvent(QResizeEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchView_resizeEvent)
{
    mScheduleSearchView->resize(400, 300);
    QResizeEvent event(QSize(400, 300), QSize(200, 200));
    mScheduleSearchView->resizeEvent(&event);
}

// void mousePressEvent(QMouseEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchView_mousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mScheduleSearchView->mousePressEvent(&event);
}

// void keyPressEvent(QKeyEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchView_keyPressEvent)
{
    QKeyEvent upEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    mScheduleSearchView->keyPressEvent(&upEvent);

    QKeyEvent downEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    mScheduleSearchView->keyPressEvent(&downEvent);
}

// void focusInEvent(QFocusEvent *e)
TEST_F(test_schedulesearchview, CScheduleSearchView_focusInEvent)
{
    QFocusEvent event(QEvent::FocusIn, Qt::TabFocusReason);
    mScheduleSearchView->focusInEvent(&event);
}

// ==================== createItemWidget coverage tests ====================
// Cover createItemWidget(QDate) and createItemWidget(DSchedule::Ptr, QDate, int)
// by populating m_vlistData directly and calling updateDateShow()

// Single schedule -> createItemWidget(DSchedule::Ptr, QDate, 1)
TEST_F(test_schedulesearchview, CScheduleSearchView_updateDateShow_singleSchedule)
{
    Stub stub;
    calendarDDialogExecStub(stub);

    DSchedule::Ptr schedule = createTestSchedule();
    QDate testDate = QDate::currentDate();
    schedule->setDtStart(QDateTime(testDate, QTime(10, 0)));
    schedule->setDtEnd(QDateTime(testDate, QTime(11, 0)));

    DSchedule::List list;
    list.append(schedule);
    mScheduleSearchView->m_vlistData[testDate] = list;

    mScheduleSearchView->updateDateShow();
    mScheduleSearchView->m_vlistData.clear();
}

// Two schedules -> createItemWidget(DSchedule::Ptr, QDate, 3) + (..., 2)
TEST_F(test_schedulesearchview, CScheduleSearchView_updateDateShow_twoSchedules)
{
    Stub stub;
    calendarDDialogExecStub(stub);

    QDate testDate = QDate::currentDate();
    DSchedule::Ptr sched1 = createTestSchedule();
    sched1->setDtStart(QDateTime(testDate, QTime(10, 0)));
    sched1->setDtEnd(QDateTime(testDate, QTime(11, 0)));
    DSchedule::Ptr sched2 = createTestSchedule();
    sched2->setSummary("second schedule");
    sched2->setDtStart(QDateTime(testDate, QTime(14, 0)));
    sched2->setDtEnd(QDateTime(testDate, QTime(15, 0)));

    DSchedule::List list;
    list.append(sched1);
    list.append(sched2);
    mScheduleSearchView->m_vlistData[testDate] = list;

    mScheduleSearchView->updateDateShow();
    mScheduleSearchView->m_vlistData.clear();
}

// Three+ schedules -> createItemWidget(DSchedule::Ptr, QDate, 3) + (..., 0) + (..., 2)
TEST_F(test_schedulesearchview, CScheduleSearchView_updateDateShow_threeSchedules)
{
    Stub stub;
    calendarDDialogExecStub(stub);

    QDate testDate = QDate::currentDate();
    DSchedule::List list;
    for (int i = 0; i < 3; ++i) {
        DSchedule::Ptr s = createTestSchedule();
        s->setSummary(QString("schedule %1").arg(i));
        s->setDtStart(QDateTime(testDate, QTime(8 + i, 0)));
        s->setDtEnd(QDateTime(testDate, QTime(9 + i, 0)));
        list.append(s);
    }
    mScheduleSearchView->m_vlistData[testDate] = list;

    mScheduleSearchView->updateDateShow();
    mScheduleSearchView->m_vlistData.clear();
}

// Cover no-results branch (empty m_vlistData)
TEST_F(test_schedulesearchview, CScheduleSearchView_updateDateShow_noResults)
{
    Stub stub;
    calendarDDialogExecStub(stub);

    mScheduleSearchView->updateDateShow();
}

// ==================== CScheduleSearchDateItem tests ====================

// Constructor
TEST_F(test_schedulesearchview, CScheduleSearchDateItem_Constructor)
{
    CScheduleSearchDateItem *item = new CScheduleSearchDateItem();
    delete item;
}

// void setBackgroundColor(QColor color1)
TEST_F(test_schedulesearchview, CScheduleSearchDateItem_setBackgroundColor)
{
    mScheduleSearchDateItem->setBackgroundColor(QColor(255, 0, 0));
}

// void setText(QColor tColor, QFont font)
TEST_F(test_schedulesearchview, CScheduleSearchDateItem_setText)
{
    QFont font;
    mScheduleSearchDateItem->setText(QColor(0, 0, 255), font);
}

// void setDate(QDate date)
TEST_F(test_schedulesearchview, CScheduleSearchDateItem_setDate)
{
    mScheduleSearchDateItem->setDate(QDate::currentDate());
}

// void paintEvent(QPaintEvent *e)
TEST_F(test_schedulesearchview, CScheduleSearchDateItem_paintEvent)
{
    mScheduleSearchDateItem->resize(200, 30);
    mScheduleSearchDateItem->show();
    QTest::qWait(50);
    mScheduleSearchDateItem->hide();
}

// void mousePressEvent(QMouseEvent *event)
TEST_F(test_schedulesearchview, CScheduleSearchDateItem_mousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mScheduleSearchDateItem->mousePressEvent(&event);
}

// ==================== CScheduleListWidget tests ====================

// Constructor
TEST_F(test_schedulesearchview, CScheduleListWidget_Constructor)
{
    CScheduleListWidget *widget = new CScheduleListWidget();
    delete widget;
}

// Destructor
TEST_F(test_schedulesearchview, CScheduleListWidget_Destructor)
{
    CScheduleListWidget *widget = new CScheduleListWidget();
    delete widget;
}

// void mousePressEvent(QMouseEvent *event)
TEST_F(test_schedulesearchview, CScheduleListWidget_mousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(10, 10), QPointF(10, 10),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    mScheduleListWidget->mousePressEvent(&event);
}

// void paintEvent(QPaintEvent *e)
TEST_F(test_schedulesearchview, CScheduleListWidget_paintEvent)
{
    mScheduleListWidget->resize(200, 200);
    mScheduleListWidget->show();
    QTest::qWait(50);
    mScheduleListWidget->hide();
}
