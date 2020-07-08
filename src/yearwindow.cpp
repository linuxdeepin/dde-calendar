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
#include "yearwindow.h"
#include "yearview.h"
#include "constants.h"
#include "calendardbus.h"
#include <QMessageBox>
#include <DPalette>
#include <DHiDPIHelper>
#include <QMenuBar>
#include <QMouseEvent>
#include "schcedulesearchview.h"
DGUI_USE_NAMESPACE
CYearWindow::CYearWindow(QWidget *parent): QMainWindow (parent)
{
    initUI();
    initConnection();
    setWindowFlags(Qt::FramelessWindowHint);//去掉标题
    setContentsMargins(0, 0, 0, 0);
}

CYearWindow::~CYearWindow()
{

}

bool CYearWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_today) {
        if (event->type() == QEvent::MouseButtonPress) {
            slottoday();
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void CYearWindow::mousePressEvent(QMouseEvent *event)
{
    m_YearWidget->slotHideInfo();
    QMainWindow::mousePressEvent(event);
}

void CYearWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_topWidget->setGeometry(0, 0, this->width(), DDEMonthCalendar::M_YTopHeight);

    if (m_searchfalg) {
        m_tmainLayout->setContentsMargins(10, 0, 2, 0);
        m_topWidget->setContentsMargins(10, 0, 2, 0);
    } else {
        m_tmainLayout->setContentsMargins(10, 0, 10, 0);
        m_topWidget->setContentsMargins(10, 0, 10, 0);
    }
}

void CYearWindow::setDate(QDate date)
{
    if (!date.isValid()) return;
    m_YearWidget->setDate(date);
    m_YearWidget->slotcurrentDateChanged(date);
    emit signalCurrentDate(date);
    slotUpdateCurrentDate(date);
}

void CYearWindow::initUI()
{
    m_contentBackground = new DFrame(this);

    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setAutoFillBackground(true);
    m_contentBackground->setPalette(anipa);
    m_today = new QLabel(this);
    m_today->installEventFilter(this);

    QFont todayfont/*("SourceHanSansSC-Normal")*/;
    todayfont.setPixelSize(16);
    m_today->setFont(todayfont);
    m_today->setAlignment(Qt::AlignCenter);

    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedWidth(88);
    m_today->setAutoFillBackground(true);
    m_today->setFixedHeight(DDEYearCalendar::Y_MLableHeight - 4);
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    m_prevButton->setFixedWidth(DDEYearCalendar::Y_MLableHeight);
    m_prevButton->setFixedHeight(DDEYearCalendar::Y_MLableHeight);


    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    m_nextButton->setFixedWidth(DDEYearCalendar::Y_MLableHeight);
    m_nextButton->setFixedHeight(DDEYearCalendar::Y_MLableHeight);

    m_YearLabel = new QLabel(this);
    m_YearLabel->setFixedHeight(DDEYearCalendar::Y_YLableHeight);
    //m_currentMouth->setStyleSheet("border: 1px solid rgba(0, 0, 0, 0.05);");

    QFont t_labelF;
//    t_labelF.setFamily("SourceHanSansSC");
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(24);
    m_YearLabel->setFont(t_labelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(pa);

    m_YearLunarLabel = new QLabel(m_contentBackground);
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunatLabelWindth, DDEMonthCalendar::M_YLunatLabelHeight);

    QFont ylabelF;
//    ylabelF.setFamily("SourceHanSansSC");
    ylabelF.setWeight(QFont::Medium);
    ylabelF.setPixelSize(14);
    m_YearLunarLabel->setFont(ylabelF);
    DPalette Lunapa = m_YearLunarLabel->palette();
    Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_YearLunarLabel->setPalette(Lunapa);
    //m_YearLunarLabel->move(116, 27);

    m_YearLunarDayLabel = new QLabel(m_contentBackground);
    m_YearLunarDayLabel->setFixedSize(108, DDEMonthCalendar::M_YLunatLabelHeight);
    m_YearLunarDayLabel->setFont(ylabelF);
    m_YearLunarDayLabel->setPalette(Lunapa);
    m_YearLunarDayLabel->setAlignment(Qt::AlignRight);


    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    //yeartitleLayout->setContentsMargins(2, 10, 2, 0);
    yeartitleLayout->setContentsMargins(11, 12, 8, 10);
    yeartitleLayout->addWidget(m_YearLabel);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    //yeartitleLayout1->setContentsMargins(0, 10, 8, 5);
    yeartitleLayout1->setContentsMargins(4, 9, 0, 7);
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout1->addSpacing(390);
    yeartitleLayout1->addStretch();
    yeartitleLayout1->addWidget(m_YearLunarDayLabel, 0, Qt::AlignVCenter);

    m_todayframe = new CustomFrame(this);
    m_todayframe->setContentsMargins(0, 0, 0, 0);
    m_todayframe->setRoundState(true, true, true, true);
    m_todayframe->setBColor(Qt::white);
    m_todayframe->setFixedHeight(DDEYearCalendar::Y_MLableHeight);
    m_todayframe->setboreder(1);
    QHBoxLayout *todaylayout = new QHBoxLayout;
    todaylayout->setMargin(0);
    todaylayout->setSpacing(0);
    todaylayout->addWidget(m_prevButton);
    todaylayout->addWidget(m_today, 0, Qt::AlignCenter);
    todaylayout->addWidget(m_nextButton);
    m_todayframe->setLayout(todaylayout);
    yeartitleLayout1->addSpacing(10);
    yeartitleLayout1->addWidget(m_todayframe);
    yeartitleLayout->addLayout(yeartitleLayout1);

    m_topWidget = new DWidget(this);
    m_topWidget->setLayout(yeartitleLayout);


    YearWidget_First = new YearFrame();
    YearWidget_Second = new YearFrame();

    m_StackedWidget = new  AnimationStackedWidget(AnimationStackedWidget::TB);
    m_StackedWidget->addWidget(YearWidget_First);
    m_StackedWidget->addWidget(YearWidget_Second);
    m_StackedWidget->setContentsMargins(0, 0, 0, 0);
    m_StackedWidget->setDuration(600);

    m_YearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(0));
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->addWidget(m_StackedWidget);


    m_tmainLayout = new QHBoxLayout;
    m_tmainLayout->setMargin(0);
    m_tmainLayout->setSpacing(0);
    m_tmainLayout->setContentsMargins(10, 0, 10, 0);
    m_tmainLayout->addLayout(hhLayout);

    m_contentBackground->setContentsMargins(0, 0, 0, 0);
    m_contentBackground->setLayout(m_tmainLayout);

    //menuBar()->hide();
    setCentralWidget(m_contentBackground);

}

