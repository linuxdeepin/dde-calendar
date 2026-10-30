// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_scheduleremindwidget.h"

#include <QPixmap>
#include <QPainter>
#include <QTest>
#include "dschedule.h"
#include <scheduledatamanage.h>
#include <QApplication>

static DSchedule::Ptr createTestSchedule(bool allDay = false)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Test Schedule Summary");
    schedule->setDtStart(QDateTime::currentDateTime());
    schedule->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    schedule->setAllDay(allDay);
    return schedule;
}

static CSchedulesColor getTestColor()
{
    return CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType("2");
}

test_scheduleremindwidget::test_scheduleremindwidget()
{
    mScheduleRemindWidget = new ScheduleRemindWidget();
    mCenterWidget = new CenterWidget();
}

test_scheduleremindwidget::~test_scheduleremindwidget()
{
    delete mScheduleRemindWidget;
    mScheduleRemindWidget = nullptr;
    delete mCenterWidget;
    mCenterWidget = nullptr;
}

// ===== ScheduleRemindWidget tests =====

// ScheduleRemindWidget::ScheduleRemindWidget / ~ScheduleRemindWidget
TEST_F(test_scheduleremindwidget, constructor)
{
    ScheduleRemindWidget *w = new ScheduleRemindWidget();
    EXPECT_NE(w, nullptr);
    delete w;
}

// void ScheduleRemindWidget::setData(const DSchedule::Ptr &vScheduleInfo, const CSchedulesColor &gcolor)
TEST_F(test_scheduleremindwidget, setData)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mScheduleRemindWidget->setData(schedule, color);
}

// void ScheduleRemindWidget::setData — all-day schedule
TEST_F(test_scheduleremindwidget, setData_allDay)
{
    DSchedule::Ptr schedule = createTestSchedule(true);
    CSchedulesColor color = getTestColor();
    mScheduleRemindWidget->setData(schedule, color);
}

// void ScheduleRemindWidget::setDirection(DArrowRectangle::ArrowDirection value)
TEST_F(test_scheduleremindwidget, setDirection)
{
    mScheduleRemindWidget->setDirection(DArrowRectangle::ArrowDirection::ArrowLeft);
    mScheduleRemindWidget->setDirection(DArrowRectangle::ArrowDirection::ArrowRight);
    mScheduleRemindWidget->setDirection(DArrowRectangle::ArrowDirection::ArrowTop);
    mScheduleRemindWidget->setDirection(DArrowRectangle::ArrowDirection::ArrowBottom);
}

// void ScheduleRemindWidget::setTimeFormat(QString timeformat)
TEST_F(test_scheduleremindwidget, setTimeFormat)
{
    mScheduleRemindWidget->setTimeFormat("h:mm AP");
    mScheduleRemindWidget->setTimeFormat("hh:mm");
}

// void ScheduleRemindWidget::show(int x, int y)
TEST_F(test_scheduleremindwidget, show)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mScheduleRemindWidget->setData(schedule, color);
    mScheduleRemindWidget->setFixedSize(300, 100);
    mScheduleRemindWidget->show(100, 100);
    QTest::qWait(50);
    mScheduleRemindWidget->hide();
}

// void ScheduleRemindWidget::updateArrowPosition() — private, called via show
TEST_F(test_scheduleremindwidget, updateArrowPosition)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mScheduleRemindWidget->setData(schedule, color);
    mScheduleRemindWidget->setFixedSize(300, 100);
    mScheduleRemindWidget->show(50, 50);
    QTest::qWait(50);
    mScheduleRemindWidget->hide();
}

// void ScheduleRemindWidget::updatePopupGeometry() — private, called via setData/setDirection
TEST_F(test_scheduleremindwidget, updatePopupGeometry)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mScheduleRemindWidget->setData(schedule, color);
    mScheduleRemindWidget->setDirection(DArrowRectangle::ArrowDirection::ArrowRight);
}

// Render ScheduleRemindWidget for paint coverage
TEST_F(test_scheduleremindwidget, render)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mScheduleRemindWidget->setData(schedule, color);
    mScheduleRemindWidget->setFixedSize(300, 200);
    QPixmap pixmap(mScheduleRemindWidget->size());
    mScheduleRemindWidget->render(&pixmap);
}

// ===== CenterWidget tests =====

// CenterWidget::CenterWidget / ~CenterWidget
TEST_F(test_scheduleremindwidget, centerWidget_constructor)
{
    CenterWidget *w = new CenterWidget();
    EXPECT_NE(w, nullptr);
    delete w;
}

// void CenterWidget::setData(const DSchedule::Ptr &vScheduleInfo, const CSchedulesColor &gcolor)
TEST_F(test_scheduleremindwidget, centerWidget_setData)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
}

// void CenterWidget::setData — all-day schedule
TEST_F(test_scheduleremindwidget, centerWidget_setData_allDay)
{
    DSchedule::Ptr schedule = createTestSchedule(true);
    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
}

// void CenterWidget::setTheMe(const int type)
TEST_F(test_scheduleremindwidget, centerWidget_setTheMe)
{
    mCenterWidget->setTheMe(0);
    mCenterWidget->setTheMe(1);
    mCenterWidget->setTheMe(2);
}

// void CenterWidget::updateFont()
TEST_F(test_scheduleremindwidget, centerWidget_updateFont_noData)
{
    mCenterWidget->updateFont();
}

