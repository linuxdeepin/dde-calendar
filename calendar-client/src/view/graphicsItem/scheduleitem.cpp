// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "scheduleitem.h"
#include "schedulecoormanage.h"
#include "scheduledatamanage.h"
#include "calendarmanage.h"
#include "commondef.h"

#include <DFontSizeManager>

#include <QDebug>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QMarginsF>

#include <dtkwidget_global.h>

DWIDGET_USE_NAMESPACE

CScheduleItem::CScheduleItem(QRectF rect, QGraphicsItem *parent, int type)
    : DragInfoItem(rect, parent)
    , m_type(type)
    , m_totalNum(0)
    , m_transparentcolor("#000000")
    , m_timeFormat(CalendarManager::getInstance()->getTimeFormat())
{
    qCDebug(ClientLogger) << "CScheduleItem constructor - rect:" << rect << "type:" << type;
    m_transparentcolor.setAlphaF(0.05);
    connect(CalendarManager::getInstance(), &CalendarManager::signalTimeFormatChanged, this, &CScheduleItem::timeFormatChanged);
}

CScheduleItem::~CScheduleItem()
{
    qCDebug(ClientLogger) << "CScheduleItem destructor";
}

/**
 * @brief CScheduleItem::setData        设置显示数据
 * @param info
 * @param date
 * @param totalNum
 */
void CScheduleItem::setData(const DSchedule::Ptr &info, QDate date, int totalNum)
{
    // qCDebug(ClientLogger) << "CScheduleItem::setData - schedule:" << (info ? info->summary() : "null") 
    //                      << "date:" << date << "totalNum:" << totalNum;
    m_vScheduleInfo = info;
    m_totalNum = totalNum;
    setDate(date);
    update();
}

/**
 * @brief CScheduleItem::hasSelectSchedule      是否含有选中日程
 * @param info
 * @return
 */
bool CScheduleItem::hasSelectSchedule(const DSchedule::Ptr &info)
{
    bool result = (info == m_vScheduleInfo);
    // qCDebug(ClientLogger) << "CScheduleItem::hasSelectSchedule - schedule:" << (info ? info->summary() : "null") 
    //                      << "result:" << result;
    return result;
}

/**
 * @brief CScheduleItem::splitText      根据字体大小,宽度和高度将标题切换为多行
 * @param font
 * @param w
 * @param h
 * @param str
 * @param listStr
 * @param fontM
 */
void CScheduleItem::splitText(QFont font, int w, int h, QString str, QStringList &listStr, QFontMetrics &fontM)
{
    // qCDebug(ClientLogger) << "CScheduleItem::splitText - width:" << w << "height:" << h 
    //                      << "text:" << str;
    if (str.isEmpty()) {
        qCDebug(ClientLogger) << "CScheduleItem::splitText - Text is empty, returning";
        return;
    }
    QFontMetrics fontMetrics(font);
    int heightT = fontM.height();
    QString tStr;
    QStringList tListStr;

    for (int i = 0; i < str.count(); i++) {
        tStr.append(str.at(i));
        int widthT = fontMetrics.horizontalAdvance(tStr) + 5;

        if (widthT >= w) {
            tStr.chop(1);
            if (tStr.isEmpty())
                break;
            tListStr.append(tStr);
            tStr.clear();
            i--;
        }
    }
    tListStr.append(tStr);

    if (w < 30) {
        qCDebug(ClientLogger) << "Width too small (< 30), special handling for text";
        QFontMetrics fm_s(fontM);
        QFontMetrics f_st(font);
        QString s = tListStr.at(0) + "...";

        if (h < 23) {
            tListStr.append("");
        } else {
            if (tListStr.isEmpty()) {
                listStr.append("");
            } else {
                QString c = str.at(0);
                QString str = c + "...";
                QFontMetrics fm(font);
                while (f_st.horizontalAdvance(str) > w && f_st.horizontalAdvance(str) > 24) {
                    str.chop(1);
                }
                listStr.append(str);
            }
        }
    } else {
        qCDebug(ClientLogger) << "Normal width handling for text";
        for (int i = 0; i < tListStr.count(); i++) {
            if ((i + 1) * heightT <= h - 1) {
                listStr.append(tListStr.at(i));
            } else {
                if (i == 0) {
                    break;
                } else {
                    QString s;
                    QFontMetrics fm_str(fontM);

                    if (i == tListStr.count())
                        s = fontM.elidedText(tListStr.at(i - 1), Qt::ElideRight, w);
                    else {
                        s = fontM.elidedText(tListStr.at(i - 1) + "...", Qt::ElideRight, w);
                    }
                    listStr.removeAt(i - 1);
                    listStr.append(s);
                    break;
                }
            }
        }
    }
    qCDebug(ClientLogger) << "Final text split into" << listStr.count() << "lines";
}

