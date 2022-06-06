/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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

#include "monthview.h"
#include "scheduledlg.h"
#include "scheduledatamanage.h"
#include "calendarglobalenv.h"

#include <DHiDPIHelper>
#include <DPalette>

#include <QPainter>
#include <QEvent>
#include <QTime>
#include <QApplication>
#include <QMouseEvent>

DGUI_USE_NAMESPACE

/**
 * @brief setTheMe  根据系统主题类型设置颜色
 * @param type      系统主题类型
 */
void CMonthView::setTheMe(int type)
{
    m_weekIndicator->setTheMe(type);
    m_monthGraphicsView->setTheMe(type);
}

CMonthView::CMonthView(QWidget *parent) : DWidget(parent)
{
    m_weekIndicator = new CMonthWeekView;
    m_monthGraphicsView = new CMonthGraphicsview(this);

    connect(m_monthGraphicsView, &CMonthGraphicsview::signalsViewSelectDate, this, &CMonthView::signalsViewSelectDate);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalScheduleShow, this, &CMonthView::slotScheduleRemindWidget);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalAngleDelta, this, &CMonthView::signalAngleDelta);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalSwitchPrePage, this, &CMonthView::signalSwitchPrePage);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalSwitchNextPage, this, &CMonthView::signalSwitchNextPage);
    connect(m_monthGraphicsView, &CMonthGraphicsview::signalGotoDayView, this, &CMonthView::signalsViewSelectDate);
    //新建最终布局
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_weekIndicator);
    m_mainLayout->addWidget(m_monthGraphicsView);

    setLayout(m_mainLayout);
    m_createAction = new QAction(tr("New event"), this);

    m_remindWidget = new ScheduleRemindWidget(this);
    setMouseTracking(true);
}

CMonthView::~CMonthView()
{
}

void CMonthView::setSelectSchedule(const DSchedule::Ptr &scheduleInfo)
{
    m_monthGraphicsView->setSelectSearchSchedule(scheduleInfo);
}

void CMonthView::slotScheduleRemindWidget(const bool isShow, const DSchedule::Ptr &out)
{
    if (isShow) {
        //获取当前鼠标位置
        QVariant variant;
        CalendarGlobalEnv::getGlobalEnv()->getValueByKey(DDECalendar::CursorPointKey, variant);
        QPoint remindPos = variant.value<QPoint>();
        //根据类型获取颜色
        CSchedulesColor gdColor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(out->scheduleTypeID());
        m_remindWidget->setData(out, gdColor);
        //获取屏幕大小
        QRect desktopRect = QApplication::desktop()->rect();
        //根据提示框在屏幕的位置设置箭头方向
        if ((remindPos.x() + m_remindWidget->width() + 10) > desktopRect.width()) {
            m_remindWidget->setDirection(DArrowRectangle::ArrowRight);
            remindPos.setX(remindPos.x() - 5);
        } else {
            m_remindWidget->setDirection(DArrowRectangle::ArrowLeft);
            remindPos.setX(remindPos.x() + 5);
        }
        m_remindWidget->show(remindPos.x(), remindPos.y());
    } else {
        m_remindWidget->hide();
    }
}

void CMonthView::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    QMargins margins = m_mainLayout->contentsMargins();
    m_weekIndicator->setFixedSize(width() - margins.left(), static_cast<int>(height() * 0.1042 + 0.5));
}

void CMonthView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    slotScheduleRemindWidget(false);
}

bool CMonthView::event(QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        m_monthGraphicsView->setFocus(Qt::TabFocusReason);
        return true;
    }
    return DWidget::event(event);
}

/**
 * @brief setFirstWeekday   设置每周的第一天是周几
 * @param weekday           周几
 */
void CMonthView::setFirstWeekday(Qt::DayOfWeek weekday)
{
    m_firstWeekDay = weekday;
    m_weekIndicator->setFirstDay(weekday);
}

/**
 * @brief CMonthView::setShowDate       设置显示日期
 * @param showDate
 */
void CMonthView::setShowDate(const QVector<QDate> &showDate)
{
    m_showDate = showDate;
    m_monthGraphicsView->setDate(m_showDate);
}

/**
 * @brief CMonthView::setHuangLiInfo        设置黄历信息
 * @param huangLiInfo
 */
void CMonthView::setHuangLiInfo(const QMap<QDate, CaHuangLiDayInfo> &huangLiInfo)
{
    m_monthGraphicsView->setLunarInfo(huangLiInfo);
}

/**
 * @brief CMonthView::setFestival       设置班休信息
 * @param festivalInfo
 */
void CMonthView::setFestival(const QMap<QDate, int> &festivalInfo)
{
    m_monthGraphicsView->setFestival(festivalInfo);
}

/**
 * @brief CMonthView::setScheduleInfo       设置显示日程
 * @param scheduleInfo
 */
void CMonthView::setScheduleInfo(const QMap<QDate, DSchedule::List> &scheduleInfo)
{
    m_monthGraphicsView->setScheduleInfo(scheduleInfo);
}

/**
 * @brief CMonthView::setSearchScheduleInfo     设置搜索日程
 * @param searchScheduleInfo
 */
void CMonthView::setSearchScheduleInfo(const DSchedule::List &searchScheduleInfo)
{
    m_monthGraphicsView->setSearchScheduleInfo(searchScheduleInfo);
}

/**
 * @brief CMonthView::setCurrentDate        设置当前时间
 * @param currentDate
 */
void CMonthView::setCurrentDate(const QDate &currentDate)
{
    m_weekIndicator->setCurrentDate(currentDate);
    m_monthGraphicsView->setCurrentDate(currentDate);
}

void CMonthView::setRemindWidgetTimeFormat(QString timeformat)
{
    m_remindWidget->setTimeFormat(timeformat);
}

/**
 * @brief CMonthView::deleteSelectSchedule 快捷键删除日程
 */
void CMonthView::deleteSelectSchedule()
{
    m_monthGraphicsView->slotDeleteItem();
}

void CMonthView::setLunarVisible(bool visible)
{
    m_monthGraphicsView->setLunarVisible(visible);
}

DSchedule::Ptr CMonthView::getScheduleInfo(const QDate &beginDate, const QDate &endDate)
{
    DSchedule::Ptr info;
    info.reset(new DSchedule());
    if (beginDate.daysTo(endDate) > 0) {
        info->setDtStart(QDateTime(beginDate, QTime(0, 0, 0)));
        info->setDtEnd(QDateTime(endDate, QTime(23, 59, 59)));
    } else {
        info->setDtStart(QDateTime(endDate, QTime(0, 0, 0)));
        info->setDtEnd(QDateTime(beginDate, QTime(23, 59, 00)));
    }
    info->setSummary(tr("New Event"));
    info->setAllDay(true);
    info->setAlarmType(DSchedule::Alarm_15Hour_Front);
    return info;
}
