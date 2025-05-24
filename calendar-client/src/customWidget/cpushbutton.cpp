// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cpushbutton.h"
#include <DPaletteHelper>

#include <DGuiApplicationHelper>
#include <QMouseEvent>
#include <QPainter>
#include <QHBoxLayout>
#include <QDebug>

// Add logging category
Q_LOGGING_CATEGORY(cpushbuttonLog, "calendar.widget.cpushbutton")

CPushButton::CPushButton(QWidget *parent) : QWidget(parent)
{
    qCDebug(cpushbuttonLog) << "Constructing CPushButton";
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
    qCDebug(cpushbuttonLog) << "Setting highlight status to:" << status;
    if (status == m_Highlighted) {
        qCDebug(cpushbuttonLog) << "Highlight status unchanged, returning";
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
    qCDebug(cpushbuttonLog) << "Mouse press event at:" << event->pos();
    Q_UNUSED(event);
    m_pressed = true;
}

void CPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(cpushbuttonLog) << "Mouse release event at:" << event->pos();
    if (m_pressed && rect().contains(event->pos())){
        qCDebug(cpushbuttonLog) << "Click event emitted";
        emit clicked();
    }
    m_pressed = false;
}

void CPushButton::paintEvent(QPaintEvent *event)
{
    qCDebug(cpushbuttonLog) << "Paint event triggered, highlighted:" << m_Highlighted;
    QWidget::paintEvent(event);
    DPalette palette = DPaletteHelper::instance()->palette(this);
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
