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
#include "sidebaritemwidget.h"
#include <DFontSizeManager>
#include <QVBoxLayout>

SidebarItemWidget::SidebarItemWidget(QWidget *parent): QWidget (parent)
{
    initView();
}

/**
 * @brief SidebarItemWidget::initView
 * 初始化视图
 */
void SidebarItemWidget::initView()
{
    QHBoxLayout *vLayout = new QHBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    m_headWidget = new QWidget;
    m_headWidget->setFixedSize(QSize(20, 20));
    m_headWidget->hide();   //头部控件默认隐藏
    m_titleLabel = new DLabel(this);
    m_titleLabel->setFixedHeight(30);
    m_rearIconButton = new DIconButton(this);
    m_rearIconButton->setFixedSize(QSize(20, 20));
    m_rearIconButton->setIconSize(QSize(10, 10));
    m_rearIconButton->hide();   //尾部控件默认隐藏
    m_rearIconButton->setFocusPolicy(Qt::NoFocus);
    vLayout->addWidget(m_headWidget);
    vLayout->addWidget(m_titleLabel, 1);
    vLayout->addWidget(m_rearIconButton);
    this->setLayout(vLayout);

    setFixedHeight(36);
}

/**
 * @brief SidebarItemWidget::setSelectStatus
 * 设置选中状态
 * @param status    将被设置的状态
 */
void SidebarItemWidget::setSelectStatus(bool status)
{
    if (status == m_selectStatus) {
        return;
    }
    m_selectStatus = status;
    //根据控件类型设置响应控件状态
    switch (m_headType) {
    case HeadInvalid:
        break;
    case HeadIcon:
        if (m_headIconButton != nullptr) {
            if (m_selectStatus) {
                m_headIconButton->setIcon(DStyle::SP_ArrowDown);
            } else {
                m_headIconButton->setIcon(DStyle::SP_ArrowRight);
            }
        }
        break;
    case HeadCheckBox:
        if (m_checkBox != nullptr) {
            if (m_selectStatus != m_checkBox->isChecked()) {
                m_checkBox->setChecked(m_selectStatus);
            }
        }
        break;
    }
    emit signalStatusChange(m_selectStatus, m_id);
}

/**
 * @brief SidebarItemWidget::switchState
 * 切换状态
 */
void SidebarItemWidget::switchState()
{
    setSelectStatus(!m_selectStatus);
}

/**
 * @brief SidebarItemWidget::setItem
 * 设置关联的item
 * @param item 待关联的item
 */
void SidebarItemWidget::setItem(QTreeWidgetItem* item)
{
    m_item = item;
    //连接状态切换事件
    connect(this, &SidebarItemWidget::signalStatusChange, this, [this](bool status) {
        if (m_item->isExpanded() == status) {
            return ;
        }
        //设置列表展开状态
        m_item->setExpanded(status);
    });
}

/**
 * @brief SidebarItemWidget::setHeadWidget
 * 设置头部控件
 * @param widget
 */
void SidebarItemWidget::setHeadWidget(QWidget* widget)
{
    if (m_headLayout) {
        m_headLayout->deleteLater();
    }
    widget->setFocusPolicy(Qt::NoFocus);
    m_headLayout = new QHBoxLayout(m_headWidget);
    m_headLayout->setMargin(0);
    m_headLayout->setSpacing(0);
    m_headLayout->addWidget(widget);
    m_headLayout->setAlignment(Qt::AlignCenter);
    m_headWidget->show();
}

/**
 * @brief SidebarItemWidget::getTopLevelWidget
 * 获取顶层控件，有箭头
 * @param title 显示文本
 * @param icon 尾部icon
 * @return 顶层选项控件
 */
SidebarItemWidget* SidebarItemWidget::getTopLevelWidget(QString& title, QIcon& icon)
{
    SidebarItemWidget* item = getTopLevelWidget(title);
    //设置尾部控件
    item->m_rearIconButton->setIcon(icon);
    item->m_rearIconButton->show();
    return item;
}

/**
 * @brief SidebarItemWidget::getTopLevelWidget
 * 获取顶层控件，有箭头, 无尾部控件
 * @param title 显示文本
 * @param icon 尾部icon
 * @return 顶层选项控件
 */
