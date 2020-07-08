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
#include <QMessageBox>
#include <QDate>
#include <QHBoxLayout>
#include <DPalette>
#include <DHiDPIHelper>
#include "schcedulesearchview.h"
#include "todybutton.h"
#include <scheduledatamanage.h>

DGUI_USE_NAMESPACE
CWeekWindow::CWeekWindow(QWidget *parent): QMainWindow (parent)
{
    setContentsMargins(0, 0, 0, 0);
    initUI();
    initConnection();

}

CWeekWindow::~CWeekWindow()
{

}

void CWeekWindow::setDate(QDate date)
{
    if (!date.isValid()) return;
    m_currentdate = date;
    m_weekview->setCurrentDate(date);
    update();
    emit signalCurrentDate(date);
}

void CWeekWindow::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    m_scheduleView->setSelectSchedule(scheduleInfo);
}

void CWeekWindow::setFirstWeekday(int weekday)
{
    m_weekview->setFirstWeekDay(weekday);
    m_weekHeadView->setFirstWeekday(weekday);
    m_scheduleView->setFirstWeekday(weekday);
}

void CWeekWindow::setLunarVisible(bool state)
{
    m_weekHeadView->setLunarVisible(state);
    m_YearLunarLabel->setVisible(state);
    m_scheduleView->setLunarVisible(state);
}

