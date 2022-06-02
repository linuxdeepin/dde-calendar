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
#include "sidebaritemwidget.h"
#include "accountmanager.h"
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
    void slotItemWidgetStatusChange(bool status, QString id);
    //账户更新事件
    void slotAccountUpdate();
    //日程类型更新事件
    void slotScheduleTypeUpdate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initView();
    void initConnection();
    //初始化数据
    void initData();
    //初始化本地账户列表
    void initLocalAccountItem();
    //初始化union账户列表
    void initUnionAccountItem();

    //重置日程类型item
    void resetJobTypeChildItem(SidebarAccountItemWidget *parentItemWidget);
    //重置item位置
    void resetTreeItemPos();

private:
    QTreeWidget *m_treeWidget = nullptr;        //树结构
    SidebarAccountItemWidget* m_localItemWidget = nullptr;
    SidebarAccountItemWidget* m_unionItemWidget = nullptr;

    SidebarCalendarWidget *m_calendarWidget = nullptr;  //小日历
};

#endif // SIDEBARVIEW_H
