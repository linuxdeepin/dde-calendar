// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingdialog.h"
#include "dcalendareventlog.h"
#include "cdynamicicon.h"
#include "accountmanager.h"
#include "commondef.h"
#include "dcalendargeneralsettings.h"
#include "dcaldavaccountstatus.h"
#include "dcaldavprofile.h"
#include "settingWidget/userloginwidget.h"
#include "accountmanager.h"
#include "calendarmanage.h"
#include "caldavaccountdialog.h"
#include "units.h"

#include <QSpacerItem>
#include <QAction>
#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>

#include <DBackgroundGroup>
#include <DComboBox>
#include <DSysInfo>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DIcon>
#include <DPalette>
#include <DFontSizeManager>
#include <DToolButton>

#include <qglobal.h>
#include <qloggingcategory.h>

const QString ControlCenterDBusName = "org.deepin.dde.ControlCenter1";
const QString ControlCenterDBusPath = "/org/deepin/dde/ControlCenter1";
const QString ControlCenterPage = "datetime/region";

using namespace SettingWidget;

//静态的翻译不会真的翻译，但是会更新ts文件
//像static QString a = QObject::tr("hello"), a实际等于hello，但是ts会有hello这个词条
//调用DSetingDialog时会用到上述场景
static CalendarSettingSetting setting_account = {
    "setting_account",
    QObject::tr("Account settings"),
    {
        {
            "account",
            QObject::tr("UOS ID"),
            {}
        },
        {
            "third_party_accounts",
            QObject::tr("Third-party accounts"),
            {}
        },
    }
};

static CalendarSettingSetting setting_base = {
    "setting_base",
    QObject::tr("Manage calendar"),
    {
        {
            "acccount_items",
            "",
            {
                {
                    "AccountCombobox",                //key
                    QObject::tr("Associated account"), //name
                    "AccountCombobox",                //type
                    ""
                }
            }
        },
        {
            "event_types",
            QObject::tr("Event types"),
            {
                {
                    "JobTypeListView",   //key
                    "",                  //name
                    "JobTypeListView",   //type
                    ""                   //default
                }
            }
        }
    }
};

static CalendarSettingSetting setting_base_noaccount = {
    "setting_base",
    QObject::tr("Manage calendar"),
    {
        {
            "event_types",
            QObject::tr("Event types"),
            {
                {
                    "JobTypeListView",   //key
                    "",                  //name
                    "JobTypeListView",   //type
                    ""                   //default
                }
            }
        }
    }
};

static CalendarSettingSetting setting_general = {
    "setting_general",
    QObject::tr("General settings"),
    {
        {
            "general",
            QObject::tr("General"),
            {
                { "firstday", QObject::tr("First day of week"), "FirstDayofWeek", "", "Sunday" },
                { "time", QObject::tr("Time"), "Time", "" },
                { "control-center-button", "", "ControlCenterLink", "" },
            },
        },
    }
};
CSettingDialog::CSettingDialog(QWidget *parent) : DSettingsDialog(parent)
{
    qCDebug(ClientLogger) << "Creating settings dialog";
    m_controlCenterProxy = new ControlCenterProxy(ControlCenterDBusName,
                                                  ControlCenterDBusPath,
                                                  QDBusConnection::sessionBus(),
                                                  this);
    initWidget();
    initConnect();
    initData();
    initWidgetDisplayStatus();
    initView();
    qCDebug(ClientLogger) << "Settings dialog initialization complete";
}

