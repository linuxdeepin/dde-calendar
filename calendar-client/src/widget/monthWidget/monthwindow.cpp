// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "monthwindow.h"
#include "monthview.h"
#include "monthdayview.h"
#include "constants.h"
#include "schedulesearchview.h"
#include "todaybutton.h"
#include "scheduledatamanage.h"

#include <DPalette>

#include <QMessageBox>
#include <QPainter>

DGUI_USE_NAMESPACE
CMonthWindow::CMonthWindow(QWidget *parent)
    : CScheduleBaseWidget(parent)
{
    initUI();
    initConnection();
    setLunarVisible(m_calendarManager->getShowLunar());
}

CMonthWindow::~CMonthWindow()
{
    disconnect(m_today, &CTodayButton::clicked, this, &CMonthWindow::slottoday);
    delete  m_monthDayView;
    m_monthDayView = nullptr;
    delete  m_monthView;
    m_monthView = nullptr;
}

/**
 * @brief setLunarVisible   设置是否显示阴历信息
 * @param state             是否显示阴历信息
 */
void CMonthWindow::setLunarVisible(bool state)
{
    m_monthView->setLunarVisible(state);
    m_YearLunarLabel->setVisible(state);
}

/**
 * @brief setTheMe  根据系统主题类型设置颜色
 * @param type      系统主题类型
 */
void CMonthWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        DPalette todayPa = m_today->palette();
        todayPa.setColor(DPalette::ButtonText, todayColor);
        todayPa.setColor(DPalette::Dark, Qt::white);
        todayPa.setColor(DPalette::Light, Qt::white);
        QColor sbColor("#002A57");
        sbColor.setAlphaF(0.05);
        todayPa.setColor(DPalette::Shadow, sbColor);
        m_today->setPalette(todayPa);
        QColor todayhover = "#000000";
        todayhover.setAlphaF(0.1);
        QColor todaypress = "#000000";
        todaypress.setAlphaF(0.2);
        m_today->setBColor("#FFFFFF", todayhover, todaypress, "#FFFFFF", todayhover, todaypress);
        m_today->setTColor(todayColor, "#001A2E", "#0081FF");
        m_today->setshadowColor(sbColor);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);

        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#F8F8F8");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);
    } else if (type == 2) {
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        DPalette todayPa = m_today->palette();
        todayPa.setColor(DPalette::ButtonText, todayColor);
        todayPa.setColor(DPalette::Light, "#484848");
        todayPa.setColor(DPalette::Dark, "#414141");
        QColor sbColor("#000000");
        sbColor.setAlphaF(0.05);
        todayPa.setColor(DPalette::Shadow, sbColor);
        m_today->setPalette(todayPa);

        m_today->setBColor("#484848", "#727272", "#242424", "#414141", "#535353", "#282828");
        m_today->setTColor(todayColor, "#FFFFFF", "#0081FF");
        m_today->setshadowColor(sbColor);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#252525");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);
    }
    m_monthDayView->setTheMe(type);
    m_monthView->setTheMe(type);
}

