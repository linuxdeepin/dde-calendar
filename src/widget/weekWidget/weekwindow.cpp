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
#include "weekwindow.h"
#include "scheduleview.h"
#include "constants.h"
#include "calendardbus.h"
#include "weekheadview.h"
#include "weekview.h"
#include "schcedulesearchview.h"
#include "todybutton.h"
#include <scheduledatamanage.h>

#include <DPalette>
#include <DHiDPIHelper>

#include <QMessageBox>
#include <QDate>
#include <QHBoxLayout>

DGUI_USE_NAMESPACE
/**
 * @brief CWeekWindow 构造函数
 * @param parent 父类
 */
CWeekWindow::CWeekWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //设置边距
    setContentsMargins(0, 0, 0, 0);
    //初始化界面
    initUI();
    //初始化信号连接
    initConnection();
}
/**
  *@brief ~CWeekWindow 析构函数
  */
CWeekWindow::~CWeekWindow()
{

}
/**
 * @brief setDate 设置今天的日期
 * @param date 日期
 */
void CWeekWindow::setDate(QDate date)
{
    //日期无效返回
    if (!date.isValid()) return;
    //设置当前时间
    m_currentdate = date;
    //设置时间
    m_weekview->setCurrentDate(date);
    //更新
    update();
    emit signalCurrentDate(date);
}
/**
 * @brief setSelectSchedule 设置选择的日程
 * @param scheduleInfo 日程信息
 */
void CWeekWindow::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_scheduleView->setSelectSchedule(scheduleInfo);
}
/**
 * @brief setFirstWeekday 设置每周以周几开始
 * @param weekday 周几
 */
void CWeekWindow::setFirstWeekday(int weekday)
{
    m_weekview->setFirstWeekDay(weekday);
    m_weekHeadView->setFirstWeekday(weekday);
    m_scheduleView->setFirstWeekday(weekday);
}
/**
 * @brief setLunarVisible 设置是否显示阴历信息
 * @param state 是否显示阴历信息
 */
void CWeekWindow::setLunarVisible(bool state)
{
    m_weekHeadView->setLunarVisible(state);
    m_YearLunarLabel->setVisible(state);
    m_scheduleView->setLunarVisible(state);
}
/**
 * @brief initUI 初始化界面设置
 */