void CWeekWindow::initUI()
{
    m_contentBackground = new DFrame;
    m_contentBackground->setAutoFillBackground(true);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setPalette(anipa);
    m_contentBackground->setContentsMargins(0, 0, 0, 0);

    m_today = new CTodyButton;
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedSize(DDEWeekCalendar::WTodayWindth, DDEWeekCalendar::WTodayHeight);
    DPalette todaypa = m_today->palette();
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
//    todaypa.setColor(DPalette::ButtonText, QColor("#0098FF"));
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
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    //m_prevButton->setIconSize(QSize(36, 36));
    //m_prevButton->setIcon(QIcon(DHiDPIHelper::loadNxPixmap(":/resources/icon/previous_normal.svg")));
    //m_prevButton->setArrowDirection(DArrowButton::ArrowLeft);

    m_prevButton->setFixedSize(36, 36);

    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    //m_nextButton->setIconSize(QSize(36, 36));
    //m_nextButton->setIcon(QIcon(DHiDPIHelper::loadNxPixmap(":/resources/icon/next_normal.svg")));
    m_nextButton->setFixedSize(36, 36);
    //m_nextButton->setFixedSize(DDEWeekCalendar::WWeekCellWidth, DDEWeekCalendar::WWeekCellHeight);

    m_YearLabel = new QLabel();
    m_YearLabel->setFixedHeight(DDEWeekCalendar::W_YLableHeight);
    //m_currentMouth->setStyleSheet("border: 1px solid rgba(0, 0, 0, 0.05);");

    QFont t_labelF;
//    t_labelF.setFamily("SourceHanSansSC");
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(24);
    m_YearLabel->setFont(t_labelF);
    DPalette Lunadpa = m_YearLabel->palette();
    Lunadpa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(Lunadpa);
    //m_YearLabel->setStyleSheet("color:#3B3B3B;");

    m_YearLunarLabel = new QLabel(m_contentBackground);
    m_YearLunarLabel->setFixedSize(DDEWeekCalendar::W_YLunatLabelWindth, DDEWeekCalendar::W_YLunatLabelHeight);

    m_weekview  = new CWeekView(this);
    // m_weekview->setFixedHeight(DDEWeekCalendar::W_YLunatLabelHeight + 8);

    m_weekLabel = new QLabel();
    m_weekLabel->setFixedHeight(DDEWeekCalendar::W_YLableHeight);
    QFont weeklabelF;
//    weeklabelF.setFamily("PingFangSC");
    weeklabelF.setWeight(QFont::Medium);
    weeklabelF.setPixelSize(14);
    m_weekLabel->setFont(weeklabelF);
    DPalette wpa = m_weekLabel->palette();
    wpa.setColor(DPalette::WindowText, QColor("#717171"));
    m_weekLabel->setPalette(wpa);
    m_weekLabel->setText(tr("Week"));

    QFont ylabelF;
//    ylabelF.setFamily("SourceHanSansSC");
    ylabelF.setWeight(QFont::Medium);
    ylabelF.setPixelSize(14);
    m_YearLunarLabel->setFont(ylabelF);
    DPalette YearLpa = m_YearLunarLabel->palette();
    YearLpa.setColor(DPalette::WindowText, QColor("#8A8A8A"));

    m_YearLunarLabel->setPalette(YearLpa);
    //m_YearLunarLabel->move(116, 27);

    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->setContentsMargins(21, 20, 8, 0);
    yeartitleLayout->addWidget(m_YearLabel);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->setContentsMargins(14, 9, 0, 7);
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout->addLayout(yeartitleLayout1);



    //QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    //yeartitleLayout->setMargin(0);
    //yeartitleLayout->setSpacing(0);
    //yeartitleLayout->setContentsMargins(21, 10, 20, 20);
    // yeartitleLayout->addWidget(m_YearLabel);
    //yeartitleLayout->addSpacing(14);
    //yeartitleLayout->addWidget(m_YearLunarLabel);
    //yeartitleLayout->addSpacing(30);
    //m_spaceitem = new QSpacerItem(30, 36, QSizePolicy::Fixed, QSizePolicy::Fixed);

    //yeartitleLayout->addItem(m_spaceitem);
    yeartitleLayout->addStretch();
    m_todayframe = new CustomFrame;
    m_todayframe->setContentsMargins(0, 0, 0, 0);
    m_todayframe->setRoundState(true, true, true, true);
    m_todayframe->setBColor(Qt::white);
    m_todayframe->setFixedHeight(DDEYearCalendar::Y_MLableHeight);
    m_todayframe->setboreder(1);
    QHBoxLayout *todaylayout = new QHBoxLayout;
    todaylayout->setMargin(0);
    todaylayout->setSpacing(0);
    todaylayout->addWidget(m_prevButton);
    todaylayout->addWidget(m_weekview);
    todaylayout->addWidget(m_nextButton);
    m_todayframe->setLayout(todaylayout);

    yeartitleLayout->addWidget(m_todayframe, 0, Qt::AlignCenter);
    yeartitleLayout->addSpacing(10);
    yeartitleLayout->addWidget(m_weekLabel, 0, Qt::AlignCenter);
    yeartitleLayout->addStretch();
    yeartitleLayout->addWidget(m_today, 0, Qt::AlignRight);

    m_weekHeadView = new CWeekHeadView(this);
    m_scheduleView = new CScheduleView(this);
    //m_weekHeadView->setFixedWidth(840);
    m_scheduleView->setviewMagin(73, 109 + 30, 0, 0);
    m_scheduleView->setRange(763, 1032, QDate(2019, 8, 12), QDate(2019, 8, 18));
    //m_scheduleView->setFixedSize(838, 450);

    m_mainhLayout = new QVBoxLayout;
    m_mainhLayout->setMargin(0);
    m_mainhLayout->setSpacing(0);
    m_mainhLayout->setContentsMargins(8, 20, 0, 9);
    m_mainhLayout->addWidget(m_weekHeadView);
    m_mainhLayout->addWidget(m_scheduleView);
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->addLayout(yeartitleLayout);
    hhLayout->addLayout(m_mainhLayout);

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
    ssLayout->setContentsMargins(0, 0, 0, 10);
    //ssLayout->addWidget(m_schceduleSearchView);
    m_tmainLayout->addLayout(ssLayout);
    //m_schceduleSearchView->setVisible(false);

    m_contentBackground->setLayout(m_tmainLayout);
    setCentralWidget(m_contentBackground);
}

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
    // connect(m_schceduleSearchView, &CSchceduleSearchView::signalsUpdateShcedule, this, &CWeekWindow::slotTransitSearchSchedule);
    // connect(m_schceduleSearchView, &CSchceduleSearchView::signalDate, this, &CWeekWindow::slotsearchDateSelect);

}

void CWeekWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        todaypa.setColor(DPalette::ButtonText, todayColor);
//        todaypa.setColor(DPalette::ButtonText, QColor("#1D81EC"));
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

        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        prevpa.setColor(DPalette::Light, QColor("#E3E3E3"));
        // m_prevButton->setPalette(prevpa);

        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        nextvpa.setColor(DPalette::Light, QColor("#E3E3E3"));
        //m_nextButton->setPalette(nextvpa);
        m_todayframe->setBColor(Qt::white);
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        DPalette wpa = m_weekLabel->palette();
        wpa.setColor(DPalette::WindowText, QColor("#717171"));
        m_weekLabel->setPalette(wpa);
        m_weekLabel->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#252525");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        todaypa.setColor(DPalette::ButtonText, todayColor);
