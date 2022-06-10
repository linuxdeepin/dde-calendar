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
#include "widget/yearWidget/yearwindow.h"
#include "widget/monthWidget/monthwindow.h"
#include "dbuscalendar_adaptor.h"
#include "widget/weekWidget/weekwindow.h"
#include "widget/dayWidget/daywindow.h"
#include "sidebarWidget/sidebarview.h"
#include "colorWidget/colorpickerWidget.h"
#include "scheduledatamanage.h"
#include "myscheduleview.h"
#include "configsettings.h"
#include "shortcut.h"
#include "schedulesearchview.h"
#include "cdynamicicon.h"
#include "constants.h"
#include "scheduledlg.h"
#include "ctitlewidget.h"
#include "tabletconfig.h"
#include "calendarglobalenv.h"
#include "settingWidget/userloginwidget.h"
#include "settingWidget/settingwidgets.h"

#include "scheduletypeeditdlg.h"
#include "accountmanager.h"
#include "units.h"

#include <DHiDPIHelper>
#include <DPalette>
#include <DFontSizeManager>
#include <DWidgetUtil>
#include <DTitlebar>
#include <DApplicationHelper>
#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DBackgroundGroup>


#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QShortcut>
#include <QSizePolicy>
#include <QHBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QMouseEvent>
#include <QColorDialog>
#include <QApplication>
#include <QWidgetAction>

#include <stdlib.h>
#include <functional>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static const int CalendarMWidth = 646;
static const int CalendarMHeight = 634;
const int CalendarSwitchWidth = 647; //当宽度小于这个尺寸时，标题栏需要切换显示逻辑
const int CalendarViewSwitchWidth = 804; //当宽度小于这个尺寸时，视图部分需要切换显示逻辑

const int Calendar_Default_Width = 860; //默认宽度
const int Calendar_Default_Height = 634; //默认高度

//静态的翻译不会真的翻译，但是会更新ts文件
//像static QString a = QObject::tr("hello"), a实际等于hello，但是ts会有hello这个词条
//调用DSetingDialog时会用到上述场景
static CalendarSettingSetting setting_account = {
    "setting_account",
    QObject::tr("Account setting"),
    {
        {"account", QObject::tr("Account"), {{"login", "", "login", ""}}},
        {"account_sync_items", QObject::tr("Select items to be synced"), {{"Account_Calendar", QObject::tr("Events"), "SyncTagRadioButton", ""}, {"Account_Setting", QObject::tr("General settings"), "SyncTagRadioButton", ""}}},
        {"sync_interval", "", {{"Sync_interval", QObject::tr("Sync interval"), "SyncTimeCombobox", ""}}},
        {"manual_sync", "", {{"manual_sync", "", "ManualSyncButton", ""}}},
    }};
static CalendarSettingSetting setting_base = {
    "setting_base",               QObject::tr("Manage calendar"), {
        {"acccount_items",        "",                                         {{"AccountCombobox",     QObject::tr("Calendar account"),      "AccountCombobox",      ""}}},
        {"event_types",           QObject::tr("Event types"),                 {{"JobTypeListView",     "",                                   "JobTypeListView",      ""}}}
    }
};

static CalendarSettingSetting setting_general = {
    "setting_general",
    QObject::tr("General settings"),
    {{"general", QObject::tr("General"), {{"firstday", QObject::tr("First day of week"), "FirstDayofWeek", "", "Sunday"}, {"time", QObject::tr("Time"), "Time", ""}}}}};

Calendarmainwindow::Calendarmainwindow(int index, QWidget *w)
    : DMainWindow(w)
    , m_defaultIndex(index)
{
    setContentsMargins(QMargins(0, 0, 0, 0));
    initUI();
    initConnection();
    initData();
    setMinimumSize(CalendarMWidth, CalendarMHeight);
    setWindowTitle(tr("Calendar"));
    new CalendarAdaptor(this);
    resize(CalendarMWidth, CalendarMHeight);
    //接受释放
    this->setAcceptDrops(true);
    //接收点击焦点
    setFocusPolicy(Qt::ClickFocus);
    QShortcut *shortcut = new QShortcut(this);
    shortcut->setKey(QKeySequence(QLatin1String("Ctrl+F")));
    connect(shortcut, SIGNAL(activated()), this, SLOT(slotSearchEdit()));

    QShortcut *viewshortcut = new QShortcut(this);
    viewshortcut->setKey(QKeySequence(QLatin1String("Ctrl+Shift+/")));
    connect(viewshortcut, SIGNAL(activated()), this, SLOT(slotViewShortcut()));

    setTitlebarShadowEnabled(true);
    //获取桌面窗口大小
    QDesktopWidget *desktopwidget = QApplication::desktop();
    //若分辨率改变则重新设置最大尺寸
    connect(desktopwidget, &QDesktopWidget::resized, this, &Calendarmainwindow::slotSetMaxSize);
    slotSetMaxSize();

    //兼容以前的配置信息
    if (CConfigSettings::getInstance()->contains("base.geometry")) {
        QByteArray arrayByte = CConfigSettings::getInstance()->value("base.geometry").toByteArray();
        bool isOk = false;
        int state = CConfigSettings::getInstance()->value("base.state").toInt(&isOk);
        Qt::WindowStates winStates = static_cast<Qt::WindowStates>(state);
        if (winStates.testFlag(Qt::WindowMaximized)) {
            showMaximized();
        } else {
            restoreGeometry(arrayByte);
            Dtk::Widget::moveToCenter(this);
        }
        //移除旧的配置信息
        CConfigSettings::getInstance()->remove("base.geometry");
        CConfigSettings::getInstance()->remove("base.state");
    } else if (CConfigSettings::getInstance()->contains("base.windowWidth")) {
        //获取窗口的宽度和高度
        int width = CConfigSettings::getInstance()->value("base.windowWidth").toInt();
        int height = CConfigSettings::getInstance()->value("base.windowHeight").toInt();
        QRect rect(0, 0, width, height);
        this->setGeometry(rect);
    } else {
        //如果没有相关配置则设置为默认尺寸
        QRect rect(0, 0, Calendar_Default_Width, Calendar_Default_Height);
        this->setGeometry(rect);
    }
    //在屏幕中央显示
    Dtk::Widget::moveToCenter(this);

    //注册光标位置
    CalendarGlobalEnv::getGlobalEnv()->registerKey(DDECalendar::CursorPointKey, QPoint());
    //保存主窗口指针
    CalendarGlobalEnv::getGlobalEnv()->registerKey("MainWindow", QVariant::fromValue(static_cast<void *>(this)));
}

