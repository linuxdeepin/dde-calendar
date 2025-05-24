// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "weekwindow.h"
#include "scheduleview.h"
#include "constants.h"
#include "weekheadview.h"
#include "weekview.h"
#include "commondef.h"
#include "schedulesearchview.h"
#include "todaybutton.h"
#include <scheduledatamanage.h>

#include <DPalette>

#include <QMessageBox>
#include <QDate>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QShortcut>
#include <QLoggingCategory>

// Add logging category
Q_LOGGING_CATEGORY(weekWindowLog, "calendar.week.window")

DGUI_USE_NAMESPACE
CWeekWindow::CWeekWindow(QWidget *parent)
    : CScheduleBaseWidget(parent)
    , m_today(new CTodayButton)
{
    qCDebug(weekWindowLog) << "Initializing CWeekWindow";
    setContentsMargins(0, 0, 0, 0);
    initUI();
    initConnection();
    setLunarVisible(m_calendarManager->getShowLunar());
}

CWeekWindow::~CWeekWindow()
{

}

/**
 * @brief setLunarVisible   设置是否显示阴历信息
 * @param state             是否显示阴历信息
 */
void CWeekWindow::setLunarVisible(bool state)
{
    qCDebug(weekWindowLog) << "Setting lunar visibility:" << state;
    m_weekHeadView->setLunarVisible(state);
    m_YearLunarLabel->setVisible(state);
    m_scheduleView->setLunarVisible(state);
}

/**
 * @brief initUI 初始化界面设置
 */
void CWeekWindow::initUI()
{
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(DDEWeekCalendar::WTodayWindth, DDEWeekCalendar::WTodayHeight);

    QFont todayfont;
    todayfont.setWeight(QFont::Medium);
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);
    //新建年份label
    m_YearLabel = new QLabel();
    m_YearLabel->setFixedHeight(DDEWeekCalendar::W_YLabelHeight);

    QFont t_labelF;
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(t_labelF);
    DPalette LunaPa = m_YearLabel->palette();
    LunaPa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(LunaPa);

    m_YearLunarLabel = new QLabel(this);
    m_YearLunarLabel->setFixedSize(DDEWeekCalendar::W_YLunatLabelWindth, DDEWeekCalendar::W_YLunatLabelHeight);

    m_weekview  = new CWeekView(&CalendarManager::getWeekNumOfYear, this);

    m_weekLabel = new QLabel();
    m_weekLabel->setFixedHeight(DDEWeekCalendar::W_YLabelHeight);
    QFont weeklabelF;
    weeklabelF.setWeight(QFont::Medium);
    weeklabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_weekLabel->setFont(weeklabelF);
    DPalette wpa = m_weekLabel->palette();
    wpa.setColor(DPalette::WindowText, QColor("#717171"));
    m_weekLabel->setPalette(wpa);
    m_weekLabel->setText(tr("Week"));

    QFont yLabelF;
    yLabelF.setWeight(QFont::Medium);
    yLabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_YearLunarLabel->setFont(yLabelF);
    DPalette YearLpa = m_YearLunarLabel->palette();
    YearLpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));

    m_YearLunarLabel->setPalette(YearLpa);

    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setContentsMargins(0, 0, 0, 0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->addSpacing(10);
    yeartitleLayout->addWidget(m_YearLabel);
    yeartitleLayout->addWidget(m_dialogIconButton);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setContentsMargins(0, 0, 0, 0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout->addSpacing(6);
    yeartitleLayout->addLayout(yeartitleLayout1);

    yeartitleLayout->addStretch();
    m_todayframe = new CustomFrame;
    m_todayframe->setContentsMargins(0, 0, 0, 0);
    m_todayframe->setRoundState(true, true, true, true);
    m_todayframe->setBColor(Qt::white);
    m_todayframe->setFixedHeight(DDEYearCalendar::Y_MLabelHeight);
    m_todayframe->setboreder(1);
    QHBoxLayout *todaylayout = new QHBoxLayout;
    todaylayout->setContentsMargins(0, 0, 0, 0);
    todaylayout->setSpacing(0);
    //将显示周数的view加入布局
    todaylayout->addWidget(m_weekview);
    //设置布局
    m_todayframe->setLayout(todaylayout);

    yeartitleLayout->addWidget(m_todayframe, 0, Qt::AlignCenter);
    yeartitleLayout->addSpacing(10);
    yeartitleLayout->addWidget(m_weekLabel, 0, Qt::AlignCenter);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);

    m_weekHeadView = new CWeekHeadView(this);
    m_scheduleView = new CScheduleView(this);
    m_scheduleView->setviewMargin(73, 109 + 30, 0, 0);
    m_scheduleView->setRange(763, 1032, QDate(2019, 8, 12), QDate(2019, 8, 18));

    m_mainHLayout = new QVBoxLayout;
    m_mainHLayout->setContentsMargins(0, 0, 0, 0);
    m_mainHLayout->setSpacing(0);
    m_mainHLayout->addWidget(m_weekHeadView, 1);
    m_mainHLayout->addWidget(m_scheduleView, 9);
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->setSpacing(0);
    //头部控件统一高度为 M_YTopHeight
    QWidget *top = new QWidget(this);
    top->setFixedHeight(DDEMonthCalendar::M_YTopHeight);
    top->setLayout(yeartitleLayout);
    hhLayout->addWidget(top);
    hhLayout->addLayout(m_mainHLayout);

    m_tMainLayout = new QHBoxLayout;
    m_tMainLayout->setContentsMargins(0, 0, 0, 0);
    m_tMainLayout->setSpacing(0);
    m_tMainLayout->addLayout(hhLayout);

    this->setLayout(m_tMainLayout);

    setTabOrder(m_weekview, m_today);
    setTabOrder(m_today, m_scheduleView);
}

