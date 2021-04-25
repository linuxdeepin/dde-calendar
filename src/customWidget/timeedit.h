/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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
#pragma once

#include <DComboBox>
#include <DLineEdit>
#include <DListWidget>

#include <QTime>
#include <QTimeEdit>

DWIDGET_USE_NAMESPACE
class CTimeEdit : public DComboBox
{
    Q_OBJECT

public:
    CTimeEdit(QWidget *parent = nullptr);
    ~CTimeEdit();
    void setTime(QTime time);
    QTime getTime();
private:
    void initUI();
    void initConnection();
private:
    DLineEdit *m_timeEdit = nullptr;
    int m_type = 0; // 0 h,1 m
    int m_pos = 0;
    QTime m_time;
};
