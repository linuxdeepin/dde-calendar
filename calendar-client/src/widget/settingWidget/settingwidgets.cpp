/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#include "settingwidgets.h"
#include "accountmanager.h"
#include "calendarmanage.h"
#include "units.h"
#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <DHiDPIHelper>
#include <qglobal.h>

DWIDGET_USE_NAMESPACE

SettingWidgets::SettingWidgets(QObject *parent) : QObject(parent)
{
    initWidget();
    initConnect();
    initData();
    initWidgetDisplayStatus();
}

void SettingWidgets::initWidget()
{
    initFirstDayofWeekWidget();
    initTimeTypeWidget();
    initAccountComboBoxWidget();
    initScheduleTypeWidget();
    initTypeAddWidget();
    initSyncFreqWidget();
    initManualSyncButton();
}

void SettingWidgets::initConnect()
{
    connect(gAccountManager, &AccountManager::signalGeneralSettingsUpdate, this, &SettingWidgets::slotGeneralSettingsUpdate);
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &SettingWidgets::slotAccountUpdate);
    connect(gAccountManager, &AccountManager::signalLogout, this, &SettingWidgets::slotLogout);
    connect(m_firstDayofWeekCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWidgets::slotFirstDayofWeekCurrentChanged);
    connect(m_timeTypeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWidgets::slotTimeTypeCurrentChanged);
    connect(m_accountComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWidgets::slotAccountCurrentChanged);
    connect(m_typeAddBtn, &DIconButton::clicked, this, &SettingWidgets::slotTypeAddBtnClickded);
    //当日常类型超过上限时，更新button的状态
    connect(m_scheduleTypeWidget, &JobTypeListView::signalAddStatusChanged, m_typeAddBtn, &DIconButton::setEnabled);
    //TODO:更新union帐户的的同步频率
    connect(m_syncFreqComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWidgets::slotSetUosSyncFreq);
}

void SettingWidgets::initData()
{
    //通用设置数据初始化
    slotGeneralSettingsUpdate();
    //初始化账户信息
    slotAccountUpdate();
    //日程类型添加按钮初始化
    m_typeAddBtn->setEnabled(m_scheduleTypeWidget->canAdd());
    //同步频率数据初始化
    {
        int index = 0;
        if (gUosAccountItem) {
            index = m_syncFreqComboBox->findData(gUosAccountItem->getAccount()->syncFreq());
        }
        m_syncFreqComboBox->setCurrentIndex(index);
    }
}

void SettingWidgets::initWidgetDisplayStatus()
{

}

void SettingWidgets::initFirstDayofWeekWidget()
{
    m_firstDayofWeekWidget = new QWidget();
    m_firstDayofWeekCombobox = new QComboBox(m_firstDayofWeekWidget);
    QHBoxLayout* layout = new QHBoxLayout(m_firstDayofWeekWidget);
    m_firstDayofWeekCombobox->addItem(tr("Sunday"));
    m_firstDayofWeekCombobox->addItem(tr("Monday"));

    m_firstDayofWeekCombobox->setFixedSize(150,36);
    layout->setAlignment(Qt::AlignRight);
    layout->addWidget(m_firstDayofWeekCombobox);
    m_firstDayofWeekWidget->setLayout(layout);
}

void SettingWidgets::initTimeTypeWidget()
{
    m_timeTypeWidget = new QWidget();
    m_timeTypeCombobox = new QComboBox(m_timeTypeWidget);
    QHBoxLayout* layout = new QHBoxLayout(m_timeTypeWidget);
    m_timeTypeCombobox->addItem(tr("24-hour clock"));
    m_timeTypeCombobox->addItem(tr("12-hour clock"));

    m_timeTypeCombobox->setFixedSize(150,36);
    layout->setAlignment(Qt::AlignRight);
    layout->addWidget(m_timeTypeCombobox);
    m_timeTypeWidget->setLayout(layout);
}

void SettingWidgets::initAccountComboBoxWidget()
{
    m_accountComboBox = new QComboBox();
    m_accountComboBox->setFixedSize(150, 36);
}

void SettingWidgets::initTypeAddWidget()
{
    m_typeAddBtn = new DIconButton(DStyle::SP_IncreaseElement, nullptr);
    m_typeAddBtn->setFixedSize(20, 20);
}