void CSettingDialog::initView()
{
    qCDebug(ClientLogger) << "Initializing settings dialog view";
    setIcon(CDynamicIcon::getInstance()->getPixmap());
    widgetFactory()->registerWidget("FirstDayofWeek",     std::bind(&CSettingDialog::createFirstDayofWeekWidget,  this, std::placeholders::_1));
    widgetFactory()->registerWidget("Time",               std::bind(&CSettingDialog::createTimeTypeWidget,        this, std::placeholders::_1));
    widgetFactory()->registerWidget("ControlCenterLink",  std::bind(&CSettingDialog::createControlCenterLink,     this, std::placeholders::_1));
    widgetFactory()->registerWidget("AccountCombobox",    std::bind(&CSettingDialog::createAccountCombobox,       this, std::placeholders::_1));
    widgetFactory()->registerWidget("JobTypeListView",    std::bind(&CSettingDialog::createJobTypeListView,       this, std::placeholders::_1));
    widgetFactory()->registerWidget("CalDavAccountList",  std::bind(&CSettingDialog::createCalDavAccountListWidget, this, std::placeholders::_1));
    qCDebug(ClientLogger) << "Registered all custom widgets";
    QString strJson;

    CalendarSettingSettings calendarSettings;
    calendarSettings.append(setting_account);
    calendarSettings.append(setting_base);

    calendarSettings.append(setting_general);

    QJsonObject obj;
    obj.insert("groups", calendarSettings.toJson());
    strJson = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    auto settings = Dtk::Core::DSettings::fromJson(strJson.toLatin1());
    setObjectName("SettingDialog");
    updateSettings(settings);

    //恢复默认设置按钮不显示
    setResetVisible(false);
    //QList<Widget>
    QList<QWidget *> lstwidget = findChildren<QWidget *>();
    QWidget *thirdPartyTitleRow = nullptr;
    if (lstwidget.size() > 0) { //accessibleName
        for (QWidget *wid : lstwidget) {
            if ("ContentWidgetForsetting_account.third_party_accounts" == wid->accessibleName()) {
                thirdPartyTitleRow = wid;
            }
            if (wid->accessibleName().contains("DefaultWidgetAtContentRow")
                    && wid->findChild<JobTypeListView *>(QStringLiteral("JobTypeListView"))) {
                wid->layout()->setContentsMargins(0, 0, 0, 0);
            }
        }
    }

    for (QWidget *wid : lstwidget) {
        const bool isEventTypeTitle =
            wid->accessibleName() == QStringLiteral("ContentWidgetForsetting_base.event_types")
            || (wid->objectName() == QStringLiteral("ContentSubTitleText")
                && wid->property("key").toString() == QStringLiteral("setting_base.event_types"));
        if (!isEventTypeTitle || wid->layout() == nullptr
            || wid->findChild<QWidget *>(QStringLiteral("ScheduleTypeMoreButton"))) {
            continue;
        }

        QBoxLayout *titleLayout = qobject_cast<QBoxLayout *>(wid->layout());
        if (titleLayout == nullptr) {
            continue;
        }
        titleLayout->addStretch();
        DToolButton *moreButton = new DToolButton(wid);
        moreButton->setObjectName(QStringLiteral("ScheduleTypeMoreButton"));
        moreButton->setAutoRaise(true);
        moreButton->setAttribute(Qt::WA_Hover, true);
        moreButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        moreButton->setFocusPolicy(Qt::NoFocus);
        moreButton->setIcon(QIcon(QStringLiteral(
            ":/icons/deepin/builtin/icons/dde_calendar_schedule_more_16px.svg")));
        moreButton->setFixedSize(32, 32);
        moreButton->setIconSize(QSize(16, 16));
        moreButton->setToolTip(tr("More"));

        QMenu *typeMenu = new QMenu(moreButton);
        typeMenu->addAction(m_typeAddAction);
        typeMenu->addAction(m_typeImportAction);
        connect(moreButton, &DToolButton::clicked, moreButton, [moreButton, typeMenu]() {
            const QSize menuSize = typeMenu->sizeHint();
            typeMenu->popup(moreButton->mapToGlobal(
                QPoint(moreButton->width() - menuSize.width(), moreButton->height())));
        });
        titleLayout->addWidget(moreButton, 0, Qt::AlignVCenter);
        const QMargins margins = titleLayout->contentsMargins();
        titleLayout->setContentsMargins(margins.left(), 0, margins.left(), 0);
    }

    if (thirdPartyTitleRow) {
        QBoxLayout *parentLayout = qobject_cast<QBoxLayout *>(thirdPartyTitleRow->parentWidget()->layout());
        const int index = parentLayout ? parentLayout->indexOf(thirdPartyTitleRow) : -1;
        if (index >= 0) {
            QWidget *titleWidget = new QWidget(thirdPartyTitleRow->parentWidget());
            titleWidget->setSizePolicy(thirdPartyTitleRow->sizePolicy());
            QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
            QMargins titleMargins = thirdPartyTitleRow->layout()->contentsMargins();
            titleMargins.setRight(titleMargins.left());
            titleLayout->setContentsMargins(titleMargins);
            titleLayout->setSpacing(thirdPartyTitleRow->layout()->spacing());

            QLabel *titleLabel = thirdPartyTitleRow->findChild<QLabel *>();
            if (titleLabel) {
                titleLabel->setParent(titleWidget);
                titleLayout->addWidget(titleLabel);
            } else {
                titleLayout->addWidget(new QLabel(tr("Third-party accounts"), titleWidget));
            }
            titleLayout->addStretch();

            m_calDavAccountAddButton = new DIconButton(titleWidget);
            m_calDavAccountAddButton->setObjectName(QStringLiteral("CalDavAccountAddButton"));
            m_calDavAccountAddButton->setIcon(QIcon(":/icons/deepin/builtin/icons/dde_calendar_add_16px.svg"));
            m_calDavAccountAddButton->setFixedSize(36, 36);
            m_calDavAccountAddButton->setIconSize(QSize(16, 16));
            m_calDavAccountAddButton->setToolTip(tr("Add"));
            connect(m_calDavAccountAddButton, &DIconButton::clicked, this, &CSettingDialog::slotAddCalDavAccount);
            titleLayout->addWidget(m_calDavAccountAddButton);
            updateCalDavAddButtonVisibility();

            parentLayout->removeWidget(thirdPartyTitleRow);
            parentLayout->insertWidget(index, titleWidget);
            thirdPartyTitleRow->deleteLater();
        }
    }

    DBackgroundGroup *uosPlaceholder = nullptr;
    DBackgroundGroup *thirdPartyPlaceholder = nullptr;
    for (DBackgroundGroup *backgroundGroup : findChildren<DBackgroundGroup *>()) {
        const QString key = backgroundGroup->property("key").toString();
        if (key == QStringLiteral("setting_account.account")) {
            uosPlaceholder = backgroundGroup;
        } else if (key == QStringLiteral("setting_account.third_party_accounts")) {
            thirdPartyPlaceholder = backgroundGroup;
        }
    }

    if (uosPlaceholder) {
        QBoxLayout *contentLayout = qobject_cast<QBoxLayout *>(uosPlaceholder->parentWidget()->layout());
        const int index = contentLayout ? contentLayout->indexOf(uosPlaceholder) : -1;
        if (index >= 0) {
            QVBoxLayout *uosLayout = new QVBoxLayout;
            DBackgroundGroup *uosGroup = new DBackgroundGroup(uosLayout, uosPlaceholder->parentWidget());
            uosGroup->setItemSpacing(1);
            uosGroup->setItemMargins(QMargins(0, 0, 0, 0));
            uosGroup->setBackgroundRole(QPalette::Window);
            uosGroup->setUseWidgetBackground(false);
            m_uosAccountSettingsWidget = uosGroup;

            auto addUosRow = [uosGroup, uosLayout](QWidget *widget, const QString &label) {
                QWidget *row = new QWidget(uosGroup);
                QHBoxLayout *rowLayout = new QHBoxLayout(row);
                rowLayout->setContentsMargins(10, 6, 10, 6);
                rowLayout->setSpacing(0);
                if (!label.isEmpty()) {
                    rowLayout->addWidget(new QLabel(label, row));
                    rowLayout->addStretch();
                }
                rowLayout->addWidget(widget);
                uosLayout->addWidget(row);
                return row;
            };

            m_uosLoginRow = addUosRow(m_uosLoginWidget, QString());
            m_uosSyncItemsRow = addUosRow(m_uosSyncItemsWidget, tr("Sync items"));
            m_uosSyncFreqRow = addUosRow(m_syncFreqWidget, tr("Sync interval"));
            m_uosManualSyncRow = addUosRow(m_manualSyncWidget, QString());

            contentLayout->removeWidget(uosPlaceholder);
            contentLayout->insertWidget(index, uosGroup);
            uosPlaceholder->deleteLater();
        }
    }
    if (thirdPartyPlaceholder) {
        QBoxLayout *contentLayout = qobject_cast<QBoxLayout *>(thirdPartyPlaceholder->parentWidget()->layout());
        const int index = contentLayout ? contentLayout->indexOf(thirdPartyPlaceholder) : -1;
        if (index >= 0) {
            contentLayout->removeWidget(thirdPartyPlaceholder);
            contentLayout->insertWidget(index, m_calDavAccountListWidget);
            thirdPartyPlaceholder->deleteLater();
        }
    }

    updateUosAccountSettingsVisibility();

    //首次显示JobTypeListView时，更新日程类型
    if (m_scheduleTypeWidget && m_accountComboBox) {
        m_scheduleTypeWidget->updateCalendarAccount(m_accountComboBox->currentData().toString());
        setTypeEnable(m_accountComboBox->currentIndex());
    }


    //账户登出登入时，隐藏显示相关界面
    connect(gAccountManager, &AccountManager::signalAccountUpdate,
            this, &CSettingDialog::updateUosAccountSettingsVisibility);

    connect(gAccountManager, &AccountManager::signalAccountStateChange, this, &CSettingDialog::slotSyncTagButtonUpdate);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &CSettingDialog::slotSyncTagButtonUpdate);
    if (m_radiobuttonAccountCalendar)
        connect(m_radiobuttonAccountCalendar, &QCheckBox::clicked,
                this, &CSettingDialog::slotSyncAccountStateUpdate);
    if (m_radiobuttonAccountSetting)
        connect(m_radiobuttonAccountSetting, &QCheckBox::clicked,
                this, &CSettingDialog::slotSyncAccountStateUpdate);

    slotSyncTagButtonUpdate();
}