Calendarmainwindow::~Calendarmainwindow()
{
    CDynamicIcon::releaseInstance();
}

void Calendarmainwindow::slotViewShortcut()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    Shortcut sc;
    QStringList shortcutString;
    QString param1 = "-j=" + sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProc = new QProcess(this);
    shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProc, SIGNAL(finished(int)), shortcutViewProc, SLOT(deleteLater()));
}

/**
 * @brief Calendarmainwindow::slotCurrentDateUpdate     更新当前时间
 */
void Calendarmainwindow::slotCurrentDateUpdate()
{
    //获取当前时间
    const QDateTime _currentDate = QDateTime::currentDateTime();
    //设置当前时间
    m_DayWindow->setCurrentDateTime(_currentDate);
    //如果当前日期与动态图标日期不一样则重新生成动态图标
    if (_currentDate.date() != CDynamicIcon::getInstance()->getDate()) {
        CDynamicIcon::getInstance()->setDate(QDate::currentDate());
        CDynamicIcon::getInstance()->setIcon();
        //更新视图数据显示
        for (int i = 0; i < m_stackWidget->count(); ++i) {
            CScheduleBaseWidget *widget = qobject_cast<CScheduleBaseWidget *>(m_stackWidget->widget(i));
            widget->updateData();
        }
        //设置年视图年数据时间显示
        m_yearwindow->setYearData();
        //更新月视图当前周横线绘制
        m_monthWindow->setCurrentDateTime(_currentDate);
    }
}

/**
 * @brief Calendarmainwindow::slotSetSearchFocus 设置搜索框焦点
 */
void Calendarmainwindow::slotSetSearchFocus()
{
    //设置输入框获取焦点
    slotSearchEdit();
}

/**
 * @brief Calendarmainwindow::viewWindow        切换视图
 * @param type                                  视图索引
 * @param showAnimation                         是否显示动画
 */
void Calendarmainwindow::viewWindow(int type, const bool showAnimation)
{
    if (type < 0 || type >= m_stackWidget->count()) {
        qWarning() << "set view error,set index:" << type;
        return;
    }
    if (showAnimation) {
        m_stackWidget->setCurrent(type);
    } else {
        m_stackWidget->setCurrentIndex(type);
    }
    if (m_buttonBox->checkedId() != type) {
        //设置选中
        m_buttonBox->button(type)->setChecked(true);
    }
    switch (type) {
    case DDECalendar::CalendarYearWindow: {
        //更新界面显示
        m_yearwindow->updateData();
    } break;
    case DDECalendar::CalendarMonthWindow: {
    } break;
    case DDECalendar::CalendarWeekWindow: {
    } break;
    case DDECalendar::CalendarDayWindow: {
        m_DayWindow->setTime();
        m_searchflag = true;
    } break;
    }
    m_priindex = type == 0 ? m_priindex : type;
    //为了与老版本配置兼容
    CConfigSettings::getInstance()->setOption("base.view", type + 1);
}

