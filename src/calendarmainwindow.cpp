/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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
#include "calendarmainwindow.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QWidget>
#include <DAboutDialog>
#include <DHiDPIHelper>
#include <QSizePolicy>
#include <DPalette>
#include <QMenuBar>
#include <com_deepin_daemon_calendar_scheduler.h>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QMessageBox>
#include <QShortcut>
#include <QDesktopWidget>
#include <QApplication>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DWidgetUtil>

#include "yearwindow.h"
#include "monthwindow.h"
#include "dbuscalendar_adaptor.h"
#include "weekwindow.h"
#include "daywindow.h"
#include "scheduledatamanage.h"
#include "myschceduleview.h"
#include "creatorparschedule.h"
#include "configsettings.h"
#include "shortcut.h"
#include "schcedulesearchview.h"

#include "cdynamicicon.h"


DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static const int CalendarMWidth = 860;
static const int CalendarMHeight = 634;

Calendarmainwindow::Calendarmainwindow(QWidget *w)
    : DMainWindow (w)
{
    m_DataGetThread = new DbusDataGetThread(CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getDbus(),
                                            nullptr);
    m_currentdate = QDate::currentDate();
    setContentsMargins(QMargins(0, 0, 0, 0));
    initUI();
    initConnection();
    initLunar();
    //setFixedSize(CalendarMWidth, CalendarMHeight);
    setMinimumSize(CalendarMWidth, CalendarMHeight);
    setWindowTitle(tr("Calendar"));
    //setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    new CalendarAdaptor(this);
    resize(CalendarMWidth, CalendarMHeight);

    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QLatin1String("Ctrl+F")));
    connect(shortcut, SIGNAL(activated()), this, SLOT(slotSearchEdit()));

    QShortcut *viewshortcut = new QShortcut(this);
    viewshortcut->setKey(QKeySequence(QLatin1String("Ctrl+Shift+/")));
    connect(viewshortcut, SIGNAL(activated()), this, SLOT(onViewShortcut()));

//    QShortcut *viewmaxminshortcut = new QShortcut(this);
//    viewmaxminshortcut->setKey(QKeySequence(QLatin1String("Ctrl+Alt+F")));
//    connect(viewmaxminshortcut, SIGNAL(activated()), this, SLOT(slotmaxminViewShortcut()));

    setTitlebarShadowEnabled(true);
    setFocusPolicy(Qt::ClickFocus);
    QByteArray arrybyte = CConfigSettings::value("base.geometry").toByteArray();
    bool isOk = false;
    int state = CConfigSettings::value("base.state").toInt(&isOk);
    if (!arrybyte.isEmpty() && isOk) {
        Qt::WindowStates winStates = static_cast<Qt::WindowStates >(state);
        setWindowState(winStates);
        if (winStates !=Qt::WindowState::WindowMaximized) {
            restoreGeometry(arrybyte);
        }
    }
    Dtk::Widget::moveToCenter(this);
}

Calendarmainwindow::~Calendarmainwindow()
{
    CDynamicIcon::releaseInstance();
}

/*void Calendarmainwindow::Invoke(const QString &mothodName, const QString &content)
{
    if (mothodName == "CREATE") {
        ScheduleDtailInfo info;
        if (!analysisCreate(content, info)) {
            QMessageBox::information(this, tr("information"), tr("Failed to parse and create schedule!"));
            return;
        }
        if (CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->addSchedule(info) < 0) {
            QMessageBox::information(this, tr("information"), tr("Failed to create schedule!"));
            return;
        }
        m_dayButton->setFocus();
        m_dayButton->setChecked(true);
        m_stackWidget->setCurrentIndex(3);
        m_DayWindow->setDate(info.beginDateTime.date());
        m_DayWindow->slotupdateSchedule(0);
    }
}*/

bool Calendarmainwindow::analysisCreate(const QString &content, ScheduleDtailInfo &info)
{
    if (content.isEmpty()) return false;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(content.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonArray rootarry = jsonDoc.array();

    for (int i = 0; i < rootarry.size(); i++) {

        QJsonObject subObj = rootarry.at(i).toObject();
        if (subObj.value("name").toString() == "content") {
            info.titleName = subObj.value("value").toString();
        }
        if (subObj.value("name").toString() == "datetime") {
            QString ssubObj = subObj.value("normValue").toString();
            QJsonParseError sjson_error;
            QJsonDocument sjsonDoc(QJsonDocument::fromJson(ssubObj.toLocal8Bit(), &sjson_error));

            if (sjson_error.error != QJsonParseError::NoError) {
                return false;
            }
            QJsonObject ssobject = sjsonDoc.object();
            info.beginDateTime = QDateTime::fromString(ssobject.value("datetime").toString(), "yyyy-MM-ddThh:mm:ss");
        }
        if (subObj.value("value").toString() == "reminder") {
            info.id = 0;
            info.rpeat = 0;
            info.remind = true;
            info.remindData.n = 0;
            info.endDateTime = info.beginDateTime.addSecs(3600);
            info.allday = false;
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->GetType(1, info.type);
        }
    }
    return true;
}
void Calendarmainwindow::onViewShortcut()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    Shortcut sc;
    QStringList shortcutString;
    QString param1 = "-j=" + sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << "-b" << param1 << param2;

    QProcess *shortcutViewProc = new QProcess(this);
    shortcutViewProc->startDetached("killall deepin-shortcut-viewer");
    shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProc, SIGNAL(finished(int)), shortcutViewProc, SLOT(deleteLater()));
}

