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
#include "calendardbus.h"

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include "schceduledlg.h"
#include "scheduledatamanage.h"
#include <DHiDPIHelper>
#include <DPalette>
#include <QShortcut>
#include <DHiDPIHelper>
#include <QApplication>

DGUI_USE_NAMESPACE
void CMonthView::setTheMe(int type)
{
    m_weekIndicator->setTheMe(type);
    m_MonthGraphicsView->setTheMe(type);
}


CMonthView::CMonthView(QWidget *parent) : DWidget(parent)
{
    m_DBusInter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                   "/com/deepin/api/LunarCalendar",
                                   QDBusConnection::sessionBus(), this);
    if (!lunarCache)
        lunarCache = new QMap<QDate, CaLunarDayInfo>;

    m_weekIndicator = new CMonthWeekView;
    m_MonthGraphicsView = new CMonthGraphiview(this);

    connect(m_MonthGraphicsView,
            &CMonthGraphiview::signalsViewSelectDate,
            this,
            &CMonthView::signalsViewSelectDate);
    connect(m_MonthGraphicsView,
            &CMonthGraphiview::signalsUpdateShcedule,
            this,
            &CMonthView::slotSchceduleUpdate);
    connect(m_MonthGraphicsView,
            &CMonthGraphiview::signalsSchceduleUpdate,
            this,
            &CMonthView::signalsSchceduleUpdate);
    connect(m_MonthGraphicsView,
            &CMonthGraphiview::signalViewtransparentFrame,
            this,
            &CMonthView::signalViewtransparentFrame);
    connect(m_MonthGraphicsView,
            &CMonthGraphiview::signalScheduleShow,
            this,
            &CMonthView::slotScheduleRemindWidget);




    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 0, 0, 10);
    m_mainLayout->addWidget(m_weekIndicator);
    m_mainLayout->addWidget(m_MonthGraphicsView);

    setLayout(m_mainLayout);
    CScheduleDataCtrl  *scheduleDataCtrl = CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl();
    m_createAction = new QAction(tr("New event"), this);

    m_RemindWidget = new SchecduleRemindWidget(this);

    connect(scheduleDataCtrl, &CScheduleDataCtrl::signalsupdatescheduleD, this, &CMonthView::slotsupdatescheduleD);
    connect(this, &CMonthView::signalsupdatescheduleD, scheduleDataCtrl, &CScheduleDataCtrl::slotupdatescheduleD);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

CMonthView::~CMonthView()
{
    if (lunarCache !=nullptr) {
        delete lunarCache;
        lunarCache  = nullptr;
    }
}

void CMonthView::slotSchceduleUpdate()
{
    emit signalsSchceduleUpdate(0);
    emit signalsupdatescheduleD(this, m_days[0], m_days[41]);
}

void CMonthView::slotUpdateUI(int type)
{
    if (type == 0) {
        m_sflag = false;
    } else {
        m_sflag = true;
        slotSchceduleUpdate();
    }
}

void CMonthView::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_MonthGraphicsView->setSelectSchedule(scheduleInfo);
}

void CMonthView::slotsupdatescheduleD(QWidget *w, QVector<ScheduleDateRangeInfo> &data)
{
    if (w != this) return;
    if (m_sflag) {
        m_MonthGraphicsView->setScheduleInfo(data);
    }
    parentWidget()->setEnabled(true);
}

void CMonthView::slotdelete(int id)
{
    Q_UNUSED(id);
    emit signalsSchceduleUpdate(0);
}

void CMonthView::slotScheduleRemindWidget(const bool isShow, const ScheduleDtailInfo &out)
{
    if (isShow) {
        QPoint pos22 = QCursor::pos();
        CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(
                                      out.type.ID);
        m_RemindWidget->setData(out, gdcolor);
        m_RemindWidget->show(pos22.x() + 10, pos22.y());
        QDesktopWidget *w = QApplication::desktop();
        if ((pos22.x() + m_RemindWidget->width() + 10) > w->width()) {
            m_RemindWidget->setArrowDirection(DArrowRectangle::ArrowRight);
            m_RemindWidget->show(pos22.x() - 10, pos22.y());
        } else {
            m_RemindWidget->setArrowDirection(DArrowRectangle::ArrowLeft);
            m_RemindWidget->show(pos22.x() + 10, pos22.y());
        }

    } else {
        m_RemindWidget->hide();
    }
}

void CMonthView::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    int leftmagin = 10;
    int topmagin = static_cast<int>(height() * 0.0193 + 0.5);
    topmagin = 10;
    m_leftmaagin = leftmagin;
    m_topmagin = topmagin;
    m_mainLayout->setContentsMargins(leftmagin, topmagin, 0, 10);
    m_weekIndicator->setFixedSize(width()-leftmagin, static_cast<int>(height() * 0.1042 + 0.5));
}


void CMonthView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    slotScheduleRemindWidget(false);
}

void CMonthView::setFirstWeekday(int weekday)
{
    m_firstWeekDay = weekday;
    m_weekIndicator->setList(weekday);
    m_weekIndicator->updateWeek();
    updateDate();
}