void CMonthWindow::setYearData()
{
    if (getSelectDate() == getCurrendDateTime().date()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
    if (getShowLunar()) {
        m_YearLabel->setText(QString::number(getSelectDate().year()) + tr("Y"));
    } else {
        m_YearLabel->setText(QString::number(getSelectDate().year()));
    }
}

/**
 * @brief CMonthWindow::updateShowDate  更新显示时间
 * @param isUpdateBar
 */
void CMonthWindow::updateShowDate(const bool isUpdateBar)
{
    setYearData();
    const QDate _selectDate = m_calendarManager->getCalendarDateDataManage()->getSelectDate();
    Qt::DayOfWeek _firstWeek = m_calendarManager->getCalendarDateDataManage()->getWeekFirstDay();
    m_monthView->setFirstWeekday(_firstWeek);
    QVector<QDate> _monthShowData = m_calendarManager->getCalendarDateDataManage()->getMonthDate(_selectDate.year(), _selectDate.month());
    m_startDate = _monthShowData.first();
    m_stopDate = _monthShowData.last();
    m_monthView->setShowDate(_monthShowData);
    m_monthView->setRemindWidgetTimeFormat(m_calendarManager->getCalendarDateDataManage()->getTimeFormat());
    if (isUpdateBar)
        m_monthDayView->setSelectDate(_selectDate); //设置12个月份显示
    //如果为中文环境则显示班休和农历信息
    if (getShowLunar()) {
        updateShowLunar();
    }
    //切换月份须更新显示日程
    updateShowSchedule();
}

/**
 * @brief CMonthWindow::setCurrentDateTime      设置当前时间
 * @param currentDate
 */
void CMonthWindow::setCurrentDateTime(const QDateTime &currentDate)
{
    CScheduleBaseWidget::setCurrentDateTime(currentDate);
    //更新当前时间
    m_monthView->setCurrentDate(currentDate.date());
}

/**
 * @brief CMonthWindow::updateShowSchedule  更新日程显示
 */
void CMonthWindow::updateShowSchedule()
{
    QMap<QDate, QVector<ScheduleDataInfo> > _monthScheduleInfo = m_calendarManager->getScheduleTask()->getScheduleInfo(m_startDate, m_stopDate);
    m_monthView->setScheduleInfo(_monthScheduleInfo);
}

/**
 * @brief CMonthWindow::updateShowLunar     更新显示农历信息
 */
void CMonthWindow::updateShowLunar()
{
    getLunarInfo();
    m_YearLunarLabel->setText(m_lunarYear);
    QMap<QDate, int> _monthFestivalInfo = m_calendarManager->getScheduleTask()->getFestivalInfo(m_startDate, m_stopDate);
    m_monthView->setFestival(_monthFestivalInfo);
    QMap<QDate, CaHuangLiDayInfo> _monthHuangLiInfo = m_calendarManager->getScheduleTask()->getHuangLiInfo(m_startDate, m_stopDate);
    m_monthView->setHuangLiInfo(_monthHuangLiInfo);
}

/**
 * @brief CMonthWindow::updateSearchScheduleInfo        更新搜索日程信息
 */
void CMonthWindow::updateSearchScheduleInfo()
{
    //获取搜索日程信息
    QVector<ScheduleDataInfo> _searchSchedule = m_calendarManager->getScheduleTask()->getSearchScheduleInfoVector();
    m_monthView->setSearchScheduleInfo(_searchSchedule);
}

/**
 * @brief CMonthWindow::setSelectSearchScheduleInfo     设置选中日程
 * @param info
 */
void CMonthWindow::setSelectSearchScheduleInfo(const ScheduleDataInfo &info)
{
    m_monthView->setSelectSchedule(info);
}

/**
 * @brief CMonthWindow::setSearchWFlag      设置是否显示搜索界面
 * @param flag
 */
void CMonthWindow::setSearchWFlag(bool flag)
{
    m_searchFlag = flag;
    m_monthDayView->setSearchflag(flag);
}

/**
 * @brief CMonthWindow::deleteselectSchedule 快捷键删除日程
 */
void CMonthWindow::deleteselectSchedule()
{
    m_monthView->deleteSelectSchedule();
}

/**
 * @brief previousMonth 选择上一个月份
 */
void CMonthWindow::previousMonth()
{
    slideMonth(false);
}

/**
 * @brief nextMonth     选择下一个月份
 */
void CMonthWindow::nextMonth()
{
    slideMonth(true);
}

/**
 * @brief initUI 初始化界面
 */
void CMonthWindow::initUI()
{
    m_today = new CTodayButton;
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(DDEMonthCalendar::MTodayWindth, DDEMonthCalendar::MTodayHeight);
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    DPalette todayPa = m_today->palette();
    todayPa.setColor(DPalette::ButtonText, todayColor);
    todayPa.setColor(DPalette::Dark, Qt::white);
    todayPa.setColor(DPalette::Light, Qt::white);

    QColor sbColor("#002A57");
    sbColor.setAlphaF(0.05);
    todayPa.setColor(DPalette::Shadow, sbColor);
    QFont todayfont;
    todayfont.setWeight(QFont::Medium);
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);
    m_today->setPalette(todayPa);
    m_YearLabel = new QLabel();
    m_YearLabel->setFixedHeight(DDEMonthCalendar::M_YLabelHeight);
    m_YearLunarLabel = new QLabel();
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunaLabelWindth, DDEMonthCalendar::M_YLunaLabelHeight);

    QFont yLabelF;
    yLabelF.setWeight(QFont::Medium);
    yLabelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(yLabelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(pa);

    yLabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_YearLunarLabel->setFont(yLabelF);
    DPalette Lunarpa = m_YearLunarLabel->palette();
    Lunarpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_YearLunarLabel->setPalette(Lunarpa);

    m_monthDayView = new CMonthDayView(this);

    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->setContentsMargins(21, 20, 8, 10);
    yeartitleLayout->addWidget(m_YearLabel);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->setContentsMargins(14, 9, 0, 7);
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout->addLayout(yeartitleLayout1);

    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_monthDayView, 0, Qt::AlignCenter);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);

    m_monthView = new CMonthView(this);
    m_monthView->setObjectName("monthViewWidget");
    m_monthView->setAccessibleName("monthViewWidget");
    m_monthView->setCurrentDate(getCurrendDateTime().date());
    QVBoxLayout *mhLayout = new QVBoxLayout;
    mhLayout->setMargin(0);
    mhLayout->setSpacing(0);
    mhLayout->addWidget(m_monthView);

    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setSpacing(0);
    hhLayout->setMargin(0);

    hhLayout->addLayout(yeartitleLayout);
    m_gridWidget = new DWidget();
    m_gridWidget->setContentsMargins(0, 0, 0, 0);
    m_gridWidget->setAutoFillBackground(true);
    m_gridWidget->setLayout(mhLayout);

    hhLayout->addWidget(m_gridWidget);

    m_tMainLayout = new QHBoxLayout;
    m_tMainLayout->setMargin(0);
    m_tMainLayout->setSpacing(0);
    m_tMainLayout->setContentsMargins(0, 0, 10, 0);
    m_tMainLayout->addLayout(hhLayout);

    QVBoxLayout *ssLayout = new QVBoxLayout;
    ssLayout->setMargin(0);
    ssLayout->setSpacing(0);
    ssLayout->setContentsMargins(0, 10, 0, 10);
    m_tMainLayout->addLayout(ssLayout);

    this->setLayout(m_tMainLayout);
}