void Calendarmainwindow::slotmaxminViewShortcut()
{
    if (windowState() == Qt::WindowMaximized) {
        showNormal();
    } else {
        showMaximized();
    }
}

void Calendarmainwindow::slotGetScheduleInfoSuccess()
{
    m_yearwindow->getScheduleInfo();
}

void Calendarmainwindow::slotDynamicIconUpdate()
{
    if (QDate::currentDate() !=CDynamicIcon::getInstance()->getDate()) {
        CDynamicIcon::getInstance()->setDate(QDate::currentDate());
        CDynamicIcon::getInstance()->setIcon();
    }

}
void Calendarmainwindow::viewWindow(int type, QDateTime datetime)
{
    if (type < 0 || type > m_stackWidget->count()) {
        return;
    }
    m_stackWidget->setCurrentIndex(type - 1);
    if (type - 1 != 0) {
        m_priindex = type - 1;
    }
//    if (datetime.date().year() <=1900)
//        return;
    switch (type) {
    case 1: {
        m_yearButton->setFocus();
        m_yearButton->setChecked(true);
        m_yearwindow->setDate(datetime.date());
    }
    break;
    case 2: {
        m_monthButton->setFocus();
        m_monthButton->setChecked(true);
        m_monthWindow->setDate(datetime.date());
        m_monthWindow->slotupdateSchedule(0);
    }
    break;
    case 3: {
        m_weekButton->setFocus();
        m_weekButton->setChecked(true);
        m_weekWindow->setDate(datetime.date());
        m_weekWindow->setTime(datetime.time());
        m_weekWindow->slotupdateSchedule(0);
    }
    break;
    case 4: {
        m_dayButton->setFocus();
        m_dayButton->setChecked(true);
        m_DayWindow->setDate(datetime.date());
//        m_DayWindow->setTime(datetime.time());
        m_DayWindow->slotupdateSchedule(0);
        m_searchflag = true;
    }
    break;
    }
    CConfigSettings::setOption("base.view", type);
//    m_priindex = 3;
}

void Calendarmainwindow::UpdateJob()
{
    int index = m_stackWidget->currentIndex();
    if (index < 0 || index > m_stackWidget->count() - 1) {

        return;
    }
    switch (index) {
    case 1: {
        //m_monthWindow->slotReturnTodayUpdate();
        m_monthWindow->slotupdateSchedule(0);
    }
    break;
    case 2: {
        //m_weekWindow->slotReturnTodayUpdate();
        m_weekWindow->slotupdateSchedule(0);
    }
    break;
    case 3: {
        //m_DayWindow->slotReturnTodayUpdate();
        m_DayWindow->slotupdateSchedule(0);
    }
    break;
    }
}

void Calendarmainwindow::updateHigh()
{
    int index = m_stackWidget->currentIndex();
    if (index < 0 || index > m_stackWidget->count() - 1) {

        return;
    }
    switch (index) {
    case 0: {
//        m_yearwindow->updateHigh();
    }
    break;
    case 1: {

    }
    break;
    case 2: {
        m_weekWindow->updateHigh();
    }
    break;
    case 3: {
        m_DayWindow->updateHigh();
    }
    break;
    }
}

void Calendarmainwindow::setSearchWidth(int w)
{
    m_contentBackground->setFixedWidth(w);
}