void CYearWindow::initConnection()
{
    connect(m_prevButton, &DIconButton::clicked, this, &CYearWindow::slotprev);
    //connect(m_today, &DPushButton::clicked, this, &CYearWindow::slottoday);
    connect(m_nextButton, &DIconButton::clicked, this, &CYearWindow::slotnext);
    //connect(m_schceduleSearchView, &CSchceduleSearchView::signalsUpdateShcedule, this, &CYearWindow::slotTransitSearchSchedule);
    // connect(m_schceduleSearchView, &CSchceduleSearchView::signalDate, this, &CYearWindow::slotsearchDateSelect);
    connect(YearWidget_First,
            &YearFrame::signaldoubleclickDate,
            this,
            &CYearWindow::signaldoubleclickDate);
    connect(YearWidget_Second,
            &YearFrame::signaldoubleclickDate,
            this,
            &CYearWindow::signaldoubleclickDate);

    connect(YearWidget_First,
            &YearFrame::signalselectMonth,
            this,
            &CYearWindow::signalselectMonth);

    connect(YearWidget_Second,
            &YearFrame::signalselectMonth,
            this,
            &CYearWindow::signalselectMonth);
    connect(YearWidget_First,
            &YearFrame::signalselectWeekwindow,
            this,
            &CYearWindow::signalselectWeekwindow);

    connect(YearWidget_Second,
            &YearFrame::signalselectWeekwindow,
            this,
            &CYearWindow::signalselectWeekwindow);

    connect(YearWidget_First,
            &YearFrame::signalUpdateYearDate,
            this,
            &CYearWindow::slotUpdateCurrentDate);
    connect(YearWidget_Second,
            &YearFrame::signalUpdateYearDate,
            this,
            &CYearWindow::slotUpdateCurrentDate);
    connect(YearWidget_First,
            &YearFrame::signalupdateschcedule,
            this,
            &CYearWindow::signalupdateschcedule);

    connect(m_StackedWidget,
            &AnimationStackedWidget::signalIsFinished,
            this,
            &CYearWindow::setYearData);
}

