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
#include "accountmanager.h"
#include <DFontSizeManager>
#include <QVBoxLayout>

SidebarItemWidget::SidebarItemWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(220);
}

SidebarItemWidget *SidebarItemWidget::getAccountItemWidget(AccountItem::Ptr ptr)
{
    return new SidebarAccountItemWidget(ptr);
}

/**
 * @brief SidebarItemWidget::getLocalWidget
 * 获取子层本地日程item控件，有复选框
 * @param info 本地日程类型
 * @return 子层选项控件
 */
SidebarItemWidget *SidebarItemWidget::getTypeItemWidget(DScheduleType::Ptr ptr)
{
    return new SidebarTypeItemWidget(ptr);
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
    updateStatus();

    if (m_item && m_item->isExpanded() != status) {
        //设置列表展开状态
        m_item->setExpanded(status);
    }
    emit signalStatusChange(m_selectStatus, m_id);
}

/**
 * @brief SidebarItemWidget::getSelectStatus
 * 获取当前状态
 * @return
 */
bool SidebarItemWidget::getSelectStatus()
{
    return m_selectStatus;
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
void SidebarItemWidget::setItem(QTreeWidgetItem *item)
{
    m_item = item;
}

QTreeWidgetItem* SidebarItemWidget::getTreeItem()
{
    return m_item;
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

SidebarTypeItemWidget::SidebarTypeItemWidget(DScheduleType::Ptr ptr, QWidget *parent)
    : SidebarItemWidget(parent)
    , m_scheduleType(ptr)
{
    initView();
    m_id = m_scheduleType->accountID();
}

void SidebarTypeItemWidget::initView()
{
    QHBoxLayout *vLayout = new QHBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    m_checkBox = new QCheckBox(this);
    QPalette palette = m_checkBox->palette();
    palette.setBrush(QPalette::Highlight, QColor(m_scheduleType->getColorCode()));
    m_checkBox->setPalette(palette);
    setSelectStatus((m_scheduleType->showState() == DScheduleType::Show));
    connect(m_checkBox, &QCheckBox::clicked, this, [this]() {
        setSelectStatus(m_checkBox->isChecked());
    });

    m_titleLabel = new DLabel(this);
    m_titleLabel->setFixedHeight(30);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T6);
    m_titleLabel->setElideMode(Qt::ElideRight);
    //设置初始字体大小
    DFontSizeManager::instance()->setFontGenericPixelSize(static_cast<quint16>(DFontSizeManager::fontPixelSize(QFont())));
    QFont labelF = DFontSizeManager::instance()->t6();
    labelF.setWeight(QFont::Normal);
    m_titleLabel->setFont(labelF);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setText(m_scheduleType->displayName());

    vLayout->addSpacing(2);
    vLayout->addWidget(m_checkBox);
    vLayout->addWidget(m_titleLabel, 1);
    this->setLayout(vLayout);

    setFixedHeight(40);
}

void SidebarTypeItemWidget::updateStatus()
{
    AccountItem::Ptr account = gAccounManager->getAccountItemByAccountId(m_scheduleType->accountID());
    if (account) {
        if (m_selectStatus != (m_scheduleType->showState() == DScheduleType::Show)) {
            m_scheduleType->setShowState(m_selectStatus? DScheduleType::Show:DScheduleType::Hide);
            account->updateScheduleTypeShowState(m_scheduleType);
        }
        m_checkBox->setChecked(m_selectStatus);
    }
}

SidebarAccountItemWidget::SidebarAccountItemWidget(AccountItem::Ptr ptr, QWidget *parent)
    : SidebarItemWidget(parent)
    , m_accountItem(ptr)
{
    initView();
}

void SidebarAccountItemWidget::initView()
{
    QHBoxLayout *vLayout = new QHBoxLayout(this);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    m_headIconButton = new DIconButton(this);
    m_headIconButton->setFlat(true);
    m_headIconButton->setFixedSize(16, 16);
    m_headIconButton->setIconSize(QSize(10, 10));
    m_headIconButton->setIcon(DStyle::SP_ArrowRight);
    connect(m_headIconButton, &DIconButton::clicked, this, [this](){
        setSelectStatus(!m_selectStatus);
    });

    m_titleLabel = new DLabel(this);
    m_titleLabel->setFixedHeight(30);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T6);
    m_titleLabel->setElideMode(Qt::ElideRight);
    //设置初始字体大小
    DFontSizeManager::instance()->setFontGenericPixelSize(static_cast<quint16>(DFontSizeManager::fontPixelSize(QFont())));
    QFont labelF = DFontSizeManager::instance()->t6();
    labelF.setWeight(QFont::Normal);
    m_titleLabel->setFont(labelF);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setText(m_accountItem->getAccount()->accountName());

    m_rearIconButton = new DIconButton(this);
    m_rearIconButton->setFixedSize(QSize(20, 20));
    m_rearIconButton->setIconSize(QSize(10, 10));

    m_rearIconButton->setFocusPolicy(Qt::NoFocus);
    vLayout->addWidget(m_headIconButton);
    vLayout->addWidget(m_titleLabel, 1);
    vLayout->addWidget(m_rearIconButton);
    this->setLayout(vLayout);

    if (m_accountItem->getAccount()->accountType() == DAccount::Account_UnionID) {
        m_rearIconButton->setIcon(QIcon(":/resources/icon/icon_refresh.svg"));
    } else {
        m_rearIconButton->hide();   //尾部控件隐藏
    }

    setFixedHeight(36);
}

AccountItem::Ptr SidebarAccountItemWidget::getAccountItem()
{
    return m_accountItem;
}

void SidebarAccountItemWidget::updateStatus()
{
    m_accountItem->updateAccountExpandStatus(m_selectStatus);
    if (m_selectStatus) {
        m_headIconButton->setIcon(DStyle::SP_ArrowDown);
    } else {
        m_headIconButton->setIcon(DStyle::SP_ArrowRight);
    }
}

