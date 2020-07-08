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
#include <DPalette>
#include <QHBoxLayout>
#include <QMessageBox>
#include "constants.h"
#include "customframe.h"
#include "daymonthview.h"
#include "schcedulesearchview.h"
#include "scheduleview.h"
DGUI_USE_NAMESPACE
CDayWindow::CDayWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initUI();
    initConnection();
}

CDayWindow::~CDayWindow() {}
void CDayWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        // DPalette leftpa = m_leftground->palette();
        // leftpa.setColor(DPalette::Background, "#FFFFFF");
        // m_leftground->setPalette(leftpa);
        // m_leftground->setBackgroundRole(DPalette::Background);
        m_leftground->setBColor("#FFFFFF");

        DPalette ypa = m_YearLabel->palette();
        ypa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette lpa = m_LunarLabel->palette();
        lpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(DPalette::WindowText);
        DPalette spa = m_SolarDay->palette();
        spa.setColor(DPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        DPalette anipa = m_contentBackground->palette();
        QColor bcolor = "#252525";
        // bcolor.setAlphaF(0.05);
        anipa.setColor(DPalette::Background, bcolor);
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);
        // DPalette leftpa = m_leftground->palette();
        // leftpa.setColor(DPalette::Background, "#282828");
        // m_leftground->setPalette(leftpa);
        // m_leftground->setBackgroundRole(DPalette::Background);
        m_leftground->setBColor("#282828");

        DPalette ypa = m_YearLabel->palette();
        ypa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(ypa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette lpa = m_LunarLabel->palette();
        lpa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_LunarLabel->setPalette(lpa);
        m_LunarLabel->setForegroundRole(DPalette::WindowText);
        DPalette spa = m_SolarDay->palette();
        spa.setColor(DPalette::WindowText, Qt::red);
        m_SolarDay->setPalette(spa);
        m_SolarDay->setForegroundRole(DPalette::WindowText);
    }
    m_daymonthView->setTheMe(type);
    // m_schceduleSearchView->setTheMe(type);
    m_scheduleView->setTheMe(type);
}

void CDayWindow::setTime(QTime time)
{
    m_scheduleView->setTime(time);
}

void CDayWindow::updateHigh()
{
    m_scheduleView->updateHigh();
}
void CDayWindow::setDate(QDate date)
{
    if (!date.isValid())
        return;
    m_currentdate = date;
    m_daymonthView->setFirstWeekday(0);
    m_daymonthView->setCurrentDate(date);
    int w = m_scheduleView->width() - 72;
    m_scheduleView->setRange(w, 1032, m_currentdate, m_currentdate);
    emit signalCurrentDate(date);
}

void CDayWindow::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_scheduleView->setSelectSchedule(scheduleInfo);

}

void CDayWindow::setSearchWFlag(bool flag)
{
    m_searchfalg = flag;
    m_daymonthView->setSearchFlag(flag);
    // m_schceduleSearchView->setVisible(flag);
}

void CDayWindow::clearSearch()
{
    // m_schceduleSearchView->clearSearch();
}

void CDayWindow::setSearchText(QString str)
{
    m_searchText = str;
    // m_schceduleSearchView->slotsetSearch(str);
}

void CDayWindow::setLunarVisible(bool state)
{
    m_LunarLabel->setVisible(state);
    m_SolarDay->setVisible(state);
    m_scheduleView->setLunarVisible(state);
    m_daymonthView->setLunarVisible(state);
}

void CDayWindow::initUI()
{
    m_contentBackground = new DFrame;
    m_contentBackground->setAutoFillBackground(true);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setPalette(anipa);
    m_contentBackground->setFrameRounded(false);
    m_contentBackground->setLineWidth(0);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 9, 0, 3);

    m_YearLabel = new QLabel();
    //    m_YearLabel->setMinimumSize(175, DDEDayCalendar::D_YLableHeight);
    m_YearLabel->setMinimumHeight(DDEDayCalendar::D_YLableHeight);
    QFont labelF;