void CWeekWindow::initUI()
{
    //新建背景frame
    m_contentBackground = new DFrame;
    m_contentBackground->setAutoFillBackground(true);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setPalette(anipa);
    m_contentBackground->setContentsMargins(0, 0, 0, 0);
    //新建返回今天按钮
    m_today = new CTodyButton;
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(DDEWeekCalendar::WTodayWindth, DDEWeekCalendar::WTodayHeight);
    DPalette todaypa = m_today->palette();
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
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
    //上一周按钮
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    m_prevButton->setFixedSize(36, 36);
    //下一周按钮
    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    m_nextButton->setFixedSize(36, 36);
    //新建年份label
    m_YearLabel = new QLabel();
    m_YearLabel->setFixedHeight(DDEWeekCalendar::W_YLableHeight);
    //字体
    QFont t_labelF;
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(t_labelF);
    DPalette Lunadpa = m_YearLabel->palette();
    Lunadpa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(Lunadpa);
    //新建阴历年份label
    m_YearLunarLabel = new QLabel(m_contentBackground);
    m_YearLunarLabel->setFixedSize(DDEWeekCalendar::W_YLunatLabelWindth, DDEWeekCalendar::W_YLunatLabelHeight);
    //新建显示周数的view
    m_weekview  = new CWeekView(this);
    //新建周label
    m_weekLabel = new QLabel();
    m_weekLabel->setFixedHeight(DDEWeekCalendar::W_YLableHeight);
    QFont weeklabelF;
    weeklabelF.setWeight(QFont::Medium);
    weeklabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_weekLabel->setFont(weeklabelF);
    DPalette wpa = m_weekLabel->palette();
    wpa.setColor(DPalette::WindowText, QColor("#717171"));
    m_weekLabel->setPalette(wpa);
    m_weekLabel->setText(tr("Week"));
    //字体
    QFont ylabelF;
    ylabelF.setWeight(QFont::Medium);
    ylabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_YearLunarLabel->setFont(ylabelF);
    DPalette YearLpa = m_YearLunarLabel->palette();
    YearLpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    //设置阴历年份的颜色
    m_YearLunarLabel->setPalette(YearLpa);
    //新建title布局
    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->setContentsMargins(21, 20, 8, 0);
    //将年份label添加到title布局中
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
    yeartitleLayout->addStretch();
    //新建title中间的frame
    m_todayframe = new CustomFrame;
    m_todayframe->setContentsMargins(0, 0, 0, 0);
    m_todayframe->setRoundState(true, true, true, true);
    m_todayframe->setBColor(Qt::white);
    m_todayframe->setFixedHeight(DDEYearCalendar::Y_MLableHeight);
    m_todayframe->setboreder(1);
    //设置title中间的布局
    QHBoxLayout *todaylayout = new QHBoxLayout;
    todaylayout->setMargin(0);
    todaylayout->setSpacing(0);
    //将上一周的按钮加入布局
    todaylayout->addWidget(m_prevButton);
    //将显示周数的view加入布局
    todaylayout->addWidget(m_weekview);
    //将下一周的按钮加入布局
    todaylayout->addWidget(m_nextButton);
    //设置布局
    m_todayframe->setLayout(todaylayout);
    //将title中间的frame添加到title布局中
    yeartitleLayout->addWidget(m_todayframe, 0, Qt::AlignCenter);
    yeartitleLayout->addSpacing(10);
    //将周label添加到title布局中
    yeartitleLayout->addWidget(m_weekLabel, 0, Qt::AlignCenter);
    //添加弹簧
    yeartitleLayout->addStretch();
    //将返回今天按钮添加到title布局中
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);
    //新建周视图headview
    m_weekHeadView = new CWeekHeadView(this);
    //新建日程的view
    m_scheduleView = new CScheduleView(this);
    m_scheduleView->setviewMagin(73, 109 + 30, 0, 0);
    m_scheduleView->setRange(763, 1032, QDate(2019, 8, 12), QDate(2019, 8, 18));
    //新建周视图布局
    m_mainhLayout = new QVBoxLayout;
    m_mainhLayout->setMargin(0);
    m_mainhLayout->setSpacing(0);
    m_mainhLayout->setContentsMargins(8, 20, 0, 9);
    //将headview添加到周视图布局中
    m_mainhLayout->addWidget(m_weekHeadView);
    //将日程view添加到周视图布局中
    m_mainhLayout->addWidget(m_scheduleView);
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->addLayout(yeartitleLayout);
    hhLayout->addLayout(m_mainhLayout);
    //新建总体布局
    m_tmainLayout = new QHBoxLayout;
    m_tmainLayout->setMargin(0);
    m_tmainLayout->setSpacing(0);
    m_tmainLayout->setContentsMargins(0, 0, 10, 0);
    m_tmainLayout->addLayout(hhLayout);

    QVBoxLayout *ssLayout = new QVBoxLayout;
    ssLayout->setMargin(0);
    ssLayout->setSpacing(0);
    ssLayout->setContentsMargins(0, 0, 0, 10);
    m_tmainLayout->addLayout(ssLayout);
    //设置总体布局
    m_contentBackground->setLayout(m_tmainLayout);
    setCentralWidget(m_contentBackground);
}
/**
 * @brief initConnection 初始化信号和槽的连接
 */
