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
#ifndef SIDEBARVIEW_H
#define SIDEBARVIEW_H

#include "sidebarcalendarwidget.h"
#include <DTreeWidget>
#include <QWidget>
#include <QTreeWidgetItem>

class SidebarView : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarView(QWidget *parent = nullptr);

signals:

public slots:
    //自定义itemWidget控件状态改变事件
    void slotItemWidgetStatusChange(bool status, int id);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initView();
    void initConnection();
    void initData();

    //重置本地日程类型item
    void resetJobTypeChildItem(QTreeWidgetItem* parentItem);

private:
    QTreeWidget *m_treeWidget = nullptr;        //树结构
    SidebarCalendarWidget *m_calendarWidget = nullptr;  //小日历
};

#endif // SIDEBARVIEW_H