//    labelF.setFamily("SourceHanSansSC");
    labelF.setWeight(QFont::Medium);
    labelF.setPixelSize(24);
    m_YearLabel->setFont(labelF);
    DPalette ypa = m_YearLabel->palette();
    ypa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(ypa);
    titleLayout->addWidget(m_YearLabel);
    // titleLayout->addStretch(1);
    m_LunarLabel = new QLabel();
    titleLayout->addSpacing(15);
    m_LunarLabel->setFixedHeight(DDEDayCalendar::D_YLableHeight);
    labelF.setPixelSize(14);
    m_LunarLabel->setFont(labelF);
    m_LunarLabel->setAlignment(Qt::AlignCenter);
    DPalette lpa = m_LunarLabel->palette();
    lpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_LunarLabel->setPalette(lpa);
    titleLayout->addWidget(m_LunarLabel);
    // titleLayout->addStretch(1);
    m_SolarDay = new QLabel();
    labelF.setPixelSize(10);
    m_SolarDay->setFixedHeight(DDEDayCalendar::D_YLableHeight);
    m_SolarDay->setFont(labelF);
    m_SolarDay->setAlignment(Qt::AlignCenter);
    DPalette spa = m_SolarDay->palette();
    spa.setColor(DPalette::WindowText, Qt::red);
    m_SolarDay->setPalette(spa);
    titleLayout->addWidget(m_SolarDay);
    titleLayout->addStretch();

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setMargin(0);
    leftLayout->setSpacing(0);
    m_scheduleView = new CScheduleView(this, 1);
    // leftLayout->setContentsMargins(12, 0, 10, 12);
    m_scheduleView->setviewMagin(72, 109, 20, 0);
    m_scheduleView->setRange(423, 1032, QDate(2019, 8, 12), QDate(2019, 8, 12));
    // m_scheduleView->setFixedSize(513, 450);
    leftLayout->addLayout(titleLayout);
    leftLayout->addWidget(m_scheduleView);

    m_verline = new DVerticalLine;
    m_verline->setFixedWidth(2);

    m_daymonthView = new CDayMonthView(this);

    QHBoxLayout *lfetmainLayout = new QHBoxLayout;
    lfetmainLayout->setMargin(0);
    lfetmainLayout->setSpacing(1);
    lfetmainLayout->setContentsMargins(0, 0, 0, 0);
    lfetmainLayout->addLayout(leftLayout);
    lfetmainLayout->addWidget(m_verline);

    lfetmainLayout->addWidget(m_daymonthView);
    m_leftground = new CustomFrame();
    // m_leftground->setContentsMargins(0, 0, 0, 0);
    m_leftground->setRoundState(true, true, true, true);
    m_leftground->setLayout(lfetmainLayout);
    // m_leftground->setAutoFillBackground(true);
    // m_leftground->setFrameRounded(true);
    m_leftground->setBColor("#FFFFFF");
    // DPalette leftpa = m_leftground->palette();
    // leftpa.setColor(DPalette::Background, "#FFFFFF");
    // m_leftground->setPalette(leftpa);

    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->addWidget(m_leftground);
    // mainLayout->addStretch(1);

    // m_schceduleSearchView = new CSchceduleSearchView(this);
    // m_schceduleSearchView->setFixedWidth(200);
    // m_mainLayout->addWidget(m_schceduleSearchView);
    // m_schceduleSearchView->setVisible(false);

    m_contentBackground->setLayout(m_mainLayout);

    setCentralWidget(m_contentBackground);
}