void Calendarmainwindow::slotTheme(int type)
{
    if (type == 0) {
        type = DGuiApplicationHelper::instance()->themeType();
    }
    if (type == 1) {

        DPalette pl = m_yearButton->palette();
        pl.setColor(DPalette::ButtonText, QColor("#414D68"));
        pl.setColor(DPalette::Light, QColor("#E6E6E6"));
        pl.setColor(DPalette::Dark, QColor("#E3E3E3"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        m_yearButton->setPalette(pl);
        m_monthButton->setPalette(pl);
        m_weekButton->setPalette(pl);
        m_dayButton->setPalette(pl);
        // DPalette maintpl = palette();
        //maintpl.setColor(DPalette::Window, "#FFFFFF");
        //setPalette(maintpl);

        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#F8F8F8");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        DPalette tframepa = m_transparentFrame->palette();
        QColor tColor = "#FFFFFF";
        tColor.setAlphaF(0.3);
        tframepa.setColor(DPalette::Background, tColor);
        m_transparentFrame->setPalette(tframepa);
        m_transparentFrame->setBackgroundRole(DPalette::Background);

    } else {
        DPalette pl = m_yearButton->palette();
        pl.setColor(DPalette::ButtonText, QColor("#C0C6D4"));
        pl.setColor(DPalette::Light, QColor("#484848"));
        pl.setColor(DPalette::Dark, QColor("#414141"));
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.08);
        pl.setColor(DPalette::Shadow, sbcolor);
        m_yearButton->setPalette(pl);
        m_monthButton->setPalette(pl);
        m_weekButton->setPalette(pl);
        m_dayButton->setPalette(pl);
        //DPalette maintpl = palette();
        //maintpl.setColor(DPalette::Window, Qt::black);
        //setPalette(maintpl);

        DPalette anipa = m_contentBackground->palette();
        anipa.setColor(DPalette::Background, "#252525");
        m_contentBackground->setPalette(anipa);
        m_contentBackground->setBackgroundRole(DPalette::Background);

        DPalette tframepa = m_transparentFrame->palette();
        QColor tColor = "#000000";
        tColor.setAlphaF(0.3);
        tframepa.setColor(DPalette::Background, tColor);
        m_transparentFrame->setPalette(tframepa);
        m_transparentFrame->setBackgroundRole(DPalette::Background);
    }
    CScheduleDataManage::getScheduleDataManage()->setTheMe(type);
    m_yearwindow->setTheMe(type);
    m_monthWindow->setTheMe(type);
    m_weekWindow->setTheMe(type);
    m_DayWindow->setTheMe(type);
    m_schceduleSearchView->setTheMe(type);
}

void Calendarmainwindow::OpenSchedule(QString job)
{
    if (job.isEmpty()) return;
    ScheduleDtailInfo out;
    if (CreatOrParSchedule::GetJob(job, out)) {
        m_dayButton->setFocus();
        m_dayButton->setChecked(true);
        m_stackWidget->setCurrentIndex(3);
        m_DayWindow->setDate(out.beginDateTime.date());
        m_DayWindow->slotupdateSchedule(0);
        CMySchceduleView dlg(out, this);
//        dlg.setSchedules(out);
        dlg.exec();
        m_DayWindow->slotupdateSchedule(0);
        slotWUpdateShcedule(nullptr,0);
    }
}

void Calendarmainwindow::ActiveWindow()
{
    raise();
}

void Calendarmainwindow::RaiseWindow()
{
    raise();
}
void Calendarmainwindow::initUI()
{
    m_DynamicIconUpdateTimer = new QTimer(this);
    m_DynamicIconUpdateTimer->start(3000);

    QFrame *titleframe = new QFrame(this);
    titleframe->setObjectName("TitleBar");
    titleframe->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    CDynamicIcon::getInstance()->setTitlebar(this->titlebar());
    CDynamicIcon::getInstance()->setIcon();





    QStringList titlelist;
    titlelist << tr("Y") << tr("M") << tr("W") << tr("D");
    m_buttonBox = new DButtonBox(this);
    //DPalette wpa = m_segmentedControl->palette();
    //wpa.setColor(DPalette::ButtonText, QColor("#FFFFFF"));
    //wpa.setColor(DPalette::Button, QColor("#0081FF "));
    // wpa.setColor(DPalette::Dark, QColor("#0081FF"));
    //wpa.setColor(DPalette::Light, QColor("#0081FF"));
    // wpa.setColor(DPalette::Window, QColor("#0081FF"));
    // wpa.setColor(DPalette::WindowText, QColor("#414D68"));
    //m_segmentedControl->setPalette(wpa);
    //m_segmentedControl->addSegmented(titlelist);
    // m_segmentedControl->setFixedSize(200, 36);

    m_yearButton = new DButtonBoxButton(tr("Y"), this);
    DPalette pl = m_yearButton->palette();
    pl.setColor(DPalette::ButtonText, QColor("#414D68"));
    pl.setColor(DPalette::Light, QColor("#E6E6E6"));
    pl.setColor(DPalette::Dark, QColor("#E3E3E3"));
    QColor sbcolor("#000000");
    sbcolor.setAlphaF(0.08);
    pl.setColor(DPalette::Shadow, sbcolor);

    QFont viewfont;
//    viewfont.setFamily("SourceHanSansSC");
    viewfont.setWeight(QFont::Medium);
    viewfont.setPixelSize(14);


    m_yearButton->setFixedSize(50, 36);
    m_monthButton = new DButtonBoxButton(tr("M"), this);
    m_monthButton->setFixedSize(50, 36);
    m_weekButton = new DButtonBoxButton(tr("W"), this);
    m_weekButton->setFixedSize(50, 36);
    m_dayButton = new DButtonBoxButton(tr("D"), this);
    m_dayButton->setFixedSize(50, 36);
    m_yearButton->setPalette(pl);
    m_monthButton->setPalette(pl);
    m_weekButton->setPalette(pl);
    m_dayButton->setPalette(pl);
    m_yearButton->setFont(viewfont);
    m_monthButton->setFont(viewfont);
    m_weekButton->setFont(viewfont);
    m_dayButton->setFont(viewfont);

    QList<DButtonBoxButton *> btlist;
    btlist.append(m_yearButton);
    btlist.append(m_monthButton);
    btlist.append(m_weekButton);
    btlist.append(m_dayButton);
    m_buttonBox->setButtonList(btlist, true);

    m_buttonBox->setId(m_yearButton, 0);
    m_buttonBox->setId(m_monthButton, 1);
    m_buttonBox->setId(m_weekButton, 2);
    m_buttonBox->setId(m_dayButton, 3);
    m_buttonBox->setFixedSize(200, 36);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    //titleLayout->setContentsMargins(0, 0, 0, 0);
    //titleLayout->addWidget(m_ybutton);
    // titleLayout->addWidget(m_mbutton);
    //titleLayout->addWidget(m_wbutton);
    titleLayout->addSpacing(8);
    //titleLayout->addWidget(m_icon);
    //titleLayout->addSpacing(18);
    titleLayout->addWidget(m_buttonBox);
    titleLayout->addSpacing(52);
    // QSpacerItem *lspaceitem = new QSpacerItem(30, CalendarMTitleHeight, QSizePolicy::Expanding, QSizePolicy::Fixed);
    //titleLayout->addSpacerItem(lspaceitem);
    m_searchEdit = new DSearchEdit(this);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);
    m_searchEdit->setFixedHeight(36);
    m_searchEdit->setMinimumWidth(240);
    m_searchEdit->setMaximumWidth(354);
    m_searchEdit->setFont(viewfont);
    titleLayout->addStretch();
    //QWidget *buttonW = new QWidget;
    titleframe->setLayout(titleLayout);

    DTitlebar *titlebar = this->titlebar();
    titlebar->setFixedHeight(50);
    titlebar->addWidget(titleframe, Qt::AlignLeft | Qt::AlignVCenter);
    titlebar->setCustomWidget(m_searchEdit, true);


//    titlebar->move(36, 3);
    m_stackWidget = new AnimationStackedWidget();
    m_stackWidget->setContentsMargins(0, 0, 0, 0);
    m_stackWidget->setDuration(250);
//    m_stackWidget->setMargin(0);
//    m_stackWidget->setSpacing(0);
    //m_stackWidget->setFixedSize(WorkViewWidth, WorkViewHeight);
    createview();
    QHBoxLayout *tmainLayout = new QHBoxLayout;
    tmainLayout->setMargin(0);
    tmainLayout->setSpacing(0);
    tmainLayout->setContentsMargins(0, 0, 0, 0);
    tmainLayout->addWidget(m_stackWidget);
    //mainLayout->addStretch(1);

    m_contentBackground = new DFrame;
    m_contentBackground->setContentsMargins(0, 0, 0, 0);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setAutoFillBackground(true);
    m_contentBackground->setPalette(anipa);

    m_schceduleSearchView = new CSchceduleSearchView(this);
    //m_schceduleSearchView->setFixedWidth(200);

    QVBoxLayout *ssLayout = new QVBoxLayout;
    ssLayout->setMargin(0);
    ssLayout->setSpacing(0);
    ssLayout->setContentsMargins(0, 10, 0, 10);
    ssLayout->addWidget(m_schceduleSearchView);
    m_contentBackground->setLayout(ssLayout);
    tmainLayout->addWidget(m_contentBackground);
//    m_contentBackground->setFixedWidth(0);
    m_contentBackground->setVisible(false);

    DWidget *maincentralWidget = new DWidget(this);

    maincentralWidget->setLayout(tmainLayout);

    setCentralWidget(maincentralWidget);
    //m_stackWidget->setGeometry(0, 50, 860, 584);
    //m_bttongroup->button(0)->setChecked(true);
    m_yearButton->setFocus();
    m_yearButton->setChecked(true);
    m_transparentFrame = new DFrame(this);
    m_transparentFrame->setAutoFillBackground(true);
    m_transparentFrame->hide();

    m_animation = new QPropertyAnimation(this);

}

