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
#include "monthwindow.h"
#include "monthview.h"
#include "monthdayview.h"
#include "constants.h"
#include "calendardbus.h"
#include "schcedulesearchview.h"
#include "todybutton.h"
#include "scheduledatamanage.h"

#include <DPalette>

#include <QMessageBox>
#include <QPainter>
#include <QWheelEvent>

DGUI_USE_NAMESPACE
/**
 * @brief CMonthWindow 构造函数
 * @param parent 父类
 */
CMonthWindow::CMonthWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //设置边距
    setContentsMargins(0, 0, 0, 0);
    //初始化界面
    initUI();
    //初始化信号和槽的连接
    initConnection();
    //初始化阴历信息
    initLunar();
}
/**
  * @brief ~CMonthWindow 析构函数
  */
CMonthWindow::~CMonthWindow()
{
    disconnect(m_today, &CTodyButton::clicked, this, &CMonthWindow::slottoday);
    delete  m_monthDayView;
    m_monthDayView = nullptr;
    delete  m_monthView;
    m_monthView = nullptr;
}
/**
 * @brief setFirstWeekday 设置每周第一天是周几
 * @param weekday 周几
 */
void CMonthWindow::setFirstWeekday(int weekday)
{
    m_monthView->setFirstWeekday(weekday);
}
/**
 * @brief setDate 设置时间
 * @param date 日期
 */
void CMonthWindow::setDate(QDate date)
{
    //如果日期无效，return
    if (!date.isValid())
        return;
    //设置当天时间
    m_monthDayView->setCurrentDate(date);
    //如果date正好是当天时间，return
    if (m_currentdate == date)
        return;
    //设置当天时间为date
    m_currentdate = date;
    QLocale locale;
    //根据系统语言设置显示方式
    if (locale.language() == QLocale::Chinese) {
        //中文系统，显示XXXX年
        m_YearLabel->setText(QString::number(date.year()) + tr("Y"));
    } else {
        //其他语言，只显示年份
        m_YearLabel->setText(QString::number(date.year()));
    }
    //设置当前时间
    m_monthView->setCurrentDate(date);
    emit signalCurrentDate(date);
}
/**
 * @brief setLunarVisible 设置是否显示阴历信息
 * @param state 是否显示阴历信息
 */
void CMonthWindow::setLunarVisible(bool state)
{
    m_monthView->setLunarVisible(state);
    m_YearLunarLabel->setVisible(state);
}
/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void CMonthWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        //系统主题为浅色或跟随系统
        //设置背景色
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置返回今天按钮的颜色
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        DPalette todaypa = m_today->palette();
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Dark, Qt::white);
        todaypa.setColor(DPalette::Light, Qt::white);
        QColor sbcolor("#002A57");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        QColor todayhover = "#000000";
        todayhover.setAlphaF(0.1);
        QColor todaypress = "#000000";
        todaypress.setAlphaF(0.2);
        m_today->setBColor("#FFFFFF", todayhover, todaypress, "#FFFFFF", todayhover, todaypress);
        m_today->setTColor(todayColor, "#001A2E", "#0081FF");
        m_today->setshadowColor(sbcolor);
        //设置年份label的颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置年份阴历的颜色
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        //设置背景色
        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#F8F8F8");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);
    } else if (type == 2) {
        //系统主题为深色
        //设置返回今天按钮的颜色
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        DPalette todaypa = m_today->palette();
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Light, "#484848");
        todaypa.setColor(DPalette::Dark, "#414141");
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        m_today->setBColor("#484848", "#727272", "#242424", "#414141", "#535353", "#282828");
        m_today->setTColor(todayColor, "#FFFFFF", "#0081FF");
        m_today->setshadowColor(sbcolor);
        //设置年份label的颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年份的颜色
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        //设置背景色
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#252525");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置背景色
        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#252525");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);
    }
    //设置月份的系统主题
    m_monthDayView->setTheMe(type);
    //设置monthview的系统主题
    m_monthView->setTheMe(type);
}
/**
 * @brief previousMonth 选择上一个月份
 */
void CMonthWindow::previousMonth()
{
    slideMonth(false);
}
/**
 * @brief nextMonth 选择下一个月份
 */
void CMonthWindow::nextMonth()
{
    slideMonth(true);
}
/**
 * @brief slotsearchDateSelect 选择搜索到的日期
 * @param date 日期
 */
void CMonthWindow::slotsearchDateSelect(QDate date)
{
    //设置时间
    setDate(date);
    //更新日程信息
    slotupdateSchedule();
}
/**
 * @brief setSearchWFlag 设置是否选择的标志
 * @param flag 是否选择的标志
 */
void CMonthWindow::setSearchWFlag(bool flag)
{
    //设置搜索标志
    m_searchfalg = flag;
    //设置月份的搜索标志
    m_monthDayView->setsearchfalg(flag);
}
/**
 * @brief clearSearch
 */
void CMonthWindow::clearSearch()
{
}
/**
 * @brief wheelEvent 鼠标滚轮事件
 */
