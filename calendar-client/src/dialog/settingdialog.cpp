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
#include "settingdialog.h"
#include "cdynamicicon.h"
#include "accountmanager.h"
#include "settingWidget/userloginwidget.h"

#include <DSysInfo>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DBackgroundGroup>

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

CSettingDialog::CSettingDialog(QWidget *parent) : DSettingsDialog(parent)
{
    initView();
}

void CSettingDialog::initView()
{
    m_settingWidgets = new SettingWidgets(this);
    setIcon(CDynamicIcon::getInstance()->getPixmap());
    setFixedSize(682, 506);
    widgetFactory()->registerWidget("login", UserloginWidget::createloginButton);
    widgetFactory()->registerWidget("FirstDayofWeek",     std::bind(&SettingWidgets::createFirstDayofWeekWidget,       m_settingWidgets, std::placeholders::_1));
    widgetFactory()->registerWidget("Time",               std::bind(&SettingWidgets::createTimeTypeWidget,       m_settingWidgets, std::placeholders::_1));
    widgetFactory()->registerWidget("AccountCombobox",    std::bind(&SettingWidgets::createAccountCombobox,       m_settingWidgets, std::placeholders::_1));
    widgetFactory()->registerWidget("JobTypeListView",    std::bind(&SettingWidgets::createJobTypeListView,       m_settingWidgets, std::placeholders::_1));
    widgetFactory()->registerWidget("SyncTagRadioButton", std::bind(&SettingWidgets::createSyncTagRadioButton,    m_settingWidgets, std::placeholders::_1));
    widgetFactory()->registerWidget("SyncTimeCombobox",   std::bind(&SettingWidgets::createSyncFreqCombobox,      m_settingWidgets, std::placeholders::_1));
    widgetFactory()->registerWidget("ManualSyncButton",   std::bind(&SettingWidgets::createManualSyncButton,      m_settingWidgets, std::placeholders::_1));
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
    setObjectName("SettingDialog");
    updateSettings(settings);
    //恢复默认设置按钮不显示
    setResetVisible(false);
    //QList<Widget>
    QList<QWidget *> lstwidget = findChildren<QWidget *>();
    if (lstwidget.size() > 0) { //accessibleName
        for (QWidget *wid : lstwidget) {
            if ("ContentWidgetForsetting_base.event_types" == wid->accessibleName()) {
                JobTypeListView *view = findChild<JobTypeListView *>("JobTypeListView");
                if (!view)
                    return;
                DIconButton *addButton = m_settingWidgets->createTypeAddButton();
                wid->layout()->addWidget(addButton);
                //使addButton的右边距等于view的右边距
                int leftMargin = wid->layout()->contentsMargins().left();
                wid->layout()->setContentsMargins(leftMargin, 0, leftMargin, 0);
            }
            if (wid->accessibleName().contains("DefaultWidgetAtContentRow")) {
                //DefaultWidgetAtContentRow是设置对话框右边每一个option条目对应widget的accessibleName的前缀，所以如果后续有更多条目，需要做修改
                wid->layout()->setMargin(0);
            }
        }
    }
}