void CSettingDialog::initWidget()
{
    qCDebug(ClientLogger) << "Initializing settings dialog widgets";
    initFirstDayofWeekWidget();
    initTimeTypeWidget();
    initAccountComboBoxWidget();
    initScheduleTypeWidget();
    initTypeAddWidget();
    initSyncFreqWidget();
    initManualSyncButton();
    initUosAccountSettingsWidget();
    m_uosLoginWidget = new UserloginWidget(this);
    m_calDavAccountListWidget = new CalDavAccountListWidget(this);
    qCDebug(ClientLogger) << "Settings dialog widgets initialized";
}

void CSettingDialog::initConnect()
{
    qCDebug(ClientLogger) << "Initializing settings dialog connections";
    connect(gAccountManager, &AccountManager::signalGeneralSettingsUpdate, this, &CSettingDialog::slotGeneralSettingsUpdate);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &CSettingDialog::slotAccountUpdate);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &CSettingDialog::slotAccountStateChange);
    connect(gAccountManager, &AccountManager::signalLogout, this, &CSettingDialog::slotLogout);
    connect(gAccountManager, &AccountManager::signalAccountStateChange, this, &CSettingDialog::slotAccountStateChange);
    connect(m_firstDayofWeekCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotFirstDayofWeekCurrentChanged);
    connect(m_timeTypeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotTimeTypeCurrentChanged);
    connect(m_accountComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotAccountCurrentChanged);
    connect(m_calDavAccountListWidget, &CalDavAccountListWidget::addAccountRequested,
            this, &CSettingDialog::slotAddCalDavAccount);
    connect(m_calDavAccountListWidget, &CalDavAccountListWidget::deleteAccountRequested, this,
            [this](const QString &accountID) {
        const int index = m_accountComboBox->findData(accountID);
        if (index >= 0) {
            m_accountComboBox->setCurrentIndex(index);
            slotDeleteCalDavAccount();
        }
    });
    connect(gAccountManager, &AccountManager::signalDeleteCalDavAccountFinish,
            this, &CSettingDialog::slotDeleteCalDavAccountFinished);
    const auto refreshScheduleTypeActions = [this]() {
        if (m_accountComboBox) {
            setTypeEnable(m_accountComboBox->currentIndex());
        }
    };
    connect(gAccountManager, &AccountManager::signalCalDavAccountStatusChanged, this,
            [refreshScheduleTypeActions](const QString &) {
        refreshScheduleTypeActions();
    });
    connect(gAccountManager, &AccountManager::signalCalDavAccountStatusReady, this,
            refreshScheduleTypeActions);
    connect(m_typeAddAction, &QAction::triggered, this, &CSettingDialog::slotTypeAddBtnClickded);
    connect(m_typeImportAction, &QAction::triggered, this, &CSettingDialog::slotTypeImportBtnClickded);
    //当日常类型超过上限时，更新添加菜单项的状态
    connect(m_scheduleTypeWidget, &JobTypeListView::signalAddStatusChanged,
            this, [this](bool) {
        if (m_accountComboBox) {
            setTypeEnable(m_accountComboBox->currentIndex());
        }
    });
    //TODO:更新union帐户的的同步频率
    connect(m_syncFreqComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingDialog::slotSetUosSyncFreq);
    connect(m_syncBtn, &QPushButton::clicked, this, &CSettingDialog::slotUosManualSync);
    m_syncTimeoutTimer = new QTimer(this);
    m_syncTimeoutTimer->setSingleShot(true);
    connect(m_syncTimeoutTimer, &QTimer::timeout, this, &CSettingDialog::slotSyncTimeout);
    connect(m_ptrNetworkState, &DOANetWorkDBus::sign_NetWorkChange, this, &CSettingDialog::slotNetworkStateChange);
    qCDebug(ClientLogger) << "Settings dialog connections initialized";
}

void CSettingDialog::slotNetworkStateChange(DOANetWorkDBus::NetWorkState state)
{
    slotSyncTagButtonUpdate();
    slotAccountStateChange();

    if (m_syncBtn) {
        const bool canSync = !gUosAccountItem.isNull()
            && (gUosAccountItem->isCanSyncSetting() || gUosAccountItem->isCanSyncShedule());
        m_syncBtn->setEnabled(state == DOANetWorkDBus::Active && canSync);
    }
}