/**
 * @brief CScheduleItem::timeFormatChanged 更新时间显示格式
 */
void CScheduleItem::timeFormatChanged(int value)
{
    qCDebug(ClientLogger) << "CScheduleItem::timeFormatChanged - value:" << value;
    if (value) {
        m_timeFormat = "hh:mm";
    } else {
        m_timeFormat = "h:mm";
    }
    update();
}

/**
 * @brief CScheduleItem::paintBackground        绘制item显示效果
 * @param painter
 * @param rect
 * @param isPixMap
 */
void CScheduleItem::paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap)
{
    // qCDebug(ClientLogger) << "CScheduleItem::paintBackground - rect:" << rect 
    //                      << "isPixMap:" << isPixMap
    //                      << "schedule:" << (m_vScheduleInfo ? m_vScheduleInfo->summary() : "null");
    Q_UNUSED(isPixMap);
    //根据日程类型获取颜色
    CSchedulesColor gdColor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_vScheduleInfo->scheduleTypeID());

    QColor textPenColor = CScheduleDataManage::getScheduleDataManage()->getTextColor();
    //判断是否为选中日程
    if (m_vScheduleInfo == m_pressInfo) {
        // qCDebug(ClientLogger) << "Schedule is selected";
        //判断当前日程是否为拖拽移动日程
        if (m_vScheduleInfo->isMoved() == m_pressInfo->isMoved()) {
            // qCDebug(ClientLogger) << "Schedule is highlighted";
            m_vHighflag = true;
        } else {
            // qCDebug(ClientLogger) << "Schedule is dimmed (opacity 0.4)";
            painter->setOpacity(0.4);
            textPenColor.setAlphaF(0.4);
            gdColor.orginalColor.setAlphaF(0.4);
            m_vHighflag = false;
        }
        m_vSelectflag = m_press;
    }

    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    QColor bColor = gdColor.normalColor;
    QFontMetrics fm = painter->fontMetrics();
    int h = fm.height();

    if (m_vHoverflag) {
        // qCDebug(ClientLogger) << "Schedule is hovered";
        bColor = gdColor.hoverColor;
    } else if (m_vHighflag) {
        // qCDebug(ClientLogger) << "Schedule is highlighted";
        bColor = gdColor.hightColor;
    } else if (m_vSelectflag) {
        // qCDebug(ClientLogger) << "Schedule is selected";
        bColor = gdColor.pressColor;
    }
    painter->setBrush(bColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);

    if (m_vHoverflag && !m_vSelectflag) {
        // qCDebug(ClientLogger) << "Drawing hover effect";
        painter->save();
        QRectF tRect = QRectF(rect.x() + 0.5, rect.y() + 0.5, rect.width() - 1, rect.height() - 1);
        QPen tPen;
        QColor cc = "#FFFFFF";
        if (themetype == 2) {
            cc = "#FFFFFF";
        } else {
            cc = "#000000";
        }
        cc.setAlphaF(0.08);
        tPen.setColor(cc);
        tPen.setWidthF(1);
        tPen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(tPen);
        painter->drawRect(tRect);
        painter->restore();
    }
    if (m_vSelectflag) {
        // qCDebug(ClientLogger) << "Adjusting opacity for selected schedule";
        if (themetype == 0 || themetype == 1) {
            textPenColor.setAlphaF(0.4);
            gdColor.orginalColor.setAlphaF(0.4);
        } else if (themetype == 2) {
            textPenColor.setAlphaF(0.6);
            gdColor.orginalColor.setAlphaF(0.6);
        }
    }

    painter->save();
    QPen pen(gdColor.orginalColor);
    pen.setWidth(2);
    painter->setPen(pen);
    //左侧绘制竖线
    QPointF top(rect.topLeft().x(), rect.topLeft().y() + 1);
    QPointF bottom(rect.bottomLeft().x(), rect.bottomLeft().y() - 1);
    painter->drawLine(top, bottom);
    painter->restore();
    int tMargin = 10;

    if (m_totalNum > 1)
        tMargin = 5;

    if (m_type == 0) {
        // qCDebug(ClientLogger) << "Drawing normal schedule item";
        int timeTextHight = 0;
        QFont font;
        font.setWeight(QFont::Normal);

        font = DFontSizeManager::instance()->get(DFontSizeManager::T8, font);

        //绘制日程起始时间
        if (m_vScheduleInfo->dtStart().date() == getDate()) {
            // qCDebug(ClientLogger) << "Drawing start time for schedule";
            painter->save();
            painter->setFont(font);
            painter->setPen(gdColor.orginalColor);

            QTime stime = m_vScheduleInfo->dtStart().time();
            QString str = stime.toString((CalendarManager::getInstance()->getTimeShowType() ? "AP " : "") + m_timeFormat);
            // qCDebug(ClientLogger) << "Start time text:" << str;
            QFontMetrics fontMetrics(font);
            qreal drawTextWidth = rect.width() - m_offset * 2;

            if (fm.horizontalAdvance(str) > drawTextWidth - 5) {
                // qCDebug(ClientLogger) << "Time text too long, truncating";
                QString tStr;
                for (int i = 0; i < str.count(); i++) {
                    tStr.append(str.at(i));
                    int widthT = fm.horizontalAdvance(tStr) - 5;

                    if (widthT >= drawTextWidth) {
                        if (i < 1) {
                            tStr.chop(1);
                        } else {
                            tStr.chop(2);
                        }
                        tStr = tStr + "...";
                        break;
                    }
                }
                QString tStrs = fontMetrics.elidedText(str, Qt::ElideRight, qRound(drawTextWidth - 5));
                // qCDebug(ClientLogger) << "Final time text:" << tStrs;
                painter->drawText(
                    QRectF(rect.topLeft().x() + tMargin, rect.topLeft().y() + 3, drawTextWidth - 5, h),
                    Qt::AlignLeft, tStrs);

            } else {
                painter->drawText(
                    QRectF(rect.topLeft().x() + tMargin, rect.topLeft().y() + 3, drawTextWidth - 5, h),
                    Qt::AlignLeft, str);
            }
            painter->restore();
        } else {
            // qCDebug(ClientLogger) << "Not drawing start time (different date)";
            timeTextHight = -20;
        }
        painter->save();

        //绘制日程标题
        // qCDebug(ClientLogger) << "Drawing schedule title";
        font = DFontSizeManager::instance()->get(DFontSizeManager::T6, font);
        font.setLetterSpacing(QFont::PercentageSpacing, 105);
        painter->setFont(font);
        painter->setPen(textPenColor);
        QStringList liststr;
        QRect textRect = rect.toRect();
        textRect.setWidth(textRect.width() - m_offset * 2);
        splitText(font,
                  textRect.width() - tMargin - 8,
                  textRect.height() - 20,
                  m_vScheduleInfo->summary(),
                  liststr, fm);

        for (int i = 0; i < liststr.count(); i++) {
            if ((20 + timeTextHight + (i + 1) * (h - 3)) > rect.height())
                return;
            // qCDebug(ClientLogger) << "Drawing title line" << i+1 << ":" << liststr.at(i);
            painter->drawText(
                QRect(textRect.topLeft().x() + tMargin,
                      textRect.topLeft().y() + 20 + timeTextHight + i * (h - 3),
                      textRect.width() - 2,
                      h),
                Qt::AlignLeft, liststr.at(i));
        }
        painter->restore();
    } else {
        // qCDebug(ClientLogger) << "Drawing more item indicator";
        painter->save();
        QFont font;
        font.setWeight(QFont::Normal);
        font = DFontSizeManager::instance()->get(DFontSizeManager::T8, font);
        painter->setFont(font);
        painter->setPen(textPenColor);
        painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, "...");
        painter->restore();
    }
    if (m_vSelectflag) {
        // qCDebug(ClientLogger) << "Drawing selection overlay";
        QColor selcolor = m_transparentcolor;
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);
    }
    if (getItemFocus()) {
        // qCDebug(ClientLogger) << "Drawing focus frame";
        //获取tab图形
        QRectF drawRect = rect.marginsRemoved(QMarginsF(1, 1, 1, 1));
        painter->setBrush(Qt::NoBrush);
        QPen framePen;
        framePen.setWidth(2);
        framePen.setColor(getSystemActiveColor());
        painter->setPen(framePen);
        painter->drawRect(drawRect);
    }
}