void Calendarmainwindow::updateHeight()
{
    int index = m_stackWidget->currentIndex();

    if (index < 0 || index > m_stackWidget->count() - 1) {

        return;
    }

    switch (index) {
    case DDECalendar::CalendarYearWindow: {
    } break;
    case DDECalendar::CalendarMonthWindow: {
    } break;
    case DDECalendar::CalendarWeekWindow: {
        m_weekWindow->updateHeight();
    } break;
    case DDECalendar::CalendarDayWindow: {
        m_DayWindow->updateHeight();
    } break;
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
    m_scheduleSearchView->setTheMe(type);
}

void Calendarmainwindow::slotOpenSchedule(QString job)
{
    if (job.isEmpty())
        return;
    DSchedule::Ptr out;
    DSchedule::fromJsonString(out, job);

    //设置被选中
    m_buttonBox->button(DDECalendar::CalendarDayWindow)->setChecked(true);
    //设置获取焦点
    m_buttonBox->button(DDECalendar::CalendarDayWindow)->setFocus();
    //切换到日视图
    m_stackWidget->setCurrentIndex(DDECalendar::CalendarDayWindow);
    //设置选择时间
    m_DayWindow->setSelectDate(out->dtStart().date());
    //更新界面显示
    m_DayWindow->updateData();
    //设置非全天时间定位位置
    m_DayWindow->setTime(out->dtStart().time());
    //弹出编辑对话框
    CMyScheduleView dlg(out, this);
    dlg.exec();
    slotWUpdateSchedule();
}
void Calendarmainwindow::initUI()
{
    //设置主窗口辅助技术显示名称和描述
    this->setObjectName("MainWindow");
    this->setAccessibleName("MainWindow");
    this->setAccessibleDescription("This is the main window");
    m_currentDateUpdateTimer = new QTimer(this);
    //1.5秒更新当前时间
    m_currentDateUpdateTimer->start(1500);

    CDynamicIcon::getInstance()->setTitlebar(this->titlebar());
    CDynamicIcon::getInstance()->setIcon();

    m_titleWidget = new CTitleWidget(this);
    m_titleWidget->setFocusPolicy(Qt::TabFocus);
    this->titlebar()->setCustomWidget(m_titleWidget);
    setTabOrder(this->titlebar(), m_titleWidget);
    //设置状态栏焦点代理为标题窗口
    this->titlebar()->setFocusProxy(m_titleWidget);
    //this->titlebar()->setFocusProxy(titleWidget);

    this->titlebar()->setQuitMenuVisible(true);//先设置后，才可以获取menu内容。因为setQuitMenuVisible接口中进行了action的添加操作
    QMenu *menuTitleBar = this->titlebar()->menu();

    QAction *pSetting = new QAction(tr("Manage"), menuTitleBar);
    //menuTitleBar->addAction(pSetting);
    menuTitleBar->insertSeparator(menuTitleBar->actions()[0]);
    menuTitleBar->insertAction(menuTitleBar->actions()[0], pSetting);

    connect(pSetting, &QAction::triggered, this, &Calendarmainwindow::slotOpenSettingDialog);

    //接收设置按键焦点
    connect(m_titleWidget, &CTitleWidget::signalSetButtonFocus, [=] {
        m_setButtonFocus = true;
    });
    connect(m_titleWidget, &CTitleWidget::signalSearchFocusSwitch, this,
            &Calendarmainwindow::slotSearchFocusSwitch);

    m_searchEdit = m_titleWidget->searchEdit();
    m_buttonBox = m_titleWidget->buttonBox();
    m_newScheduleBtn = m_titleWidget->newScheduleBtn();

    QHBoxLayout *tMainLayout = new QHBoxLayout;
    tMainLayout->setContentsMargins(10, 10, 10, 10);
    tMainLayout->setSpacing(1);

    m_sidebarView = new SidebarView(this);
    tMainLayout->addWidget(m_sidebarView, 1);

    m_stackWidget = new AnimationStackedWidget();
    m_stackWidget->setObjectName("StackedWidget");
    m_stackWidget->setAccessibleName("StackedWidget");
    m_stackWidget->setAccessibleDescription("Container for year window, month window, week window and day window");
    m_stackWidget->setContentsMargins(0, 0, 0, 0);
    m_stackWidget->setDuration(250);
    createview();

    tMainLayout->addWidget(m_stackWidget, 4);
    m_contentBackground = new DFrame;
    m_contentBackground->setAccessibleName("ScheduleSearchWidgetBackgroundFrame");
    m_contentBackground->setObjectName("ScheduleSearchWidgetBackgroundFrame");
    m_contentBackground->setContentsMargins(0, 0, 0, 0);
    DPalette anipa = m_contentBackground->palette();
    anipa.setColor(DPalette::Background, "#F8F8F8");
    m_contentBackground->setAutoFillBackground(true);
    m_contentBackground->setPalette(anipa);

    m_scheduleSearchView = new CScheduleSearchView(this);
    m_scheduleSearchView->setObjectName("ScheduleSearchWidget");
    m_scheduleSearchView->setAccessibleName("ScheduleSearchWidget");
    m_scheduleSearchView->setAccessibleDescription("Window showing search results");

    QVBoxLayout *ssLayout = new QVBoxLayout;
    ssLayout->setMargin(0);
    ssLayout->setSpacing(0);
    ssLayout->addWidget(m_scheduleSearchView);
    m_contentBackground->setLayout(ssLayout);
    tMainLayout->addWidget(m_contentBackground);
    m_contentBackground->setVisible(false);

    DWidget *maincentralWidget = new DWidget(this);
    maincentralWidget->setAccessibleName("mainCentralWidget");

    maincentralWidget->setLayout(tMainLayout);

    setCentralWidget(maincentralWidget);
    m_transparentFrame = new DFrame(this);
    m_transparentFrame->setAutoFillBackground(true);
    m_transparentFrame->hide();

    m_animation = new QPropertyAnimation(this);

    //delete快捷键
    QShortcut *dShortcut = new QShortcut(this);
    dShortcut->setKey(QKeySequence(QLatin1String("Delete")));
    connect(dShortcut, &QShortcut::activated, this, &Calendarmainwindow::slotDeleteitem);
}

void Calendarmainwindow::initConnection()
{
    connect(m_stackWidget, &AnimationStackedWidget::signalIsFinished, this,
            &Calendarmainwindow::slotSetButtonBox);
    connect(m_buttonBox, &DButtonBox::buttonClicked, this, &Calendarmainwindow::slotstackWClicked);
    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &Calendarmainwindow::slotSreturnPressed);
    connect(m_searchEdit, &DSearchEdit::textChanged, this, &Calendarmainwindow::slotStextChanged);
    connect(m_searchEdit, &DSearchEdit::focusChanged, this, &Calendarmainwindow::slotStextfocusChanged);
    //监听当前应用主题切换事件
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this,
            &Calendarmainwindow::slotTheme);
    //编辑搜索日程刷新界面
    connect(m_scheduleSearchView, &CScheduleSearchView::signalSelectSchedule, this,
            &Calendarmainwindow::slotSearchSelectSchedule);
    connect(m_scheduleSearchView, &CScheduleSearchView::signalScheduleHide, this,
            &Calendarmainwindow::setScheduleHide);
    connect(m_scheduleSearchView, &CScheduleSearchView::signalSelectCurrentItem, this,
            &Calendarmainwindow::slotSetSearchFocus);
    //更新当前时间
    connect(m_currentDateUpdateTimer, &QTimer::timeout, this,
            &Calendarmainwindow::slotCurrentDateUpdate);
    //切换视图
    connect(m_yearwindow, &CYearWindow::signalSwitchView, this, &Calendarmainwindow::slotSwitchView);
    connect(m_monthWindow, &CMonthWindow::signalSwitchView, this, &Calendarmainwindow::slotSwitchView);
    connect(m_weekWindow, &CWeekWindow::signalSwitchView, this, &Calendarmainwindow::slotSwitchView);
    //按钮关联新建日程
    connect(m_newScheduleBtn, &DToolButton::clicked, this, &Calendarmainwindow::slotNewSchedule);

    connect(qApp, &QGuiApplication::applicationStateChanged, this, &Calendarmainwindow::slotapplicationStateChanged);
    connect(m_titleWidget, &CTitleWidget::signalSidebarStatusChange, this, &Calendarmainwindow::slotSidebarStatusChange);
}

