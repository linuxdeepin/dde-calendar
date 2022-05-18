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
#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include "scheduledatamanage.h"
#include "dscheduletype.h"

#include <DIconButton>
#include <DLabel>
#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QTreeWidgetItem>

DWIDGET_USE_NAMESPACE

class SidebarItemWidget : public QWidget
{
    Q_OBJECT
public:
    SidebarItemWidget(QWidget *parent = nullptr);

    //头控件类型
    enum HeadType {
        HeadInvalid = 0,
        HeadIcon,
        HeadCheckBox,
    };

    //获取顶层item控件，有箭头
    static SidebarItemWidget *getTopLevelWidget(QString &, QIcon &);
    static SidebarItemWidget *getTopLevelWidget(QString &);
    //获取子层本地日程item控件，有复选框
    static SidebarItemWidget *getLocalWidget(DScheduleType &);

    //设置选中状态
    void setSelectStatus(bool);
    //切换状态
    void switchState();
    //设置item
    void setItem(QTreeWidgetItem *);

signals:
    //状态改变信号
    void signalStatusChange(bool status, QString id = "");

public slots:
    //头部控件点击事件
    void slotHeadWidgetClicked();
    //尾部icon控件点击事件
    void slotRearIconClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initView();

    //设置头部控件内容
    void setHeadWidget(QWidget *);

private:
    QString m_title = "";   //标题
    QIcon m_icon;       //尾部控件显示的icon
    QString m_id = ""; //数据id

    HeadType m_headType = HeadInvalid;  //控件类型
    bool m_selectStatus = false;    //选中状态
    QTreeWidgetItem *m_item = nullptr; //关联的item

    QHBoxLayout *m_headLayout = nullptr;    //头部控件布局
    QWidget *m_headWidget = nullptr;    //头部控件显示区域
    DLabel *m_titleLabel = nullptr; //标题显示区域
    QCheckBox *m_checkBox = nullptr;    //复选框
    DIconButton *m_rearIconButton = nullptr;    //尾部icon控件
    DIconButton *m_headIconButton = nullptr;    //头部icon控件
};

#endif // ITEMWIDGET_H
