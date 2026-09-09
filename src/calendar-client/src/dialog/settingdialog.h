// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "settingWidget/settingwidgets.h"
#include "doanetworkdbus.h"
#include "controlCenterProxy.h"
#include "caldavaccountlistwidget.h"
#include <DSettingsDialog>
#include <DIconButton>
#include <DCommandLinkButton>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class QAction;
class QCheckBox;
class QPushButton;
class UserloginWidget;
class QTimer;

class CSettingDialog : public DSettingsDialog
{
    Q_OBJECT
public:
    explicit CSettingDialog(QWidget *parent = nullptr);

private:
    QPair<QWidget*, QWidget*> createFirstDayofWeekWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createTimeTypeWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createAccountCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncItemsWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncFreqCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createManualSyncButton(QObject *obj);
    QWidget *createCalDavAccountListWidget(QObject *obj);
    QWidget *createJobTypeListView(QObject *obj);
    QWidget *createControlCenterLink(QObject *obj);

public slots:
    void slotGeneralSettingsUpdate();
    void slotAccountUpdate();
    void slotLogout(DAccount::Type);
    void slotLastSyncTimeUpdate(const QString &datetime);
    //帐户状态发送改变
    void slotAccountStateChange();

    void slotFirstDayofWeekCurrentChanged(int index);
    void slotTimeTypeCurrentChanged(int index);
    void slotAccountCurrentChanged(int index);
    void slotAddCalDavAccount();
    void slotDeleteCalDavAccount();
    void slotDeleteCalDavAccountFinished(bool success);
    void slotTypeAddBtnClickded();
    void slotTypeImportBtnClickded();
    void slotSetUosSyncFreq(int freq);
    void slotUosManualSync();
    void slotNetworkStateChange(DOANetWorkDBus::NetWorkState state);
    //更新同步项按钮状态
    void slotSyncTagButtonUpdate();
    //点击同步项时，更新uos账户状态
    void slotSyncAccountStateUpdate(bool);
    void slotSyncStateChange(DAccount::AccountSyncState state);
    void slotSyncTimeout();

private:
    void initFirstDayofWeekWidget();
    void initTimeTypeWidget();
    void initAccountComboBoxWidget();
    void initTypeAddWidget();
    void initScheduleTypeWidget();
    void initSyncFreqWidget();
    void initManualSyncButton();
    void initUosAccountSettingsWidget();
    void updateUosAccountSettingsVisibility();

    void setFirstDayofWeek(int value);
    void setTimeType(int value);
    void accountUpdate();
    void updateCalDavAddButtonVisibility();

    void setTypeEnable(int index);
    void updateSyncStatusDisplay(const QString &datetime, DAccount::AccountSyncState state);
private:
    void initWidget();
    void initConnect();
    void initData();

    void initWidgetDisplayStatus();
    void initView();

private:
    //一周首日
    QWidget *m_firstDayofWeekWidget = nullptr;
    QComboBox *m_firstDayofWeekCombobox= nullptr;

    //时间格式
    QWidget *m_timeTypeWidget = nullptr;
    QComboBox *m_timeTypeCombobox = nullptr;

    //帐户选择
    QComboBox *m_accountComboBox = nullptr;
    CalDavAccountListWidget *m_calDavAccountListWidget = nullptr;
    DIconButton *m_calDavAccountAddButton = nullptr;
    QWidget *m_uosAccountSettingsWidget = nullptr;
    UserloginWidget *m_uosLoginWidget = nullptr;
    QWidget *m_uosLoginRow = nullptr;
    QWidget *m_uosSyncItemsRow = nullptr;
    QWidget *m_uosSyncFreqRow = nullptr;
    QWidget *m_uosManualSyncRow = nullptr;
    //同步频率
    QComboBox *m_syncFreqComboBox = nullptr;
    QWidget *m_syncFreqWidget = nullptr;

    QAction *m_typeAddAction = nullptr;
    QAction *m_typeImportAction = nullptr;

    JobTypeListView *m_scheduleTypeWidget = nullptr;

    //手动同步按钮和同步时间显示
    QLabel *m_syncTimeLabel = nullptr;
    DLabel *m_syncTimeValueLabel = nullptr;
    QLabel *m_syncStatusIconLabel = nullptr;
    QPushButton *m_syncBtn = nullptr;
    QWidget *m_manualSyncWidget = nullptr;
    QWidget *m_uosSyncItemsWidget = nullptr;
    DOANetWorkDBus *m_ptrNetworkState;
    QTimer *m_syncTimeoutTimer = nullptr;
    QCheckBox *m_radiobuttonAccountCalendar = nullptr;
    QCheckBox *m_radiobuttonAccountSetting = nullptr;
    ControlCenterProxy *m_controlCenterProxy;
};

#endif // SETTINGDIALOG_H
