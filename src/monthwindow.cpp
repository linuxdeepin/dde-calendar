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
#include <QMessageBox>
#include <QPainter>
#include <DPalette>
#include "schcedulesearchview.h"
#include "todybutton.h"
#include "scheduledatamanage.h"
DGUI_USE_NAMESPACE
CMonthWindow::CMonthWindow(QWidget *parent): QMainWindow (parent)
{
    setContentsMargins(0, 0, 0, 0);
    initUI();
    initConnection();
    initLunar();
}

CMonthWindow::~CMonthWindow()
{
    disconnect(m_today, &CTodyButton::clicked, this, &CMonthWindow::slottoday);
    delete  m_monthDayView;
    m_monthDayView = nullptr;
    delete  m_monthView;
    m_monthView = nullptr;
}

void CMonthWindow::setFirstWeekday(int weekday)
{
    m_monthView->setFirstWeekday(weekday);
}

void CMonthWindow::setDate(QDate date)
{
    if (!date.isValid())
        return;
    m_monthDayView->setCurrentDate(date);
    if (m_currentdate == date)
        return;
    m_currentdate = date;
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
        m_YearLabel->setText(QString::number(date.year()) + tr("Y"));
    } else {
        m_YearLabel->setText(QString::number(date.year()));
    }
    m_monthView->setCurrentDate(date);
    emit signalCurrentDate(date);
}

void CMonthWindow::setLunarVisible(bool state)
{
    m_monthView->setLunarVisible(state);
    m_YearLunarLabel->setVisible(state);
}

void CMonthWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        DPalette todaypa = m_today->palette();
//        todaypa.setColor(DPalette::ButtonText, QColor("#1D81EC"));
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
//        m_today->setTColor("#1D81EC", "#001A2E", "#0081FF");
        m_today->setTColor(todayColor, "#001A2E", "#0081FF");
        m_today->setshadowColor(sbcolor);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);

        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#F8F8F8");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);

    } else if (type == 2) {
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        DPalette todaypa = m_today->palette();
//        todaypa.setColor(DPalette::ButtonText, QColor("#0081FF"));
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Light, "#484848");
        todaypa.setColor(DPalette::Dark, "#414141");
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);

        m_today->setBColor("#484848", "#727272", "#242424", "#414141", "#535353", "#282828");
//        m_today->setTColor("#0081FF", "#FFFFFF", "#0081FF");
        m_today->setTColor(todayColor, "#FFFFFF", "#0081FF");
        m_today->setshadowColor(sbcolor);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#252525");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        DPalette gpa = m_gridWidget->palette();
        gpa.setColor(DPalette::Background, "#252525");
        m_gridWidget->setPalette(gpa);
        m_gridWidget->setBackgroundRole(DPalette::Background);
    }
    m_monthDayView->setTheMe(type);
    m_monthView->setTheMe(type);
}

void CMonthWindow::previousMonth()
{
    slideMonth(false);
}

void CMonthWindow::nextMonth()
{
    slideMonth(true);
}

void CMonthWindow::slotsearchDateSelect(QDate date)
{
    setDate(date);
    slotupdateSchedule();
}

void CMonthWindow::setSearchWFlag(bool flag)
{
    m_searchfalg = flag;
    m_monthDayView->setsearchfalg(flag);
    //m_schceduleSearchView->setVisible(flag);
}

void CMonthWindow::clearSearch()
{
    //m_schceduleSearchView->clearSearch();
}

void CMonthWindow::wheelEvent(QWheelEvent *e)
{
    QPoint numDegrees = e->angleDelta();
    if (numDegrees.y()>0) {
        nextMonth();
    } else {
        previousMonth();
    }
}

void CMonthWindow::initUI()
{
    m_contentBackground = new DFrame;
    m_contentBackground->setContentsMargins(0, 0, 0, 0);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setAutoFillBackground(true);
    m_contentBackground->setPalette(anipa);

    m_today = new CTodyButton;
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(DDEMonthCalendar::MTodayWindth, DDEMonthCalendar::MTodayHeight);
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    DPalette todaypa = m_today->palette();
//    todaypa.setColor(DPalette::ButtonText, QColor("#1D81EC"));
    todaypa.setColor(DPalette::ButtonText, todayColor);
    todaypa.setColor(DPalette::Dark, Qt::white);
    todaypa.setColor(DPalette::Light, Qt::white);

    QColor sbcolor("#002A57");
    sbcolor.setAlphaF(0.05);
    todaypa.setColor(DPalette::Shadow, sbcolor);
    QFont todayfont;
//    todayfont.setFamily("SourceHanSansSC");
    todayfont.setWeight(QFont::Medium);
    todayfont.setPixelSize(14);
    m_today->setFont(todayfont);
    m_today->setPalette(todaypa);
    m_YearLabel = new QLabel();
    m_YearLabel->setFixedHeight(DDEMonthCalendar::M_YLableHeight);
    m_YearLunarLabel = new QLabel();
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunatLabelWindth, DDEMonthCalendar::M_YLunatLabelHeight);

    QFont ylabelF;