/**
 * @brief initConnection 初始化信号和槽的连接
 */
void CMonthWindow::initConnection()
{
    connect(m_today, &DPushButton::clicked, this, &CMonthWindow::slottoday);
    connect(m_monthDayView, &CMonthDayView::signalsSelectDate, this, &CMonthWindow::slotSetSelectDate);
    //双击时间修改选择时间和切换到日视图
    connect(m_monthView, &CMonthView::signalsViewSelectDate, this, &CMonthWindow::slotViewSelectDate);
    connect(m_monthView, &CMonthView::signalAngleDelta, this, &CMonthWindow::slotAngleDelta);
    //月份控件区域左右滚动信号关联
    connect(m_monthDayView, &CMonthDayView::signalAngleDelta, this, &CMonthWindow::slotAngleDelta);
    connect(m_monthView, &CMonthView::signalSwitchPrePage, this, &CMonthWindow::slotSwitchPrePage);
    connect(m_monthView, &CMonthView::signalSwitchNextPage, this, &CMonthWindow::slotSwitchNextPage);
}

/**
 * @brief slideMonth    切换月份，并更新信息
 * @param next          是否切换到下一个月
 */
void CMonthWindow::slideMonth(bool next)
{
    slotScheduleHide();
    if (next) {
        setSelectDate(getSelectDate().addMonths(-1), true);
    } else {
        setSelectDate(getSelectDate().addMonths(1), true);
    }
    //更新日程
    updateShowDate();
}

/**
 * @brief slotScheduleHide 隐藏日程浮框
 */
void CMonthWindow::slotScheduleHide()
{
    m_monthView->slotScheduleRemindWidget(false);
}

/**
 * @brief slotAngleDelta    接受滚动事件滚动相对量
 * @param delta             滚动相对量
 */
void CMonthWindow::slotAngleDelta(int delta)
{
    //拖拽时禁用
    if (!m_monthView->isDragging()) {
        if (delta > 0) {
            //下一个月
            nextMonth();
        } else {
            //上一个月
            previousMonth();
        }
    }
}

/**
 * @brief CMonthWindow::slotViewSelectDate          设置选择时间切换日视图
 * @param date
 */
void CMonthWindow::slotViewSelectDate(const QDate &date)
{
    slotScheduleHide();
    if (setSelectDate(date, true)) {
        //更新界面
        updateData();
        emit signalSwitchView(3);
    }
}

void CMonthWindow::slotSwitchPrePage()
{
    nextMonth();
}

void CMonthWindow::slotSwitchNextPage()
{
    previousMonth();
}

/**
 * @brief resizeEvent                   窗口大小调整
 * @param event                         窗口大小调整事件
 */
void CMonthWindow::resizeEvent(QResizeEvent *event)
{
    qreal dw = width() * 0.5023 + 0.5;
    int dh = 36;
    if (m_searchFlag) {
        m_tMainLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        m_tMainLayout->setContentsMargins(0, 0, 10, 0);
    }
    m_monthDayView->setFixedSize(qRound(dw), dh);
    QWidget::resizeEvent(event);
}

/**
 * @brief CMonthWindow::slottoday               返回当前时间
 */
void CMonthWindow::slottoday()
{
    //隐藏提示框
    slotScheduleHide();
    setSelectDate(getCurrendDateTime().date(), true);
    updateData();
}

/**
 * @brief CMonthWindow::slotSetSelectDate       设置选择时间
 * @param date
 */
void CMonthWindow::slotSetSelectDate(const QDate &date)
{
    slotScheduleHide();
    if (setSelectDate(date, true)) {
        updateShowDate(false);
    }
}
