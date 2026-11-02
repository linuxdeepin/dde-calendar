// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_myscheduleview.h"

#include "../dialog_stub.h"

#include <QEvent>
#include <QApplication>
#include <QFont>
#include <QDate>
#include <QDateTime>

static DSchedule::Ptr createTestSchedule()
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Test Schedule");
    QDateTime dt = QDateTime::currentDateTime();
    schedule->setDtStart(dt);
    schedule->setDtEnd(dt.addDays(1));
    schedule->setAllDay(true);
    schedule->setLunnar(false);
    schedule->setScheduleTypeID("1");
    return schedule;
}

test_myscheduleview::test_myscheduleview()
{
    DSchedule::Ptr schedule = createTestSchedule();
    mScheduleView = new CMyScheduleView(schedule);
}

test_myscheduleview::~test_myscheduleview()
{
    delete mScheduleView;
    mScheduleView = nullptr;
}

//CMyScheduleView constructor
TEST_F(test_myscheduleview, Constructor)
{
    DSchedule::Ptr schedule = createTestSchedule();
    CMyScheduleView view(schedule);
}

//void CMyScheduleView::setLabelTextColor(const int type)
TEST_F(test_myscheduleview, setLabelTextColor)
{
    mScheduleView->setLabelTextColor(1);
    mScheduleView->setLabelTextColor(2);
}

//void CMyScheduleView::setPaletteTextColor(QWidget *widget, QColor textColor)
TEST_F(test_myscheduleview, setPaletteTextColor)
{
    QWidget *widget = new QWidget();
    mScheduleView->setPaletteTextColor(widget, QColor(255, 0, 0));
    mScheduleView->setPaletteTextColor(nullptr, QColor(255, 0, 0));
    delete widget;
}

//void CMyScheduleView::setSchedules(const DSchedule::Ptr &schduleInfo)
TEST_F(test_myscheduleview, setSchedules)
{
    DSchedule::Ptr schedule = createTestSchedule();
    schedule->setSummary("Updated Schedule");
    mScheduleView->setSchedules(schedule);
}

//QString CMyScheduleView::getDataByFormat(const QDate &date, QString format)
TEST_F(test_myscheduleview, getDataByFormat)
{
    QDate date = QDate::currentDate();
    QString result = mScheduleView->getDataByFormat(date, "yyyy-MM-dd");
    EXPECT_FALSE(result.isEmpty());
}

//void CMyScheduleView::updateDialogIcon()
TEST_F(test_myscheduleview, updateDialogIcon)
{
    mScheduleView->updateDialogIcon();
}

//void CMyScheduleView::updateDateTimeFormat()
TEST_F(test_myscheduleview, updateDateTimeFormat)
{
    mScheduleView->updateDateTimeFormat();
}

//void CMyScheduleView::updateFormat()
TEST_F(test_myscheduleview, updateFormat)
{
    mScheduleView->updateFormat();
}

//void CMyScheduleView::slotAutoFeed(const QFont &font)
TEST_F(test_myscheduleview, slotAutoFeed)
{
    QFont font;
    mScheduleView->slotAutoFeed(font);
}

//void CMyScheduleView::slotBtClick(int buttonIndex, const QString &buttonName)
TEST_F(test_myscheduleview, slotBtClick)
{
    Stub stub;
    calendarDDialogExecStub(stub);
    mScheduleView->slotBtClick(1, "edit");
}

//FontChange
TEST_F(test_myscheduleview, FontChange)
{
    QEvent event(QEvent::FontChange);
    QApplication::sendEvent(mScheduleView, &event);
}

//void CMyScheduleView::initUI()
TEST_F(test_myscheduleview, initUI)
{
    mScheduleView->initUI();
}

//void CMyScheduleView::initConnection()
TEST_F(test_myscheduleview, initConnection)
{
    mScheduleView->initConnection();
}

//void CMyScheduleView::slotAccountStateChange()
TEST_F(test_myscheduleview, slotAccountStateChange)
{
    mScheduleView->slotAccountStateChange();
}