//    ylabelF.setFamily("SourceHanSansSC");
    ylabelF.setWeight(QFont::Medium);
    ylabelF.setPixelSize(24);
    m_YearLabel->setFont(ylabelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(pa);
    //m_YearLabel->setStyleSheet("color:#3B3B3B;");

    ylabelF.setPixelSize(14);
    m_YearLunarLabel->setFont(ylabelF);
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
    //yeartitleLayout->addSpacing(30);
    //m_spaceitem = new QSpacerItem(30, 36, QSizePolicy::Fixed, QSizePolicy::Fixed);

    //yeartitleLayout->addItem(m_spaceitem);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_monthDayView, 0, Qt::AlignCenter);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);

    m_monthView = new CMonthView(this);
    QVBoxLayout *mhLayout = new QVBoxLayout;
    mhLayout->setMargin(0);
    mhLayout->setSpacing(0);
    // mhLayout->setContentsMargins(10, 2, 10, 10);
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

    m_tmainLayout = new QHBoxLayout;
    m_tmainLayout->setMargin(0);
    m_tmainLayout->setSpacing(0);
    m_tmainLayout->setContentsMargins(0, 0, 10, 0);
    m_tmainLayout->addLayout(hhLayout);
    //mainLayout->addStretch(1);

    //m_schceduleSearchView = new CSchceduleSearchView(this);
    //m_schceduleSearchView->setFixedWidth(200);

    QVBoxLayout *ssLayout = new QVBoxLayout;
    ssLayout->setMargin(0);
    ssLayout->setSpacing(0);
    ssLayout->setContentsMargins(0, 10, 0, 10);
    //ssLayout->addWidget(m_schceduleSearchView);
    m_tmainLayout->addLayout(ssLayout);
    //m_schceduleSearchView->setVisible(false);

    m_contentBackground->setLayout(m_tmainLayout);

    // m_animationContainer = new DFrame(m_contentBackground);
    // DPalette anipa = m_animationContainer->palette();
    // anipa.setColor(DPalette::Background, Qt::white);
    //m_animationContainer->setAutoFillBackground(true);
    //m_animationContainer->setPalette(anipa);
    //m_animationContainer->setStyleSheet("QFrame { background: rgba(0, 0, 0, 0) }");
    // m_animationContainer->setFixedSize(m_monthView->width(),
    //                                   m_monthView->height() - DDEMonthCalendar::MDayCellHeight);
    // m_animationContainer->move(10, 130);
    // m_animationContainer->hide();

    setCentralWidget(m_contentBackground);


}

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

void CMonthWindow::initLunar()
{
    m_monthView->setLunarVisible(true);
}

void CMonthWindow::slideMonth(bool next)
{
    QDate currentDate;
    if (next) {
        if (m_currentdate.year() == 1900 && m_currentdate.month() == 1) return;
        currentDate = m_currentdate.addMonths(-1);
    } else {
        currentDate = m_currentdate.addMonths(1);
    }
    setDate(currentDate);
    QDate tdate = QDate(m_currentdate.year(), m_currentdate.month(), 1);
    emit signalCurrentDate(tdate);
}

void CMonthWindow::slotReturnTodayUpdate()
{
    setDate(QDate::currentDate());
}

void CMonthWindow::slotScheduleHide()
{
    m_monthView->slotScheduleRemindWidget(false);
}

void CMonthWindow::slotupdateSchedule(int id)
{
    Q_UNUSED(id);
    m_monthView->slotSchceduleUpdate();
}

void CMonthWindow::slotTransitSchedule(int id)
{
    emit signalsWUpdateShcedule(this, id);
}

void CMonthWindow::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_monthView->setSelectSchedule(scheduleInfo);
}

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
        //m_monthDayView->setwindowFixw(dw, width());
    } else {
        //m_monthDayView->setwindowFixw(dw, width() - 0.2325 * width() + 0.5 - 260);
        m_monthDayView->setFixedSize(qRound(dw), dh);
    }
    //m_schceduleSearchView->setFixedWidth(0.2325 * width() + 0.5);
    QMainWindow::resizeEvent(event);
}

void CMonthWindow::slottoday()
{
    emit signalsReturnTodayUpdate(this);
    setDate(QDate::currentDate());
}

void CMonthWindow::slotcurrentDateLunarChanged(QDate date, CaLunarDayInfo detail, int type)
{
    QDate currentdate = m_currentdate;
    m_currentdate = date;
    if (type == 1) {
        QLocale locale;
        if (locale.language() == QLocale::Chinese) {
            m_YearLabel->setText(QString::number(date.year()) + tr("Y"));
        } else {
            m_YearLabel->setText(QString::number(date.year()));
        }
        // m_YearLabel->setText(QString::number(date.year()) + tr("Y"));
        m_YearLunarLabel->setText("-" + detail.mGanZhiYear + detail.mZodiac + "年-");
        //m_animationContainer->hide();

    } else if (type == 0) {
        if (date.month() != currentdate.month()) {
            m_monthDayView->setRCurrentDate(date);
        }
    }
    //m_monthDayView->setRCurrentDate(date);
}

void CMonthWindow::slotcurrentDateChanged(QDate date)
{
    m_currentdate = date;
    if (m_currentdate == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
}

void CMonthWindow::slotSelectedMonth(QDate date)
{
    m_currentdate = date;
    m_monthView->setCurrentDate(date);
    emit signalCurrentDate(date);
}
