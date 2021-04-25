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
#include "daywindow.h"
#include "constants.h"
#include "customframe.h"
#include "daymonthview.h"
#include "schcedulesearchview.h"
#include "scheduleview.h"

#include <DPalette>

#include <QHBoxLayout>
#include <QMessageBox>

DGUI_USE_NAMESPACE
/**
 * @brief CDayWindow::CDayWindow 构造函数
 * @param parent 父类
 */
CDayWindow::CDayWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //初始化界面
    initUI();
    //初始化连接
    initConnection();
}
/**
 * @brief CDayWindow::~CDayWindow 析构函数
 */
CDayWindow::~CDayWindow() {}
/**
 * @brief CDayWindow::setTheMe 设置系统主题类型
 * @param type 系统主题类型
 */
void CDayWindow::setTheMe(int type)
{
    //根据系统主题类型设置颜色
    if (type == 0 || type == 1) {
        //系统主题类型为浅色或跟随系统
        //设置背景色
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置左边背景色
        m_leftground->setBColor("#FFFFFF");
        //设置年份label颜色
        DPalette ypa = m_YearLabel->palette();
        ypa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年份颜色
        DPalette lpa = m_LunarLabel->palette();
        lpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历颜色
        DPalette spa = m_SolarDay->palette();
        spa.setColor(DPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        //系统主题类型为深色
        //设置背景色
        DPalette anipa = m_contentBackground->palette();
        QColor bcolor = "#252525";
        anipa.setColor(DPalette::Background, bcolor);
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置左边背景色
        m_leftground->setBColor("#282828");
        //设置年份label颜色
        DPalette ypa = m_YearLabel->palette();
        ypa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年份label
        DPalette lpa = m_LunarLabel->palette();
        lpa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历颜色
        DPalette spa = m_SolarDay->palette();
        spa.setColor(DPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(DPalette::WindowText);
    }
    //设置daymonthview系统主题类型
    m_daymonthView->setTheMe(type);
    //设置scheduleview系统主题类型
    m_scheduleView->setTheMe(type);
}
/**
 * @brief CDayWindow::setTime 设置时间
 * @param time 时间
 */
void CDayWindow::setTime(QTime time)
{
    m_scheduleView->setTime(time);
}
/**
 * @brief CDayWindow::updateHigh 更新高度
 */
void CDayWindow::updateHigh()
{
    m_scheduleView->updateHigh();
}
/**
 * @brief CDayWindow::setDate 设置日期
 * @param date 日期
 */
void CDayWindow::setDate(QDate date)
{
    //无效日期
    if (!date.isValid())
        return;
    //当前时间
    m_currentdate = date;
    m_daymonthView->setFirstWeekday(0);
    //设置当前时间
    m_daymonthView->setCurrentDate(date);
    //日程宽度
    int w = m_scheduleView->width() - 72;
    m_scheduleView->setRange(w, 1032, m_currentdate, m_currentdate);
    emit signalCurrentDate(date);
}
/**
 * @brief CDayWindow::setSelectSchedule 设置选择的日程
 * @param scheduleInfo 日程信息
 */
void CDayWindow::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_scheduleView->setSelectSchedule(scheduleInfo);

}
/**
 * @brief CDayWindow::setSearchWFlag 设置搜素标志
 * @param flag 标志
 */
void CDayWindow::setSearchWFlag(bool flag)
{
    m_searchfalg = flag;
    //设置daymonthview搜索标志
    m_daymonthView->setSearchFlag(flag);
}
/**
 * @brief CDayWindow::clearSearch
 */
void CDayWindow::clearSearch()
{
}
/**
 * @brief CDayWindow::setLunarVisible 阴历使能
 * @param state 是否设置阴历信息
 */
void CDayWindow::setLunarVisible(bool state)
{
    m_LunarLabel->setVisible(state);
    m_SolarDay->setVisible(state);
    m_scheduleView->setLunarVisible(state);
    m_daymonthView->setLunarVisible(state);
}
/**
 * @brief CDayWindow::initUI 初始化界面操作
 */
void CDayWindow::initUI()
{
    //背景frame
    m_contentBackground = new DFrame;
    m_contentBackground->setAutoFillBackground(true);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setPalette(anipa);
    m_contentBackground->setFrameRounded(true);
    m_contentBackground->setLineWidth(0);
    //title布局
    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 9, 0, 3);
    //年份label
    m_YearLabel = new QLabel();
    m_YearLabel->setMinimumHeight(DDEDayCalendar::D_YLableHeight);
    QFont labelF;
    labelF.setWeight(QFont::Medium);
    labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(labelF);
    DPalette ypa = m_YearLabel->palette();
    ypa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(ypa);
    //将年份label添加到title布局
    titleLayout->addWidget(m_YearLabel);
    //阴历年份label
    m_LunarLabel = new QLabel();
    titleLayout->addSpacing(15);
    m_LunarLabel->setFixedHeight(DDEDayCalendar::D_YLableHeight);
    labelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_LunarLabel->setFont(labelF);
    m_LunarLabel->setAlignment(Qt::AlignCenter);
    DPalette lpa = m_LunarLabel->palette();
    lpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_LunarLabel->setPalette(lpa);
    //将阴历年份label添加到title布局
    titleLayout->addWidget(m_LunarLabel);
    //
    m_SolarDay = new QLabel();
    labelF.setPixelSize(DDECalendar::FontSizeTen);
    m_SolarDay->setFixedHeight(DDEDayCalendar::D_YLableHeight);
    m_SolarDay->setFont(labelF);
    m_SolarDay->setAlignment(Qt::AlignCenter);
    DPalette spa = m_SolarDay->palette();
    spa.setColor(DPalette::WindowText, Qt::red);
    m_SolarDay->setPalette(spa);
    titleLayout->addWidget(m_SolarDay);
    titleLayout->addStretch();
    //设置年份和日程布局
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    //日程view
    m_scheduleView = new CScheduleView(this, 1);
    m_scheduleView->setviewMagin(72, 109, 20, 0);
    m_scheduleView->setRange(423, 1032, QDate(2019, 8, 12), QDate(2019, 8, 12));
    //将title布局添加到年份和日程布局中
    leftLayout->addLayout(titleLayout);
    //将日程view添加到年份和日程布局中
    leftLayout->addWidget(m_scheduleView);
    //线
    m_verline = new DVerticalLine;
    m_verline->setFixedWidth(2);
    //daymonthview右边部分
    m_daymonthView = new CDayMonthView(this);
    //整体布局
    QHBoxLayout *lfetmainLayout = new QHBoxLayout;
    lfetmainLayout->setMargin(0);
    lfetmainLayout->setSpacing(1);
    lfetmainLayout->setContentsMargins(0, 0, 0, 0);
    //将年份和日程局添加到整体布局中
    lfetmainLayout->addLayout(leftLayout);
    //将线添加到整体布局中
    lfetmainLayout->addWidget(m_verline);
    //将daymonthview添加到整体布局中
    lfetmainLayout->addWidget(m_daymonthView);
    //frame
    m_leftground = new CustomFrame();
    m_leftground->setRoundState(true, true, true, true);
    //设置frame布局
    m_leftground->setLayout(lfetmainLayout);
    m_leftground->setBColor("#FFFFFF");
    //总体布局
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    //将frame添加到总体布局中
    m_mainLayout->addWidget(m_leftground);
    //设置总体布局
    m_contentBackground->setLayout(m_mainLayout);
    //设置main widget
    setCentralWidget(m_contentBackground);
}
/**
 * @brief CDayWindow::initConnection 初始化connect连接
 */