void Calendarmainwindow::initData()
{
    //根据配置设置侧边栏状态
    m_titleWidget->setSidebarStatus(gSetting->getUserSidebarStatus());
}

/**
 * @brief Calendarmainwindow::createview        创建视图
 */
void Calendarmainwindow::createview()
{
    m_yearwindow = new CYearWindow(this);
    m_yearwindow->setObjectName("yearwindow");
    m_yearwindow->setAccessibleName("yearwindow");
    m_yearwindow->updateData();
    m_stackWidget->addWidget(m_yearwindow);
    m_monthWindow = new CMonthWindow(this);
    m_monthWindow->setObjectName("monthWindow");
    m_monthWindow->setAccessibleName("monthWindow");
    m_monthWindow->updateData();
    m_stackWidget->addWidget(m_monthWindow);

    m_weekWindow  = new CWeekWindow(this);
    m_weekWindow->updateData();
    m_stackWidget->addWidget(m_weekWindow);

    m_DayWindow = new CDayWindow;
    m_DayWindow->updateData();
    m_stackWidget->addWidget(m_DayWindow);

    //如果默认视图不在范围内则设置为月窗口显示
    if (m_defaultIndex < 0 || m_defaultIndex >= m_stackWidget->count()) {
        m_defaultIndex = 1;
        qWarning() << "set default view index beyond,index:" << m_defaultIndex;
    }
    m_stackWidget->setCurrentIndex(m_defaultIndex);
    m_buttonBox->button(m_defaultIndex)->setChecked(true);
    //如果为日视图则设置非全天时间定位
    if (m_defaultIndex == DDECalendar::CalendarDayWindow) {
        m_DayWindow->setTime();
    }
}

/**
 * @brief Calendarmainwindow::setScheduleHide       隐藏提示框
 */
void Calendarmainwindow::setScheduleHide()
{
    m_yearwindow->slotSetScheduleHide();
    m_monthWindow->slotScheduleHide();
    m_weekWindow->slotScheduleHide();
    m_DayWindow->slotScheduleHide();
}

void Calendarmainwindow::resizeEvent(QResizeEvent *event)
{
    DMainWindow::resizeEvent(event);
    m_transparentFrame->resize(width(), height() - 50);

    if (width() < CalendarSwitchWidth) {
        m_titleWidget->setShowState(CTitleWidget::Title_State_Mini);
    } else {
        m_titleWidget->setShowState(CTitleWidget::Title_State_Normal);
    }

    if (width() < CalendarViewSwitchWidth) {
        m_isNormalStateShow = false;
        m_stackWidget->setVisible(!m_contentBackground->isVisible());
        m_scheduleSearchViewMaxWidth = this->width();
    } else {
        m_scheduleSearchViewMaxWidth = qRound(0.2325 * width() + 0.5);
        m_isNormalStateShow = true;
        m_stackWidget->setVisible(true);
        m_contentBackground->setVisible(m_opensearchflag);
    }
    m_scheduleSearchView->setMaxWidth(m_scheduleSearchViewMaxWidth);
    setSearchWidth(m_scheduleSearchViewMaxWidth);
    setScheduleHide();

    //窗口宽度小于826时隐藏侧边栏
    if (this->width() < 826) {
        m_titleWidget->setSidebarStatus(false);
    }

    //保存窗口大小
    CConfigSettings::getInstance()->setOption("base.windowWidth", event->size().width());
    CConfigSettings::getInstance()->setOption("base.windowHeight", event->size().height());
}

/**
 * @brief Calendarmainwindow::slotstackWClicked 点击按钮切换视图
 * @param bt
 */
void Calendarmainwindow::slotstackWClicked(QAbstractButton *bt)
{
    setScheduleHide();
    int index = m_buttonBox->id(bt);
    viewWindow(index, true);
}

void Calendarmainwindow::slotWUpdateSchedule()
{
    if (m_opensearchflag && !m_searchEdit->text().isEmpty()) {
        m_scheduleSearchView->slotsetSearch(m_searchEdit->text());
    }
    updateHeight();
    return;
}

void Calendarmainwindow::slotSreturnPressed()
{
    if (!m_opensearchflag && !m_searchEdit->text().isEmpty()) {
        m_opensearchflag = true;
    }
    //如果为搜索状态
    if (m_opensearchflag) {
        //根据显示显示状态，是否显示左侧视图窗口
        if (!m_isNormalStateShow) {
            m_stackWidget->setVisible(false);
        }
        m_contentBackground->setVisible(true);
    }
    m_scheduleSearchView->slotsetSearch(m_searchEdit->text());
    updateHeight();
}

void Calendarmainwindow::slotStextChanged()
{
    if (!m_searchEdit->text().isEmpty()) {
        m_yearwindow->setSearchWFlag(true);
        m_weekWindow->setSearchWFlag(true);
        m_monthWindow->setSearchWFlag(true);
        m_DayWindow->setSearchWFlag(true);
    } else {
        m_scheduleSearchView->clearSearch();
        m_yearwindow->setSearchWFlag(false);
        m_monthWindow->setSearchWFlag(false);
        m_weekWindow->setSearchWFlag(false);
        m_DayWindow->setSearchWFlag(false);
        m_contentBackground->setVisible(false);
        m_stackWidget->setVisible(true);
        m_opensearchflag = false;
    }
    updateHeight();
}

/**
 * @brief Calendarmainwindow::slotStextfocusChanged     搜索框有焦点时隐藏提示框并设置搜索日程焦点
 * @param onFocus
 */
void Calendarmainwindow::slotStextfocusChanged(bool onFocus)
{
    if (onFocus) {
        setScheduleHide();
    }
}

