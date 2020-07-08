/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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
#include "scheduleitem.h"
#include <QDebug>
#include <QFontMetricsF>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <DGraphicsView>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <dtkwidget_global.h>
#include "schedulecoormanage.h"
#include "scheduledatamanage.h"
#include "SchecduleRemindWidget.h"
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

CScheduleItem::CScheduleItem(QRectF rect, QGraphicsItem *parent, int type)
    : DragInfoItem (rect,parent)
    , m_type(type)
{
    m_transparentcolor = "#000000";
    m_transparentcolor.setAlphaF(0.05);
}

CScheduleItem::~CScheduleItem() {}

void CScheduleItem::setData(const ScheduleDtailInfo &info, QDate date,  int totalNum)
{
    m_vScheduleInfo = info;
    m_totalNum = totalNum;
    m_date = date;
    update();
}

bool CScheduleItem::hasSelectSchedule(const ScheduleDtailInfo &info)
{
    return info == m_vScheduleInfo;
}

void CScheduleItem::splitText(QFont font, int w, int h, QString str, QStringList &liststr, QFontMetrics &fontm)
{
    if (str.isEmpty())
        return;
    QFontMetrics fontmetris(font);
    int heightT = fontm.height();

    QString tstr;
    QStringList tliststr;
    for (int i = 0; i < str.count(); i++) {
        tstr.append(str.at(i));
        int widthT = fontmetris.width(tstr) + 5;
        if (widthT >= w) {
            tstr.chop(1);
            if (tstr.isEmpty())
                break;
            tliststr.append(tstr);
            tstr.clear();
            i--;
        }
    }
    tliststr.append(tstr);
    if (w < 30) {
        QFontMetrics fm_s(fontm);
        QFontMetrics f_st(font);
        QString s = tliststr.at(0) + "...";
        if (h < 23) {
            tliststr.append("");
        } else {
            if (tliststr.isEmpty()) {
                liststr.append("");
            } else {
                QString c = str.at(0);
                QString s = c + "...";
                QFontMetrics fm(font);
                while (f_st.width(s) > w && f_st.width(s) >24) {
                    s.chop(1);
                }
//                if (wi > 72) {
//                    liststr.append("...");
//                } else {
                liststr.append(s);
//                }
            }
        }
    } else {
        for (int i = 0; i < tliststr.count(); i++) {
            if ((i + 1) * heightT <= h - 1) {
                liststr.append(tliststr.at(i));
            } else {
                if (i == 0) {
                    break;
                } else {
                    QString s;
                    QFontMetrics fm_str(fontm);
                    if (i == tliststr.count())
                        s = fontm.elidedText(tliststr.at(i - 1), Qt::ElideRight, w);
                    else {
                        s = fontm.elidedText(tliststr.at(i - 1) + "...", Qt::ElideRight, w);
                    }
                    liststr.removeAt(i - 1);
                    liststr.append(s);
                    break;
                }
            }
        }
    }
}