SidebarItemWidget* SidebarItemWidget::getTopLevelWidget(QString& title)
{
    SidebarItemWidget* item = new SidebarItemWidget();
    item->m_headType = HeadIcon;
    item->setFixedHeight(36);
    DFontSizeManager::instance()->bind(item->m_titleLabel, DFontSizeManager::T6);
    item->m_titleLabel->setElideMode(Qt::ElideRight);
    //设置初始字体大小
    DFontSizeManager::instance()->setFontGenericPixelSize(static_cast<quint16>(DFontSizeManager::fontPixelSize(QFont())));
    QFont labelF = DFontSizeManager::instance()->t6();
    labelF.setWeight(QFont::Normal);
    item->m_titleLabel->setFont(labelF);
    QPalette palette = item->m_titleLabel->palette();
    palette.setBrush(QPalette::Text, QColor("#526A7F"));
    item->m_titleLabel->setPalette(palette);
    item->m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->m_titleLabel->setText(title);

    item->m_headIconButton = new DIconButton(item);
    item->m_headIconButton->setFlat(true);
    item->m_headIconButton->setFixedSize(16, 16);
    item->m_headIconButton->setIconSize(QSize(10, 10));
    item->m_headIconButton->setIcon(DStyle::SP_ArrowRight);
    connect(item->m_headIconButton, &DIconButton::clicked, item, &SidebarItemWidget::slotHeadWidgetClicked);
    item->setHeadWidget(item->m_headIconButton);
    return item;
}

/**
 * @brief SidebarItemWidget::getLocalWidget
 * 获取子层本地日程item控件，有复选框
 * @param info 本地日程类型
 * @return 子层选项控件
 */
SidebarItemWidget* SidebarItemWidget::getLocalWidget(JobTypeInfo& info)
{
    SidebarItemWidget* item = new SidebarItemWidget();
    item->m_id = info.getJobTypeNo();
    item->setFixedHeight(40);
    item->m_headType = HeadCheckBox;
    item->layout()->setSpacing(9);
    item->layout()->setContentsMargins(5, 0, 0, 0);
    qobject_cast<QBoxLayout*>(item->layout())->addStretch(1);
    //绑定字体大小
    DFontSizeManager::instance()->bind(item->m_titleLabel, DFontSizeManager::T5);
    item->m_titleLabel->setElideMode(Qt::ElideRight);
    //设置初始字体大小
    DFontSizeManager::instance()->setFontGenericPixelSize(static_cast<quint16>(DFontSizeManager::fontPixelSize(QFont())));
    QFont labelF = DFontSizeManager::instance()->t5();
    labelF.setWeight(QFont::Normal);
    item->m_titleLabel->setFont(labelF);
    item->m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QPalette tPalette = item->m_titleLabel->palette();
    tPalette.setBrush(QPalette::Text, QColor("#001A2E"));
    item->m_titleLabel->setPalette(tPalette);
    item->m_titleLabel->setMaximumWidth(120);
    item->m_titleLabel->setText(info.getJobTypeName());

    item->m_rearIconButton->hide();

    item->m_checkBox = new QCheckBox(item);
    QPalette palette = item->m_checkBox->palette();
    palette.setBrush(QPalette::Highlight, QColor(info.getColorInfo().getColorHex()));
    item->m_checkBox->setPalette(palette);
    connect(item->m_checkBox, &QCheckBox::clicked, item, &SidebarItemWidget::slotHeadWidgetClicked);
    item->setHeadWidget(item->m_checkBox);
    return item;
}

/**
 * @brief SidebarItemWidget::slotHeadWidgetClicked
 * 头部控件点击事件
 */
void SidebarItemWidget::slotHeadWidgetClicked()
{
    switch (m_headType) {
    case HeadInvalid:
        break;
    case HeadIcon:
        if (m_headIconButton != nullptr) {
            setSelectStatus(!m_selectStatus);
        }
        break;
    case HeadCheckBox:
        if (m_checkBox != nullptr) {
            setSelectStatus(m_checkBox->isChecked());
        }
        break;
    }
}

/**
 * @brief SidebarItemWidget::slotRearIconClicked
 * 尾部控件点击事件，暂未实现
 */
void SidebarItemWidget::slotRearIconClicked()
{

}

void SidebarItemWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    switchState();
}

void SidebarItemWidget::mouseReleaseEvent(QMouseEvent *)
{
    //屏蔽鼠标释放事件，解决双击时触发重复事件的问题
}
