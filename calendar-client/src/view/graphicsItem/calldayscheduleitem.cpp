// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calldayscheduleitem.h"
#include "commondef.h"

#include <QPainter>

CAllDayScheduleItem::CAllDayScheduleItem(QRectF rect, QGraphicsItem *parent)
    : DragInfoItem(rect, parent)
{
    qCDebug(ClientLogger) << "CAllDayScheduleItem constructor - rect:" << rect;
}

bool CAllDayScheduleItem::hasSelectSchedule(const DSchedule::Ptr &info)
{
    // qCDebug(ClientLogger) << "CAllDayScheduleItem::hasSelectSchedule";
    return info == m_vScheduleInfo;
}

void CAllDayScheduleItem::paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap)
{
    // qCDebug(ClientLogger) << "CAllDayScheduleItem::paintBackground - rect:" << rect;
    Q_UNUSED(isPixMap);
    m_font = DFontSizeManager::instance()->get(m_sizeType, m_font);
    painter->setRenderHints(QPainter::Antialiasing);
    //根据日程类型获取类型颜色
    CSchedulesColor gdColor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_vScheduleInfo->scheduleTypeID());
    QRectF drawrect = rect;

    QColor textcolor = CScheduleDataManage::getScheduleDataManage()->getTextColor();

    //判断是否为选中日程
    if (m_vScheduleInfo == m_pressInfo) {
        // qCDebug(ClientLogger) << "Schedule is pressed schedule";
        //判断当前日程是否为拖拽移动日程
        if (m_vScheduleInfo->isMoved() == m_pressInfo->isMoved()) {
            // qCDebug(ClientLogger) << "Setting high flag to true";
            m_vHighflag = true;
        } else {
            // qCDebug(ClientLogger) << "Setting opacity to 0.4";
            painter->setOpacity(0.4);
            textcolor.setAlphaF(0.4);
        }
        m_vSelectflag = m_press;
    }
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    // qCDebug(ClientLogger) << "Current theme type:" << themetype;

    QColor brushColor = gdColor.normalColor;
    if (m_vHoverflag) {
        // qCDebug(ClientLogger) << "Schedule is hovered, using hover color";
        brushColor = gdColor.hoverColor;
    } else if (m_vHighflag) {
        // qCDebug(ClientLogger) << "Schedule is highlighted, using press color";
        brushColor = gdColor.pressColor;
    } else if (m_vSelectflag) {
        // qCDebug(ClientLogger) << "Schedule is selected, using press color with reduced opacity";
        brushColor = gdColor.pressColor;
        textcolor.setAlphaF(0.4);
    }
    QRectF fillRect = QRectF(drawrect.x(),
                             drawrect.y(),
                             drawrect.width(),
                             drawrect.height() - 2);
    //将直线开始点设为0，终点设为1，然后分段设置颜色
    painter->setBrush(brushColor);
    if (getItemFocus() && isPixMap == false) {
        // qCDebug(ClientLogger) << "Item has focus, drawing frame with system active color";
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
    } else {
        painter->setPen(Qt::NoPen);
    }
    painter->drawRoundedRect(fillRect, rect.height() / 3, rect.height() / 3);
    painter->setFont(m_font);
    painter->setPen(textcolor);
    QFontMetrics fm = painter->fontMetrics();
    QString tSTitleName = m_vScheduleInfo->summary();
    tSTitleName.replace("\n", "");
    QString str = tSTitleName;
    QString tStr;
    int _rightOffset = fm.horizontalAdvance("...");
    //显示宽度  左侧偏移13右侧偏移8
    qreal _showWidth = fillRect.width() - 13 - 8 - m_offset * 2;
    // qCDebug(ClientLogger) << "Schedule title:" << str << "available width:" << _showWidth;
    //如果标题总长度大于显示长度则显示长度须减去"..."的长度
    if (fm.horizontalAdvance(str) > _showWidth) {
        // qCDebug(ClientLogger) << "Title too long, truncating";
        _showWidth -= _rightOffset;
        for (int i = 0; i < str.count(); i++) {
            tStr.append(str.at(i));
            int widthT = fm.horizontalAdvance(tStr);
            //如果宽度大于显示长度则去除最后添加的字符
            if (widthT > _showWidth) {
                tStr.chop(1);
                break;
            }
        }
        if (tStr != str) {
            tStr = tStr + "...";
        }
    } else {
        tStr = str;
    }
    // qCDebug(ClientLogger) << "Final display text:" << tStr;

    painter->drawText(QRectF(fillRect.topLeft().x() + 13, fillRect.y(), fillRect.width(), fillRect.height()),
                      Qt::AlignLeft | Qt::AlignVCenter, tStr);
    if (m_vHoverflag && !m_vSelectflag) {
        // qCDebug(ClientLogger) << "Drawing hover effect";
        QRectF tRect = QRectF(fillRect.x() + 0.5, fillRect.y() + 0.5, fillRect.width() - 1, fillRect.height() - 1);
        painter->save();

        QPen pen;
        QColor selcolor;

        if (themetype == 2) {
            selcolor = "#FFFFFF";
        } else {
            selcolor = "#000000";
        }
        selcolor.setAlphaF(0.08);
        pen.setColor(selcolor);
        pen.setWidthF(1);
        pen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->drawRoundedRect(tRect, rect.height() / 3, rect.height() / 3);
        painter->restore();
    }
    if (m_vSelectflag) {
        // qCDebug(ClientLogger) << "Drawing selection effect";
        QColor selcolor = "#000000";
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(fillRect, rect.height() / 3, rect.height() / 3);
    }
}