//        todaypa.setColor(DPalette::ButtonText, QColor("#0081FF"));
        todaypa.setColor(DPalette::Dark, "#414141");
        todaypa.setColor(DPalette::Light, "#484848");
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        m_today->setBColor("#484848", "#727272", "#242424", "#414141", "#535353", "#282828");
//        m_today->setTColor("#0081FF", "#FFFFFF", "#0081FF");
        m_today->setTColor(todayColor, "#FFFFFF", "#0081FF");
        m_today->setshadowColor(sbcolor);

        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#484848"));
        prevpa.setColor(DPalette::Light, QColor("#414141"));
        //prevpa.setColor(DPalette::Dark, Qt::black);
        //prevpa.setColor(DPalette::Light, Qt::black);
        //m_prevButton->setPalette(prevpa);

        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#484848"));
        nextvpa.setColor(DPalette::Light, QColor("#414141"));
        //m_nextButton->setPalette(nextvpa);
        QColor bcolor = "#FFFFFF";
        bcolor.setAlphaF(0.05);
        m_todayframe->setBColor(bcolor);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        DPalette wpa = m_weekLabel->palette();
        wpa.setColor(DPalette::WindowText, QColor("#717171"));
        m_weekLabel->setPalette(wpa);
        m_weekLabel->setForegroundRole(DPalette::WindowText);
    }
    m_weekview->setTheMe(type);
    m_weekHeadView->setTheMe(type);
    m_scheduleView->setTheMe(type);
    //m_schceduleSearchView->setTheMe(type);
}

void CWeekWindow::setTime(QTime time)
{
    m_scheduleView->setTime(time);
}
void CWeekWindow::setSearchWFlag(bool flag)
{
    m_searchfalg = flag;
    m_weekview->setsearchfalg(flag);
    update();
    //m_schceduleSearchView->setVisible(flag);
}

void CWeekWindow::clearSearch()
{
    //m_schceduleSearchView->clearSearch();
}

void CWeekWindow::setSearchText(QString str)
{
    m_searchText = str;
    //m_schceduleSearchView->slotsetSearch(str);
}

void CWeekWindow::updateHigh()
{
    m_scheduleView->updateHigh();
}
void CWeekWindow::slotReturnTodayUpdate()
{
    setDate(QDate::currentDate());
}

void CWeekWindow::slotupdateSchedule(int id)
{
    Q_UNUSED(id);
    m_scheduleView->slotupdateSchedule();
    //m_scheduleView->setTime(QTime::currentTime());
}

void CWeekWindow::slotTransitSchedule(int id)
{
    //m_schceduleSearchView->slotsetSearch(m_searchText);
    emit signalsWUpdateShcedule(this, id);
}

void CWeekWindow::slotTransitSearchSchedule(int id)
{
    m_scheduleView->slotupdateSchedule();
    // m_schceduleSearchView->slotsetSearch(m_searchText);
    emit signalsWUpdateShcedule(this, id);
}

void CWeekWindow::slotprev()
{
    slotScheduleHide();
    QDate tcurrent = m_currentdate.addDays(-7);
    if (tcurrent.year() < 1900) return;
    if (m_currentdate.year() >= 1900) {
        m_currentdate = tcurrent;
        setDate(m_currentdate);
    } else {
        //QMessageBox::information(this, tr("infomation"), tr("Year less than 1900!"));
    }
}

void CWeekWindow::slotnext()
{
    slotScheduleHide();
    m_currentdate = m_currentdate.addDays(7);;
    setDate(m_currentdate);
}

void CWeekWindow::slottoday()
{
    slotScheduleHide();
    emit signalsReturnTodayUpdate(this);
    setDate(QDate::currentDate());
}

