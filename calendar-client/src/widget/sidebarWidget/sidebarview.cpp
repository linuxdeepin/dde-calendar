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
#include "sidebarview.h"
#include "sidebaritemwidget.h"
#include "calendarmanage.h"
#include "widget/monthWidget/monthdayview.h"
#include "widget/dayWidget/daymonthview.h"
#include "DPaletteHelper"
#include <QVBoxLayout>
#include <QPixmap>

SidebarView::SidebarView(QWidget *parent) : QWidget(parent)
{
    initView();
    initConnection();
    initData();
}

/**
 * @brief SidebarView::initView
 * 初始化视图
 */
void SidebarView::initView()
{
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->setContentsMargins(0, 8, 0, 10);
    hLayout->setSpacing(0);

    m_treeWidget = new QTreeWidget();
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setExpandsOnDoubleClick(false);
    m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection); //屏蔽item选择效果
    m_treeWidget->setFrameStyle(QFrame::NoFrame); //去除边框
    m_treeWidget->setIndentation(0);    //设置item前间距为0
    m_treeWidget->setFocusPolicy(Qt::TabFocus);

    m_calendarWidget = new SidebarCalendarWidget(this);
    m_calendarWidget->setFixedSize(180, 220);

    DPushButton *btn = new DPushButton;
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setFixedHeight(1);
    hLayout->addWidget(m_treeWidget, 1);
    hLayout->addWidget(btn);
    hLayout->addWidget(m_calendarWidget, 1);

    setLayout(hLayout);
    setFixedWidth(180);
}

void SidebarView::initConnection()
{

}

/**
 * @brief SidebarView::initData
 * 初始化数据
 */
void SidebarView::initData()
{
    m_treeWidget->clear();
    QTreeWidgetItem *localItem = new QTreeWidgetItem();
    m_treeWidget->addTopLevelItem(localItem);
    localItem->setSizeHint(0, QSize(200, 30));
    QString localName = tr("Local Calendar");
    SidebarItemWidget* localWidget = SidebarItemWidget::getTopLevelWidget(localName);
    m_treeWidget->setItemWidget(localItem, 0, localWidget);
    localWidget->setItem(localItem);

    QString unionName("UnionID");
    QTreeWidgetItem *unionItem = new QTreeWidgetItem();
    m_treeWidget->addTopLevelItem(unionItem);
    QIcon unionIcon(":/resources/icon/icon_refresh.svg");
    SidebarItemWidget* unionWidget = SidebarItemWidget::getTopLevelWidget(unionName, unionIcon);
    m_treeWidget->setItemWidget(unionItem, 0, unionWidget);
    unionWidget->setItem(unionItem);

    resetJobTypeChildItem(localItem);
    resetJobTypeChildItem(unionItem);
}

/**
 * @brief SidebarView::resetLocalChildItem
 * 重置本地日程类型选项
 * @param parentItem
 */
void SidebarView::resetJobTypeChildItem(QTreeWidgetItem* parentItem)
{
    QTreeWidgetItem *item;
    for (auto info : JobTypeInfoManager::instance()->getJobTypeList()) {
        item = new QTreeWidgetItem(parentItem);
        item->setTextAlignment(0, Qt::AlignVCenter|Qt::AlignLeft);
        parentItem->addChild(item);
        SidebarItemWidget* widget = SidebarItemWidget::getLocalWidget(info);
        m_treeWidget->setItemWidget(item, 0, widget);
        connect(widget, &SidebarItemWidget::signalStatusChange, this, &SidebarView::slotItemWidgetStatusChange);
    }
}

/**
 * @brief SidebarView::slotItemWidgetStatusChange
 * itemWidget状态改变事件
 * @param status 状态
 * @param id    id
 */
void SidebarView::slotItemWidgetStatusChange(bool status, int id)
{
    Q_UNUSED(status)
    Q_UNUSED(id)
}

void SidebarView::paintEvent(QPaintEvent *event)
{
    //绘制背景
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().color(QPalette::Base));
    int radius = 8;
    //绘制圆角
    painter.drawRoundedRect(0, 0, width(), height(), radius, radius);
    //绘制右侧直角
    painter.drawRect(radius, 0, width()-radius, height());

    QWidget::paintEvent(event);
}
