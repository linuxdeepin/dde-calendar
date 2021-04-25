/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#include "customframe.h"
#include "constants.h"

#include <DPalette>

#include <QPainter>
#include <QFontMetrics>
#include <QPainterPath>

DGUI_USE_NAMESPACE
CustomFrame::CustomFrame(QWidget *parent)
    : QFrame(parent)
{
    m_font.setWeight(QFont::Medium);
    m_font.setPixelSize(DDECalendar::FontSizeFourteen);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
    this->setWindowFlags(Qt::FramelessWindowHint);   //设置无边框窗口
    setContentsMargins(0, 0, 0, 0);
}

void CustomFrame::setBColor(QColor normalC)
{
    m_bnormalColor = normalC;
    m_bflag = true;
    update();
}

void CustomFrame::setRoundState(bool lstate, bool tstate, bool rstate, bool bstate)
{
    m_lstate = lstate;
    m_tstate = tstate;
    m_rstate = rstate;
    m_bstate = bstate;
}

void CustomFrame::setTextStr(QFont font, QColor tc, QString strc, int flag)
{
    m_font = font;
    m_tnormalColor = tc;
    m_text = strc;
    m_textflag = flag;
}

void CustomFrame::setTextStr(QString strc)
{
    m_text = strc;

    if (!m_fixsizeflag) {
        QFontMetrics fm(m_font);
        int w = fm.width(m_text);
        setMinimumWidth(w);
    }
    update();
}

void CustomFrame::setTextColor(QColor tc)
{
    m_tnormalColor = tc;
    update();
}

void CustomFrame::setTextFont(QFont font)
{
    m_font = font;

    if (!m_fixsizeflag) {
        QFontMetrics fm(m_font);
        int w = fm.width(m_text);
        setMinimumWidth(w);
    }
}

void CustomFrame::setTextAlign(int flag)
{
    m_textflag = flag;
}

void CustomFrame::setRadius(int radius)
{
    m_radius = radius;
}

void CustomFrame::setboreder(int framew)
{
    m_borderframew = framew;
}

void CustomFrame::setFixedSize(int w, int h)
{
    m_fixsizeflag = true;
    QFrame::setFixedSize(w, h);
}

void CustomFrame::paintEvent(QPaintEvent *e)
{
    int labelwidth = width() - 2 * m_borderframew;
    int labelheight = height() - 2 * m_borderframew;

    QPainter painter(this);
    QRect fillRect = QRect(m_borderframew, m_borderframew, labelwidth, labelheight);

    if (m_bflag) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        painter.setBrush(QBrush(m_bnormalColor));
        painter.setPen(Qt::NoPen);
        QPainterPath painterPath;
        painterPath.moveTo(m_radius, m_borderframew);

        if (m_lstate) {
            painterPath.arcTo(QRect(m_borderframew, m_borderframew, m_radius * 2, m_radius * 2), 90, 90);
        } else {
            painterPath.lineTo(m_borderframew, m_borderframew);
            painterPath.lineTo(m_borderframew, m_radius);
        }
        painterPath.lineTo(0, labelheight - m_radius);

        if (m_bstate) {
            painterPath.arcTo(QRect(m_borderframew, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 180, 90);
        } else {
            painterPath.lineTo(m_borderframew, labelheight);
            painterPath.lineTo(m_radius, labelheight);
        }
        painterPath.lineTo(labelwidth - m_radius, labelheight);

        if (m_rstate) {
            painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);
        } else {
            painterPath.lineTo(labelwidth, labelheight);
            painterPath.lineTo(labelwidth, labelheight - m_radius);
        }
        painterPath.lineTo(labelwidth, m_radius);

        if (m_tstate) {
            painterPath.arcTo(QRect(labelwidth - m_radius * 2, m_borderframew, m_radius * 2, m_radius * 2), 0, 90);
        } else {
            painterPath.lineTo(labelwidth, m_borderframew);
            painterPath.lineTo(labelwidth - m_radius, m_borderframew);
        }
        painterPath.lineTo(m_radius, m_borderframew);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
        painter.restore();
    }

    if (!m_text.isEmpty()) {
        painter.save();
        painter.setRenderHints(QPainter::HighQualityAntialiasing);
        painter.setFont(m_font);
        painter.setPen(m_tnormalColor);
        painter.drawText(fillRect, m_textflag, m_text);
        painter.restore();
    }
    QFrame::paintEvent(e);
}