void CMonthWindow::wheelEvent(QWheelEvent *e)
{
    //拖拽时禁用
    if (!m_monthView->isDragging()) {
        QPoint numDegrees = e->angleDelta();

        if (numDegrees.y()>0) {
            //下一个月
            nextMonth();
        } else {
            //上一个月
            previousMonth();
        }
    }
}
/**
 * @brief initUI 初始化界面
 */
void CMonthWindow::initUI()
{
    //新建背景frame
    m_contentBackground = new DFrame;
    m_contentBackground->setContentsMargins(0, 0, 0, 0);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setAutoFillBackground(true);
    m_contentBackground->setPalette(anipa);
    //设新建返回今天的按钮
    m_today = new CTodyButton;
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(DDEMonthCalendar::MTodayWindth, DDEMonthCalendar::MTodayHeight);
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    DPalette todaypa = m_today->palette();
    todaypa.setColor(DPalette::ButtonText, todayColor);
    todaypa.setColor(DPalette::Dark, Qt::white);
    todaypa.setColor(DPalette::Light, Qt::white);
    QColor sbcolor("#002A57");
    sbcolor.setAlphaF(0.05);
    todaypa.setColor(DPalette::Shadow, sbcolor);
    QFont todayfont;
    todayfont.setWeight(QFont::Medium);
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);
    m_today->setPalette(todaypa);
    //新建年份label
    m_YearLabel = new QLabel();
    m_YearLabel->setFixedHeight(DDEMonthCalendar::M_YLableHeight);
    //新建阴历年份label
    m_YearLunarLabel = new QLabel();
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunatLabelWindth, DDEMonthCalendar::M_YLunatLabelHeight);
    //设置年份label的字体
    QFont ylabelF;
    ylabelF.setWeight(QFont::Medium);
    ylabelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(ylabelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(pa);
    ylabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_YearLunarLabel->setFont(ylabelF);
    DPalette Lunarpa = m_YearLunarLabel->palette();
    Lunarpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_YearLunarLabel->setPalette(Lunarpa);
    //新建月份显示
    m_monthDayView = new CMonthDayView(this);
    //新建title布局
    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->setContentsMargins(21, 20, 8, 10);
    yeartitleLayout->addWidget(m_YearLabel);
    //新建title1布局
    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->setContentsMargins(14, 9, 0, 7);
    //将阴历年份label添加到title1布局中
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    //将title1布局添加到title布局中
    yeartitleLayout->addLayout(yeartitleLayout1);
    //添加弹簧
    yeartitleLayout->addStretch();
    //将月份添加到title布局中
    yeartitleLayout->addWidget(m_monthDayView, 0, Qt::AlignCenter);
    yeartitleLayout->addStretch();
    //将返回今天按钮添加到title布局中
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);
    //新建一个月的视图
    m_monthView = new CMonthView(this);
    //新建一个月的布局
    QVBoxLayout *mhLayout = new QVBoxLayout;
    mhLayout->setMargin(0);
    mhLayout->setSpacing(0);
    //将视图添加到布局中
    mhLayout->addWidget(m_monthView);
    //新建垂直布局
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setSpacing(0);
    hhLayout->setMargin(0);
    //将title布局添加到垂直布局中
    hhLayout->addLayout(yeartitleLayout);
    //新建月widget
    m_gridWidget = new DWidget();
    m_gridWidget->setContentsMargins(0, 0, 0, 0);
    m_gridWidget->setAutoFillBackground(true);
    //月widget设置布局为一个月的布局
    m_gridWidget->setLayout(mhLayout);
    //将月widget添加到垂直布局中
    hhLayout->addWidget(m_gridWidget);
    //新建最终布局
    m_tmainLayout = new QHBoxLayout;
    m_tmainLayout->setMargin(0);
    m_tmainLayout->setSpacing(0);
    m_tmainLayout->setContentsMargins(0, 0, 10, 0);
    //将垂直布局添加到最终布局中
    m_tmainLayout->addLayout(hhLayout);

    QVBoxLayout *ssLayout = new QVBoxLayout;
    ssLayout->setMargin(0);
    ssLayout->setSpacing(0);
    ssLayout->setContentsMargins(0, 10, 0, 10);
    m_tmainLayout->addLayout(ssLayout);
    //frame设置布局为最终布局
    m_contentBackground->setLayout(m_tmainLayout);
    //设置中心widget为frame
    setCentralWidget(m_contentBackground);
}
/**
 * @brief initConnection 初始化信号和槽的连接
 */