void Calendarmainwindow::initConnection()
{
    connect(m_DataGetThread,
            &DbusDataGetThread::signalGetScheduleSuccess,
            this,
            &Calendarmainwindow::slotGetScheduleInfoSuccess);

    connect(m_stackWidget
            , &AnimationStackedWidget::signalIsFinished
            , this
            , &Calendarmainwindow::slotSetButtonBox);
    connect(m_buttonBox, &DButtonBox::buttonClicked, this, &Calendarmainwindow::slotstackWClicked);
    //connect(m_bttongroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &Calendarmainwindow::slotstackWClicked);
    connect(m_weekWindow, &CWeekWindow::signalsWUpdateShcedule, this, &Calendarmainwindow::slotWUpdateShcedule);
    connect(m_monthWindow, &CMonthWindow::signalsWUpdateShcedule, this, &Calendarmainwindow::slotWUpdateShcedule);
    connect(m_DayWindow, &CDayWindow::signalsWUpdateShcedule, this, &Calendarmainwindow::slotWUpdateShcedule);
    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &Calendarmainwindow::slotSreturnPressed);
    connect(m_searchEdit, &DSearchEdit::textChanged, this, &Calendarmainwindow::slotStextChanged);
    connect(m_searchEdit, &DSearchEdit::focusChanged, this, &Calendarmainwindow::slotStextfocusChanged);
    connect(m_weekWindow, &CWeekWindow::signalsReturnTodayUpdate, this, &Calendarmainwindow::slotReturnTodyUpdate);
    connect(m_monthWindow, &CMonthWindow::signalsReturnTodayUpdate, this, &Calendarmainwindow::slotReturnTodyUpdate);
    connect(m_DayWindow, &CDayWindow::signalsReturnTodayUpdate, this, &Calendarmainwindow::slotReturnTodyUpdate);
    connect(m_yearwindow, &CYearWindow::signalsReturnTodayUpdate, this, &Calendarmainwindow::slotReturnTodyUpdate);
    //监听当前应用主题切换事件
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &Calendarmainwindow::slotTheme);

    m_dbus = new com::deepin::daemon::calendar::Scheduler("com.deepin.daemon.Calendar",
                                                          "/com/deepin/daemon/Calendar/Scheduler",
                                                          QDBusConnection::sessionBus(), this);
    connect(m_dbus, &com::deepin::daemon::calendar::Scheduler::JobsUpdated,
            this, &Calendarmainwindow::slotJobsUpdated);
    connect(m_schceduleSearchView, &CSchceduleSearchView::signalsUpdateShcedule, this, &Calendarmainwindow::slotTransitSearchSchedule);
    connect(m_schceduleSearchView, &CSchceduleSearchView::signalDate, this, &Calendarmainwindow::slotsearchDateSelect);
    connect(m_schceduleSearchView, &CSchceduleSearchView::signalSelectSchedule,
            this, &Calendarmainwindow::slotSearchSelectSchedule);
    connect(m_schceduleSearchView, &CSchceduleSearchView::signalScheduleHide,
            this, &Calendarmainwindow::setScheduleHide);

    connect(m_yearwindow, &CYearWindow::signaldoubleclickDate, this, &Calendarmainwindow::slotdoubleclickDate);
    connect(m_yearwindow, &CYearWindow::signalselectWeekwindow, this, &Calendarmainwindow::slotselectWeek);
    connect(m_yearwindow, &CYearWindow::signalselectMonth, this, &Calendarmainwindow::slotselectMonth);

    connect(m_monthWindow, &CMonthWindow::signalsViewSelectDate, this, &Calendarmainwindow::slotViewSelectDate);
    connect(m_monthWindow, &CMonthWindow::signalsCurrentScheduleDate, this, &Calendarmainwindow::slotCurrentScheduleDate);
    connect(m_weekWindow, &CWeekWindow::signalsCurrentScheduleDate, this, &Calendarmainwindow::slotCurrentScheduleDate);
    connect(m_weekWindow, &CWeekWindow::signalsViewSelectDate, this, &Calendarmainwindow::slotViewSelectDate);
    connect(m_monthWindow, &CMonthWindow::signalViewtransparentFrame, this, &Calendarmainwindow::slotViewtransparentFrame);
    connect(m_schceduleSearchView, &CSchceduleSearchView::signalViewtransparentFrame, this, &Calendarmainwindow::slotViewtransparentFrame);
    connect(m_weekWindow, &CWeekWindow::signalViewtransparentFrame, this, &Calendarmainwindow::slotViewtransparentFrame);
    connect(m_DayWindow, &CDayWindow::signalViewtransparentFrame, this, &Calendarmainwindow::slotViewtransparentFrame);

    connect(m_weekWindow, &CWeekWindow::signalCurrentDate, this, &Calendarmainwindow::slotCurrentDate);
    connect(m_monthWindow, &CMonthWindow::signalCurrentDate, this, &Calendarmainwindow::slotCurrentDate);
    connect(m_DayWindow, &CDayWindow::signalCurrentDate, this, &Calendarmainwindow::slotCurrentDate);
    connect(m_yearwindow, &CYearWindow::signalCurrentDate, this, &Calendarmainwindow::slotCurrentDate);

    connect(m_yearwindow,
            &CYearWindow::signalupdateschcedule,
            this,
            &Calendarmainwindow::getScheduleInfo);

    connect(m_DynamicIconUpdateTimer,
            &QTimer::timeout,
            this,
            &Calendarmainwindow::slotDynamicIconUpdate);
}