void Calendarmainwindow::slotSearchEdit()
{
    m_searchEdit->lineEdit()->setFocus();
}

/**
 * @brief Calendarmainwindow::slotSearchSelectSchedule  单击搜索日程动画设置
 * @param scheduleInfo
 */
void Calendarmainwindow::slotSearchSelectSchedule(const DSchedule::Ptr &scheduleInfo)
{
    //如果小尺寸显示模式，在显示搜索窗口的时候，左侧视图会被隐藏
    //如果点击一个搜索结果则隐藏搜索窗口，展示左侧视图
    if (!m_isNormalStateShow) {
        //        CalendarGlobalEnv::getGlobalEnv()->registerKey("SearchItemEvent", "Keyboard");

        QVariant variant;
        CalendarGlobalEnv::getGlobalEnv()->getValueByKey("SearchItemEvent", variant);
        QString searchItemEvent = variant.toString();
        //如果为鼠标点击操作
        if (searchItemEvent == "MousePress") {
            m_stackWidget->setVisible(true);
            m_contentBackground->setVisible(false);
        }
    }
    //获取当前视图编号
    CScheduleBaseWidget *_showWidget = dynamic_cast<CScheduleBaseWidget *>(m_stackWidget->currentWidget());
    if (_showWidget) {
        //如果日程开始时间年份与选择时间年份不一样则切换年份显示
        bool changeYear = _showWidget->getSelectDate().year() != scheduleInfo->dtStart().date().year();
        //设置选择时间
        if (_showWidget->setSelectDate(scheduleInfo->dtStart().date(), changeYear)) {
            //更新显示数据
            _showWidget->updateData();
            //设置年份信息显示
            _showWidget->setYearData();
            //延迟150毫秒设置选中动画
            QTimer::singleShot(150, this, [ = ] {
                _showWidget->setSelectSearchScheduleInfo(scheduleInfo);
            });
        }
        //如果当前界面不为年试图则更新年视图数据
        if (_showWidget != m_yearwindow) {
            m_yearwindow->updateData();
        }
    }
}

/**
 * @brief Calendarmainwindow::slotViewtransparentFrame      添加视图阴影
 * @param isShow
 */
void Calendarmainwindow::slotViewtransparentFrame(const bool isShow)
{
    if (isShow) {
        m_transparentFrame->resize(width(), height() - 50);
        m_transparentFrame->move(0, 50);
        m_transparentFrame->show();
    } else {
        m_transparentFrame->hide();
    }
}

/**
 * @brief Calendarmainwindow::slotSetButtonBox      启用buttonbox
 */
void Calendarmainwindow::slotSetButtonBox()
{
    //如果为键盘操作则切换后设置焦点
    if (m_setButtonFocus) {
        //获取焦点
        m_buttonBox->button(m_buttonBox->checkedId())->setFocus();
    }
    m_setButtonFocus = false;
}

/**
 * @brief Calendarmainwindow::slotSwitchView        切换视图
 * @param viewIndex         切换类型
 */
void Calendarmainwindow::slotSwitchView(const int viewIndex)
{
    // 0:跳转上一个视图  1：月视图  2：周视图  3:日视图
    switch (viewIndex) {
    case 0: {
        if (m_priindex == DDECalendar::CalendarYearWindow)
            m_priindex = DDECalendar::CalendarDayWindow;
        viewWindow(m_priindex);
        break;
    }
    case 1: {
        viewWindow(DDECalendar::CalendarMonthWindow);
        break;
    }
    case 2: {
        viewWindow(DDECalendar::CalendarWeekWindow);
        break;
    }
    case 3: {
        viewWindow(DDECalendar::CalendarDayWindow);
        break;
    }
    default:
        break;
    }
}

/**
 * @brief Calendarmainwindow::slotNewSchedule       全局新建日程
 */
void Calendarmainwindow::slotNewSchedule()
{
    //设置日程开始时间
    QDateTime _beginTime(m_yearwindow->getSelectDate(), QTime::currentTime());
    //新建日程对话框
    CScheduleDlg _scheduleDig(1, this, false);
    //设置开始时间
    _scheduleDig.setDate(_beginTime);
    _scheduleDig.exec();
}

void Calendarmainwindow::slotDeleteitem()
{
    if (m_scheduleSearchView->getHasScheduleShow() && m_scheduleSearchView->getScheduleStatus()) {
        //删除选中的schedule
        m_scheduleSearchView->deleteSchedule();
    } else {
        //获取当前的
        CScheduleBaseWidget *widget = qobject_cast<CScheduleBaseWidget *>(m_stackWidget->currentWidget());
        if (widget) {
            widget->deleteselectSchedule();
        }
    }
}

/**
 * @brief Calendarmainwindow::slotSetMaxSize    根据屏幕分辨率调整窗口最大尺寸
 * @param size
 */
void Calendarmainwindow::slotSetMaxSize(int size)
{
    Q_UNUSED(size);
    //获取屏幕大小
    QSize deskSize = QApplication::desktop()->size();
    //设置最大尺寸为屏幕尺寸
    setMaximumSize(deskSize);
    if (TabletConfig::isTablet()) {
        setFixedSize(deskSize);
    }
}

void Calendarmainwindow::slotSearchFocusSwitch()
{
    //设置搜索日程展示列表焦点
    if (m_contentBackground->isVisible() && m_scheduleSearchView->getHasScheduleShow()) {
        m_scheduleSearchView->setFocus(Qt::TabFocusReason);
    }
}

void Calendarmainwindow::slotSidebarStatusChange(bool status)
{
    //设置帐户侧边栏显示状态
    m_sidebarView->setVisible(status);
    //展开侧边栏后最小宽度为826
    if (status && width() < 826) {
        resize(826, height());
    }
    //将状态保存在配置文件中
    gSetting->setUserSidebarStatus(status);
}