void CDayWindow::initConnection()
{
    //daymonthview
    connect(m_daymonthView, &CDayMonthView::signalcurrentLunarDateChanged, this,
            &CDayWindow::slotcurrentDateLunarChanged);
    connect(m_daymonthView, &CDayMonthView::signalcurrentDateChanged, this,
            &CDayWindow::slotcurrentDateChanged);
    connect(m_daymonthView, &CDayMonthView::signalsReturnDay, this,
            &CDayWindow::slotCurrentReturnDay);
    //scheduleview
    connect(m_scheduleView, &CScheduleView::signalViewtransparentFrame, this,
            &CDayWindow::signalViewtransparentFrame);
    connect(m_scheduleView, &CScheduleView::signalsUpdateShcedule, this,
            &CDayWindow::slotTransitSchedule);
    //daymonthview
    connect(m_daymonthView, &CDayMonthView::signalSchedulHide
            , this, &CDayWindow::slotScheduleHide);
    connect(m_daymonthView,&CDayMonthView::signalIsDragging,
            this,&CDayWindow::slotIsDragging);
}
/**
 * @brief CDayWindow::slotupdateSchedule 更新日程
 * @param id unused
 */
void CDayWindow::slotupdateSchedule(int id)
{
    Q_UNUSED(id);
    //设置daymonthview当前时间
    m_daymonthView->setCurrentDate(m_currentdate);
    //设置日程range
    m_scheduleView->setRange(m_currentdate, m_currentdate);
    //更新flag
    m_daymonthView->updateFlag();
}
/**
 * @brief CDayWindow::slotTransitSchedule 更新日程
 * @param id
 */
