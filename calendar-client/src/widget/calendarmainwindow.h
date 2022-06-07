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

#ifndef CALENDARMAINWINDOW_H
#define CALENDARMAINWINDOW_H

#include "dschedule.h"
#include "jobtypelistview.h"

#include <DLabel>
#include <DButtonBox>
#include <DFrame>
#include <DMainWindow>
#include <DSearchEdit>
#include <DToolButton>
#include <DMenu>
#include <DSettingsDialog>
#include <DSysInfo>

#include <QStackedLayout>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

DWIDGET_USE_NAMESPACE
class CYearWindow;
class CMonthWindow;
class CWeekWindow;
class CDayWindow;
class SidebarView;
class CScheduleSearchView;
class AnimationStackedWidget;
class CScheduleDataManage;
class CTitleWidget;

namespace  CalendarSetting{
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
public:
    SyncTagRadioButton(DAccount::AccountState type, QWidget *parent = nullptr);

    void updateState();
    void updateState(bool isOnline);

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

using namespace CalendarSetting;

class Calendarmainwindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit Calendarmainwindow(int index, QWidget *w = nullptr);
    ~Calendarmainwindow() override;
    void viewWindow(int type, const bool showAnimation = false);
    void updateHeight();
    void setSearchWidth(int w);
public slots:
    void slotTheme(int type);
    void slotOpenSchedule(QString job);
    void slotViewShortcut();
    //更新当前时间
    void slotCurrentDateUpdate();
    void slotSetSearchFocus();
private:
    void initUI();
    void initConnection();
    void initData();
    //创建视图
    void createview();
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
private slots:
    //获取程序状态
    void slotapplicationStateChanged(Qt::ApplicationState state);
    //隐藏提示框
    void setScheduleHide();
    //点击按钮切换视图
    void slotstackWClicked(QAbstractButton *bt);
    void slotWUpdateSchedule();
    void slotSreturnPressed();
    void slotStextChanged();
    //搜索框有焦点时隐藏提示框
    void slotStextfocusChanged(bool onFocus);
    void slotSearchEdit();
    //单击搜索日程动画设置
    void slotSearchSelectSchedule(const DSchedule::Ptr &scheduleInfo);
    //添加视图阴影
    void slotViewtransparentFrame(const bool isShow);
    //启用buttonbox
    void slotSetButtonBox();
    //切换视图
    void slotSwitchView(const int viewIndex);
    //新建日程
    void slotNewSchedule();
    void slotOpenSettingDialog();
    //快捷键删除选中日程
    void slotDeleteitem();
    //设置最大大小
    void slotSetMaxSize(int size = 0);
    void slotSearchFocusSwitch();
    //账户侧边栏状态发生改变事件
    void slotSidebarStatusChange(bool);
    //拖拽释放事件
    void dropEvent(QDropEvent *event) override;
    //拖拽进入事件
    void dragEnterEvent(QDragEnterEvent *event) override;

private:
    DLabel *m_icon = nullptr;
    AnimationStackedWidget *m_stackWidget = nullptr;
    DButtonBox *m_buttonBox = nullptr;
    DSearchEdit *m_searchEdit = nullptr;
    CYearWindow *m_yearwindow = nullptr;
    CMonthWindow *m_monthWindow = nullptr;
    CWeekWindow *m_weekWindow = nullptr;
    CDayWindow *m_DayWindow = nullptr;
    CTitleWidget *m_titleWidget = nullptr;
    SidebarView *m_sidebarView = nullptr;
    bool m_searchflag = false;
    CScheduleSearchView *m_scheduleSearchView = nullptr;
    DFrame *m_contentBackground = nullptr;
    int m_priindex = 3; //默认打开日视图双击
    DFrame *m_transparentFrame = nullptr;
    bool m_opensearchflag = false;
    int m_scheduleSearchViewMaxWidth;
    QPropertyAnimation *m_animation = nullptr;
    QTimer *m_currentDateUpdateTimer = nullptr;
    DIconButton *m_newScheduleBtn {nullptr}; //全局的新建日程按钮
    DSettingsDialog *m_dsdSetting {nullptr};
    JobTypeListView *m_jobTypeListView {nullptr};
    //日历打开默认显示视图
    int m_defaultIndex;
    bool m_setButtonFocus {false};
    bool m_isNormalStateShow {true}; //是否为正常状态显示
    QPoint m_startPos;

    //设置界面相关handler函数
    QPair<QWidget*, QWidget*> createAccountCombobox(QObject *obj);
    QWidget *createJobTypeListView(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncFreqCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncTagRadioButton(QObject *obj);
    QWidget *createManualSyncButton(QObject *obj);
    QPair<QWidget*, QWidget*> createComboboxFirstDayofWeek(QObject *obj);
    QPair<QWidget*, QWidget*> createComboboxTime(QObject *obj);

private slots:
    void slotSetUosSyncFreq(int freq);
    void slotUosManualSync();

signals:
    void signal_calendarAccountChanged(QString account_id);
    void signal_addScheduleType();
};

#endif // CALENDARMAINWINDOW_H