void SettingWidgets::initScheduleTypeWidget()
{
    m_scheduleTypeWidget = new JobTypeListView;
    m_scheduleTypeWidget->setObjectName("JobTypeListView");
}

void SettingWidgets::initSyncFreqWidget()
{
    m_syncFreqComboBox = new QComboBox;
    m_syncFreqComboBox->setMaximumWidth(150);
    m_syncFreqComboBox->addItem(tr("Manual"),   DAccount::SyncFreq_Maunal);
    m_syncFreqComboBox->addItem(tr("15 mins"),  DAccount::SyncFreq_15Mins);
    m_syncFreqComboBox->addItem(tr("30 mins"),  DAccount::SyncFreq_30Mins);
    m_syncFreqComboBox->addItem(tr("1 hour"),   DAccount::SyncFreq_1hour);
    m_syncFreqComboBox->addItem(tr("24 hours"), DAccount::SyncFreq_24hour);
}

void SettingWidgets::initManualSyncButton()
{
    m_manualSyncBtn = new QWidget;
    m_manualSyncBtn->setObjectName("ManualSyncWidget");
    QPushButton *button = new QPushButton(m_manualSyncBtn);
    button->setFixedSize(266, 36);
    button->setText(tr("Sync Now"));

    m_syncTimeLabel = new QLabel;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(button, 0, Qt::AlignCenter);
    layout->addWidget(m_syncTimeLabel, 0, Qt::AlignCenter);
    m_manualSyncBtn->setLayout(layout);
    connect(button, &QPushButton::clicked, this, &SettingWidgets::slotUosManualSync);
}

void SettingWidgets::slotGeneralSettingsUpdate(){
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();
    if (!setting) {
        return;
    }
    setFirstDayofWeek(setting->firstDayOfWeek());
    setTimeType(setting->timeShowType());
}

void SettingWidgets::slotAccountUpdate()
{
    accountUpdate();
    //判断账户是否为登录状态，并建立连接
    if(gUosAccountItem) {
        slotLastSyncTimeUpdate(gUosAccountItem->getDtLastUpdate());
        connect(gUosAccountItem.get(), &AccountItem::signalDtLastUpdate, this, &SettingWidgets::slotLastSyncTimeUpdate);
    }
}

void SettingWidgets::slotLogout(DAccount::Type type)
{
    if (DAccount::Account_UnionID == type) {

    }
}

void SettingWidgets::slotFirstDayofWeekCurrentChanged(int index)
{
    //此次只设置一周首日，不刷新界面
    if (index == 0) {
        gAccountManager->setFirstDayofWeek(7);
        gCalendarManager->setFirstDayOfWeek(7, false);
    } else if (index == 1) {
        gAccountManager->setFirstDayofWeek(1);
        gCalendarManager->setFirstDayOfWeek(1, false);
    }
}

void SettingWidgets::slotTimeTypeCurrentChanged(int index)
{
    gAccountManager->setTimeFormatType(index);
    //设置时间显示格式不刷新界面
    gCalendarManager->setTimeShowType(index, false);
}

void SettingWidgets::slotAccountCurrentChanged(int index)
{
    if (m_scheduleTypeWidget) {
        m_scheduleTypeWidget->updateCalendarAccount(m_accountComboBox->itemData(index).toString());
    }
}

void SettingWidgets::slotTypeAddBtnClickded()
{
    if (m_scheduleTypeWidget) {
        m_scheduleTypeWidget->slotAddScheduleType();
    }
}

void SettingWidgets::slotSetUosSyncFreq(int freq)
{
    QComboBox *com = qobject_cast<QComboBox *>(sender());
    if (!com)
        return;
    if (!gUosAccountItem)
        return;
    gUosAccountItem->setSyncFreq(DAccount::SyncFreqType(com->itemData(freq).toInt()));
}

void SettingWidgets::slotUosManualSync()
{
    if (!gUosAccountItem)
        return;
    gAccountManager->downloadByAccountID(gUosAccountItem->getAccount()->accountID());
}

void SettingWidgets::slotLastSyncTimeUpdate(const QString &datetime)
{
    if(m_syncTimeLabel && gUosAccountItem) {
        m_syncTimeLabel->setText(tr("Last sync") + ":" + dtFromString(datetime).toString("yyyy/MM/dd hh:mm"));
    }
}