void CDayWindow::slotTransitSchedule(int id)
{
    emit signalsWUpdateShcedule(this, id);
    m_daymonthView->updateFlag();
}
/**
 * @brief CDayWindow::slotTransitSearchSchedule 更新选择的日程
 * @param id
 */
void CDayWindow::slotTransitSearchSchedule(int id)
{
    m_scheduleView->slotupdateSchedule();
    emit signalsWUpdateShcedule(this, id);
}
/**
 * @brief CDayWindow::slotReturnTodayUpdate 返回今天
 */
void CDayWindow::slotReturnTodayUpdate()
{
    m_daymonthView->slottoday();
}
/**
 * @brief CDayWindow::slotCurrentReturnDay 返回今天
 */
void CDayWindow::slotCurrentReturnDay()
{
    emit signalsReturnTodayUpdate(this);
}
/**
 * @brief CDayWindow::slotScheduleHide 隐藏日程浮框
 */
void CDayWindow::slotScheduleHide()
{
    m_scheduleView->slotScheduleShow(false);
}
/**
 * @brief CDayWindow::resizeEvent 更新大小
 * @param event 窗口大小改变
 */
void CDayWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    qreal dw = 0.4046 * width();
    int dh = height() - 20;
    //根据是否搜索设置边距
    if (m_searchfalg) {
        m_mainLayout->setContentsMargins(10, 10, 0, 10);
    } else {
        m_mainLayout->setContentsMargins(10, 10, 10, 10);
    }

    if (dw < 350)
        dw = 350;
    m_daymonthView->setFixedSize(qRound(dw), dh);
}
/**
 * @brief CDayWindow::slotcurrentDateLunarChanged 当前时间阴历改变
 * @param date 日期
 * @param detail 黄历
 * @param type
 */
void CDayWindow::slotcurrentDateLunarChanged(QDate date, CaHuangLiDayInfo detail, int type)
{
    //当前时间
    m_currentdate = date;
    if (type == 1) {
        QLocale locale;
        //根据系统语言分别设置
        if (locale.language() == QLocale::Chinese) {
            m_YearLabel->setText(QString::number(date.year()) + tr("Y") +
                                 QString::number(date.month()) + tr("M") +
                                 QString::number(date.day()) + tr("D"));
        } else {
            m_YearLabel->setText(locale.toString(date, "yyyy/M/d"));
        }
        //干支月干支天
        m_LunarLabel->setText(detail.mLunarMonthName + detail.mLunarDayName);
    }
}
/**
 * @brief CDayWindow::slotcurrentDateChanged 当前日期改变
 * @param date 日期
 */
void CDayWindow::slotcurrentDateChanged(QDate date)
{
    setDate(date);
    slotupdateSchedule(1);
}
/**
 * @brief CDayWindow::slotsearchDateSelect 选择搜索日期
 * @param date 日期
 */
void CDayWindow::slotsearchDateSelect(QDate date)
{
    setDate(date);
    slotupdateSchedule(1);
}
/**
 * @brief CDayWindow::slotIsDragging 正在移动
 * @param isDragging 是否在移动
 */
void CDayWindow::slotIsDragging(bool &isDragging)
{
    isDragging = m_scheduleView->IsDragging();
}