void CWeekWindow::initConnection()
{
    connect(m_prevButton, &DIconButton::clicked, this, &CWeekWindow::slotprev);
    connect(m_today, &CTodyButton::clicked, this, &CWeekWindow::slottoday);
    connect(m_nextButton, &DIconButton::clicked, this, &CWeekWindow::slotnext);
    connect(m_weekview, &CWeekView::signalsSelectDate, this, &CWeekWindow::slotCurrentWeek);
    connect(m_weekHeadView, &CWeekHeadView::signalcurrentLunarDateChanged, this, &CWeekWindow::slotcurrentDateLunarChanged);
    connect(m_weekHeadView, &CWeekHeadView::signalcurrentDateChanged, this, &CWeekWindow::slotcurrentDateChanged);
    connect(m_scheduleView, &CScheduleView::signalsUpdateShcedule, this, &CWeekWindow::slotTransitSchedule);
    connect(m_scheduleView, &CScheduleView::signalsCurrentScheduleDate, this, &CWeekWindow::signalsCurrentScheduleDate);
    connect(m_scheduleView, &CScheduleView::signalViewtransparentFrame, this, &CWeekWindow::signalViewtransparentFrame);
    connect(m_weekHeadView, &CWeekHeadView::signalsViewSelectDate, this, &CWeekWindow::signalsViewSelectDate);
    connect(m_weekHeadView, &CWeekHeadView::signaleSchedulHide, this, &CWeekWindow::slotScheduleHide);
    connect(m_weekview,&CWeekView::signalIsDragging,this,&CWeekWindow::slotIsDragging);
}
/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void CWeekWindow::setTheMe(int type)
{
    //根据系统主题类型设置颜色
    if (type == 0 || type == 1) {
        //系统主题为浅色或跟随系统
        //设置背景色
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置返回今天按钮的颜色
        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
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
        //设置上一周按钮的颜色
        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        prevpa.setColor(DPalette::Light, QColor("#E3E3E3"));
        //设置下一周按钮的颜色
        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        nextvpa.setColor(DPalette::Light, QColor("#E3E3E3"));
        //返回今天按钮的背景色
        m_todayframe->setBColor(Qt::white);
        //设置年份label的颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年份label的颜色
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        //设置周label的颜色
        DPalette wpa = m_weekLabel->palette();
        wpa.setColor(DPalette::WindowText, QColor("#717171"));
        m_weekLabel->setPalette(wpa);
        m_weekLabel->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        //系统主题类型为神色
        //设置背景色
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#252525");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        //设置今天按钮的颜色
        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Dark, "#414141");
        todaypa.setColor(DPalette::Light, "#484848");
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        m_today->setBColor("#484848", "#727272", "#242424", "#414141", "#535353", "#282828");
        m_today->setTColor(todayColor, "#FFFFFF", "#0081FF");
        m_today->setshadowColor(sbcolor);
        //设置上一周按钮的颜色
        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#484848"));
        prevpa.setColor(DPalette::Light, QColor("#414141"));
        //设置下一周按钮的颜色
        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#484848"));
        nextvpa.setColor(DPalette::Light, QColor("#414141"));
        //设置返回今天按钮的背景色
        QColor bcolor = "#FFFFFF";
        bcolor.setAlphaF(0.05);
        m_todayframe->setBColor(bcolor);
        //设置年份label的颜色
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        //设置阴历年份label的颜色
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        //设置周label的颜色
        DPalette wpa = m_weekLabel->palette();
        wpa.setColor(DPalette::WindowText, QColor("#717171"));
        m_weekLabel->setPalette(wpa);
        m_weekLabel->setForegroundRole(DPalette::WindowText);
    }
    //设置weekview的系统主题
    m_weekview->setTheMe(type);
    //设置weekheadview的系统主题
    m_weekHeadView->setTheMe(type);
    //设置scheduleview的系统主题
    m_scheduleView->setTheMe(type);
}
/**
 * @brief setTime 设置CScheduleView的时间
 * @param time 时间
 */
void CWeekWindow::setTime(QTime time)
{
    m_scheduleView->setTime(time);
}
/**
 * @brief setSearchWFlag 设置搜索标志
 * @param flag 是否进行了搜索
 */
void CWeekWindow::setSearchWFlag(bool flag)
{
    m_searchfalg = flag;
    //设置weekview的搜索标志
    m_weekview->setsearchfalg(flag);
    update();
}
/**
 * @brief clearSearch
 */
void CWeekWindow::clearSearch()
{
}
/**
 * @brief updateHigh CScheduleView updateHigh
 */
void CWeekWindow::updateHigh()
{
    m_scheduleView->updateHigh();
}
/**
 * @brief slotReturnTodayUpdate 返回今天按钮，设置当前时间
 */
void CWeekWindow::slotReturnTodayUpdate()
{
    setDate(QDate::currentDate());
}
/**
 * @brief slotIsDragging CScheduleView IsDragging,判断是否可以拖拽
 * @param isDragging 是否可以拖拽
 */
void CWeekWindow::slotIsDragging(bool &isDragging)
{
    isDragging = m_scheduleView->IsDragging();
}
/**
 * @brief slotupdateSchedule 更新日程
 * @param id
 */
void CWeekWindow::slotupdateSchedule(int id)
{
    Q_UNUSED(id);
    m_scheduleView->slotupdateSchedule();
}
/**
 * @brief slotTransitSchedule 发送更新日程的信号
 * @param id
 */
void CWeekWindow::slotTransitSchedule(int id)
{
    emit signalsWUpdateShcedule(this, id);
}
/**
 * @brief slotTransitSearchSchedule 更新日程并发送信号
 * @param id
 */
void CWeekWindow::slotTransitSearchSchedule(int id)
{
    m_scheduleView->slotupdateSchedule();
    emit signalsWUpdateShcedule(this, id);
}
/**
 * @brief slotprev 切换到上一周，隐藏日程浮框
 */
void CWeekWindow::slotprev()
{
    //隐藏日程浮框
    slotScheduleHide();
    //设置今天的上一周时间
    QDate tcurrent = m_currentdate.addDays(-DDEWeekCalendar::AFewDaysofWeek);
    //早于1900年，return
    if (tcurrent.year() < DDECalendar::QueryEarliestYear)
        return;
    if (m_currentdate.year() >= DDECalendar::QueryEarliestYear) {
        //设置时间
        m_currentdate = tcurrent;
        setDate(m_currentdate);
    } else {
    }
}
/**
 * @brief slotnext 切换到下一周，隐藏日程浮框
 */