// void CenterWidget::updateFont() — with data set
TEST_F(test_scheduleremindwidget, centerWidget_updateFont_withData)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
    mCenterWidget->updateFont();
}

// void CenterWidget::setTimeFormat(QString timeFormat)
TEST_F(test_scheduleremindwidget, centerWidget_setTimeFormat)
{
    mCenterWidget->setTimeFormat("h:mm AP");
    mCenterWidget->setTimeFormat("hh:mm");
}

// void CenterWidget::UpdateTextList() — private, needs valid m_ScheduleInfo
TEST_F(test_scheduleremindwidget, centerWidget_updateTextList)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
    mCenterWidget->UpdateTextList();
}

// void CenterWidget::paintEvent(QPaintEvent *e) — render after setting data
TEST_F(test_scheduleremindwidget, centerWidget_paintEvent)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
    mCenterWidget->setFixedSize(300, 200);
    QPixmap pixmap(mCenterWidget->size());
    mCenterWidget->render(&pixmap);
}

// void CenterWidget::paintEvent — all-day schedule
TEST_F(test_scheduleremindwidget, centerWidget_paintEvent_allDay)
{
    DSchedule::Ptr schedule = createTestSchedule(true);
    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
    mCenterWidget->setFixedSize(300, 200);
    QPixmap pixmap(mCenterWidget->size());
    mCenterWidget->render(&pixmap);
}

// void CenterWidget::paintEvent — with organizer/attendee info (long summary)
TEST_F(test_scheduleremindwidget, centerWidget_paintEvent_longSummary)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("This is a very long schedule summary that should trigger text wrapping "
                         "and multiple lines of text in the CenterWidget paint event for coverage");
    schedule->setDtStart(QDateTime::currentDateTime());
    schedule->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    schedule->setAllDay(false);
    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
    mCenterWidget->setFixedSize(300, 300);
    QPixmap pixmap(mCenterWidget->size());
    mCenterWidget->render(&pixmap);
}

// ===== Coverage for attendeeDisplayName + wrapDetailText + appendToolTipText lambdas =====

// CenterWidget::UpdateTextList with organizer and attendees (long names to trigger wrapping)
TEST_F(test_scheduleremindwidget, centerWidget_updateTextList_withAttendees)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("Meeting with team");
    schedule->setDtStart(QDateTime::currentDateTime());
    schedule->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    schedule->setAllDay(false);

    // Set organizer with a long name to trigger text wrapping in wrapDetailText
    schedule->setOrganizer(KCalendarCore::Person(
        QStringLiteral("Organizer With A Very Long Display Name That Wraps"),
        QStringLiteral("organizer@example.com")));

    // Add multiple attendees with long names to trigger wrapping + attendeeDisplayName
    schedule->addAttendee(KCalendarCore::Attendee(
        QStringLiteral("Alice With A Very Long Attendee Name For Wrapping"),
        QStringLiteral("alice@example.com")));
    schedule->addAttendee(KCalendarCore::Attendee(
        QStringLiteral("Bob"),
        QStringLiteral("bob@example.com")));

    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);

    // Render to trigger paintEvent which uses the wrapped lines
    mCenterWidget->setFixedSize(400, 400);
    QPixmap pixmap(mCenterWidget->size());
    mCenterWidget->render(&pixmap);
}

// CenterWidget::UpdateTextList with very long summary to trigger appendToolTipText
TEST_F(test_scheduleremindwidget, centerWidget_updateTextList_longSummary_toolTip)
{
    DSchedule::Ptr schedule(new DSchedule());
    schedule->setSummary("This is an extremely long schedule summary that will definitely "
                         "exceed the maximum text width and trigger both the multi-line "
                         "wrapping logic and the appendToolTipText lambda for tooltip generation "
                         "which wraps text at 300px width with newlines and eliding logic");
    schedule->setDtStart(QDateTime::currentDateTime());
    schedule->setDtEnd(QDateTime::currentDateTime().addSecs(3600));
    schedule->setAllDay(false);

    schedule->setOrganizer(KCalendarCore::Person(
        QStringLiteral("Org"),
        QStringLiteral("org@example.com")));
    schedule->addAttendee(KCalendarCore::Attendee(
        QStringLiteral("Att"),
        QStringLiteral("att@example.com")));

    CSchedulesColor color = getTestColor();
    mCenterWidget->setData(schedule, color);
    mCenterWidget->setFixedSize(400, 400);
    QPixmap pixmap(mCenterWidget->size());
    mCenterWidget->render(&pixmap);
}

// ===== Coverage for ScheduleRemindWidget constructor lambda (fontChanged) =====

// Trigger DApplication::fontChanged to cover the constructor's font change lambda
TEST_F(test_scheduleremindwidget, fontChanged_lambda)
{
    DSchedule::Ptr schedule = createTestSchedule(false);
    CSchedulesColor color = getTestColor();
    mScheduleRemindWidget->setData(schedule, color);

    // Change the application font to trigger DApplication::fontChanged signal
    QFont newFont = qApp->font();
    newFont.setPointSize(newFont.pointSize() + 1);
    qApp->setFont(newFont);
    QTest::qWait(50);

    // Restore the original font
    newFont.setPointSize(newFont.pointSize() - 1);
    qApp->setFont(newFont);
    QTest::qWait(50);
}
