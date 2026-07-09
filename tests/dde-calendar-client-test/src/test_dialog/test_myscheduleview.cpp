// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_myscheduleview.h"

#include "lunarmanager.h"
#include "units.h"

#include "../dialog_stub.h"
#include "../third-party_stub/stub.h"

#include <QApplication>
#include <QEvent>

namespace {
bool stubNonChineseEnv()
{
    return false;
}

QString stubHuangLiShortName(void *obj, const QDate &date)
{
    Q_UNUSED(obj)
    Q_UNUSED(date)

    return QStringLiteral("六月初十");
}

DSchedule::Ptr createLunarSchedule()
{
    DSchedule::Ptr schedule(new DSchedule);
    schedule->setSummary(QStringLiteral("My Event"));
    schedule->setDtStart(QDateTime(QDate(2026, 7, 23), QTime(0, 0)));
    schedule->setDtEnd(QDateTime(QDate(2026, 7, 23), QTime(18, 45)));
    schedule->setAllDay(false);
    schedule->setLunnar(true);
    schedule->setUid(QStringLiteral("test-lunar-schedule"));
    schedule->setScheduleTypeID(QStringLiteral("1"));
    return schedule;
}
}

test_myscheduleview::test_myscheduleview()
{
}

test_myscheduleview::~test_myscheduleview()
{
    delete mScheduleView;
    mScheduleView = nullptr;
}

TEST_F(test_myscheduleview, lunarScheduleDateOmitsLunarNameOutsideChineseEnv)
{
    Stub stub;
    stub.set(isChineseEnv, stubNonChineseEnv);
    stub.set(ADDR(LunarManager, getHuangLiShortName), stubHuangLiShortName);

    mScheduleView = new CMyScheduleView(createLunarSchedule());

    EXPECT_EQ(QString("2026/07/23"),
              mScheduleView->getDataByFormat(QDate(2026, 7, 23), QString("yyyy/MM/dd")));
}

TEST_F(test_myscheduleview, FontChange)
{
    mScheduleView = new CMyScheduleView(createLunarSchedule());

    QEvent event(QEvent::FontChange);
    QApplication::sendEvent(mScheduleView, &event);
}

TEST_F(test_myscheduleview, slotBtClick)
{
    mScheduleView = new CMyScheduleView(createLunarSchedule());

    Stub stub;
    calendarDDialogExecStub(stub);
    mScheduleView->slotBtClick(1, "tt");
}
