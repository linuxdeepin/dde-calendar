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

DWIDGET_USE_NAMESPACE

class SettingWidgets : public QObject
{
    Q_OBJECT
public:

    explicit SettingWidgets(QObject *parent = nullptr);

    void clear();

    QPair<QWidget*, QWidget*> createFirstDayofWeekWidget(QObject *obj);
    QPair<QWidget*, QWidget*> createTimeTypeWidget(QObject *obj);
    QPair<QWidget *, QWidget *> createAccountCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncFreqCombobox(QObject *obj);
    QPair<QWidget*, QWidget*> createSyncTagRadioButton(QObject *obj);
    QWidget *createManualSyncButton(QObject *obj);
    QWidget *createJobTypeListView(QObject *obj);
    DIconButton *createTypeAddButton();

signals:

public slots:
    void slotGeneralSettingsUpdate();
    void slotAccountUpdate();

    void slotFirstDayofWeekCurrentChanged(int index);
    void slotTimeTypeCurrentChanged(int index);
    void slotAccountCurrentChanged(int index);
    void slotTypeAddBtnClickded();
    void slotSetUosSyncFreq(int freq);
    void slotUosManualSync();

protected:

    void clearWidget();
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

    QWidget *m_manualSyncBtn = nullptr;

};

#endif // SETTINGWIDGETS_H