void Calendarmainwindow::mouseMoveEvent(QMouseEvent *event)
{
    //如果为平板模式则不可移动
    if (TabletConfig::isTablet()) {
        Q_UNUSED(event);
    } else {
        DMainWindow::mouseMoveEvent(event);
        setScheduleHide();
    }
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
    setScheduleHide();
    if (event->button() & Qt::LeftButton)
        m_startPos = event->pos();
}

bool Calendarmainwindow::event(QEvent *event)
{
    //如果窗口移动，隐藏日程悬浮框
    if (event->type() == QEvent::Move) {
        setScheduleHide();
    }

    //如果为活动窗口则显示视图阴影
    if (event->type() == QEvent::WindowActivate) {
        slotViewtransparentFrame(false);
    }
    //如果不为活动窗口则隐藏视图阴影
    if (event->type() == QEvent::WindowDeactivate) {
        slotViewtransparentFrame(true);
    }
    return DMainWindow::event(event);
}

void Calendarmainwindow::slotapplicationStateChanged(Qt::ApplicationState state)
{
    static QDateTime currentDateTime = QDateTime::currentDateTime();
    //TODO:目前没有找到窗口显示不是激活状态的原因（会先激活然后为非激活状态）。
    //当间隔时间少于10毫秒且状态为非激活时，将窗口设置显示在最顶端并设置激活状态
    if (state == Qt::ApplicationInactive) {
        //获取当前时间
        QDateTime currentTime = QDateTime::currentDateTime();
        //考虑到会修改系统时间的情况，时间差在0～30毫秒内才会设置
        if (currentDateTime.msecsTo(currentTime) < 30 && currentDateTime.msecsTo(currentTime) >= 0) {
            activateWindow();
            raise();
        }
        //断开信号连接
        disconnect(qApp, &QGuiApplication::applicationStateChanged, this, &Calendarmainwindow::slotapplicationStateChanged);
    }
}

void Calendarmainwindow::slotOpenSettingDialog()
{
    SettingWidgets::init();
    if (nullptr == m_dsdSetting) {
        m_dsdSetting = new DSettingsDialog(this);
        m_dsdSetting->setIcon(CDynamicIcon::getInstance()->getPixmap());
        m_dsdSetting->setFixedSize(682, 506);
        m_dsdSetting->widgetFactory()->registerWidget("login", UserloginWidget::createloginButton);
        m_dsdSetting->widgetFactory()->registerWidget("FirstDayofWeek",     SettingWidgets::createFirstDayofWeekWidget);
        m_dsdSetting->widgetFactory()->registerWidget("Time",               SettingWidgets::createTimeTypeWidget);
        m_dsdSetting->widgetFactory()->registerWidget("AccountCombobox",    std::bind(&Calendarmainwindow::createAccountCombobox,       this, std::placeholders::_1));
        m_dsdSetting->widgetFactory()->registerWidget("JobTypeListView",    std::bind(&Calendarmainwindow::createJobTypeListView,       this, std::placeholders::_1));
        m_dsdSetting->widgetFactory()->registerWidget("SyncTagRadioButton", std::bind(&Calendarmainwindow::createSyncTagRadioButton,    this, std::placeholders::_1));
        m_dsdSetting->widgetFactory()->registerWidget("SyncTimeCombobox",   std::bind(&Calendarmainwindow::createSyncFreqCombobox,      this, std::placeholders::_1));
        m_dsdSetting->widgetFactory()->registerWidget("ManualSyncButton",   std::bind(&Calendarmainwindow::createManualSyncButton,      this, std::placeholders::_1));
        QString strJson;

        CalendarSettingSettings calendarSettings;
        calendarSettings.append(setting_account);
        calendarSettings.append(setting_base);
        calendarSettings.append(setting_general);

        //社区版不含云同步相关内容
        if (DSysInfo::uosEditionType() == DSysInfo::UosCommunity) {
            calendarSettings.removeGroup("setting_account");
        }
        //未登录uos帐号时，移除部分选项
        if (!gUosAccountItem) {
            calendarSettings.removeGroup("setting_account", "account_sync_items");
            calendarSettings.removeGroup("setting_account", "sync_interval");
            calendarSettings.removeGroup("setting_account", "manual_sync");
        }

        QJsonObject obj;
        obj.insert("groups", calendarSettings.toJson());
        strJson = QJsonDocument(obj).toJson(QJsonDocument::Compact);

        auto settings = Dtk::Core::DSettings::fromJson(strJson.toLatin1());
        m_dsdSetting->setObjectName("SettingDialog");
        m_dsdSetting->updateSettings(settings);
        //恢复默认设置按钮不显示
        m_dsdSetting->setResetVisible(false);
        //QList<Widget>
        QList<QWidget *> lstwidget = m_dsdSetting->findChildren<QWidget *>();
        if (lstwidget.size() > 0) { //accessibleName
            for (QWidget *wid : lstwidget) {
                if ("ContentWidgetForsetting_base.event_types" == wid->accessibleName()) {
                    JobTypeListView *view = m_dsdSetting->findChild<JobTypeListView *>("JobTypeListView");
                    if (!view)
                        return;
                    DIconButton *addButton = new DIconButton(DStyle::SP_IncreaseElement, nullptr);
                    //跟UI沟通size设置为20*20
                    addButton->setFixedSize(20, 20);
                    wid->layout()->addWidget(addButton);
                    //使addButton的右边距等于view的右边距
                    int leftMargin = wid->layout()->contentsMargins().left();
                    wid->layout()->setContentsMargins(leftMargin, 0, leftMargin, 0);

                    addButton->setEnabled(view->canAdd());

                    //当日常类型超过上限时，更新button的状态
                    connect(view, &JobTypeListView::signalAddStatusChanged, addButton, &DIconButton::setEnabled);
                    //新增类型
                    connect(addButton, &DIconButton::clicked, this, &Calendarmainwindow::signal_addScheduleType);
                }
                if (wid->accessibleName().contains("DefaultWidgetAtContentRow")) {
                    //DefaultWidgetAtContentRow是设置对话框右边每一个option条目对应widget的accessibleName的前缀，所以如果后续有更多条目，需要做修改
                    wid->layout()->setMargin(0);
                }
            }
        }
    }

    //移除立刻同步按钮的背景色
    QWidget  *ManualSyncWidget = m_dsdSetting->findChild<QWidget *>("ManualSyncWidget");
    ManualSyncWidget     = ManualSyncWidget == nullptr ? nullptr : ManualSyncWidget->parentWidget();
    ManualSyncWidget     = ManualSyncWidget == nullptr ? nullptr : ManualSyncWidget->parentWidget();
    DBackgroundGroup *bk = ManualSyncWidget == nullptr ? nullptr : qobject_cast<DBackgroundGroup *>(ManualSyncWidget);
    if (bk) {
        bk->setBackgroundRole(QPalette::Base);
    }
    //首次显示JobTypeListView时，更新日程类型
    if (DComboBox *combobox = m_dsdSetting->findChild<DComboBox *>("AccountCombobox")) {
        if (JobTypeListView *listview = m_dsdSetting->findChild<JobTypeListView *>("JobTypeListView")) {
            listview->updateCalendarAccount(combobox->currentData().toString());
        }
    }

    //内容定位到顶端
    m_dsdSetting->exec();
    //使用完后释放
    delete m_dsdSetting;
    gCalendarManager->updateData();
    m_dsdSetting = nullptr;
}

