// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cpushbutton.h"
#include "commondef.h"
#include <DPaletteHelper>

#include <DGuiApplicationHelper>
#include <QMouseEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>

CPushButton::CPushButton(QWidget *parent) : QWidget(parent)
{
    qCDebug(ClientLogger) << "CPushButton constructor initialized";
    QHBoxLayout *layoutAddType = new QHBoxLayout();
    m_textLabel = new QLabel(tr("New event type"));

    m_textLabel->setFixedSize(100,34);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_textLabel->setFixedSize(200,34);
    layoutAddType->setSpacing(0);
    layoutAddType->setContentsMargins(0, 0, 0, 0);
    layoutAddType->setAlignment(Qt::AlignLeft);
    m_iconButton = new DIconButton(this);
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setFixedSize(16, 16);
    m_iconButton->setFlat(true);

    DPalette palette = DPaletteHelper::instance()->palette(this);
    QPalette pa = m_textLabel->palette();

    //设置深浅色主题下正常状态时的文本颜色，与下拉框颜色对其
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        qCDebug(ClientLogger) << "Dark theme detected, setting text color to white";
        pa.setBrush(QPalette::WindowText, QColor("#FFFFFF"));
    } else {
        qCDebug(ClientLogger) << "Light theme detected, setting text color to black";
        pa.setBrush(QPalette::WindowText, QColor("#000000"));
    }
    m_textLabel->setPalette(pa);

    layoutAddType->setContentsMargins(33,0,0,0);
    layoutAddType->addWidget(m_iconButton);
    layoutAddType->addSpacing(5);
    layoutAddType->addWidget(m_textLabel);
    setFixedHeight(34);
    setLayout(layoutAddType);
    qCDebug(ClientLogger) << "CPushButton initialization completed";
}

void CPushButton::setHighlight(bool status)
{
    qCDebug(ClientLogger) << "CPushButton::setHighlight - Setting highlight status to:" << status;
    if (status == m_Highlighted) {
        qCDebug(ClientLogger) << "Highlight status unchanged, returning";
        return;
    }
    m_Highlighted = status;
    update();
}

bool CPushButton::isHighlight()
{
    qCDebug(ClientLogger) << "CPushButton::isHighlight - Current status:" << m_Highlighted;
    return m_Highlighted;
}

void CPushButton::mousePressEvent(QMouseEvent *event)
{
    // qCDebug(ClientLogger) << "CPushButton::mousePressEvent - Position:" << event->pos();
    Q_UNUSED(event);
    m_pressed = true;
}

void CPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    // qCDebug(ClientLogger) << "CPushButton::mouseReleaseEvent - Position:" << event->pos();
    if (m_pressed && rect().contains(event->pos())){
        qCDebug(ClientLogger) << "Mouse released within button bounds, emitting clicked signal";
        emit clicked();
    }
    m_pressed = false;
}

void CPushButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    DPalette palette = DPaletteHelper::instance()->palette(this);
    QPainter painter(this);
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    m_iconButton->setIcon(QIcon::fromTheme("dde_calendar_create"));
    if (m_Highlighted) {
        // qCDebug(ClientLogger) << "Button is highlighted, setting highlight appearance";
        //背景设置为高亮色
        m_iconButton->setIcon(QIcon(":/icons/deepin/builtin/dark/icons/dde_calendar_create_32px.svg"));
        painter.setBrush(palette.highlight());
        m_textLabel->setBackgroundRole(QPalette::Highlight);       
    } else {
        // qCDebug(ClientLogger) << "Button is not highlighted, setting normal appearance";
        //背景透明
        painter.setBrush(QBrush("#00000000"));
        m_textLabel->setBackgroundRole(QPalette::Window);

    }
    //绘制背景
    painter.drawRect(this->rect());
}