void CDayWindow::initConnection()
{
    connect(m_daymonthView, &CDayMonthView::signalcurrentLunarDateChanged, this,
            &CDayWindow::slotcurrentDateLunarChanged);
    connect(m_daymonthView, &CDayMonthView::signalcurrentDateChanged, this,
            &CDayWindow::slotcurrentDateChanged);
    connect(m_daymonthView, &CDayMonthView::signalsReturnDay, this,
            &CDayWindow::slotCurrentReturnDay);
    connect(m_scheduleView, &CScheduleView::signalViewtransparentFrame, this,
            &CDayWindow::signalViewtransparentFrame);

    connect(m_scheduleView, &CScheduleView::signalsUpdateShcedule, this,
            &CDayWindow::slotTransitSchedule);
    connect(m_daymonthView, &CDayMonthView::signalSchedulHide
            , this, &CDayWindow::slotScheduleHide);
    // connect(m_schceduleSearchView, &CSchceduleSearchView::signalsUpdateShcedule, this,
    // &CDayWindow::slotTransitSearchSchedule);
    // connect(m_schceduleSearchView, &CSchceduleSearchView::signalDate, this,
    // &CDayWindow::slotsearchDateSelect);
}

void CDayWindow::slotupdateSchedule(int id)
{
    Q_UNUSED(id);
    m_daymonthView->setCurrentDate(m_currentdate);
    m_scheduleView->setRange(m_currentdate, m_currentdate);
    m_daymonthView->updateFlag();
//    if (m_daymonthView->getCurrDate() != m_currentdate) {
//        m_scheduleView->setRange(m_currentdate, m_currentdate);
//        qDebug()<<Q_FUNC_INFO;
////        m_scheduleView->setDate(m_currentdate);
////        m_scheduleView->slotupdateSchedule(id);
//    } else {
//        m_scheduleView->slotupdateSchedule(id);
//        qDebug()<<Q_FUNC_INFO<<"2222";
//    }
}

void CDayWindow::slotTransitSchedule(int id)
{
    // m_schceduleSearchView->slotsetSearch(m_searchText);
    emit signalsWUpdateShcedule(this, id);
    m_daymonthView->updateFlag();
}

void CDayWindow::slotTransitSearchSchedule(int id)
{
    m_scheduleView->slotupdateSchedule();
    // m_schceduleSearchView->slotsetSearch(m_searchText);
    emit signalsWUpdateShcedule(this, id);
}

void CDayWindow::slotReturnTodayUpdate()
{
    m_daymonthView->slottoday();
}

void CDayWindow::slotCurrentReturnDay()
{
    emit signalsReturnTodayUpdate(this);
}

void CDayWindow::slotScheduleHide()
{
    m_scheduleView->slotScheduleShow(false);
}

void CDayWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    qreal dw = 0.4046 * width();
    int dh = height() - 20;
    if (m_searchfalg) {
        m_mainLayout->setContentsMargins(10, 10, 0, 10);
    } else {
        m_mainLayout->setContentsMargins(10, 10, 10, 10);
    }
    if (dw < 350)
        dw = 350;
    m_daymonthView->setFixedSize(qRound(dw), dh);
}

void CDayWindow::slotcurrentDateLunarChanged(QDate date, CaHuangLiDayInfo detail, int type)
{
    m_currentdate = date;
    if (type == 1) {
        QLocale locale;
        if (locale.language() == QLocale::Chinese) {
            m_YearLabel->setText(QString::number(date.year()) + tr("Y") +
                                 QString::number(date.month()) + tr("M") +
                                 QString::number(date.day()) + tr("D"));
        } else {
            m_YearLabel->setText(locale.toString(date, "yyyy/M/d"));
        }
        m_LunarLabel->setText(detail.mLunarMonthName + detail.mLunarDayName);
    }
//    m_scheduleView->setRange(m_currentdate, m_currentdate);
//    emit signalCurrentDate(date);
}

void CDayWindow::slotcurrentDateChanged(QDate date)
{
    setDate(date);
    slotupdateSchedule(1);
}

void CDayWindow::slotsearchDateSelect(QDate date)
{
    setDate(date);
    slotupdateSchedule(1);
}