/**
 * @brief CDayWindow::dragEnterEvent      拖拽进入事件
 * @param event
 */
void Calendarmainwindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

/**
 * @brief Calendarmainwindow::createAccountCombobox 关联帐户的combobox
 */
QPair<QWidget *, QWidget *> Calendarmainwindow::createAccountCombobox(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    DComboBox *widget = new DComboBox;
    widget->setObjectName("AccountCombobox");
    widget->setFixedSize(150, 36);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);

    //更新帐户列表
    auto accountUpdate = [=]() {
        //TODO:外部退出帐户时处于新建日程弹窗界面时会崩溃
        QVariant oldAccountID = widget->currentData();
        widget->blockSignals(true);
        widget->clear();
        for (auto account : gAccountManager->getAccountList()) {
            widget->addItem(account->getAccount()->accountName(), account->getAccount()->accountID());
        }
        widget->setCurrentIndex(widget->findData(oldAccountID));
        if (widget->currentIndex() < 0)
            widget->setCurrentIndex(0);
        widget->blockSignals(false);

        emit signal_calendarAccountChanged(widget->currentData().toString());
    };
    //TODO:控制中心退出帐号时，更新帐户列表
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, accountUpdate);

    //TODO:切换帐号时，更新日程类型
    connect(widget, QOverload<int>::of(&DComboBox::currentIndexChanged), this, [=](int index) {
        emit signal_calendarAccountChanged(widget->itemData(index).toString());
    });

    accountUpdate();
    return optionWidget;
}

/**
 * @brief Calendarmainwindow::createJobTypeListView 日程类型的listview
 */
QWidget *Calendarmainwindow::createJobTypeListView(QObject *obj)
{
    Q_UNUSED(obj)
    JobTypeListView *lv = new JobTypeListView;
    lv->setObjectName("JobTypeListView");
    connect(this, &Calendarmainwindow::signal_calendarAccountChanged, lv, &JobTypeListView::updateCalendarAccount);
    connect(this, &Calendarmainwindow::signal_addScheduleType, lv, &JobTypeListView::slotAddScheduleType);

    return lv;
}

/**
 * @brief Calendarmainwindow::createSyncFreqCombobox 同步频率的combobox
 */
QPair<QWidget *, QWidget *> Calendarmainwindow::createSyncFreqCombobox(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    DComboBox *widget = new DComboBox;
    widget->setMaximumWidth(150);
    widget->addItem(tr("Manual"),   DAccount::SyncFreq_Maunal);
    widget->addItem(tr("15 mins"),  DAccount::SyncFreq_15Mins);
    widget->addItem(tr("30 mins"),  DAccount::SyncFreq_30Mins);
    widget->addItem(tr("1 hour"),   DAccount::SyncFreq_1hour);
    widget->addItem(tr("24 hours"), DAccount::SyncFreq_24hour);

    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);

    int index = -1;
    if (gUosAccountItem)
        index = widget->findData(gUosAccountItem->getAccount()->syncFreq());
    widget->setCurrentIndex(index);

    //TODO:更新union帐户的的同步频率
    connect(widget, QOverload<int>::of(&DComboBox::currentIndexChanged), this, &Calendarmainwindow::slotSetUosSyncFreq);

    return optionWidget;
}

/**
 * @brief Calendarmainwindow::createSyncTagRadioButton 同步项的radiobutton
 */
QPair<QWidget *, QWidget *> Calendarmainwindow::createSyncTagRadioButton(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    DAccount::AccountState type = DAccount::Account_Calendar;
    if (option->key().endsWith("Account_Calendar"))
        type = DAccount::Account_Calendar;
    if (option->key().endsWith("Account_Setting"))
        type = DAccount::Account_Setting;

    SyncTagRadioButton *widget = new SyncTagRadioButton(type);
    widget->setFixedWidth(16);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);

    //iconLabel
    QLabel *iconLabel = new QLabel;
    iconLabel->setFixedHeight(16);
    if (DAccount::Account_Calendar == type)
        iconLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/sync_schedule.svg"));
    if (DAccount::Account_Setting == type)
        iconLabel->setPixmap(DHiDPIHelper::loadNxPixmap(":/resources/icon/sync_setting.svg"));

    //iconWidget
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(iconLabel);
    layout->addWidget(optionWidget.first);
    layout->setContentsMargins(0, 1, 0, 1);
    QWidget *iconWidget = new QWidget;
    iconWidget->setLayout(layout);
    optionWidget.first = iconWidget;

    return optionWidget;
}

