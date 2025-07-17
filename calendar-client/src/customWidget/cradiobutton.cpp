// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cradiobutton.h"
#include "commondef.h"
#include <DPaletteHelper>
#include <QPainterPath>
#include <QPainter>

DWIDGET_USE_NAMESPACE

CRadioButton::CRadioButton(QWidget *parent) : QRadioButton(parent)
{
    qCDebug(ClientLogger) << "CRadioButton constructor initialized";
}

void CRadioButton::setColor(const QColor &color)
{
    // qCDebug(ClientLogger) << "CRadioButton::setColor - Setting color to:" << color;
    m_color = color;
}

QColor CRadioButton::getColor()
{
    // qCDebug(ClientLogger) << "CRadioButton::getColor - Returning color:" << m_color;
    return m_color;
}

void CRadioButton::paintEvent(QPaintEvent *event)
{
    // qCDebug(ClientLogger) << "CRadioButton::paintEvent";
    Q_UNUSED(event);

    QPainter painter(this);
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    int w = width();
    int h = height();

    if (!isChecked()) {
        // qCDebug(ClientLogger) << "CRadioButton::paintEvent - Drawing unchecked state";
        // 设置画刷颜色
        painter.setBrush(m_color);
        // 绘制圆
        painter.drawEllipse(QPointF(w / 2, h / 2), w / 2 - 1, h / 2 - 1);
        //绘制黑色描边
        QPainterPath path;
        path.addEllipse(0, 0, w, h);
        path.addEllipse(2, 2, w - 4, h - 4);
        QColor c = QColor("#000000");
        c.setAlphaF(0.1);
        painter.setBrush(c);
        painter.drawPath(path);
    } else {
        // qCDebug(ClientLogger) << "CRadioButton::paintEvent - Drawing checked state";
        DPalette palette = DPaletteHelper::instance()->palette(this);
        QPainterPath path;
        path.addEllipse(0, 0, w, h);
        path.addEllipse(2, 2, w - 4, h - 4);
        painter.setBrush(palette.highlight());
        painter.drawPath(path);

        // 设置画刷颜色
        painter.setBrush(m_color);
        // 绘制圆
        painter.drawEllipse(QPointF(w / 2, h / 2), w / 2 - 3, h / 2 - 3);
    }
}
