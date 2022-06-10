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

#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>

class SettingWidgets : public QObject
{
    Q_OBJECT
public:

    static void init();

    static QPair<QWidget*, QWidget*> createFirstDayofWeekWidget(QObject *obj);
    static QPair<QWidget*, QWidget*> createTimeTypeWidget(QObject *obj);

signals:

public slots:
    void slotGeneralSettingsUpdate();

public slots:
    void slotFirstDayofWeekCurrentChanged(int index);
    void slotTimeTypeCurrentChanged(int index);

protected:
    static SettingWidgets* getInstance();
    explicit SettingWidgets(QObject *parent = nullptr);
    void initWidget();

    QWidget* getFirstDayofWeekWidget();
    void setFirstDayofWeek(int value);

    QWidget* getTimeTypeWidget();
    void setTimeType(int value);

private:
    void initConnect();

private:
    //一周首日
    QWidget *m_firstDayofWeekWidget = nullptr;
    QComboBox *m_firstDayofWeekCombobox = nullptr;

    //时间格式
    QWidget *m_timeTypeWidget = nullptr;
    QComboBox *m_timeTypeCombobox = nullptr;

};

#endif // SETTINGWIDGETS_H