/**
 * @brief initConnection 初始化信号和槽的连接
 */
void CWeekWindow::initConnection()
{
    connect(m_today, &CTodayButton::clicked, this, &CWeekWindow::slottoday);
    //周数信息区域前按钮点击事件关联触发前一周
    connect(m_weekview, &CWeekView::signalBtnPrev, this, &CWeekWindow::slotprev);
    //周数信息区域后按钮点击事件关联触发后一周
    connect(m_weekview, &CWeekView::signalBtnNext, this, &CWeekWindow::slotnext);
    connect(m_weekview, &CWeekView::signalsSelectDate, this, &CWeekWindow::slotSelectDate);
    connect(m_weekHeadView, &CWeekHeadView::signalsViewSelectDate, this, &CWeekWindow::slotViewSelectDate);
    connect(m_weekview, &CWeekView::signalIsDragging, this, &CWeekWindow::slotIsDragging);
    //日程信息区域滚动信号关联
    connect(m_scheduleView, &CScheduleView::signalAngleDelta, this, &CWeekWindow::slotAngleDelta);
    //每周信息区域滚动信号关联
    connect(m_weekHeadView, &CWeekHeadView::signalAngleDelta, this, &CWeekWindow::slotAngleDelta);
    //双击"..."标签切换日视图
    connect(m_scheduleView, &CScheduleView::signalsCurrentScheduleDate, this, &CWeekWindow::slotViewSelectDate);

    connect(m_scheduleView, &CScheduleView::signalSwitchPrePage, this, &CWeekWindow::slotSwitchPrePage);
    connect(m_scheduleView, &CScheduleView::signalSwitchNextPage, this, &CWeekWindow::slotSwitchNextPage);
}

/**
 * @brief setTheMe  根据系统主题类型设置颜色
 * @param type      系统主题类型
 */