void CYearWindow::setLunarVisible(bool state)
{
    m_YearLunarLabel->setVisible(state);
    m_YearLunarDayLabel->setVisible(state);
}

void CYearWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        DPalette todaypa = m_today->palette();
        todaypa.setColor(DPalette::WindowText, QColor("#000000"));
        todaypa.setColor(DPalette::Background, Qt::white);
        //todaypa.setColor(DPalette::Light, Qt::white);
        m_today->setPalette(todaypa);
        m_today->setForegroundRole(DPalette::WindowText);
        m_today->setBackgroundRole(DPalette::Background);

        m_todayframe->setBColor(Qt::white);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);

        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

        m_YearLunarDayLabel->setPalette(Lunapa);
        m_YearLunarDayLabel->setForegroundRole(DPalette::WindowText);

    } else if (type == 2) {
        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#252525");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        DPalette todaypa = m_today->palette();
        todaypa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        //todaypa.setColor(DPalette::Dark, "#414141");
        QColor tbcolor = "#414141";
        tbcolor.setAlphaF(0.0);
        todaypa.setColor(DPalette::Background, tbcolor);
        m_today->setPalette(todaypa);
        m_today->setForegroundRole(DPalette::WindowText);
        m_today->setBackgroundRole(DPalette::Background);
        QColor tbcolor2 = "#414141";
        tbcolor2.setAlphaF(0.3);
        m_todayframe->setBColor(tbcolor2);
        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
        m_YearLunarDayLabel->setPalette(Lunapa);
        m_YearLunarDayLabel->setForegroundRole(DPalette::WindowText);
    }
    YearWidget_First->setTheMe(type);
    YearWidget_Second->setTheMe(type);

    DPalette palette = m_topWidget->palette();
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::Window));
    m_topWidget->setAutoFillBackground(true);
    m_topWidget->setPalette(palette);
}
void CYearWindow::setSearchWFlag(bool flag)
{
    m_searchfalg = flag;
    m_YearWidget->setSearchWFlag(flag);
}

void CYearWindow::clearSearch()
{
    //m_schceduleSearchView->clearSearch();
}

void CYearWindow::setSearchText(QString str)
{
    m_searchText = str;
    // m_schceduleSearchView->slotsetSearch(str);
}

void CYearWindow::getScheduleInfo()
{
    m_YearWidget->getInfoAndSetLineFlag();
}

void CYearWindow::slotTransitSearchSchedule(int id)
{
    // m_schceduleSearchView->slotsetSearch(m_searchText);
    emit signalsWUpdateShcedule(this, id);
}

void CYearWindow::slotSetSchceduleHide()
{
    m_YearWidget->slotHideInfo();
}


void CYearWindow::slotprev()
{
    m_YearWidget->slotHideInfo();
    if (m_currentdate.year() == 1900) return;
    if (m_StackedWidget->IsRunning()) return;
    QDate tcurrent = QDate(m_currentdate.year() - 1, m_currentdate.month(), m_currentdate.day());
    if (!tcurrent.isValid()) {
        m_currentdate = QDate(m_currentdate.year() - 1, m_currentdate.month(), 1);
    } else {
        m_currentdate = tcurrent;
    }
    if (m_currentdate.year() >= 1900) {
        //m_currentdate = QDate(m_currentdate.year() - 1, m_currentdate.month(), m_currentdate.day());
        int index = m_StackedWidget->currentIndex();
        index = qAbs(index - 1) % 2;
        m_YearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(index));
        m_YearWidget->setDate(m_currentdate);
        m_StackedWidget->setPre();
        emit signalCurrentDate(m_currentdate);
    } else {
        //QMessageBox::information(this, tr("infomation"), tr("Year less than 1900!"));
    }
}

