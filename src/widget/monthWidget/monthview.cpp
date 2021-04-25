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
#include "schceduledlg.h"
#include "scheduledatamanage.h"

#include <DHiDPIHelper>
#include <DHiDPIHelper>
#include <DPalette>

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include <QShortcut>
#include <QApplication>
#include <QMouseEvent>

DGUI_USE_NAMESPACE
/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type  系统主题类型
 */
void CMonthView::setTheMe(int type)
{
    //设置周的系统主题类型
    m_weekIndicator->setTheMe(type);
    //设置视图的系统主题类型
    m_MonthGraphicsView->setTheMe(type);
}
/**
 * @brief CMonthView 构造函数
 * @param parent 父类
 */
CMonthView::CMonthView(QWidget *parent) : DWidget(parent)
{
    m_DBusInter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                   "/com/deepin/api/LunarCalendar",
                                   QDBusConnection::sessionBus(), this);
    if (!lunarCache)
        lunarCache = new QMap<QDate, CaLunarDayInfo>;
    //新建周显示
    m_weekIndicator = new CMonthWeekView;
    //新建月的视图
    m_MonthGraphicsView = new CMonthGraphiview(this);
    //创建信号连接
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
    //新建最终布局
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 0, 0, 10);
    //将周的显示添加到布局中
    m_mainLayout->addWidget(m_weekIndicator);
    //将整个月的视图添加到布局中
    m_mainLayout->addWidget(m_MonthGraphicsView);
    //设置布局
    setLayout(m_mainLayout);
    CScheduleDataCtrl  *scheduleDataCtrl = CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl();
    m_createAction = new QAction(tr("New event"), this);
    //日程浮框
    m_RemindWidget = new SchecduleRemindWidget(this);

    connect(scheduleDataCtrl, &CScheduleDataCtrl::signalsupdatescheduleD, this, &CMonthView::slotsupdatescheduleD);
    connect(this, &CMonthView::signalsupdatescheduleD, scheduleDataCtrl, &CScheduleDataCtrl::slotupdatescheduleD);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}
/**
  * @brief ~CMonthView 析构函数
  */
CMonthView::~CMonthView()
{
    if (lunarCache !=nullptr) {
        delete lunarCache;
        lunarCache  = nullptr;
    }
}
/**
 * @brief slotSchceduleUpdate 更新日程信息
 */
void CMonthView::slotSchceduleUpdate()
{
    emit signalsSchceduleUpdate(0);
    emit signalsupdatescheduleD(this, m_days[0], m_days[41]);
}
/**
 * @brief setSelectSchedule 设置选择的日程
 * @param scheduleInfo 日程信息
 */
void CMonthView::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_MonthGraphicsView->setSelectSchedule(scheduleInfo);
}
/**
 * @brief slotsupdatescheduleD 更新日程
 * @param w 判断是否是当前的widget
 * @param data 日程信息
 */
void CMonthView::slotsupdatescheduleD(QWidget *w, QVector<ScheduleDateRangeInfo> &data)
{
    if (w != this) return;
    if (m_sflag) {
        m_MonthGraphicsView->setScheduleInfo(data);
    }
    parentWidget()->setEnabled(true);
}
/**
 * @brief slotdelete 发送更新日程的信号
 * @param id
 */
void CMonthView::slotdelete(int id)
{
    Q_UNUSED(id);
    emit signalsSchceduleUpdate(0);
}
/**
 * @brief slotScheduleRemindWidget 日程浮框
 * @param isShow 是否显示日程浮框
 * @param out 日程信息
 */
void CMonthView::slotScheduleRemindWidget(const bool isShow, const ScheduleDtailInfo &out)
{
    if (isShow) {
        //展示日程浮框
        QPoint pos22 = QCursor::pos();
        //根据日程类型获取颜色
        CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(
                                      out.type.ID);
        //设置日程信息和颜色
        m_RemindWidget->setData(out, gdcolor);
        //设置日程显示位置
        m_RemindWidget->show(pos22.x() + 10, pos22.y());
        QDesktopWidget *w = QApplication::desktop();
        //根据日历位置决定日程浮框显示位置
        if ((pos22.x() + m_RemindWidget->width() + 10) > w->width()) {
            //日程浮框在左边
            m_RemindWidget->setArrowDirection(DArrowRectangle::ArrowRight);
            m_RemindWidget->show(pos22.x() - 10, pos22.y());
        } else {
            //日程浮框在右边
            m_RemindWidget->setArrowDirection(DArrowRectangle::ArrowLeft);
            m_RemindWidget->show(pos22.x() + 10, pos22.y());
        }
    } else {
        //隐藏日程浮框
        m_RemindWidget->hide();
    }
}
/**
 * @brief resizeEvent 窗口大小调整
 * @param event 窗口大小调整事件
 */
void CMonthView::resizeEvent(QResizeEvent *event)
{
    DWidget::resizeEvent(event);
    int leftmagin = 10;
    int topmagin = static_cast<int>(height() * 0.0193 + 0.5);
    topmagin = 10;
    m_leftmaagin = leftmagin;
    m_topmagin = topmagin;
    //设置布局显示边距
    m_mainLayout->setContentsMargins(leftmagin, topmagin, 0, 10);
    m_weekIndicator->setFixedSize(width()-leftmagin, static_cast<int>(height() * 0.1042 + 0.5));
}
/**
 * @brief mousePressEvent 鼠标单击
 * @param event 鼠标事件
 */