void CScheduleItem::paintBackground(QPainter *painter, const QRectF &rect, const int isPixMap)
{
    Q_UNUSED(isPixMap);
    CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(
                                  m_vScheduleInfo.type.ID);
    m_vHighflag = CScheduleDataManage::getScheduleDataManage()->getSearchResult(m_vScheduleInfo);

    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo() == m_vScheduleInfo ) {
        if (m_vScheduleInfo.IsMoveInfo ==
                CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().IsMoveInfo) {
            m_vHighflag = true;
        } else {
            painter->setOpacity(0.4);
            gdcolor.textColor.setAlphaF(0.4);
            gdcolor.timeColor.setAlphaF(0.4);
        }
        m_vSelectflag = m_press;
    }
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();

    QColor bcolor = gdcolor.Purecolor;

    QFontMetrics fm = painter->fontMetrics();
    int h = fm.height();
    if (m_vHoverflag) {
        bcolor = gdcolor.hoverPurecolor;
    } else if (m_vHighflag) {
        bcolor = gdcolor.hightlightPurecolor;
    }
    if (m_vSelectflag) {
        bcolor = gdcolor.pressPurecolor;
    }
    painter->setBrush(bcolor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);
    if (m_vHoverflag && !m_vSelectflag) {
        painter->save();
        QRectF trect = QRectF(rect.x() + 0.5, rect.y() + 0.5, rect.width() - 1, rect.height() - 1);
        QPen tpen;
        QColor cc = "#FFFFFF";
        if (themetype == 2) {
            cc = "#FFFFFF";
        } else {
            cc = "#000000";
        }
        cc.setAlphaF(0.08);
        tpen.setColor(cc);
        tpen.setWidthF(1);
        tpen.setStyle(Qt::SolidLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(tpen);
        painter->drawRect(trect);
        painter->restore();
    }
    if (m_vSelectflag) {
        if (themetype == 0 || themetype ==1) {
            gdcolor.textColor.setAlphaF(0.4);
            gdcolor.timeColor.setAlphaF(0.4);
        } else if (themetype ==2) {
            gdcolor.textColor.setAlphaF(0.6);
            gdcolor.timeColor.setAlphaF(0.6);
        }
    }
    painter->save();

    QPen pen(gdcolor.shadowcolor);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawLine(rect.topLeft(), rect.bottomLeft());
    painter->restore();
    int tmagin = 10;
    if (m_totalNum > 1)
        tmagin = 5;
    if (m_type == 0) {
        int timeTextHight = 0;
        QFont font/*("SourceHanSansSC")*/;
        font.setWeight(QFont::Normal);

        font = DFontSizeManager::instance()->get(DFontSizeManager::T8, font);

        //绘制日程起始时间
        if (m_vScheduleInfo.beginDateTime.date() == m_date) {
            painter->save();
            painter->setFont(font);
            painter->setPen(gdcolor.timeColor);
            QTime stime = m_vScheduleInfo.beginDateTime.time();
            QString str = stime.toString("AP h:mm");

            QFontMetrics fontmetris(font);
            qreal drawTextWidth = rect.width() - m_offset*2;
            if (fm.width(str) > drawTextWidth -5) {
                QString tstr;
                for (int i = 0; i < str.count(); i++) {
                    tstr.append(str.at(i));
                    int widthT = fm.width(tstr) - 5;
                    if (widthT >= drawTextWidth) {
                        if (i < 1) {
                            tstr.chop(1);
                        } else {
                            tstr.chop(2);
                        }
                        tstr = tstr + "...";
                        break;
                    }
                }
                QString tstrs = fontmetris.elidedText(str,Qt::ElideRight,qRound(drawTextWidth - 5));
                painter->drawText(
                    QRectF(rect.topLeft().x() + tmagin, rect.topLeft().y() + 3, drawTextWidth- 5, h),
                    Qt::AlignLeft, tstrs);

            } else {
                painter->drawText(
                    QRectF(rect.topLeft().x() + tmagin, rect.topLeft().y() + 3, drawTextWidth - 5, h),
                    Qt::AlignLeft, str);
            }

            painter->restore();

        } else {
            timeTextHight = -20;
        }
        painter->save();

        //绘制日程标题
        font = DFontSizeManager::instance()->get(DFontSizeManager::T6, font);
        font.setLetterSpacing(QFont::PercentageSpacing, 105);
        painter->setFont(font);
        painter->setPen(gdcolor.textColor);
        QStringList liststr;

        QRect textRect = rect.toRect();
        textRect.setWidth(textRect.width() -m_offset*2);
        splitText(font,
                  textRect.width() - tmagin - 8,
                  textRect.height() - 20,
                  m_vScheduleInfo.titleName,
                  liststr, fm);
        for (int i = 0; i < liststr.count(); i++) {
            if ((20 + timeTextHight + (i + 1) * (h - 3)) > rect.height())
                return;
            painter->drawText(
                QRect(textRect.topLeft().x() + tmagin,
                      textRect.topLeft().y() + 20 + timeTextHight + i * (h - 3),
                      textRect.width() - 2,
                      h ),
                Qt::AlignLeft, liststr.at(i));
        }
        painter->restore();
    } else {
        painter->save();
        QFont font/*("SourceHanSansSC")*/;
        font.setWeight(QFont::Normal);
        font = DFontSizeManager::instance()->get(DFontSizeManager::T8, font);
//        font.setPixelSize(12);
        painter->setFont(font);
        painter->setPen(gdcolor.textColor);
        painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, "...");
        painter->restore();
    }
    if (m_vSelectflag) {
        QColor selcolor = m_transparentcolor;
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);
    }

}
