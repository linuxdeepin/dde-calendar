// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cmonthschedulenumitem.h"

#include <QPainter>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(monthScheduleNumLog, "calendar.view.monthschedulenum")

CMonthScheduleNumItem::CMonthScheduleNumItem(QGraphicsItem *parent)
    : CFocusItem(parent)
    , m_num(0)
{
    qCDebug(monthScheduleNumLog) << "Create month schedule num item";
    setItemType(COTHER);
}

CMonthScheduleNumItem::~CMonthScheduleNumItem()
{
    qCDebug(monthScheduleNumLog) << "Destroy month schedule num item";
}

/**
 * @brief CMonthScheduleNumItem::setColor       设置背景色
 * @param color1
 * @param color2
 */
void CMonthScheduleNumItem::setColor(QColor color1, QColor color2)
{
    qCDebug(monthScheduleNumLog) << "Set colors - color1:" << color1 << "color2:" << color2;
    m_color1 = color1;
    m_color2 = color2;
}

/**
 * @brief CMonthScheduleNumItem::setText        这是字体颜色
 * @param tColor
 * @param font
 */
void CMonthScheduleNumItem::setText(QColor tColor, QFont font)
{
    qCDebug(monthScheduleNumLog) << "Set text color:" << tColor << "font family:" << font.family();
    m_textcolor = tColor;
    m_font = font;
}

/**
 * @brief CMonthScheduleNumItem::setSizeType        设置字体大小
 * @param sizeType
 */
void CMonthScheduleNumItem::setSizeType(DFontSizeManager::SizeType sizeType)
{
    qCDebug(monthScheduleNumLog) << "Set font size type:" << sizeType;
    m_SizeType = sizeType;
}

void CMonthScheduleNumItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    qCDebug(monthScheduleNumLog) << "Paint schedule num item, number:" << m_num;
    Q_UNUSED(option);
    Q_UNUSED(widget);
    qreal labelwidth = this->rect().width();
    qreal labelheight = this->rect().height() - 6;
    qreal rectX = this->rect().x();
    qreal rectY = this->rect().y();
    //绘制背景
    m_font = DFontSizeManager::instance()->get(m_SizeType, m_font);
    //将直线开始点设为0，终点设为1，然后分段设置颜色
    QLinearGradient linearGradient(0, 0, labelwidth, 0);
    linearGradient.setColorAt(0, m_color1);
    linearGradient.setColorAt(1, m_color2);

    painter->setRenderHints(QPainter::Antialiasing);
    painter->setBrush(linearGradient);
    if (getItemFocus()) {
        qCDebug(monthScheduleNumLog) << "Item has focus, drawing focus frame";
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->drawRoundedRect(rect(), rect().height() / 3, rect().height() / 3);
    //绘制文字
    painter->setFont(m_font);
    painter->setPen(m_textcolor);
    QString str = QString(tr("%1 more")).arg(m_num) + "...";
    QFontMetrics fm = painter->fontMetrics();
    QString tStr;
    for (int i = 0; i < str.count(); i++) {
        tStr.append(str.at(i));
        int widthT = fm.horizontalAdvance(tStr) + 5;
        if (widthT >= labelwidth) {
            tStr.chop(2);
            break;
        }
    }
    if (tStr != str) {
        qCDebug(monthScheduleNumLog) << "Text truncated due to width limit";
        tStr = tStr + "...";
    }
    painter->drawText(QRectF(rectX, rectY, labelwidth, labelheight + 4), Qt::AlignCenter, tStr);
}
