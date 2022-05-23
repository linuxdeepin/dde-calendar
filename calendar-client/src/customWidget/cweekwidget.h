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
#ifndef CWEEKWIDGET_H
#define CWEEKWIDGET_H

#include <QPushButton>

class CWeekWidget : public QPushButton
{
    Q_OBJECT
public:
    explicit CWeekWidget(QWidget *parent = nullptr);

    //设置一周首日
    void setFirstDay(Qt::DayOfWeek);
    //设置是否根据配置自动设置
    void setAutoFirstDay(bool);
    //设置字体大小是否跟随界面大小
    void setAutoFontSizeByWindow(bool);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_autoFirstDay = true; //是否根据配置自动设置
    bool m_autoFontSizeByWindow = true; //字体大小是否跟随界面大小

    Qt::DayOfWeek m_firstDay = Qt::Monday;  //一周首日
};

#endif // CWEEKWIDGET_H