void Calendarmainwindow::initLunar()
{
    QLocale locale;
    bool flag = false;
    if (locale.language() == QLocale::Chinese) {
        flag = true;
    }
    m_yearwindow->setLunarVisible(flag);
    m_monthWindow->setLunarVisible(flag);
    m_weekWindow->setLunarVisible(flag);
    m_DayWindow->setLunarVisible(flag);
    //m_yearwindow->setLunarVisible(QLocale::system().name().contains("zh"));
    //m_monthWindow->setLunarVisible(QLocale::system().name().contains("zh"));
    //m_weekWindow->setLunarVisible(QLocale::system().name().contains("zh"));
    //m_DayWindow->setLunarVisible(QLocale::system().name().contains("zh"));
}

void Calendarmainwindow::createview()
{
    CScheduleDataManage::getScheduleDataManage()->setFirstWeekDay(0);
    CScheduleDataManage::getScheduleDataManage()->setCurrentYear(QDate::currentDate().year());

    getScheduleInfo();
    m_yearwindow = new CYearWindow(this);
    m_yearwindow->setDate(QDate::currentDate());
    m_stackWidget->addWidget(m_yearwindow);
#if 1
    m_monthWindow = new CMonthWindow;
    //0:周日 1～6：周一～周六
    m_monthWindow->setFirstWeekday(0);
    m_monthWindow->setDate(QDate::currentDate());
    m_stackWidget->addWidget(m_monthWindow);

    m_weekWindow  = new CWeekWindow(this);
    m_weekWindow->setFirstWeekday(0);
    m_weekWindow->setDate(QDate::currentDate());
    m_weekWindow->slotupdateSchedule(0);
    m_stackWidget->addWidget(m_weekWindow);

    m_DayWindow = new CDayWindow;
    QTimer::singleShot(500,[=] {
        m_DayWindow->setDate(QDate::currentDate());
    });
    m_stackWidget->addWidget(m_DayWindow);
#else
    CYearWindow *yearwindow1 = new CYearWindow;
    yearwindow1->setDate(QDate(2014, 2, 1));
    m_stackWidget->addWidget(yearwindow1);
    CYearWindow *yearwindow2 = new CYearWindow;
    yearwindow2->setDate(QDate(2014, 2, 1));
    m_stackWidget->addWidget(yearwindow2);

    CYearWindow *yearwindow3 = new CYearWindow;
    yearwindow3->setDate(QDate(2018, 2, 1));
    m_stackWidget->addWidget(yearwindow3);
#endif

}

