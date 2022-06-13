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
#ifndef SETTINGWIDGETS_H
#define SETTINGWIDGETS_H

#include "jobtypelistview.h"

#include <DIconButton>

#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

DWIDGET_USE_NAMESPACE


namespace  SettingWidget{
struct CalendarSettingOption{
    CalendarSettingOption(){}
    CalendarSettingOption(const std::initializer_list<QString> &list){
        int index = 0;
        for(auto d : list) {
            if(0 == index) _key     = d;
            if(1 == index) _name    = d;
            if(2 == index) _type    = d;
            if(3 == index) _default = d;
            if(4 == index) _text    = d;
            index ++;
        }
    }

    QJsonObject toJson(){
        QJsonObject obj;
        obj.insert("key",      _key);
        obj.insert("type",     _type);
        obj.insert("name",     _name);
        obj.insert("default",  _default);
        obj.insert("text",     _text);
        return obj;
    }

    QString _key;
    QString _type;
    QString _name;
    QString _default;
    QString _text;
};
struct CalendarSettingOptions : public QList<CalendarSettingOption>{
    CalendarSettingOptions(){}
    CalendarSettingOptions(const std::initializer_list<CalendarSettingOption> &list){
        for(auto d : list)
            append(d);
    }
    QJsonArray tojson()
    {
        QJsonArray arr;
        for(auto option: *this) {
            arr.append(option.toJson());
        }
        return arr;
    }
};

struct CalendarSettingGroup{
    QString _key;
    QString _name;
    CalendarSettingOptions _options;
    QJsonObject toJson(){
        QJsonObject obj;
        obj.insert("key",      _key);
        obj.insert("name",     _name);
        obj.insert("options",  _options.tojson());
        return obj;
    }
};

struct CalendarSettingGroups : public QList<CalendarSettingGroup>
{
    CalendarSettingGroups(){}
    CalendarSettingGroups(const std::initializer_list<CalendarSettingGroup> &list){
        for(auto d : list)
            append(d);
    }
    QJsonArray toJson()
    {
        QJsonArray arr;
        for(auto group: *this) {
            arr.append(group.toJson());
        }
        return arr;
    }
};

struct CalendarSettingSetting {
    QString _key;
    QString _name;
    CalendarSettingGroups _groups;
    QJsonObject toJson() {

        QJsonObject obj;
        obj.insert("key",      _key);
        obj.insert("name",     _name);
        obj.insert("groups",   _groups.toJson());
        return obj;
    }
};

struct CalendarSettingSettings : public QList<CalendarSettingSetting>
{
    CalendarSettingSettings(){}
    CalendarSettingSettings(const std::initializer_list<CalendarSettingSetting> &list){
        for(auto d : list)
            append(d);
    }
    QJsonArray toJson()
    {
        QJsonArray arr;
        for(auto setting: *this) {
            arr.append(setting.toJson());
        }
        return arr;
    }

    void removeGroup(const QString &groupName, const QString &groupName2);
    void removeGroup(const QString &groupName);
    int indexOf(const CalendarSettingGroups &groups, const QString groupName);
    int indexOf(const CalendarSettingSettings &groups, const QString groupName);
};


/**
 * @brief The SyncTagRadioButton class 实现model/checkstate效果的类
 */
class SyncTagRadioButton : public QWidget
{
    Q_OBJECT
public:
    SyncTagRadioButton(DAccount::AccountState type, QWidget *parent = nullptr);

    void updateAccountState();
    void updateOnLineState(bool isOnline);

    bool isChecked();
    DAccount::AccountState type();

private:
    /**
     * @brief setChecked 是否选中
     */
    void setChecked(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_checked = true;
    DAccount::AccountStates m_state;
    bool m_isOnline = true;
    const DAccount::AccountState m_type;
};
}//~CalendarSetting

using namespace SettingWidget;

class SettingWidgets : public QObject
{
    Q_OBJECT
public:

    explicit SettingWidgets(QObject *parent = nullptr);

    QPair<QWidget*, QWidget*> createFirstDayofWeekWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createTimeTypeWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createAccountCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncFreqCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncTagRadioButton(QObject *obj);
    QWidget *createManualSyncButton(QObject *obj);
    QWidget *createJobTypeListView(QObject *obj);
    DIconButton *createTypeAddButton();

signals:

public slots:
    void slotGeneralSettingsUpdate();
    void slotAccountUpdate();
    void slotLogout(DAccount::Type);
    void slotLastSyncTimeUpdate(const QString &datetime);

    void slotFirstDayofWeekCurrentChanged(int index);
    void slotTimeTypeCurrentChanged(int index);
    void slotAccountCurrentChanged(int index);
    void slotTypeAddBtnClickded();
    void slotSetUosSyncFreq(int freq);
    void slotUosManualSync();

protected:
    void initFirstDayofWeekWidget();
    void initTimeTypeWidget();
    void initAccountComboBoxWidget();
    void initTypeAddWidget();
    void initScheduleTypeWidget();
    void initSyncFreqWidget();
    void initManualSyncButton();

    void setFirstDayofWeek(int value);
    void setTimeType(int value);
    void accountUpdate();

private:
    void initWidget();
    void initConnect();
    void initData();

    void initWidgetDisplayStatus();

private:
    //一周首日
    QWidget *m_firstDayofWeekWidget = nullptr;
    QComboBox *m_firstDayofWeekCombobox = nullptr;

    //时间格式
    QWidget *m_timeTypeWidget = nullptr;
    QComboBox *m_timeTypeCombobox = nullptr;

    //帐户选择
    QComboBox *m_accountComboBox = nullptr;
    //同步频率
    QComboBox *m_syncFreqComboBox = nullptr;

    DIconButton *m_typeAddBtn = nullptr;

    JobTypeListView *m_scheduleTypeWidget = nullptr;

    //手动同步按钮和同步时间显示
    QLabel *m_syncTimeLabel = nullptr;
    QWidget *m_manualSyncBtn = nullptr;

};

#endif // SETTINGWIDGETS_H