void CMonthView::setCurrentDate(const QDate date)
{
    slotScheduleRemindWidget(false);
    qDebug() << "set current date " << date;
    if (date.year() < 1900) return;
    if (date.month() != m_currentDate.month()) {
        m_festivallist.clear();
        m_DBusInter->GetFestivalMonth(date.addMonths(-1).year(), date.addMonths(-1).month(), m_festivallist);
        m_DBusInter->GetFestivalMonth(date.year(), date.month(), m_festivallist);
        m_DBusInter->GetFestivalMonth(date.addMonths(1).year(), date.addMonths(1).month(), m_festivallist);
    }
    m_currentDate = date;
    m_MonthGraphicsView->setFestivalInfo(m_festivallist);
    updateDate();
    getDbusData();
    m_MonthGraphicsView->setLunarInfo(lunarCache);
    emit signalcurrentDateChanged(m_currentDate);
    m_weekIndicator->updateWeek();
    emit signalsupdatescheduleD(this, m_days[0], m_days[41]);
}

void CMonthView::setLunarVisible(bool visible)
{
    m_MonthGraphicsView->setLunarVisible(visible);
}


int CMonthView::getDateIndex(const QDate &date) const
{
    for (int i = 0; i != 42; ++i)
        if (m_days[i] == date)
            return i;

    return 0;
}

void CMonthView::updateDate()
{
    const QDate firstDay(m_currentDate.year(), m_currentDate.month(), 1);
    int offset = firstDay.dayOfWeek() % 7 - m_firstWeekDay ;

    const int day = offset <0 ?offset +7:offset;
    const int currentIndex = day + m_currentDate.day() - 1;

    if (currentIndex < 0) {
        return;
    }
    for (int i(0); i != 42; ++i) {
        m_days[i] = firstDay.addDays(i - day);
    }
    m_MonthGraphicsView->setDate(m_days);
    update();
}

void CMonthView::updateCurrentLunar(const CaLunarDayInfo &info)
{
    emit signalcurrentLunarDateChanged(m_currentDate, info, 1);
}


ScheduleDtailInfo CMonthView::getScheduleInfo(const QDate &beginDate, const QDate &endDate)
{
    ScheduleDtailInfo info;
    if (beginDate.daysTo(endDate)>0) {
        info.beginDateTime = QDateTime(beginDate,QTime(0,0,0));
        info.endDateTime = QDateTime(endDate,QTime(23,59,59));
    } else {
        info.beginDateTime = QDateTime(endDate,QTime(0,0,0));
        info.endDateTime = QDateTime(beginDate,QTime(23,59,00));
    }
    info.titleName = tr("New Event");
    info.allday = true;
    info.remind = true;
    info.remindData.n = 1;
    info.remindData.time = QTime(9, 0);
    info.RecurID = 0;
    info.id = 0;
    CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->GetType(
        1, info.type);
    info.rpeat = 0;

    return info;
}

const QDate CMonthView::getCellDate(int pos)
{
    return m_days[pos];
}

const CaLunarDayInfo CMonthView::getCaLunarDayInfo(const QDate &date)
{
    return lunarCache->value(date);
}

void CMonthView::getDbusData()
{
    lunarCache->clear();

    CaLunarDayInfo currentDayInfo;
    bool o1 = true;
    QDate getDate = m_currentDate;
    for (int i = -1; i < 2; ++i) {
        getDate = m_currentDate.addMonths(i);
        QDBusReply<CaLunarMonthInfo> reply = m_DBusInter->GetLunarMonthCalendar(getDate.year(),
                                                                                getDate.month(), false, o1);

        QDate cacheDate;
        cacheDate.setDate(getDate.year(), getDate.month(), 1);
        foreach (const CaLunarDayInfo &dayInfo, reply.value().mCaLunarDayInfo) {
//            if (cacheDate == QDate(2020,6,21)) {
//                qDebug()<<"mGanZhiYear:"<<dayInfo.mGanZhiYear;
//                qDebug()<<"mGanZhiMonth:"<<dayInfo.mGanZhiMonth;
//                qDebug()<<"mGanZhiDay:"<<dayInfo.mGanZhiDay;
//                qDebug()<<"mLunarMonthName:"<<dayInfo.mLunarMonthName;
//                qDebug()<<"mLunarDayName:"<<dayInfo.mLunarDayName;
//                qDebug()<<"mLunarLeapMonth:"<<dayInfo.mLunarLeapMonth;
//                qDebug()<<"mZodiac:"<<dayInfo.mZodiac;
//                qDebug()<<"mTerm:"<<dayInfo.mTerm;
//                qDebug()<<"mSolarFestival:"<<dayInfo.mSolarFestival;
//                qDebug()<<"mLunarFestival:"<<dayInfo.mLunarFestival;
//                qDebug()<<"mWorktime:"<<dayInfo.mWorktime;
//            }
            lunarCache->insert(cacheDate, dayInfo);
            cacheDate = cacheDate.addDays(1);
        }
    }
    // refresh   lunar info
    updateCurrentLunar(lunarCache->value(m_currentDate));
}


