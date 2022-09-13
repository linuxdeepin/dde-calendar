// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "todaybutton.h"
#include "constants.h"

#include <DPalette>

#include <QPainter>
#include <QMouseEvent>

DGUI_USE_NAMESPACE
CTodayButton::CTodayButton(QWidget *parent)
    : DPushButton(parent)
{
    m_font.setWeight(QFont::Medium);
    m_font.setPixelSize(DDECalendar::FontSizeFourteen);
}

void CTodayButton::setBColor(QColor normalC, QColor hoverC, QColor pressc, QColor normalC1, QColor hoverC1, QColor pressc1)
{
    m_bnormalColor = normalC;
    m_bhoverColor = hoverC;
    m_bPressColor = pressc;
    m_dbnormalColor = normalC1;
    m_dbhoverColor = hoverC1;
    m_dbPressColor = pressc1;
    m_shadowColor = m_bnormalColor;
}

void CTodayButton::setTColor(QColor normalC, QColor hoverC, QColor pressc)
{
    m_tnormalColor = normalC;
    m_thoverColor = hoverC;
    m_tPressColor = pressc;
}

void CTodayButton::setshadowColor(QColor sc)
{
    m_shadowColor = sc;
}

void CTodayButton::mousePressEvent(QMouseEvent *event)
{
    DPalette todaypa = palette();
    todaypa.setColor(DPalette::ButtonText, m_tPressColor);
    todaypa.setColor(DPalette::Dark, m_dbPressColor);
    todaypa.setColor(DPalette::Light, m_bPressColor);
    todaypa.setColor(DPalette::Shadow, m_shadowColor);
    setPalette(todaypa);
    DPushButton::mousePressEvent(event);
}

void CTodayButton::mouseReleaseEvent(QMouseEvent *event)
{
    DPalette todaypa = palette();
    todaypa.setColor(DPalette::ButtonText, m_tnormalColor);
    todaypa.setColor(DPalette::Dark, m_dbnormalColor);
    todaypa.setColor(DPalette::Light, m_bnormalColor);
    todaypa.setColor(DPalette::Shadow, m_shadowColor);
    setPalette(todaypa);
    DPushButton::mouseReleaseEvent(event);
}

void CTodayButton::focusOutEvent(QFocusEvent *event)
{
    DPalette todaypa = palette();
    todaypa.setColor(DPalette::ButtonText, m_tnormalColor);
    todaypa.setColor(DPalette::Dark, m_dbnormalColor);
    todaypa.setColor(DPalette::Light, m_bnormalColor);
    todaypa.setColor(DPalette::Shadow, m_shadowColor);
    setPalette(todaypa);
    DPushButton::focusOutEvent(event);
}

void CTodayButton::enterEvent(QEvent *event)
{
    DPalette todaypa = palette();
    todaypa.setColor(DPalette::ButtonText, m_thoverColor);
    todaypa.setColor(DPalette::Dark, m_dbhoverColor);
    todaypa.setColor(DPalette::Light, m_bhoverColor);
    todaypa.setColor(DPalette::Shadow, m_shadowColor);
    setPalette(todaypa);
    DPushButton::enterEvent(event);
}

void CTodayButton::leaveEvent(QEvent *event)
{
    DPalette todaypa = palette();
    todaypa.setColor(DPalette::ButtonText, m_tnormalColor);
    todaypa.setColor(DPalette::Dark, m_dbnormalColor);
    todaypa.setColor(DPalette::Light, m_bnormalColor);
    todaypa.setColor(DPalette::Shadow, m_shadowColor);
    setPalette(todaypa);
    DPushButton::leaveEvent(event);
}

void CTodayButton::keyPressEvent(QKeyEvent *event)
{
    //添加回车点击效果处理
    if (event->key() == Qt::Key_Return) {
        click();
    }
    DPushButton::keyPressEvent(event);
}