void CMonthView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    slotScheduleRemindWidget(false);
}
/**
 * @brief setFirstWeekday 设置每周的第一天是周几
 * @param weekday 周几
 */
void CMonthView::setFirstWeekday(int weekday)
{
    m_firstWeekDay = weekday;
    //设置周列表
    m_weekIndicator->setList(weekday);
    //更新周
    m_weekIndicator->updateWeek();
    //更新数据
    updateDate();
}
/**
 * @brief setCurrentDate 设置当前的日程
 * @param date 日期
 */
void CMonthView::setCurrentDate(const QDate date)
{
    slotScheduleRemindWidget(false);
    qDebug() << "set current date " << date;
    //如果日期早于1900年，return
    if (date.year() < DDECalendar::QueryEarliestYear)
        return;
    //如果不是本月，重新获取节日信息
    if (date.month() != m_currentDate.month()) {
        m_festivallist.clear();
        m_DBusInter->GetFestivalMonth(date.addMonths(-1).year(), date.addMonths(-1).month(), m_festivallist);
        m_DBusInter->GetFestivalMonth(date.year(), date.month(), m_festivallist);
        m_DBusInter->GetFestivalMonth(date.addMonths(1).year(), date.addMonths(1).month(), m_festivallist);
    }
    //设置当前时间
    m_currentDate = date;
    //设置节日信息
    m_MonthGraphicsView->setFestivalInfo(m_festivallist);
    //更新数据
    updateDate();
    //获取dbus数据
    getDbusData();
    //设置阴历信息
    m_MonthGraphicsView->setLunarInfo(lunarCache);
    emit signalcurrentDateChanged(m_currentDate);
    m_weekIndicator->updateWeek();
    emit signalsupdatescheduleD(this, m_days[0], m_days[41]);
}
/**
 * @brief setLunarVisible 设置是否显示阴历信息
 * @param visible 是否显示阴历信息
 */
void CMonthView::setLunarVisible(bool visible)
{
    m_MonthGraphicsView->setLunarVisible(visible);
}
/**
 * @brief getDateIndex 获取日期的索引值
 * @param date 日期
 * @return 索引值
 */
int CMonthView::getDateIndex(const QDate &date) const
{
    for (int i = 0; i != DDEMonthCalendar::ItemSizeofMonthDay; ++i)
        if (m_days[i] == date)
            return i;

    return 0;
}
/**
 * @brief updateDate 更新数据
 */
void CMonthView::updateDate()
{
    const QDate firstDay(m_currentDate.year(), m_currentDate.month(), 1);
    int offset = firstDay.dayOfWeek() % DDEMonthCalendar::AFewDaysofWeek - m_firstWeekDay;
    const int day = offset < 0 ? offset + DDEMonthCalendar::AFewDaysofWeek : offset;
    const int currentIndex = day + m_currentDate.day() - 1;
    //如果当前日期的索引小于0,return
    if (currentIndex < 0) {
        return;
    }
    //设置一个月的时间
    for (int i(0); i != DDEMonthCalendar::ItemSizeofMonthDay; ++i) {
        m_days[i] = firstDay.addDays(i - day);
    }
    //设置时间
    m_MonthGraphicsView->setDate(m_days);
    update();
}
/**
 * @brief updateCurrentLunar 更新当前的阴历信息
 * @param info 阴历信息
 */
void CMonthView::updateCurrentLunar(const CaLunarDayInfo &info)
{
    emit signalcurrentLunarDateChanged(m_currentDate, info, 1);
}
/**
 * @brief getScheduleInfo 获取日程信息
 * @param beginDate 日程开始时间
 * @param endDate 日程结束时间
 * @return 日程信息
 */
ScheduleDtailInfo CMonthView::getScheduleInfo(const QDate &beginDate, const QDate &endDate)
{
    ScheduleDtailInfo info;
    //设置日程的默认信息
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
/**
 * @brief getCellDate 通过索引获取日期
 * @param pos 索引值
 * @return 日期
 */
const QDate CMonthView::getCellDate(int pos)
{
    return m_days[pos];
}
/**
 * @brief getCaLunarDayInfo 获取阴历信息
 * @param date 日期
 * @return 阴历信息
 */
const CaLunarDayInfo CMonthView::getCaLunarDayInfo(const QDate &date)
{
    return lunarCache->value(date);
}
/**
 * @brief getDbusData 获取dbus数据
 */
void CMonthView::getDbusData()
{
    lunarCache->clear();

    CaLunarDayInfo currentDayInfo;
    bool o1 = true;
    QDate getDate = m_currentDate;

    for (int i = -1; i < 2; ++i) {
        getDate = m_currentDate.addMonths(i);
        //获取阴历信息
        QDBusReply<CaLunarMonthInfo> reply = m_DBusInter->GetLunarMonthCalendar(getDate.year(),
                                                                                getDate.month(), false, o1);

        QDate cacheDate;
        cacheDate.setDate(getDate.year(), getDate.month(), 1);
        foreach (const CaLunarDayInfo &dayInfo, reply.value().mCaLunarDayInfo) {
            lunarCache->insert(cacheDate, dayInfo);
            cacheDate = cacheDate.addDays(1);
        }
    }
    // refresh   lunar info
    updateCurrentLunar(lunarCache->value(m_currentDate));
}