void CWeekWindow::setTheMe(int type)
{
    qCDebug(weekWindowLog) << "Setting theme type:" << type;
    if (type == 0 || type == 1) {

        //返回今天按钮的背景色
        m_todayframe->setBColor(Qt::white);
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        DPalette wpa = m_weekLabel->palette();
        wpa.setColor(DPalette::WindowText, QColor("#717171"));
        m_weekLabel->setPalette(wpa);
        m_weekLabel->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {

        //设置返回今天按钮的背景色
        QColor bColor = "#FFFFFF";
        bColor.setAlphaF(0.05);
        m_todayframe->setBColor(bColor);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        DPalette wpa = m_weekLabel->palette();
        wpa.setColor(DPalette::WindowText, QColor("#717171"));
        m_weekLabel->setPalette(wpa);
        m_weekLabel->setForegroundRole(DPalette::WindowText);
    }
    m_weekview->setTheMe(type);
    m_weekHeadView->setTheMe(type);
    m_scheduleView->setTheMe(type);
}

/**
 * @brief setTime 设置CScheduleView的时间
 * @param time 时间
 */
void CWeekWindow::setTime(QTime time)
{
    qCDebug(weekWindowLog) << "Setting time:" << time;
    m_scheduleView->setTime(time);
}

/**
 * @brief setSearchWFlag 设置搜索标志
 * @param flag 是否进行了搜索
 */
void CWeekWindow::setSearchWFlag(bool flag)
{
    qCDebug(weekWindowLog) << "Setting search flag:" << flag;
    m_searchFlag = flag;
    update();
}

/**
 * @brief CWeekWindow::updateHeight       更新全天区域高度
 */
void CWeekWindow::updateHeight()
{
    qCDebug(weekWindowLog) << "Updating schedule view height";
    m_scheduleView->updateHeight();
}

/**
 * @brief CWeekWindow::setYearData      设置年显示和今天按钮显示
 */
void CWeekWindow::setYearData()
{
    qCDebug(weekWindowLog) << "Setting year data for date:" << getSelectDate();
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
 * @brief CWeekWindow::updateShowDate       更新显示时间
 * @param isUpdateBar
 */
void CWeekWindow::updateShowDate(const bool isUpdateBar)
{
    qCDebug(weekWindowLog) << "Updating show date, isUpdateBar:" << isUpdateBar;
    setYearData();
    QVector<QDate> _weekShowData = m_calendarManager->getWeekDate(getSelectDate());
    m_weekHeadView->setWeekDay(_weekShowData, getSelectDate());
    //获取一周的开始结束时间
    m_startDate = _weekShowData.first();
    m_stopDate = _weekShowData.last();
    //如果时间无效则打印log
    if (m_startDate.isNull() || m_stopDate.isNull()) {
        qCWarning(weekWindowLog) << "Week start or stop date is invalid";
    }
    //设置全天和非全天显示时间范围
    m_scheduleView->setRange(m_startDate, m_stopDate);
    m_scheduleView->setTimeFormat((m_calendarManager->getTimeShowType() ? "AP " : "") + m_calendarManager->getTimeFormat());
    //是否更新显示周数窗口
    if (isUpdateBar) {
        m_weekview->setCurrent(getCurrendDateTime());
        m_weekview->setSelectDate(getSelectDate());
    }
    if (getShowLunar())
        updateShowLunar();
    updateShowSchedule();
    update();
}

/**
 * @brief CWeekWindow::updateShowSchedule       更新日程显示
 */
void CWeekWindow::updateShowSchedule()
{
    qCDebug(weekWindowLog) << "Updating schedule display for dates:" << m_startDate << "to" << m_stopDate;
    m_scheduleView->setShowScheduleInfo(gScheduleManager->getScheduleMap(m_startDate, m_stopDate));
}

/**
 * @brief CWeekWindow::updateShowLunar                  更新显示农历信息
 */
void CWeekWindow::updateShowLunar()
{
    qCDebug(weekWindowLog) << "Updating lunar display";
    getLunarInfo();
    m_YearLunarLabel->setText(m_lunarYear);
    QMap<QDate, CaHuangLiDayInfo> weekHuangLiInfo = gLunarManager->getHuangLiDayMap(m_startDate, m_stopDate);
    m_weekHeadView->setHunagLiInfo(weekHuangLiInfo);
}

void CWeekWindow::updateSearchScheduleInfo()
{
    m_scheduleView->slotUpdateScene();
}

/**
 * @brief CWeekWindow::setSelectSearchScheduleInfo      设置选中搜索日程
 * @param info
 */
void CWeekWindow::setSelectSearchScheduleInfo(const DSchedule::Ptr &info)
{
    m_scheduleView->setSelectSchedule(info);
}

/**
 * @brief CWeekWindow::deleteselectSchedule 快捷键删除日程
 */
void CWeekWindow::deleteselectSchedule()
{
    m_scheduleView->slotDeleteitem();
}

/**
 * @brief CWeekWindow::slotIsDragging                   判断是否可以拖拽
 * @param isDragging
 */
void CWeekWindow::slotIsDragging(bool &isDragging)
{
    isDragging = m_scheduleView->IsDragging();
    qCDebug(weekWindowLog) << "Checking drag status:" << isDragging;
}

/**
 * @brief CWeekWindow::slotViewSelectDate       切换日视图并设置选择时间
 * @param date
 */
void CWeekWindow::slotViewSelectDate(const QDate &date)
{
    qCDebug(weekWindowLog) << "View select date:" << date;
    if (setSelectDate(date)) {
        //更加界面
        updateData();
        emit signalSwitchView(3);
    }
}

void CWeekWindow::slotSwitchPrePage()
{
    slotprev();
}

void CWeekWindow::slotSwitchNextPage()
{
    slotnext();
}

/**
 * @brief slotprev 切换到上一周，隐藏日程浮框
 */
void CWeekWindow::slotprev()
{
    if (m_isSwitchStatus) {
        qCDebug(weekWindowLog) << "Switch in progress, ignoring prev request";
        return;
    }

    qCDebug(weekWindowLog) << "Switching to previous week";
    m_isSwitchStatus = true;

    QTimer::singleShot(5, [this]() {
        switchDate(getSelectDate().addDays(-7));
        m_isSwitchStatus = false;
    });
}

/**
 * @brief slotnext 切换到下一周，隐藏日程浮框
 */
void CWeekWindow::slotnext()
{
    if (m_isSwitchStatus) {
        qCDebug(weekWindowLog) << "Switch in progress, ignoring next request";
        return;
    }

    qCDebug(weekWindowLog) << "Switching to next week";
    m_isSwitchStatus = true;

    QTimer::singleShot(5, [this]() {
        switchDate(getSelectDate().addDays(7));
        m_isSwitchStatus = false;
    });
}

/**
 * @brief slottoday     返回到当前时间，隐藏日程浮框
 */
void CWeekWindow::slottoday()
{
    qCDebug(weekWindowLog) << "Switching to today";
    switchDate(getCurrendDateTime().date());
}

/**
 * @brief CWeekWindow::slotSelectDate       修改选择时间
 * @param date
 */
void CWeekWindow::slotSelectDate(const QDate &date)
{
    qCDebug(weekWindowLog) << "Select date:" << date;
    //更新选择时间
    setSelectDate(date);
    updateShowDate(false);
}

/**
 * @brief slotAngleDelta    接受处理滚动相对量
 * @param delta             滚动相对量
 */
void CWeekWindow::slotAngleDelta(int delta)
{
    if (!m_scheduleView->IsDragging()) {
        qCDebug(weekWindowLog) << "Processing angle delta:" << delta;
        if (delta > 0) {
            slotprev();
        } else if (delta < 0) {
            slotnext();
        }
    }
}

/**
 * @brief CWeekWindow::switchDate       切换选择时间
 * @param date
 */
void CWeekWindow::switchDate(const QDate &date)
{
    qCDebug(weekWindowLog) << "Switching to date:" << date;
    //隐藏提示框
    slotScheduleHide();
    //设置选择时间
    if (setSelectDate(date, true)) {
        updateData();
    }
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
    qreal dw = width() * 0.4186 + 0.5;
    int dh = 36;

    //添加1个按钮的宽度 36。原来m_weekview 不包含前后按钮(若加2个按钮的宽度，会导致窗口缩小的时候按钮显示不全)
    if (!m_searchFlag) {
        m_weekview->setFixedSize(qRound(dw + 36), dh);
    } else {
        m_weekview->setFixedSize(qRound(dw - 100 + 36), dh);
    }
    QWidget::resizeEvent(event);
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


