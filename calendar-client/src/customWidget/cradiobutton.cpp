/*
* Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
*
* Author:     leilong <leilong@uniontech.com>
*
* Maintainer: leilong <leilong@uniontech.com>
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
#include "cradiobutton.h"
#include <DApplicationHelper>
#include <QPainterPath>
#include <QPainter>

DWIDGET_USE_NAMESPACE

CRadioButton::CRadioButton(QWidget *parent) : QRadioButton(parent)
{
}

void CRadioButton::setColor(const QColor &color)
{
    m_color = color;
}

QColor CRadioButton::getColor()
{
    return m_color;
}

void CRadioButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    // 反走样
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    int w = width();
    int h = height();

    if (!isChecked()) {
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
        DPalette palette = DApplicationHelper::instance()->palette(this);
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