void SettingWidgets::setFirstDayofWeek(int value)
{
    if (!m_firstDayofWeekCombobox)
    {
        return;
    }
    //设置一周首日并刷新界面
    if (value == 1) {
        m_firstDayofWeekCombobox->setCurrentIndex(1);
        gCalendarManager->setFirstDayOfWeek(1, true);
    } else {
        m_firstDayofWeekCombobox->setCurrentIndex(0);
        gCalendarManager->setFirstDayOfWeek(7, true);
    }
}


void SettingWidgets::setTimeType(int value)
{
    if (!m_timeTypeCombobox) {
        return;
    }
    if (value > 1 || value < 0) {
        value = 0;
    }
    //设置时间显示格式并刷新界面
    m_timeTypeCombobox->setCurrentIndex(value);
    gCalendarManager->setTimeShowType(value, true);
}

void SettingWidgets::accountUpdate()
{
    if (nullptr == m_accountComboBox) {
        return;
    }
    QVariant oldAccountID = m_accountComboBox->currentData();
    m_accountComboBox->blockSignals(true);
    m_accountComboBox->clear();
    for (auto account : gAccountManager->getAccountList()) {
        m_accountComboBox->addItem(account->getAccount()->accountName(), account->getAccount()->accountID());
    }
    m_accountComboBox->setCurrentIndex(m_accountComboBox->findData(oldAccountID));
    if (m_accountComboBox->currentIndex() < 0)
        m_accountComboBox->setCurrentIndex(0);
    m_accountComboBox->blockSignals(false);
    slotAccountCurrentChanged(m_accountComboBox->currentIndex());
}

QPair<QWidget*, QWidget*> SettingWidgets::createFirstDayofWeekWidget(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_firstDayofWeekWidget);
    // 获取初始值
    option->setValue(option->defaultValue());
    return optionWidget;
}

QPair<QWidget*, QWidget*> SettingWidgets::createTimeTypeWidget(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_timeTypeWidget);
    // 获取初始值
    option->setValue(option->defaultValue());
    return optionWidget;
}

QPair<QWidget *, QWidget *> SettingWidgets::createAccountCombobox(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_accountComboBox);
    return optionWidget;
}

QPair<QWidget*, QWidget*> SettingWidgets::createSyncFreqCombobox(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, m_syncFreqComboBox);
    return optionWidget;
}

QPair<QWidget*, QWidget*> SettingWidgets::createSyncTagRadioButton(QObject *obj)
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

QWidget *SettingWidgets::createManualSyncButton(QObject *obj)
{
    return m_manualSyncBtn;
}

QWidget *SettingWidgets::createJobTypeListView(QObject *)
{
    return m_scheduleTypeWidget;
}

DIconButton *SettingWidgets::createTypeAddButton()
{
    return m_typeAddBtn;
}

SettingWidget::SyncTagRadioButton::SyncTagRadioButton(DAccount::AccountState type, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
{

    setObjectName("SyncTagRadioButton");
    if (gUosAccountItem) {
        m_state = DAccount::Account_Close;
        m_state = gUosAccountItem->getAccount()->accountState();
        connect(gUosAccountItem.get(), &AccountItem::signalAccountStateChange, this, &SyncTagRadioButton::updateAccountState);
        updateAccountState();
    }
}

void SettingWidget::SyncTagRadioButton::updateAccountState()
{
    if (!gUosAccountItem) {
        return;
    }
    m_state = gUosAccountItem->getAccount()->accountState();
    setChecked(m_state & m_type);
    //TODO:是否联网
    //setEnabled((m_state & DAccount::Account_Open) && m_isOnline);
}

void SettingWidget::SyncTagRadioButton::updateOnLineState(bool isOnline)
{
    m_isOnline = isOnline;

    updateAccountState();
}

bool SettingWidget::SyncTagRadioButton::isChecked()
{
    return m_checked;
}

DAccount::AccountState SettingWidget::SyncTagRadioButton::type()
{
    return m_type;
}

void SettingWidget::SyncTagRadioButton::setChecked(bool checked)
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

void SettingWidget::SyncTagRadioButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QIcon icon = DStyle::standardIcon(this->style(), m_checked ? DStyle::SP_IndicatorChecked : DStyle::SP_IndicatorUnchecked);
    int y = (this->height() - 16) / 2;
    int x = (this->width() - 16) / 2;
    icon.paint(&painter, QRect(x, y, 16, 16), Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

void SettingWidget::SyncTagRadioButton::mouseReleaseEvent(QMouseEvent *event)
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
