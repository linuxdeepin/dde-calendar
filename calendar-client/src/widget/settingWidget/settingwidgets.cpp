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
#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <qglobal.h>

DWIDGET_USE_NAMESPACE

SettingWidgets::SettingWidgets(QObject *parent) : QObject(parent)
{
    initConnect();
}

void SettingWidgets::init()
{
    getInstance()->initWidget();
}

void SettingWidgets::initWidget()
{
    //一周首日
    m_firstDayofWeekWidget = nullptr;
    m_firstDayofWeekCombobox = nullptr;

    //时间格式
    m_timeTypeWidget = nullptr;
    m_timeTypeCombobox = nullptr;
}

SettingWidgets* SettingWidgets::getInstance()
{
    static SettingWidgets setting;
    return &setting;
}

void SettingWidgets::initConnect()
{
    connect(gAccountManager, &AccountManager::signalGeneralSettingsUpdate, this, &SettingWidgets::slotGeneralSettingsUpdate);
}

void SettingWidgets::slotGeneralSettingsUpdate(){
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();
    setFirstDayofWeek(setting->firstDayOfWeek());
    setTimeType(setting->timeShowType());
}

QWidget* SettingWidgets::getFirstDayofWeekWidget()
{
    m_firstDayofWeekWidget = new QWidget();
    m_firstDayofWeekCombobox = new QComboBox(m_firstDayofWeekWidget);
    QHBoxLayout* layout = new QHBoxLayout(m_firstDayofWeekWidget);
    m_firstDayofWeekCombobox->addItem(tr("Sunday"));
    m_firstDayofWeekCombobox->addItem(tr("Monday"));

    m_firstDayofWeekCombobox->setFixedSize(150,36);
    layout->setAlignment(Qt::AlignRight);
    layout->addWidget(m_firstDayofWeekCombobox);

    slotGeneralSettingsUpdate();
    connect(m_firstDayofWeekCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWidgets::slotFirstDayofWeekCurrentChanged);

    return m_firstDayofWeekWidget;
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

QPair<QWidget*, QWidget*> SettingWidgets::createFirstDayofWeekWidget(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QWidget *widget = getInstance()->getFirstDayofWeekWidget();
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);
    // 获取初始值
    option->setValue(option->defaultValue());
    widget->deleteLater();
    widget = nullptr;
    return optionWidget;
}

QWidget* SettingWidgets::getTimeTypeWidget()
{
    m_timeTypeWidget = new QWidget();
    m_timeTypeCombobox = new QComboBox(m_timeTypeWidget);
    QHBoxLayout* layout = new QHBoxLayout(m_timeTypeWidget);
    m_timeTypeCombobox->addItem(tr("24-hour clock"));
    m_timeTypeCombobox->addItem(tr("12-hour clock"));

    m_timeTypeCombobox->setFixedSize(150,36);
    layout->setAlignment(Qt::AlignRight);
    layout->addWidget(m_timeTypeCombobox);

    slotGeneralSettingsUpdate();
    connect(m_timeTypeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingWidgets::slotTimeTypeCurrentChanged);

    return m_timeTypeWidget;
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
    gCalendarManager->setTimeFormatChanged(value, true);
}

void SettingWidgets::slotTimeTypeCurrentChanged(int index)
{
    gAccountManager->setTimeFormatType(index);
    //设置时间显示格式不刷新界面
    gCalendarManager->setTimeFormatChanged(index, false);
}

QPair<QWidget*, QWidget*> SettingWidgets::createTimeTypeWidget(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QWidget *widget = getInstance()->getTimeTypeWidget();
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, widget);
    // 获取初始值
    option->setValue(option->defaultValue());
    widget->deleteLater();
    widget = nullptr;
    return optionWidget;
}