/**
 * @brief Calendarmainwindow::createManualSyncButton 立刻同步的button
 */
QWidget *Calendarmainwindow::createManualSyncButton(QObject *obj)
{
    Q_UNUSED(obj)
    QWidget *widget = new QWidget;
    widget->setObjectName("ManualSyncWidget");
    QPushButton *button = new QPushButton(widget);
    button->setFixedSize(266, 36);
    button->setText(tr("Sync Now"));

    QLabel *label = new QLabel;
    auto updateLastUpdateText = [=](const QString &datetime){
        if(gUosAccountItem) {
            label->setText(tr("Last sync") + ":" + dtFromString(datetime).toString("yyyy/MM/dd hh:mm"));
        }
    };

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(button, 0, Qt::AlignCenter);
    layout->addWidget(label, 0, Qt::AlignCenter);
    widget->setLayout(layout);
    connect(button, &QPushButton::clicked, this, &Calendarmainwindow::slotUosManualSync);
    //TODO: 更新时间
    if(gUosAccountItem) {
        updateLastUpdateText(gUosAccountItem->getDtLastUpdate());
        connect(gUosAccountItem.get(), &AccountItem::signalDtLastUpdate, this, updateLastUpdateText);
    }
    //TODO:立刻同步和最后一次同步时间
    return widget;
}

/**
 * @brief Calendarmainwindow::slotSetUosSyncFreq 设置UOS帐户的同步频率
 * @param freq
 */
void Calendarmainwindow::slotSetUosSyncFreq(int freq)
{
    QComboBox *com = qobject_cast<QComboBox *>(sender());
    if (!com)
        return;
    if (!gUosAccountItem)
        return;
    gUosAccountItem->setSyncFreq(DAccount::SyncFreqType(com->itemData(freq).toInt()));
}

void Calendarmainwindow::slotUosManualSync()
{
    if (!gUosAccountItem)
        return;
    gAccountManager->downloadByAccountID(gUosAccountItem->getAccount()->accountID());
}

/**
 * @brief CDayWindow::dropEvent          拖拽释放事件
 * @param event
 */
void Calendarmainwindow::dropEvent(QDropEvent *event)
{
    QPoint pos = event->pos();
    int diffPosx = pos.x() - m_startPos.x();
    int diffPosy = pos.y() - m_startPos.y();
    if ((diffPosx >= 16 || diffPosx <= -16) || (diffPosy >= 16 || diffPosy <= -16))
        slotNewSchedule();
}


CalendarSetting::SyncTagRadioButton::SyncTagRadioButton(DAccount::AccountState type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
{

    setObjectName("SyncTagRadioButton");
    if (gUosAccountItem) {
        m_state = DAccount::Account_Close;
        m_state = gUosAccountItem->getAccount()->accountState();
        connect(gUosAccountItem.get(), &AccountItem::signalAccountStateChange, this, &SyncTagRadioButton::updateAccountState);
        updateAccountState(m_state);
    }
}

void CalendarSetting::SyncTagRadioButton::updateAccountState(DAccount::AccountStates state)
{
    m_state = state;
    setChecked(m_state & m_type);
    //TODO:是否联网
    //setEnabled((m_state & DAccount::Account_Open) && m_isOnline);
}

void CalendarSetting::SyncTagRadioButton::updateOnLineState(bool isOnline)
{
    m_isOnline = isOnline;
    updateAccountState(m_state);
}

bool CalendarSetting::SyncTagRadioButton::isChecked()
{
    return m_checked;
}

DAccount::AccountState CalendarSetting::SyncTagRadioButton::type()
{
    return m_type;
}

void CalendarSetting::SyncTagRadioButton::setChecked(bool checked)
{
    if (m_checked == checked)
        return;
    if (!gUosAccountItem)
        return;

    m_checked = checked;
    update();

    //实现遍历所有的radiobutton获取account state
    DAccount::AccountStates states = gUosAccountItem->getAccount()->accountState();
    QObject *parent = this->parent();
    parent = parent == nullptr ? nullptr : parent->parent();
    if (parent) {
        for (auto obj : parent->findChildren<QWidget *>("SyncTagRadioButton")) {
            SyncTagRadioButton *rb = static_cast<SyncTagRadioButton *>(obj);
            if (rb->isChecked())
                states |= rb->type();
            else
                states &= ~rb->type();
        }
    }
    gUosAccountItem->setAccountState(states);

}

void CalendarSetting::SyncTagRadioButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QIcon icon = DStyle::standardIcon(this->style(), m_checked ? DStyle::SP_IndicatorChecked : DStyle::SP_IndicatorUnchecked);
    int y = (this->height() - 16) / 2;
    int x = (this->width() - 16) / 2;
    icon.paint(&painter, QRect(x, y, 16, 16), Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

void CalendarSetting::SyncTagRadioButton::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    setChecked(!m_checked);
}

void CalendarSettingSettings::removeGroup(const QString &groupName, const QString &groupName2)
{
    int index = this->indexOf(*this, groupName);
    if (index < 0)
        return;
    CalendarSettingGroups &groups = this->operator[](index)._groups;
    {
        int index = indexOf(groups, groupName2);
        if (index < 0)
            return;
        groups.removeAt(index);
    }
    if (groups.isEmpty()) {
        this->removeAt(index);
    }
}

void CalendarSettingSettings::removeGroup(const QString &groupName)
{
    int index = this->indexOf(*this, groupName);
    if (index < 0)
        return;
    this->removeAt(index);
}

int CalendarSettingSettings::indexOf(const CalendarSettingGroups &groups, const QString groupName)
{
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName)
            return k;
    }
    return -1;
}

int CalendarSettingSettings::indexOf(const CalendarSettingSettings &groups, const QString groupName)
{
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName)
            return k;
    }
    return -1;
}
