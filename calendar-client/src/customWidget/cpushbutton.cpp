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
#include "cpushbutton.h"
#include <DApplicationHelper>
#include <DHiDPIHelper>
#include <DGuiApplicationHelper>
#include <QMouseEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>

CPushButton::CPushButton(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layoutAddType = new QHBoxLayout();
    m_textLabel = new QLabel(tr("New event type"));

    m_textLabel->setFixedSize(100,34);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_textLabel->setFixedSize(200,34);
    layoutAddType->setSpacing(0);
    layoutAddType->setMargin(0);
    layoutAddType->setAlignment(Qt::AlignLeft);
    m_iconButton = new DIconButton(this);
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setFixedSize(16, 16);
    m_iconButton->setFlat(true);

    DPalette palette = DApplicationHelper::instance()->palette(this);
    QPalette pa = m_textLabel->palette();

    //设置深浅色主题下正常状态时的文本颜色，与下拉框颜色对其
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        pa.setBrush(QPalette::WindowText, QColor("#FFFFFF"));
    } else {
        pa.setBrush(QPalette::WindowText, QColor("#000000"));
    }
    m_textLabel->setPalette(pa);

    layoutAddType->setContentsMargins(33,0,0,0);
    layoutAddType->addWidget(m_iconButton);
    layoutAddType->addSpacing(5);
    layoutAddType->addWidget(m_textLabel);
    setFixedHeight(34);
    setLayout(layoutAddType);
}

void CPushButton::setHighlight(bool status)
{
    if (status == m_Highlighted) {
        return;
    }
    m_Highlighted = status;
    update();
}

bool CPushButton::isHighlight()
{
    return m_Highlighted;
}

void CPushButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_pressed = true;
}

void CPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pressed && rect().contains(event->pos())){
        emit clicked();
    }
    m_pressed = false;
}

void CPushButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    DPalette palette = DApplicationHelper::instance()->palette(this);
    QPainter painter(this);
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    m_iconButton->setIcon(QIcon::fromTheme("dde_calendar_create"));
    if (m_Highlighted) {
        //背景设置为高亮色
        m_iconButton->setIcon(QIcon(":/icons/deepin/builtin/dark/icons/dde_calendar_create_32px.svg"));
        painter.setBrush(palette.highlight());
        m_textLabel->setBackgroundRole(QPalette::Highlight);       
    } else {
        //背景透明
        painter.setBrush(QBrush("#00000000"));
        m_textLabel->setBackgroundRole(QPalette::Window);

    }
    //绘制背景
    painter.drawRect(this->rect());
}