void CWeekWindow::slotnext()
{
    //隐藏日程浮框
    slotScheduleHide();
    //设置今天的下一周时间
    m_currentdate = m_currentdate.addDays(DDEWeekCalendar::AFewDaysofWeek);
    //设置时间
    setDate(m_currentdate);
}
/**
 * @brief slottoday 返回到今天，隐藏日程浮框
 */
void CWeekWindow::slottoday()
{
    //隐藏日程浮框
    slotScheduleHide();
    emit signalsReturnTodayUpdate(this);
    //设置今天时间
    setDate(QDate::currentDate());
}
/**
 * @brief slotCurrentWeek 设置每周开始的日期和今天的日期
 * @param date 每周开始的日期
 * @param currentDate 今天的日期
 */
void CWeekWindow::slotCurrentWeek(QDate date, QDate currentDate)
{
    QVector<QDate> vDate;
    //设置一周的时间
    for (int i = 0; i < DDEWeekCalendar::AFewDaysofWeek; i++)
        vDate.append(date.addDays(i));
    emit signalCurrentDate(vDate[0]);
    //当前时间
    m_currentdate = currentDate;
    //设置一周的时间
    m_weekHeadView->setWeekDay(vDate);
    //设置当期时间
    m_weekHeadView->setCurrentDate(m_currentdate);
    //设置时间
    m_scheduleView->setRange(date, date.addDays(6));
}
/**
 * @brief slotcurrentDateLunarChanged 切换显示当天的阴历信息
 * @param vdate 一周的时间
 * @param vdetail 阴历信息
 * @param type 是否显示阴历信息
 */
void CWeekWindow::slotcurrentDateLunarChanged(QVector<QDate> vdate, QVector<CaLunarDayInfo> vdetail, int type)
{
    int offset = 0;
    //获取当天在容器中的索引值
    for (int i = 0; i < vdate.count(); ++i) {
        if (vdate.at(i) == m_currentdate) {
            offset =i;
            break;
        }
    }
    //设置返回今天按钮的文字
    if (m_currentdate == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
    if (!vdate.isEmpty()) {
        //当天对应的阴历信息
        CaLunarDayInfo detail = vdetail.at(offset);
        if (type == 1) {
            //年份
            int yearnum = vdate.at(0).year();
            //最早年份为1900年
            if (yearnum < DDECalendar::QueryEarliestYear)
                yearnum = DDECalendar::QueryEarliestYear;
            QLocale locale;
            //根据系统语言设置年份label
            if (locale.language() == QLocale::Chinese) {
                m_YearLabel->setText(QString::number(yearnum) + tr("Y"));
            } else {
                m_YearLabel->setText(QString::number(yearnum));
            }
            //设置阴历年份
            m_YearLunarLabel->setText("-" + detail.mGanZhiYear + detail.mZodiac + "年-");
        }
    }
}
/**
 * @brief slotcurrentDateChanged 切换显示当天的时间，并改变返回今天按钮显示的文字
 * @param date 时间
 */
void CWeekWindow::slotcurrentDateChanged(QDate date)
{
    //设置返回今天按钮文字
    if (m_currentdate == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
    m_currentdate = date;
    m_scheduleView->setDate(date);
}
/**
 * @brief slotsearchDateSelect 选择选中的日期
 * @param date 日期
 */
void CWeekWindow::slotsearchDateSelect(QDate date)
{
    setDate(date);
    slotupdateSchedule();
}
/**
 * @brief slotScheduleHide 隐藏日程浮框
 */
void CWeekWindow::slotScheduleHide()
{
    m_scheduleView->slotScheduleShow(false);
}
/**
 * @brief resizeEvent 调整周视图窗口
 * @param event 窗口大小调整事件
 */
void CWeekWindow::resizeEvent(QResizeEvent *event)
{
    qreal sleftMagin = 0.093 * width() + 0.5;
    qreal headh = height() * 0.0924 + 0.5;
    qreal dw = width() * 0.4186 + 0.5;
    int dh = 36;
    int winframe = 10;
    m_mainhLayout->setContentsMargins(10, 20, 0, 10);

    if (m_searchfalg) {
        m_tmainLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        winframe += 10;
        m_tmainLayout->setContentsMargins(0, 0, 10, 0);
    }

    if (!m_searchfalg) {
        m_weekview->setFixedSize(qRound(dw), dh);
    } else {
        m_weekview->setFixedSize(qRound(dw - 100), dh);
    }

    m_weekHeadView->setMounthLabelWidth(qRound(sleftMagin + 1), qRound(width() * 0.9802 + 0.5));
    m_weekHeadView->setFixedSize(width() - winframe, qRound(headh));
    QMainWindow::resizeEvent(event);
}
/**
 * @brief mousePressEvent 鼠标单击隐藏日程浮框
 * @param event 鼠标事件
 */
void CWeekWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    slotScheduleHide();
}


