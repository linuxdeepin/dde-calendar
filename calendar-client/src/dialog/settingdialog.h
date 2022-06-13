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

DWIDGET_USE_NAMESPACE

class CSettingDialog : public DSettingsDialog
{
    Q_OBJECT
public:
    explicit CSettingDialog(QWidget *parent = nullptr);

signals:

public slots:

private:
    void initView();

private:

    SettingWidgets *m_settingWidgets = nullptr;

};

#endif // SETTINGDIALOG_H