void CWeekWindow::slotCurrentWeek(QDate date, QDate currentDate)
{
    QVector<QDate> vDate;
    for (int i = 0; i < 7; i++)
        vDate.append(date.addDays(i));
    emit signalCurrentDate(vDate[0]);
    m_currentdate = currentDate;
    m_weekHeadView->setWeekDay(vDate);
    m_weekHeadView->setCurrentDate(m_currentdate);
    m_scheduleView->setRange(date, date.addDays(6));
    //m_scheduleView->setDate(m_currentdate);
}

void CWeekWindow::slotcurrentDateLunarChanged(QVector<QDate> vdate, QVector<CaLunarDayInfo> vdetail, int type)
{
    int offset = 0;
    for (int i = 0; i < vdate.count(); ++i) {
        if (vdate.at(i) == m_currentdate) {
            offset =i;
            break;
        }
    }
    if (m_currentdate == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
        //m_today->setText(tr("Return Today"));
    }
    // if (i == vdate.count()) {
    //      m_today->setEnabled(true);
    // } else {
    //     m_today->setEnabled(false);
    // }
    if (!vdate.isEmpty()) {
        CaLunarDayInfo detail = vdetail.at(offset);
        if (type == 1) {
            int yearnum = vdate.at(0).year();
            if (yearnum < 1900) yearnum = 1900;
            QLocale locale;
            if (locale.language() == QLocale::Chinese) {

                m_YearLabel->setText(QString::number(yearnum) + tr("Y"));
            } else {
                m_YearLabel->setText(QString::number(yearnum));
            }
            //m_YearLabel->setText(QString::number(vdate.at(0).year()) + tr("Y"));
            m_YearLunarLabel->setText("-" + detail.mGanZhiYear + detail.mZodiac + "年-");
        }
    }
}

void CWeekWindow::slotcurrentDateChanged(QDate date)
{
    if (m_currentdate == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
        //m_today->setText(tr("Return Today"));
    }
    //if (date != QDate::currentDate()) {
    //     m_today->setEnabled(true);
    // } else {
    //     m_today->setEnabled(false);
    //}
    m_currentdate = date;
    m_scheduleView->setDate(date);
}

void CWeekWindow::slotsearchDateSelect(QDate date)
{
    setDate(date);
    slotupdateSchedule();
}

void CWeekWindow::slotScheduleHide()
{
    m_scheduleView->slotScheduleShow(false);
}

void CWeekWindow::resizeEvent(QResizeEvent *event)
{
    qreal sleftMagin = 0.093 * width() + 0.5;
    qreal headh = height() * 0.0924 + 0.5;
    qreal dw = width() * 0.4186 + 0.5;
    int dh = 36;
//    int space = (width() - dw) / 2 - 220;

//    int sw = (width() -  width() * 0.9802 + 0.5) / 2;

    int winframe = 10;
    m_mainhLayout->setContentsMargins(10, 20, 0, 10);
    if (m_searchfalg) {

        m_tmainLayout->setContentsMargins(0, 0, 0, 0);
    } else {
        winframe += 10;
        m_tmainLayout->setContentsMargins(0, 0, 10, 0);
    }


    //m_spaceitem->changeSize(space, 36, QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (!m_searchfalg) {
        m_weekview->setFixedSize(qRound(dw), dh);
    } else {
        //m_weekview->setwindowFixw(dw, width() - 0.2325 * width() + 0.5 - 220 - 260);
        m_weekview->setFixedSize(qRound(dw - 100), dh);
    }
    //m_weekview->setFixedHeight(dh);

    // m_weekview->setFixedSize(dw, dh);

    //m_weekHeadView->setFixedSize(width() * 0.9802 + 0.5, headh);
    m_weekHeadView->setMounthLabelWidth(qRound(sleftMagin + 1), qRound(width() * 0.9802 + 0.5));
    // m_weekHeadView->setFixedHeight(headh);
    //m_weekHeadView->setFixedSize(width() * 0.9802 + 0.5, headh);
    m_weekHeadView->setFixedSize(width() - winframe, qRound(headh));
//    m_scheduleView->setviewMagin(sleftMagin, stopMagin, 0, 0);
    //m_schceduleSearchView->setFixedWidth(0.2325 * width() + 0.5);
    //m_scheduleView->setFixedSize(width() * 0.9802 + 0.5, sh);
//    m_scheduleView->setFixedSize(width() - winframe, qRound(sh - 10));
    QMainWindow::resizeEvent(event);
}

void CWeekWindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    slotScheduleHide();
}