void CYearWindow::slotnext()
{
    m_YearWidget->slotHideInfo();
    if (m_StackedWidget->IsRunning()) return;
    QDate tcurrent = QDate(m_currentdate.year() + 1, m_currentdate.month(), m_currentdate.day());
    if (!tcurrent.isValid()) {
        m_currentdate = QDate(m_currentdate.year() + 1, m_currentdate.month(), 1);
    } else {
        m_currentdate = tcurrent;
    }
    int index = m_StackedWidget->currentIndex();
    index = (index + 1) % 2;
    m_YearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(index));
    m_YearWidget->setDate(m_currentdate);
    m_StackedWidget->setNext();
    emit signalCurrentDate(m_currentdate);

}

void CYearWindow::slottoday()
{
    m_YearWidget->slotHideInfo();
    emit signalsReturnTodayUpdate(this);
    setDate(QDate::currentDate());
}

void CYearWindow::slotReturnTodayUpdate()
{
}

void CYearWindow::slotupdateSchedule(const int id)
{
    m_YearWidget->slotupdateSchedule(id);
}

void CYearWindow::slotUpdateCurrentDate(const QDate &date)
{
    m_currentdate = date;
    setYearData();
}

void CYearWindow::setYearData()
{

    if (m_currentdate == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return", "Today", "Return"));
    }
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
        m_YearLabel->setText(QString::number(m_currentdate.year()) + tr("Y"));
    } else {
        m_YearLabel->setText(QString::number(m_currentdate.year()));
    }
    m_YearLunarLabel->setText(m_YearWidget->getLunarYear());
    m_YearLunarDayLabel->setText(m_YearWidget->getLunarDay());
}
void CYearWindow::slotsearchDateSelect(QDate date)
{
    setDate(date);
}


void CYearWindow::wheelEvent(QWheelEvent *event)
{
//    for (int i = 0; i < 12; i++) {
//        m_monthViewList.at(i)->updateInfoWIndow(false);
//    }
    if (event->delta() < 0) {
        slotnext();
    } else {
        slotprev();
    }
}

YearFrame::YearFrame(DWidget *parent): DFrame (parent)
{
    m_DBusInter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                   "/com/deepin/api/LunarCalendar",
                                   QDBusConnection::sessionBus(), this);


    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setMargin(0);
    gridLayout->setSpacing(8);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            CYearView *view = new CYearView(this);
            connect(view, &CYearView::signalcurrentDateChanged, this, &YearFrame::slotcurrentDateChanged);
            connect(view, &CYearView::signaldoubleclickDate, this, &YearFrame::signaldoubleclickDate);
            connect(view, &CYearView::signalselectWeekwindow, this, &YearFrame::signalselectWeekwindow);
            connect(view, &CYearView::signalupdateschcedule, this, &YearFrame::signalupdateschcedule);
            connect(view, &CYearView::signalselectMonth, this, &YearFrame::signalselectMonth);
            connect(view, &CYearView::signalHideInfo, this, &YearFrame::slotHideInfo);
            connect(view, &CYearView::signalSelectInfo, this, &YearFrame::slotSelectInfo);
            gridLayout->addWidget(view, i, j);
            m_monthViewList.append(view);
        }
    }

    m_YearLabel = new QLabel(this);
    m_YearLabel->setFixedHeight(DDEYearCalendar::Y_YLableHeight);
    //m_currentMouth->setStyleSheet("border: 1px solid rgba(0, 0, 0, 0.05);");

    QFont t_labelF;
//    t_labelF.setFamily("SourceHanSansSC");
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(24);
    m_YearLabel->setFont(t_labelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(pa);

    m_YearLunarLabel = new QLabel();
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunatLabelWindth, DDEMonthCalendar::M_YLunatLabelHeight);

    QFont ylabelF;
