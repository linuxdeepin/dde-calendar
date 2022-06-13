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
#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "settingWidget/settingwidgets.h"

#include <DSettingsDialog>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

class CSettingDialog : public DSettingsDialog
{
    Q_OBJECT
public:
    explicit CSettingDialog(QWidget *parent = nullptr);

private:
    QPair<QWidget*, QWidget*> createFirstDayofWeekWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createTimeTypeWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createAccountCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncFreqCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncTagRadioButton(QObject *obj);
    QWidget *createManualSyncButton(QObject *obj);
    QWidget *createJobTypeListView(QObject *obj);
    DIconButton *createTypeAddButton();

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

private:
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
    void initView();

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

#endif // SETTINGDIALOG_H