void CSettingDialog::initData()
{
    qCDebug(ClientLogger) << "Initializing settings dialog data";
    //通用设置数据初始化
    slotGeneralSettingsUpdate();
    //初始化账户信息
    slotAccountUpdate();
    //日程类型添加按钮初始化
    m_typeAddAction->setEnabled(m_scheduleTypeWidget->canAdd());
    m_typeImportAction->setEnabled(m_scheduleTypeWidget->canAdd());
    qCDebug(ClientLogger) << "Schedule type add button enabled:" << m_scheduleTypeWidget->canAdd();
    //同步频率数据初始化
    {
        int index = 0;
        if (gUosAccountItem) {
            index = m_syncFreqComboBox->findData(gUosAccountItem->getAccount()->syncFreq());
            qCDebug(ClientLogger) << "Setting sync frequency index to:" << index << "for account ID:" << gUosAccountItem->getAccount()->accountID();
        }
        m_syncFreqComboBox->setCurrentIndex(index);
    }
    slotAccountStateChange();
    qCDebug(ClientLogger) << "Settings dialog data initialized";
}

void CSettingDialog::initWidgetDisplayStatus()
{

}

void CSettingDialog::initFirstDayofWeekWidget()
{
    m_firstDayofWeekWidget = new QWidget();

    m_firstDayofWeekCombobox = new QComboBox(m_firstDayofWeekWidget);
    m_firstDayofWeekCombobox->setObjectName("FirstDayofWeekCombobox");
    m_firstDayofWeekCombobox->setAccessibleName("FirstDayofWeekCombobox");
    m_firstDayofWeekCombobox->setFixedSize(150, 36);
    m_firstDayofWeekCombobox->addItem(tr("Sunday"));
    m_firstDayofWeekCombobox->addItem(tr("Monday"));
    m_firstDayofWeekCombobox->addItem(tr("Use System Setting"));

    QHBoxLayout *layout = new QHBoxLayout(m_firstDayofWeekWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(10);
    layout->addWidget(m_firstDayofWeekCombobox, 1);

    m_firstDayofWeekWidget->setLayout(layout);
}

void CSettingDialog::initTimeTypeWidget()
{
    m_timeTypeWidget = new QWidget();

    m_timeTypeCombobox = new QComboBox(m_timeTypeWidget);
    m_timeTypeCombobox->setObjectName("TimeTypeCombobox");
    m_timeTypeCombobox->setAccessibleName("TimeTypeCombobox");
    m_timeTypeCombobox->setFixedSize(150, 36);
    m_timeTypeCombobox->addItem(tr("24-hour clock"));
    m_timeTypeCombobox->addItem(tr("12-hour clock"));
    m_timeTypeCombobox->addItem(tr("Use System Setting"));

    QHBoxLayout *layout = new QHBoxLayout(m_timeTypeWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch(10);
    layout->addWidget(m_timeTypeCombobox, 1);

    m_timeTypeWidget->setLayout(layout);
}

void CSettingDialog::initAccountComboBoxWidget()
{
    m_accountComboBox = new QComboBox();
    m_accountComboBox->setObjectName("AccountComboBox");
    m_accountComboBox->setAccessibleName("AccountComboBox");
    m_accountComboBox->setMinimumWidth(200);
    m_accountComboBox->setFixedHeight(36);
    m_accountComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

void CSettingDialog::initTypeAddWidget()
{
    m_typeAddAction = new QAction(tr("Add schedule"), this);
    m_typeImportAction = new QAction(tr("Import ICS file"), this);
    m_typeImportAction->setToolTip(tr("Import events from an ICS file"));
}

void CSettingDialog::initScheduleTypeWidget()
{
    m_scheduleTypeWidget = new JobTypeListView;
    m_scheduleTypeWidget->setObjectName("JobTypeListView");
}

void CSettingDialog::initSyncFreqWidget()
{
    m_syncFreqComboBox = new QComboBox;
    m_syncFreqComboBox->setObjectName("SyncFreqComboBox");
    m_syncFreqComboBox->setAccessibleName("SyncFreqComboBox");
    m_syncFreqComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_syncFreqComboBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_syncFreqComboBox->setFixedHeight(36);
    m_syncFreqComboBox->setMaximumWidth(150);
    m_syncFreqComboBox->addItem(tr("Manual"),   DAccount::SyncFreq_Maunal);
    m_syncFreqComboBox->addItem(tr("15 mins"),  DAccount::SyncFreq_15Mins);
    m_syncFreqComboBox->addItem(tr("30 mins"),  DAccount::SyncFreq_30Mins);
    m_syncFreqComboBox->addItem(tr("1 hour"),   DAccount::SyncFreq_1hour);
    m_syncFreqComboBox->addItem(tr("24 hours"), DAccount::SyncFreq_24hour);
    m_syncFreqComboBox->setMinimumWidth(150);
    m_syncFreqComboBox->setCurrentIndex(1); // 默认15分钟

    m_syncFreqWidget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(m_syncFreqWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_syncFreqComboBox);
}

void CSettingDialog::initManualSyncButton()
{
    m_manualSyncWidget = new QWidget;
    m_ptrNetworkState = new DOANetWorkDBus(this);
    m_manualSyncWidget->setObjectName("ManualSyncWidget");
    m_syncBtn = new QPushButton(m_manualSyncWidget);
    m_syncBtn->setObjectName("SyncBtn");
    m_syncBtn->setAccessibleName("SyncBtn");
    m_syncBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_syncBtn->setFixedHeight(36);
    m_syncBtn->setText(tr("Sync Now"));

    m_syncTimeLabel = new QLabel;
    m_syncTimeValueLabel = new DLabel;
    m_syncTimeValueLabel->setForegroundRole(Dtk::Gui::DPalette::TextTips);
    DFontSizeManager::instance()->bind(m_syncTimeValueLabel, DFontSizeManager::T8);
    m_syncStatusIconLabel = new QLabel;
    m_syncStatusIconLabel->setFixedSize(16, 16);
    m_syncStatusIconLabel->hide();

    QHBoxLayout *layout = new QHBoxLayout(m_manualSyncWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(m_syncTimeLabel);
    layout->addWidget(m_syncStatusIconLabel);
    layout->addWidget(m_syncTimeValueLabel);
    layout->addStretch();
    layout->addWidget(m_syncBtn);

}

void CSettingDialog::initUosAccountSettingsWidget()
{
    m_uosSyncItemsWidget = new QWidget;
    QHBoxLayout *syncItemsLayout = new QHBoxLayout(m_uosSyncItemsWidget);
    syncItemsLayout->setContentsMargins(0, 0, 0, 0);
    syncItemsLayout->setSpacing(16);
    m_uosSyncItemsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_uosSyncItemsWidget->setMinimumHeight(m_syncFreqComboBox->sizeHint().height());
    syncItemsLayout->addStretch();
    m_radiobuttonAccountCalendar = new QCheckBox(QObject::tr("Events"), m_uosSyncItemsWidget);
    m_radiobuttonAccountCalendar->setObjectName(QStringLiteral("Account_Calendar"));
    m_radiobuttonAccountSetting = new QCheckBox(QObject::tr("General settings"), m_uosSyncItemsWidget);
    m_radiobuttonAccountSetting->setObjectName(QStringLiteral("Account_Setting"));
    syncItemsLayout->addWidget(m_radiobuttonAccountCalendar);
    syncItemsLayout->addWidget(m_radiobuttonAccountSetting);
}

void CSettingDialog::updateUosAccountSettingsVisibility()
{
    const bool visible = !gUosAccountItem.isNull();
    if (m_uosSyncItemsRow) {
        m_uosSyncItemsRow->setVisible(visible);
    }
    if (m_uosSyncFreqRow) {
        m_uosSyncFreqRow->setVisible(visible);
    }
    if (m_uosManualSyncRow) {
        m_uosManualSyncRow->setVisible(visible);
    }
}

void CSettingDialog::slotGeneralSettingsUpdate()
{
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();
    if (!setting) {
        return;
    }
    setFirstDayofWeek(setting->firstDayOfWeek());
    setTimeType(setting->timeShowType());
}

void CSettingDialog::slotAccountUpdate()
{
    updateCalDavAddButtonVisibility();
    accountUpdate();
    updateUosAccountSettingsVisibility();
    //判断账户是否为登录状态，并建立连接
    if (gUosAccountItem) {
        slotLastSyncTimeUpdate(gUosAccountItem->getDtLastUpdate());
        connect(gUosAccountItem.get(), &AccountItem::signalDtLastUpdate,
                this, &CSettingDialog::slotLastSyncTimeUpdate, Qt::UniqueConnection);
        connect(gUosAccountItem.get(), &AccountItem::signalSyncStateChange,
                this, &CSettingDialog::slotSyncStateChange, Qt::UniqueConnection);
    }
}

void CSettingDialog::slotLogout(DAccount::Type type)
{
    if (DAccount::Account_UnionID == type) {

    }
}

void CSettingDialog::slotFirstDayofWeekCurrentChanged(int index)
{
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();

    //此次只设置一周首日，不刷新界面
    if (index == 0) {
        qCDebug(ClientLogger) << "Setting first day of week to Sunday";
        gAccountManager->setFirstDayofWeek(7);
        gCalendarManager->setFirstDayOfWeek(7, false);
    } else if (index == 1) {
        qCDebug(ClientLogger) << "Setting first day of week to Monday";
        gAccountManager->setFirstDayofWeek(1);
        gCalendarManager->setFirstDayOfWeek(1, false);
    } else {
        if (gAccountManager->getFirstDayofWeekSource() != DCalendarGeneralSettings::Source_System) {
            qCDebug(ClientLogger) << "Setting first day of week to system default";
            gAccountManager->setFirstDayofWeekSource(DCalendarGeneralSettings::Source_System);
        }
    }
}

void CSettingDialog::slotTimeTypeCurrentChanged(int index)
{
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();

    if (index == 0) {
        qCDebug(ClientLogger) << "Setting time format to 24-hour";
        gAccountManager->setTimeFormatType(DCalendarGeneralSettings::TwentyFour);
        gCalendarManager->setTimeShowType(DCalendarGeneralSettings::TwentyFour, false);
    } else if (index == 1) {
        qCDebug(ClientLogger) << "Setting time format to 12-hour";
        gAccountManager->setTimeFormatType(DCalendarGeneralSettings::Twelve);
        gCalendarManager->setTimeShowType(DCalendarGeneralSettings::Twelve, false);
    } else {
        if (gAccountManager->getTimeFormatTypeSource() != DCalendarGeneralSettings::Source_System) {
            qCDebug(ClientLogger) << "Setting time format to system default";
            gAccountManager->setTimeFormatTypeSource(DCalendarGeneralSettings::Source_System);
        }
    }
}

void CSettingDialog::slotAccountCurrentChanged(int index)
{
    qCDebug(ClientLogger) << "Account changed to index:" << index;
    if (m_scheduleTypeWidget) {
        QString accountId = m_accountComboBox->itemData(index).toString();
        qCDebug(ClientLogger) << "Updating calendar account to ID:" << accountId;
        m_scheduleTypeWidget->updateCalendarAccount(accountId);
        setTypeEnable(index);
    }
}


void CSettingDialog::slotAddCalDavAccount()
{
    DCalendarEventLog::instance().reportAddAccountClicked();
    qCDebug(ClientLogger) << "Opening CalDAV account dialog";
    CalDavAccountDialog dialog(this);
    const int result = dialog.exec();
    qCDebug(ClientLogger) << "CalDAV account dialog finished with result:" << result;
}

void CSettingDialog::slotDeleteCalDavAccount()
{
    const QString accountID = m_accountComboBox->currentData().toString();
    const AccountItem::Ptr account = gAccountManager->getAccountItemByAccountId(accountID);
    if (account.isNull() || account->getAccount()->accountType() != DAccount::Account_CalDav) {
        return;
    }

    DDialog dialog(this);
    dialog.setWindowTitle(tr("Remove Calendar Account"));
    QWidget *content = new QWidget(&dialog);
    QVBoxLayout *layout = new QVBoxLayout(content);
    const DCalDavAccountStatus status = gAccountManager->getCalDavAccountStatus(accountID);
    const QString accountName = DCalDavProviderProfile::accountDisplayName(
        static_cast<DCalDavProviderProfile::ProviderType>(status.providerType),
        account->getAccount()->accountName());
    QLabel *message = new QLabel(
        tr("Are you sure you want to remove the account \"%1\"?").arg(
            accountName.isEmpty() ? account->getAccount()->displayName() : accountName), content);
    message->setWordWrap(true);
    QCheckBox *deleteLocalData = new QCheckBox(
        tr("Also remove synced events from this calendar"), content);
    deleteLocalData->setChecked(true);
    layout->addWidget(message);
    layout->addWidget(deleteLocalData);
    dialog.addContent(content, Qt::AlignCenter);
    dialog.addButton(tr("Cancel", "button"));
    dialog.addButton(tr("Delete", "button"), false, DDialog::ButtonWarning);
    QObject::connect(dialog.getButton(0), &QAbstractButton::clicked, &dialog, &QDialog::reject);
    QObject::connect(dialog.getButton(1), &QAbstractButton::clicked, &dialog, &QDialog::accept);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const bool deleteData = deleteLocalData->isChecked();
    DCalendarEventLog::instance().reportDeleteAccount(deleteData);
    gAccountManager->deleteCalDavAccountWithLocalDataOption(accountID, deleteData);
}

void CSettingDialog::slotDeleteCalDavAccountFinished(bool success)
{
    if (!success) {
        return;
    }
    accountUpdate();
}


void CSettingDialog::slotTypeAddBtnClickded()
{
    qCDebug(ClientLogger) << "Add schedule type button clicked";
    if (m_scheduleTypeWidget) {
        m_scheduleTypeWidget->slotAddScheduleType();
    }
}

void CSettingDialog::slotTypeImportBtnClickded()
{
    qCDebug(ClientLogger) << "Import schedule type button clicked";
    if (m_scheduleTypeWidget) {
        m_scheduleTypeWidget->slotImportScheduleType();
    }
}

void CSettingDialog::slotSetUosSyncFreq(int freq)
{
    qCDebug(ClientLogger) << "Setting UOS sync frequency to:" << freq;
    QComboBox *com = qobject_cast<QComboBox *>(sender());
    if (!com || !gUosAccountItem) {
        qCDebug(ClientLogger) << "No combo box or UOS account item, skipping sync frequency update";
        return;
    }
    
    qCDebug(ClientLogger) << "Setting UOS sync frequency to:" << freq;
    gUosAccountItem->setSyncFreq(DAccount::SyncFreqType(com->itemData(freq).toInt()));
}

void CSettingDialog::slotUosManualSync()
{
    qCDebug(ClientLogger) << "Manual sync requested";
    if (!gUosAccountItem) {
        qCDebug(ClientLogger) << "No UOS account item, skipping manual sync";
        return;
    }
    qCDebug(ClientLogger) << "Manual sync requested for account:" << gUosAccountItem->getAccount()->accountID();
    if (m_syncTimeValueLabel && m_syncStatusIconLabel) {
        m_syncTimeValueLabel->setText(tr("Syncing..."));
        m_syncStatusIconLabel->setPixmap(QIcon(QStringLiteral(
            ":/icons/deepin/builtin/icons/dde_calendar_spinner_32px.svg"))
            .pixmap(16, 16));
        m_syncStatusIconLabel->show();
    }
    if (m_syncTimeoutTimer) {
        m_syncTimeoutTimer->start(10000);
    }
    gAccountManager->downloadByAccountID(gUosAccountItem->getAccount()->accountID());
}

void CSettingDialog::slotSyncTagButtonUpdate()
{
    qCDebug(ClientLogger) << "Updating sync tag buttons";
    const bool canOperate = !gUosAccountItem.isNull();
    if (!gUosAccountItem) {
        if (m_radiobuttonAccountCalendar) {
            m_radiobuttonAccountCalendar->setEnabled(false);
        }
        if (m_radiobuttonAccountSetting) {
            m_radiobuttonAccountSetting->setEnabled(false);
        }
        return;
    }

    auto state = gUosAccountItem->getAccount()->accountState();
    qCDebug(ClientLogger) << "Updating sync tag buttons for account state:" << state;
    m_radiobuttonAccountCalendar->setChecked(state & DAccount::Account_Calendar);
    m_radiobuttonAccountSetting->setChecked(state & DAccount::Account_Setting);

    m_radiobuttonAccountCalendar->setEnabled(canOperate);
    m_radiobuttonAccountSetting->setEnabled(canOperate);
}

void CSettingDialog::slotSyncAccountStateUpdate(bool status)
{
    qCDebug(ClientLogger) << "Updating sync account state";
    if (!gUosAccountItem) {
        qCDebug(ClientLogger) << "No UOS account item, skipping sync account state update";
        return;
    }

    auto state = gUosAccountItem->getAccountState();

    if (m_radiobuttonAccountSetting->isChecked())
        state = state | DAccount::Account_Setting;
    else
        state = state & ~DAccount::Account_Setting;

    if (m_radiobuttonAccountCalendar->isChecked())
        state = state | DAccount::Account_Calendar;
    else
        state = state & ~DAccount::Account_Calendar;

    gUosAccountItem->setAccountState(state);
    if (status) {
        qCDebug(ClientLogger) << "Manual sync requested";
        slotUosManualSync();
    }
}

void CSettingDialog::updateSyncStatusDisplay(const QString &datetime,
                                              DAccount::AccountSyncState state)
{
    if (!m_syncTimeLabel || !m_syncTimeValueLabel || !m_syncStatusIconLabel || !gUosAccountItem) {
        return;
    }

    QString dtstr;
    if (gCalendarManager->getTimeShowType()) {
        dtstr = dtFromString(datetime).toString("yyyy/MM/dd ap hh:mm");
    } else {
        dtstr = dtFromString(datetime).toString("yyyy/MM/dd hh:mm");
    }

    m_syncTimeLabel->setText(tr("Last sync time"));
    const bool failed = state != DAccount::Sync_Normal;
    if (!failed && dtstr.isEmpty()) {
        m_syncTimeValueLabel->clear();
        m_syncStatusIconLabel->hide();
        return;
    }

    if (failed) {
        m_syncTimeValueLabel->setText(
            QCoreApplication::translate("DCalDavSyncStatus", "Sync Failed"));
    } else {
        m_syncTimeValueLabel->setText(QStringLiteral("(%1)").arg(dtstr));
    }
    m_syncStatusIconLabel->setPixmap(QIcon(failed
        ? QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_sync_failed_32px.svg")
        : QStringLiteral(":/icons/deepin/builtin/icons/dde_calendar_sync_success_32px.svg"))
        .pixmap(16, 16));
    m_syncStatusIconLabel->show();
}

void CSettingDialog::slotLastSyncTimeUpdate(const QString &datetime)
{
    qCDebug(ClientLogger) << "Last sync time updated:" << datetime;
    const DAccount::AccountSyncState state = gUosAccountItem
        ? gUosAccountItem->getAccount()->syncState()
        : DAccount::Sync_ServerException;
    updateSyncStatusDisplay(datetime, state);
}

void CSettingDialog::slotSyncStateChange(DAccount::AccountSyncState state)
{
    if (m_syncTimeoutTimer) {
        m_syncTimeoutTimer->stop();
    }
    if (!m_syncStatusIconLabel || !m_syncTimeValueLabel || !gUosAccountItem) {
        return;
    }

    updateSyncStatusDisplay(gUosAccountItem->getDtLastUpdate(), state);
}

void CSettingDialog::slotSyncTimeout()
{
    qCWarning(ClientLogger) << "UOS account sync timed out, restoring the last known sync status";
    if (!gUosAccountItem) {
        return;
    }

    slotLastSyncTimeUpdate(gUosAccountItem->getDtLastUpdate());
}

void CSettingDialog::slotAccountStateChange()
{
    qCDebug(ClientLogger) << "Updating account state change";
    const bool networkActive = m_ptrNetworkState
        && m_ptrNetworkState->getNetWorkState() == DOANetWorkDBus::Active;
    const bool canSync = !gUosAccountItem.isNull()
        && (gUosAccountItem->isCanSyncSetting() || gUosAccountItem->isCanSyncShedule());
    const bool canOperate = networkActive && canSync;
    if (m_syncBtn) {
        m_syncBtn->setEnabled(canOperate);
    }

    if (m_syncFreqComboBox) {
        m_syncFreqComboBox->setEnabled(canOperate);
    }
    if (m_accountComboBox && m_typeAddAction) {
        qCDebug(ClientLogger) << "Updating type enable";
        setTypeEnable(m_accountComboBox->currentIndex());
    }
}

void CSettingDialog::setFirstDayofWeek(int value)
{
    qCDebug(ClientLogger) << "Setting first day of week";
    if (!m_firstDayofWeekCombobox) {
        qCDebug(ClientLogger) << "No first day of week combobox, skipping";
        return;
    }
    auto sourceSystem =
        gAccountManager->getFirstDayofWeekSource() == DCalendarGeneralSettings::Source_System;

    if (sourceSystem) {
        qCDebug(ClientLogger) << "Setting first day of week to system default";
        m_firstDayofWeekCombobox->setCurrentIndex(m_firstDayofWeekCombobox->count() - 1);
    } else {
        qCDebug(ClientLogger) << "Setting first day of week to:" << value;
        if (value == 1) {
            qCDebug(ClientLogger) << "Setting first day of week to Monday";
            m_firstDayofWeekCombobox->setCurrentIndex(1);
        } else {
            qCDebug(ClientLogger) << "Setting first day of week to Sunday";
            m_firstDayofWeekCombobox->setCurrentIndex(0);
        }
    }
    // 设置一周首日并刷新界面
    gCalendarManager->setFirstDayOfWeek(value, true);
}

void CSettingDialog::setTimeType(int value)
{
    qCDebug(ClientLogger) << "Setting time type";
    if (!m_timeTypeCombobox) {
        qCDebug(ClientLogger) << "No time type combobox, skipping";
        return;
    }
    if (value > 1 || value < 0) {
        qCDebug(ClientLogger) << "Invalid time type, setting to 0";
        value = 0;
    }
    auto sourceSystem =
        gAccountManager->getTimeFormatTypeSource() == DCalendarGeneralSettings::Source_System;
    // 设置时间显示格式并刷新界面
    if (sourceSystem) {
        qCDebug(ClientLogger) << "Setting time type to system default";
        m_timeTypeCombobox->setCurrentIndex(m_firstDayofWeekCombobox->count() - 1);
    } else {
        qCDebug(ClientLogger) << "Setting time type to:" << value;
        m_timeTypeCombobox->setCurrentIndex(value);
    }
    gCalendarManager->setTimeShowType(value, true);
}


void CSettingDialog::accountUpdate()
{
    qCDebug(ClientLogger) << "Updating account list in settings dialog";
    if (nullptr == m_accountComboBox) {
        qCWarning(ClientLogger) << "Account combo box is null, cannot update";
        return;
    }
    QVariant oldAccountID = m_accountComboBox->currentData();
    qCDebug(ClientLogger) << "Current account ID:" << oldAccountID;
    m_accountComboBox->blockSignals(true);
    m_accountComboBox->clear();
    for (auto account : gAccountManager->getAccountList()) {
        // qCDebug(ClientLogger) << "Adding account:" << account->getAccount()->accountName() << "ID:" << account->getAccount()->accountID();
        const DAccount::Ptr accountInfo = account->getAccount();
        QString label;
        if (accountInfo->accountType() == DAccount::Account_Local) {
            label = tr("Local account");
        } else if (accountInfo->accountType() == DAccount::Account_UnionID) {
            label = accountInfo->accountName();
        } else {
            label = accountInfo->displayName().isEmpty() ? accountInfo->accountName()
                                                         : accountInfo->displayName();
        }
        if (accountInfo->accountType() == DAccount::Account_CalDav) {
            const DCalDavAccountStatus status = gAccountManager->getCalDavAccountStatus(
                accountInfo->accountID());
            label = DCalDavProviderProfile::accountDisplayName(
                static_cast<DCalDavProviderProfile::ProviderType>(status.providerType), label);
        }
        m_accountComboBox->addItem(label, accountInfo->accountID());
    }
    m_accountComboBox->setCurrentIndex(m_accountComboBox->findData(oldAccountID));
    if (m_accountComboBox->currentIndex() < 0) {
        qCDebug(ClientLogger) << "Previous account not found, resetting to first account";
        m_accountComboBox->setCurrentIndex(0);
    }
    m_accountComboBox->blockSignals(false);

    m_syncFreqComboBox->setCurrentIndex(1);  //每次登录的时候 默认15分钟
    slotAccountCurrentChanged(m_accountComboBox->currentIndex());
    qCDebug(ClientLogger) << "Account list updated successfully";
}

void CSettingDialog::updateCalDavAddButtonVisibility()
{
    if (m_calDavAccountAddButton == nullptr) {
        return;
    }

    bool hasCalDavAccount = false;
    for (const AccountItem::Ptr &item : gAccountManager->getAccountList()) {
        if (!item.isNull() && item->getAccount()->accountType() == DAccount::Account_CalDav) {
            hasCalDavAccount = true;
            break;
        }
    }
    m_calDavAccountAddButton->setVisible(hasCalDavAccount);
}

void CSettingDialog::setTypeEnable(int index)
{
    qCDebug(ClientLogger) << "Setting type enable for account index:" << index;
    QString accountId = m_accountComboBox->itemData(index).toString();
    qCDebug(ClientLogger) << "Account ID:" << accountId;

    AccountItem::Ptr account = gAccountManager->getAccountItemByAccountId(accountId);
    bool canEdit = false;
    bool canExport = false;
    bool canImport = false;
    bool canManageTypes = false;
    bool isCalDavAccount = false;
    if (account) {
        const DAccount::Type accountType = account->getAccount()->accountType();
        isCalDavAccount = accountType == DAccount::Account_CalDav;
        canExport = accountType == DAccount::Account_Local
            || accountType == DAccount::Account_CalDav
            || (accountType == DAccount::Account_UnionID
                && gUosAccountItem && gUosAccountItem->isCanSyncShedule());
        canManageTypes = accountType == DAccount::Account_Local
            || (accountType == DAccount::Account_UnionID
                && gUosAccountItem && gUosAccountItem->isCanSyncShedule());
        canEdit = canManageTypes && account->getScheduleTypeList().count() < 20;
        canImport = canEdit;
    }

    if (DToolButton *moreButton = findChild<DToolButton *>(
            QStringLiteral("ScheduleTypeMoreButton"))) {
        moreButton->setEnabled(!isCalDavAccount);
    }

    // Schedule types cannot be added or imported for CalDAV accounts.
    // Existing collection/category types are not editable through the generic APIs.
    m_typeAddAction->setVisible(true);
    m_typeAddAction->setEnabled(canEdit);
    m_typeImportAction->setEnabled(canImport);
    m_scheduleTypeWidget->setItemEnabled(canExport);
    qCDebug(ClientLogger) << "Schedule type editing enabled:" << canEdit
                          << "export enabled:" << canExport;
}

QPair<QWidget *, QWidget *> CSettingDialog::createFirstDayofWeekWidget(QObject *obj)
{
    qCDebug(ClientLogger) << "Creating first day of week widget";
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_firstDayofWeekWidget);
    // 获取初始值
    option->setValue(option->defaultValue());
    return optionWidget;
}

QPair<QWidget *, QWidget *> CSettingDialog::createTimeTypeWidget(QObject *obj)
{
    qCDebug(ClientLogger) << "Creating time type widget";
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_timeTypeWidget);
    // 获取初始值
    option->setValue(option->defaultValue());
    return optionWidget;
}

QPair<QWidget *, QWidget *> CSettingDialog::createAccountCombobox(QObject *obj)
{
    qCDebug(ClientLogger) << "Creating account combobox";
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_accountComboBox);
    return optionWidget;
}

QPair<QWidget *, QWidget *> CSettingDialog::createSyncItemsWidget(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    return DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_uosSyncItemsWidget);
}

