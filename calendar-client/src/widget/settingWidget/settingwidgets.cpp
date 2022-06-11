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
#include <qglobal.h>

DWIDGET_USE_NAMESPACE

SettingWidgets::SettingWidgets(QObject *parent) : QObject(parent)
{
    initWidget();
    initConnect();
    initData();
}

void SettingWidgets::clear()
{
    clearWidget();
}

void SettingWidgets::clearWidget()
{
    //一周首日
    m_firstDayofWeekWidget = nullptr;
    m_firstDayofWeekCombobox = nullptr;

    //时间格式
    m_timeTypeWidget = nullptr;
    m_timeTypeCombobox = nullptr;

    m_accountComboBox = nullptr;
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
    slotGeneralSettingsUpdate();
    accountUpdate();
    m_typeAddBtn->setEnabled(m_scheduleTypeWidget->canAdd());
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

    int index = -1;
    if (gUosAccountItem)
        index = m_syncFreqComboBox->findData(gUosAccountItem->getAccount()->syncFreq());
    m_syncFreqComboBox->setCurrentIndex(index);
}

void SettingWidgets::initManualSyncButton()
{
    m_manualSyncBtn = new QWidget;
    m_manualSyncBtn->setObjectName("ManualSyncWidget");
    QPushButton *button = new QPushButton(m_manualSyncBtn);
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
    m_manualSyncBtn->setLayout(layout);
    connect(button, &QPushButton::clicked, this, &SettingWidgets::slotUosManualSync);
    //TODO: 更新时间
    if(gUosAccountItem) {
        updateLastUpdateText(gUosAccountItem->getDtLastUpdate());
        connect(gUosAccountItem.get(), &AccountItem::signalDtLastUpdate, this, updateLastUpdateText);
    }
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