DPushButton *Calendarmainwindow::createButon(QString name)
{
    DPushButton *button = new DPushButton();
    button->setText(name);
    button->setFixedSize(50, 40);
    //button->setPressIcon(DHiDPIHelper::loadNxPixmap(":/resources/icon/dde-choose30.svg")
    //.scaled(50, 40));
    return  button;
}

void Calendarmainwindow::getScheduleInfo()
{
    YearScheduleInfo *info = CScheduleDataManage::getScheduleDataManage()->getGetAllYearScheduleInfo();
    m_DataGetThread->getScheduleInfo(info);
}

void Calendarmainwindow::setScheduleHide()
{
    m_yearwindow->slotSetSchceduleHide();
    m_monthWindow->slotScheduleHide();
    m_weekWindow->slotScheduleHide();
    m_DayWindow->slotScheduleHide();
}

void Calendarmainwindow::resizeEvent(QResizeEvent *event)
{
    m_transparentFrame->resize(width(), height() - 50);
    m_scheduleSearchViewMaxWidth = qRound(0.2325 * width() + 0.5);
    m_schceduleSearchView->setMaxWidth(m_scheduleSearchViewMaxWidth);
//    if (m_opensearchflag) {
    setSearchWidth(m_scheduleSearchViewMaxWidth);
//    }
    setScheduleHide();
    DMainWindow::resizeEvent(event);
    CConfigSettings::setOption("base.geometry", saveGeometry());
    CConfigSettings::setOption("base.state", int(windowState()));
}

void Calendarmainwindow::slotstackWClicked(QAbstractButton *bt)
{
#if 0
    int index = m_buttonBox->id(bt);
    if (index < 0 || index > m_stackWidget->count() - 1) {

        return;
    }
    m_searchflag = false;
    m_stackWidget->setCurrentIndex(index);
    if (index != 0) {
        m_priindex = index;
    }
    switch (index) {
    case 0: {
        m_yearwindow->slotReturnTodayUpdate();
    }
    break;
    case 1: {
        m_monthWindow->slotReturnTodayUpdate();
        m_monthWindow->slotupdateSchedule(0);
    }
    break;
    case 2: {
        m_weekWindow->slotReturnTodayUpdate();
        m_weekWindow->slotupdateSchedule(0);
    }
    break;
    case 3: {
        m_DayWindow->slotReturnTodayUpdate();
        m_DayWindow->slotupdateSchedule(0);
        m_searchflag = true;
    }
    break;
    }
    CConfigSettings::setOption("base.view", index + 1);
#endif
    m_buttonBox->setEnabled(false);
    setScheduleHide();
    int index = m_buttonBox->id(bt);
    if (index < 0 || index > m_stackWidget->count() - 1) {

        return;
    }
    m_searchflag = false;
    m_stackWidget->setCurrent(index);
    if (index != 0) {
        m_priindex = index;
    }
    if (m_currentdate.year() < 1900)
        return;
    switch (index) {
    case 0: {
        m_yearwindow->setDate(m_currentdate);
    }
    break;
    case 1: {
        m_monthWindow->setDate(m_currentdate);
        m_monthWindow->slotupdateSchedule(0);
    }
    break;
    case 2: {
        m_weekWindow->setDate(m_currentdate);
        m_weekWindow->slotupdateSchedule(1);
    }
    break;
    case 3: {
        m_DayWindow->setDate(m_currentdate);
        m_DayWindow->slotupdateSchedule(1);
        m_searchflag = true;
    }
    break;
    }
    CConfigSettings::setOption("base.view", index + 1);
}