QPair<QWidget *, QWidget *> CSettingDialog::createSyncFreqCombobox(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    return DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_syncFreqWidget);
}

QPair<QWidget *, QWidget *> CSettingDialog::createManualSyncButton(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    return DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_manualSyncWidget);
}

QWidget *CSettingDialog::createCalDavAccountListWidget(QObject *)
{
    return m_calDavAccountListWidget;
}

QWidget *CSettingDialog::createJobTypeListView(QObject *)
{
    // qCDebug(ClientLogger) << "Creating job type list view";
    return m_scheduleTypeWidget;
}

QWidget *CSettingDialog::createControlCenterLink(QObject *obj)
{
    qCDebug(ClientLogger) << "Creating control center link";
    DLabel *myLabel = new DLabel(tr("Please go to the <a href='/'>Control Center</a> to change system settings"), this);
    myLabel->setTextFormat(Qt::RichText);
    myLabel->setFixedHeight(36);
    connect(myLabel, &DLabel::linkActivated, this, [this]{
        qCDebug(ClientLogger) << "Opening control center";
        QString datePage = ControlCenterPage;
        auto ver = DSysInfo::majorVersion().toInt();
        if (ver > 23) {
            // v25 control center has changed the datetime page.
            datePage = "system/datetime";
            qCDebug(ClientLogger) << "Using v25+ control center page:" << datePage;
        }
        this->m_controlCenterProxy->ShowPage(datePage);
    });
    auto w = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0, 0, 10, 0);
    layout->setSpacing(0);
    layout->addStretch(10);
    layout->addWidget(myLabel, 1);

    w->setLayout(layout);
    return w;
}