//    ylabelF.setFamily("SourceHanSansSC");
    ylabelF.setWeight(QFont::Medium);
    ylabelF.setPixelSize(14);
    m_YearLunarLabel->setFont(ylabelF);
    DPalette Lunapa = m_YearLunarLabel->palette();
    Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_YearLunarLabel->setPalette(Lunapa);


    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    //yeartitleLayout->setContentsMargins(2, 10, 2, 0);
    yeartitleLayout->setContentsMargins(11, 12, 8, 10);
    yeartitleLayout->addWidget(m_YearLabel);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    //yeartitleLayout1->setContentsMargins(0, 10, 8, 5);
    yeartitleLayout1->setContentsMargins(4, 9, 0, 7);
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout1->addSpacing(390);
    yeartitleLayout1->addStretch();
    yeartitleLayout1->addSpacing(10);
    yeartitleLayout->addLayout(yeartitleLayout1);

    m_topWidget = new DWidget();
    m_topWidget->setLayout(yeartitleLayout);
    m_topWidget->setFixedHeight(DDEMonthCalendar::M_YTopHeight);
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->addWidget(m_topWidget);
    hhLayout->addLayout(gridLayout);
    this->setLayout(hhLayout);
    setContentsMargins(0, 0, 0, 10);
    setFrameRounded(true);
//    m_topWidget->hide();
}

YearFrame::~YearFrame()
{

}

void YearFrame::setDate(QDate &date)
{
    if (!date.isValid()) return;
    m_currentdate = date;
    for (int i = 0; i < 12; i++) {
        QDate tdate(m_currentdate.year(), i + 1, 1);
        m_monthViewList.at(i)->setCurrentDate(tdate, 0);
    }
    for (int i = 0; i < 12; i++) {
        QDate tdate(m_currentdate.year(), i + 1, 1);
        if (date.year() == tdate.year() && date.month() == tdate.month()) {
            m_monthViewList.at(i)->setCurrentDate(date, 1);
            break;
        }
    }
    getLunarData();
}

void YearFrame::getInfoAndSetLineFlag()
{
    for (int i = 0; i < 12; i++) {
        m_monthViewList.at(i)->getInfoAndSetLineFlag();
    }
}

void YearFrame::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        DPalette gpa = palette();
        gpa.setColor(DPalette::Background, "#F8F8F8");
        setPalette(gpa);
        setBackgroundRole(DPalette::Background);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);

        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);

    } else if (type == 2) {
        DPalette gpa = palette();
        gpa.setColor(DPalette::Background, "#252525");
        setPalette(gpa);
        setBackgroundRole(DPalette::Background);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette Lunapa = m_YearLunarLabel->palette();
        Lunapa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(Lunapa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
    }
    for (int i = 0; i < 12; i++) {
        m_monthViewList.at(i)->setTheMe(type);
    }

}

void YearFrame::setSearchWFlag(bool flag)
{
    m_searchfalg = flag;
}

void YearFrame::getLunarData()
{
    bool o1 = true;
    QDBusReply<CaLunarDayInfo> replydd = m_DBusInter->GetLunarInfoBySolar(m_currentdate.year(), m_currentdate.month(), m_currentdate.day(), o1);
    CaLunarDayInfo currentDayInfo = replydd.value();
    m_LunarYear = QString("-%0%1年-").arg(currentDayInfo.mGanZhiYear).arg(currentDayInfo.mZodiac);
    m_LunarDay = QString("-农历%0%1-").arg(currentDayInfo.mLunarMonthName).arg(currentDayInfo.mLunarDayName);
    QLocale locale;
    if (locale.language() == QLocale::Chinese) {
        m_YearLabel->setText(QString::number(m_currentdate.year()) + tr("Y"));
        m_YearLunarLabel->setText(m_LunarYear);
    } else {
        m_YearLabel->setText(QString::number(m_currentdate.year()));
        m_YearLunarLabel->setText("");
    }

}

void YearFrame::mousePressEvent(QMouseEvent *event)
{
    if (m_selectFlag) return;
    if (event->button() == Qt::LeftButton) {
        slotHideInfo();
    }
}

void YearFrame::slotcurrentDateChanged(QDate date)
{
    m_currentdate = date;
    getLunarData();
    emit signalUpdateYearDate(date);
}


void YearFrame::slotHideInfo()
{
    CYearView::SchceduleViewHide();
}

void YearFrame::slotSelectInfo(bool flag)
{
    m_selectFlag = flag;
}

void YearFrame::slotupdateSchedule(const int id)
{
    for (int i =0; i < m_monthViewList.size(); ++i) {
        m_monthViewList.at(i)->slotupdateSchedule(id);
    }
}

void YearFrame::slotSetSchceduleHide()
{
    CYearView::SchceduleViewHide();
}