void Calendarmainwindow::slotWUpdateShcedule(QMainWindow *w, int id)
{
    Q_UNUSED(w);
    Q_UNUSED(id);
    if (m_opensearchflag && !m_searchEdit->text().isEmpty()) {
        m_schceduleSearchView->slotsetSearch(m_searchEdit->text());
        m_yearwindow->slotupdateSchedule(0);
    }
    updateHigh();
    return;
}

void Calendarmainwindow::slotReturnTodyUpdate(QMainWindow *w)
{
    if (w != m_weekWindow)
        m_weekWindow->slotReturnTodayUpdate();
    if (w != m_monthWindow)
        m_monthWindow->slotReturnTodayUpdate();
    if (w != m_DayWindow)
        m_DayWindow->slotReturnTodayUpdate();
    if (w != m_yearwindow)
        m_yearwindow->slotReturnTodayUpdate();
}

void Calendarmainwindow::slotSreturnPressed()
{
#if 1
    if (!m_opensearchflag && !m_searchEdit->text().isEmpty()) {
        m_opensearchflag = true;
        m_contentBackground->setVisible(true);
//        m_animation->setTargetObject(this);
//        m_animation->setPropertyName("schedulesearchWidth");
//        m_animation->setDuration(500);
//        m_animation->setStartValue(0);
//        m_animation->setEndValue(m_scheduleSearchViewMaxWidth);
//        m_animation->start();
    }
    m_schceduleSearchView->slotsetSearch(m_searchEdit->text());
    m_yearwindow->setDate(m_currentdate);
//    m_yearwindow->slotupdateSchedule(0);
    updateHigh();
#else
    if (!m_searchflag) {
        m_dayButton->click();
        m_searchflag = true;
    }
    //m_segmentedControl->setCurrentIndex(3);
    //m_stackWidget->setCurrentIndex(3);
    m_DayWindow->setSearchText(m_searchEdit->text());
#endif
}

void Calendarmainwindow::slotStextChanged()
{
    if (!m_searchEdit->text().isEmpty()) {
        m_yearwindow->setSearchWFlag(true);
        m_weekWindow->setSearchWFlag(true);
        m_monthWindow->setSearchWFlag(true);
        m_DayWindow->setSearchWFlag(true);
        // m_contentBackground->setVisible(true);
    } else {
        m_schceduleSearchView->clearSearch();
        m_yearwindow->setSearchWFlag(false);
        m_monthWindow->setSearchWFlag(false);
        m_weekWindow->setSearchWFlag(false);
        m_DayWindow->setSearchWFlag(false);
        m_contentBackground->setVisible(false);
//        m_animation->setTargetObject(this);
//        m_animation->setDuration(500);
//        m_animation->setEndValue(0);
//        m_animation->setStartValue(m_scheduleSearchViewMaxWidth);
//        m_animation->start();
        m_opensearchflag = false;
    }
//    m_yearwindow->setDate(m_currentdate);
//    m_yearwindow->slotupdateSchedule(0);
    updateHigh();

}

void Calendarmainwindow::slotStextfocusChanged(bool onFocus)
{
    if (onFocus) {
        setScheduleHide();
    }
}

void Calendarmainwindow::slotJobsUpdated(const QList<qlonglong> &Ids)
{
    Q_UNUSED(Ids);
    int index = m_stackWidget->currentIndex();
    if (index < 0 || index > m_stackWidget->count() - 1) {
        return;
    }
    switch (index) {
    case 1: {
        //m_monthWindow->slotReturnTodayUpdate();
        m_monthWindow->slotupdateSchedule(0);
    }
    break;
    case 2: {
        //m_weekWindow->slotReturnTodayUpdate();
        m_weekWindow->slotupdateSchedule(0);
    }
    break;
    case 3: {
        //m_DayWindow->slotReturnTodayUpdate();
        m_DayWindow->slotupdateSchedule(0);
    }
    break;
    }
}

void Calendarmainwindow::slotSearchEdit()
{
    m_searchEdit->lineEdit()->setFocus();
}

void Calendarmainwindow::slotTransitSearchSchedule(int id)
{
    Q_UNUSED(id);
    getScheduleInfo();
    int index = m_stackWidget->currentIndex();
    if (index < 0 || index > m_stackWidget->count() - 1) {

        return;
    }
    switch (index) {
    case 0: {
        m_yearwindow->slotupdateSchedule(0);
    }
    break;
    case 1: {
        //m_monthWindow->slotReturnTodayUpdate();
        m_monthWindow->slotupdateSchedule(0);
    }
    break;
    case 2: {
        //m_weekWindow->slotReturnTodayUpdate();
        m_weekWindow->slotupdateSchedule(0);
    }
    break;
    case 3: {
        //m_DayWindow->slotReturnTodayUpdate();
        m_DayWindow->slotupdateSchedule(0);
    }
    break;
    }
    m_schceduleSearchView->slotsetSearch(m_searchEdit->text());
}