void CMonthWindow::initConnection()
{
    connect(m_today, &DPushButton::clicked, this, &CMonthWindow::slottoday);
    connect(m_monthView, &CMonthView::signalcurrentLunarDateChanged, this, &CMonthWindow::slotcurrentDateLunarChanged);
    connect(m_monthView, &CMonthView::signalcurrentDateChanged, this, &CMonthWindow::slotcurrentDateChanged);
    connect(m_monthDayView, &CMonthDayView::signalsSelectDate, this, &CMonthWindow::slotSelectedMonth);
    connect(m_monthView, &CMonthView::signalsSchceduleUpdate, this, &CMonthWindow::slotTransitSchedule);
    connect(m_monthDayView, &CMonthDayView::signalsCurrentDate, this, &CMonthWindow::slotSelectedMonth);
    connect(m_monthView, &CMonthView::signalsCurrentScheduleDate, this, &CMonthWindow::signalsCurrentScheduleDate);
    connect(m_monthView, &CMonthView::signalViewtransparentFrame, this, &CMonthWindow::signalViewtransparentFrame);
    connect(m_monthView, &CMonthView::signalsViewSelectDate, this, &CMonthWindow::signalsViewSelectDate);
}
/**
 * @brief initLunar 初始化阴历信息
 */
void CMonthWindow::initLunar()
{
    m_monthView->setLunarVisible(true);
}
/**
 * @brief slideMonth 切换月份，并更新信息
 * @param next 是否切换到下一个月
 */
void CMonthWindow::slideMonth(bool next)
{
    QDate currentDate;

    if (next) {
        //如果是1900年1月，return
        if (m_currentdate.year() == DDECalendar::QueryEarliestYear && m_currentdate.month() == 1)
            return;
        //上一个月
        currentDate = m_currentdate.addMonths(-1);
    } else {
        //下一个月
        currentDate = m_currentdate.addMonths(1);
    }
    //设置当前时间
    setDate(currentDate);
    QDate tdate = QDate(m_currentdate.year(), m_currentdate.month(), 1);
    emit signalCurrentDate(tdate);
}
/**
 * @brief slotReturnTodayUpdate 返回今天
 */
void CMonthWindow::slotReturnTodayUpdate()
{
    setDate(QDate::currentDate());
}
/**
 * @brief slotScheduleHide 隐藏日程浮框
 */
void CMonthWindow::slotScheduleHide()
{
    m_monthView->slotScheduleRemindWidget(false);
}
/**
 * @brief slotupdateSchedule 更新日程
 * @param id
 */
void CMonthWindow::slotupdateSchedule(int id)
{
    Q_UNUSED(id);
    m_monthView->slotSchceduleUpdate();
}
/**
 * @brief slotTransitSchedule 发送更新日程的信号
 * @param id
 */
void CMonthWindow::slotTransitSchedule(int id)
{
    emit signalsWUpdateShcedule(this, id);
}
/**
 * @brief setSelectSchedule 设置选择的日程
 * @param scheduleInfo 选择日程的信息
 */
void CMonthWindow::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_monthView->setSelectSchedule(scheduleInfo);
}
/**
 * @brief resizeEvent 窗口大小调整
 * @param event 窗口大小调整事件
 */
void CMonthWindow::resizeEvent(QResizeEvent *event)
{
    qreal dw = width() * 0.5023 + 0.5;
    int dh = 36;

    if (m_searchfalg) {
        m_tmainLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        m_tmainLayout->setContentsMargins(0, 0, 10, 0);
    }

    if (!m_searchfalg) {
        m_monthDayView->setFixedSize(qRound(dw), dh);
    } else {
        m_monthDayView->setFixedSize(qRound(dw), dh);
    }

    QMainWindow::resizeEvent(event);
}
/**
 * @brief slottoday 返回今天
 */
void CMonthWindow::slottoday()
{
    emit signalsReturnTodayUpdate(this);
    setDate(QDate::currentDate());
}
/**
 * @brief slotcurrentDateLunarChanged 当前时间改变，更新信息
 * @param date 时间
 * @param detail 阴历信息
 * @param type 是否显示阴历信息
 */
void CMonthWindow::slotcurrentDateLunarChanged(QDate date, CaLunarDayInfo detail, int type)
{
    QDate currentdate = m_currentdate;
    m_currentdate = date;

    if (type == 1) {
        QLocale locale;
        //根据系统语言设置年份
        if (locale.language() == QLocale::Chinese) {
            //中文
            m_YearLabel->setText(QString::number(date.year()) + tr("Y"));
        } else {
            //其他语言
            m_YearLabel->setText(QString::number(date.year()));
        }
        //设置阴历年份
        m_YearLunarLabel->setText("-" + detail.mGanZhiYear + detail.mZodiac + "年-");
    } else if (type == 0) {
        if (date.month() != currentdate.month()) {
            m_monthDayView->setRCurrentDate(date);
        }
    }
}
/**
 * @brief slotcurrentDateChanged 根据时间变化，返回今天按钮状态变化
 * @param date 时间
 */
void CMonthWindow::slotcurrentDateChanged(QDate date)
{
    m_currentdate = date;

    if (m_currentdate == QDate::currentDate()) {
        //如果当天日期为今天
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        //如果当前日期不是今天
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
}
/**
 * @brief slotSelectedMonth 设置选择的月份
 * @param date 日期
 */
void CMonthWindow::slotSelectedMonth(QDate date)
{
    m_currentdate = date;
    m_monthView->setCurrentDate(date);
    emit signalCurrentDate(date);
}