void Calendarmainwindow::slotsearchDateSelect(QDate date)
{
    setScheduleHide();
    int index = m_stackWidget->currentIndex();
    if (index < 0 || index > m_stackWidget->count() - 1) {
        return;
    }
    switch (index) {
    case 0: {
        m_yearwindow->setDate(date);
    }
    break;
    case 1: {
        m_monthWindow->setDate(date);
    }
    break;
    case 2: {
        m_weekWindow->setDate(date);
    }
    break;
    case 3: {
        m_DayWindow->setDate(date);
    }
    break;
    }
}

void Calendarmainwindow::slotSearchSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    int index = m_stackWidget->currentIndex();
    if (index < 0 || index > m_stackWidget->count() - 1) {
        return;
    }
    //等界面刷新完成后进行动作
    QTimer::singleShot(50, [this, index,scheduleInfo] {
        switch (index)
        {
        case 0: {
        }
        break;
        case 1: {
            m_monthWindow->setSelectSchedule(scheduleInfo);
        }
        break;
        case 2: {
            m_weekWindow->setSelectSchedule(scheduleInfo);
        }
        break;
        case 3: {
            m_DayWindow->setSelectSchedule(scheduleInfo);
        }
        break;
        }
    });

}

void Calendarmainwindow::slotdoubleclickDate(QDate date)
{
    m_stackWidget->setCurrentIndex(m_priindex);
    switch (m_priindex) {
    case 1: {
        m_monthButton->setFocus();
        m_monthButton->setChecked(true);
        m_monthWindow->setDate(date);
        m_monthWindow->slotupdateSchedule(0);
    }
    break;
    case 2: {
        m_weekButton->setFocus();
        m_weekButton->setChecked(true);
        m_weekWindow->setDate(date);
        m_weekWindow->setTime(QTime::currentTime());
        m_weekWindow->slotupdateSchedule(0);
    }
    break;
    case 3: {
        m_dayButton->setFocus();
        m_dayButton->setChecked(true);
        m_DayWindow->setDate(date);
        m_DayWindow->setTime(QTime::currentTime());
        m_DayWindow->slotupdateSchedule(0);
    }
    break;
    }
    CConfigSettings::setOption("base.view", m_priindex+1);
}

void Calendarmainwindow::slotselectMonth(QDate date)
{
    qDebug() << date;
    viewWindow(2, QDateTime(date));
}

void Calendarmainwindow::slotselectWeek(QDate date)
{
    qDebug() << date;
    viewWindow(3, QDateTime(date));
    CConfigSettings::setOption("base.view", m_priindex + 1);
}

void Calendarmainwindow::slotCurrentScheduleDate(QDate date)
{
    viewWindow(4, QDateTime(date));
}

void Calendarmainwindow::slotViewSelectDate(QDate date)
{
    if (date.year() < 1900) return;
    viewWindow(4, QDateTime(date));
}

void Calendarmainwindow::slotViewtransparentFrame(int type)
{
    static int showFrameCount =0;
    if (type) {
        m_transparentFrame->resize(width(), height() - 50);
        m_transparentFrame->move(0, 50);
        m_transparentFrame->show();
        ++showFrameCount;
    } else {
        if (showFrameCount==1)
            m_transparentFrame->hide();
        --showFrameCount;
    }

    int index = m_stackWidget->currentIndex();
    if (index < 0 || index > m_stackWidget->count() - 1) {

        return;
    }
    switch (index) {
    case 0: {
        m_yearwindow->setFocus();
    }
    break;
    case 1: {
        m_monthWindow->setFocus();
    }
    break;
    case 2: {
        m_weekWindow->setFocus();
    }
    break;
    case 3: {
        m_DayWindow->setFocus();
    }
    break;
    }
}

void Calendarmainwindow::slotCurrentDate(QDate date)
{
    m_currentdate = date;
    CScheduleDataManage::getScheduleDataManage()->setCurrentYear(date.year());
    getScheduleInfo();
}

void Calendarmainwindow::slotSetButtonBox()
{
    m_buttonBox->setEnabled(true);
}
void Calendarmainwindow::closeEvent(QCloseEvent *event)
{
//    CConfigSettings::setOption("base.geometry", saveGeometry());
//    CConfigSettings::setOption("base.state", int(windowState()));
    QWidget::closeEvent(event);
}

void Calendarmainwindow::mouseMoveEvent(QMouseEvent *event)
{
    DMainWindow::mouseMoveEvent(event);
    setScheduleHide();
}

void Calendarmainwindow::changeEvent(QEvent *event)
{
    DMainWindow::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        setScheduleHide();
    }
}

void Calendarmainwindow::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    setScheduleHide();
}

