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
#include "monthschceduleview.h"
#include <QAction>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include "schceduledlg.h"
#include "myschceduleview.h"
#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>
#include "schcedulectrldlg.h"
#include <QShortcut>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QSequentialAnimationGroup>
#include "SchecduleRemindWidget.h"
#include "monthview.h"

#include <QDebug>

DGUI_USE_NAMESPACE
CMonthSchceduleWidgetItem::CMonthSchceduleWidgetItem(QRect rect, QGraphicsItem *parent /*= nullptr*/, int edittype)
    : DragInfoItem(rect,parent),
      m_pos(13,5)
{
    Q_UNUSED(edittype);
}

CMonthSchceduleWidgetItem::~CMonthSchceduleWidgetItem()
{

}


QPixmap CMonthSchceduleWidgetItem::getPixmap()
{
    QPixmap pixmap(this->rect().size().toSize());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    paintBackground(&painter,pixmap.rect(),true);
    painter.end();
    return  pixmap;
}



void CMonthSchceduleWidgetItem::paintBackground(QPainter *painter, const QRectF &rect, const int isPixMap)
{
    qreal labelwidth = rect.width();
    qreal labelheight = rect.height();
    m_font = DFontSizeManager::instance()->get(m_sizeType, m_font);
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();
    CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_vScheduleInfo.type.ID);
    m_vHighflag = CScheduleDataManage::getScheduleDataManage()->getSearchResult(m_vScheduleInfo);
    QLinearGradient linearGradient(rect.topLeft().x(), 0, rect.topRight().x(), 0);
    QColor color1 = gdcolor.gradientFromC;
    QColor color2 = gdcolor.gradientToC;
    QColor textcolor = gdcolor.textColor;



    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo() == m_vScheduleInfo ) {
        if (m_vScheduleInfo.IsMoveInfo ==
                CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().IsMoveInfo) {
            m_vHighflag = true;
        } else {
            painter->setOpacity(0.4);
            textcolor.setAlphaF(0.4);
        }
        m_vSelectflag = m_press;
    }
    if (isPixMap) {
        painter->setOpacity(0.6);
        textcolor.setAlphaF(0.8);
    }

    if (m_vHoverflag) {
        color1 = gdcolor.hovergradientFromC;
        color2 = gdcolor.hovergradientToC;
    } else if (m_vHighflag) {
        color1 = gdcolor.hightlightgradientFromC;
        color2 = gdcolor.hightlightgradientToC;
    }
    if (m_vSelectflag) {
        color1 = gdcolor.pressgradientFromC;
        color2 = gdcolor.pressgradientToC;
        textcolor.setAlphaF(0.4);
    }

    linearGradient.setColorAt(0, color1);
    linearGradient.setColorAt(1, color2);

    QRectF fillRect = QRectF(rect.x()+2,
                             rect.y()+2,
                             labelwidth - 2,
                             labelheight - 2);
    painter->save();
    //将直线开始点设为0，终点设为1，然后分段设置颜色
    painter->setBrush(linearGradient);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(fillRect,
                             rect.height() / 3,
                             rect.height() / 3);
    painter->restore();
    painter->setFont(m_font);
    painter->setPen(textcolor);
    QFontMetrics fm = painter->fontMetrics();

    QString tStitlename = m_vScheduleInfo.titleName;
    tStitlename.replace("\n", "");
    QString str = tStitlename;
    qreal textWidth = labelwidth - m_pos.x() - m_offset*2;
    QString tstr;
    for (int i = 0; i < str.count(); i++) {
        tstr.append(str.at(i));
        int widthT = fm.width(tstr) + 5;
        if (widthT >= textWidth) {
            tstr.chop(2);
            break;
        }
    }
    if (tstr != str) {
        tstr = tstr + "...";
    }

    painter->drawText(QRectF(rect.x()+m_pos.x(),
                             rect.y()+1,
                             textWidth,
                             labelheight - m_pos.y() + 3  ),
                      Qt::AlignLeft | Qt::AlignVCenter, tstr);

    if (m_vHoverflag && !m_vSelectflag) {
        QRectF trect = QRectF(rect.x()+2.5, rect.y()+2.5, labelwidth - 3, labelheight - 3);
        painter->save();
        painter->setRenderHints(QPainter::Antialiasing);
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
        painter->drawRoundedRect(trect, rect.height() / 3, rect.height() / 3);
        painter->restore();
    }
    if (m_vSelectflag) {
        QColor selcolor = "#000000";
        selcolor.setAlphaF(0.05);
        painter->setBrush(selcolor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(fillRect, rect.height() / 3, rect.height() / 3);
    }
}


CMonthSchceduleNumButton::CMonthSchceduleNumButton(QGraphicsItem *parent /*= nullptr*/): QGraphicsRectItem (parent)
{

}

CMonthSchceduleNumButton::~CMonthSchceduleNumButton()
{

}

void CMonthSchceduleNumButton::setColor( QColor color1, QColor color2, bool GradientFlag /*= false*/ )
{
    m_color1 = color1;
    m_color2 = color2;
    m_GradientFlag = GradientFlag;
}

void CMonthSchceduleNumButton::setText( QColor tcolor, QFont font, QPoint pos)
{
    Q_UNUSED(pos);
    m_textcolor = tcolor;
    m_font = font;
//    m_pos = pos;
}

void CMonthSchceduleNumButton::setSizeType(DFontSizeManager::SizeType sizeType)
{
    m_SizeType = sizeType;
}

void CMonthSchceduleNumButton::setTransparentB(bool t, QColor tcolor)
{
    m_transparentcolor = tcolor;
    m_transparentf = t;
}

void CMonthSchceduleNumButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    qreal labelwidth = this->rect().width();
    qreal labelheight = this->rect().height() - 6;
    qreal rectx = this->rect().x();
    qreal recty = this->rect().y();
    m_font = DFontSizeManager::instance()->get(m_SizeType, m_font);
    if (m_GradientFlag) {
        QLinearGradient linearGradient(0, 0, labelwidth, 0);
        linearGradient.setColorAt(0, m_color1);
        linearGradient.setColorAt(1, m_color2);
        QRectF fillRect = QRectF(rectx+2, recty+1, labelwidth - 2, labelheight - 1);
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter->setRenderHints(QPainter::HighQualityAntialiasing);
        painter->setBrush(linearGradient);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(fillRect, 3, 2);

        painter->setFont(m_font);
        painter->setPen(m_textcolor);

        QString str =  QString(tr("%1 more")).arg(m_num) + "...";
        QFont tfont = m_font;
        QFontMetrics fm = painter->fontMetrics();

        QString tstr;
        for (int i = 0; i < str.count(); i++) {
            tstr.append(str.at(i));
            int widthT = fm.width(tstr) + 5;
            if (widthT >= labelwidth - m_pos.x()) {
                tstr.chop(2);
                break;
            }
        }
        if (tstr != str) {
            tstr = tstr + "...";
        }
        painter->drawText(QRectF(rectx+m_pos.x(), recty+m_pos.y(), labelwidth - m_pos.x(), labelheight + 4), Qt::AlignCenter, tstr);

    } else {
        QRectF fillRect = QRectF(rectx+2, recty+1, labelwidth - 2, labelheight - 1);
        //将直线开始点设为0，终点设为1，然后分段设置颜色
        painter->setRenderHints(QPainter::HighQualityAntialiasing);
        painter->setBrush(m_color1);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(fillRect, 3, 2);

        painter->setFont(m_font);
        painter->setPen(m_textcolor);
        painter->drawText(QRectF(rectx+m_pos.x(),
                                 recty+m_pos.y(),
                                 labelwidth - m_pos.x(),
                                 labelheight + 4),
                          Qt::AlignLeft,
                          QString(tr("%1 more")).arg(m_num));
    }
}

void CMonthSchceduleView::setTheMe(int type)
{
    Q_UNUSED(type);
    updateData();
}

CMonthSchceduleView::CMonthSchceduleView(QWidget *parent,QGraphicsScene *scene)
    : QObject (parent),
      m_Scene(scene)
{
    for (int i = 0; i < 6; ++i) {
        CWeekScheduleView *weekSchedule = new CWeekScheduleView(this);
        m_weekSchedule.append(weekSchedule);
    }
    slotFontChange();
}

CMonthSchceduleView::~CMonthSchceduleView()
{

}

void CMonthSchceduleView::setallsize(int w, int h, int left, int top, int buttom, int itemHeight)
{
    m_width = w;
    m_height = h;
    m_buttommagin = buttom;
    m_leftMagin = left;
    m_topMagin = top;

    m_cNum = static_cast<int>(((m_height - m_topMagin - m_buttommagin) / 6.0 + 0.5  - 27) / (itemHeight + 1));
}

void CMonthSchceduleView::setData(QVector<ScheduleDateRangeInfo> &data, int currentMonth)
{
    m_data = data;
    m_currentMonth = currentMonth;
    updateData();
}

void CMonthSchceduleView::slotdeleteitem( CMonthSchceduleWidgetItem *item)
{
    emit signalsUpdateShcedule(item->getData().id);
}

void CMonthSchceduleView::slotedititem(CMonthSchceduleWidgetItem *item, int type)
{
    Q_UNUSED(type);
    emit signalsUpdateShcedule(item->getData().id);
}


void CMonthSchceduleView::slotFontChange()
{
    QFont font;
    DFontSizeManager::instance()->setFontGenericPixelSize(
        static_cast<quint16>(DFontSizeManager::instance()->fontPixelSize(qGuiApp->font())));
    font= DFontSizeManager::instance()->t8(font);
    QFontMetrics fm(font);
    int h = fm.height();
    if (m_ItemHeight != h) {
        m_ItemHeight = h;
        updateData();
    }
}

bool MScheduleDateThan(const MScheduleDateRangeInfo &s1, const MScheduleDateRangeInfo &s2)
{
    if (s1.bdate != s1.edate && s2.bdate == s2.edate) {
        return true;
    } else if (s1.bdate == s1.edate && s2.bdate != s2.edate) {
        return false;
    } else if (s1.bdate != s1.edate && s2.bdate != s2.edate) {
        if (s1.tData.beginDateTime == s2.tData.beginDateTime) {
            return s1.bdate.daysTo(s1.edate) > s2.bdate.daysTo(s2.edate);
        } else {
            return s1.bdate < s2.bdate;
        }
    } else {
        if (s1.tData.beginDateTime == s2.tData.beginDateTime) {
            return s1.tData.titleName < s2.tData.titleName;
        } else {
            return s1.tData.beginDateTime < s2.tData.beginDateTime;
        }
    }
}
bool MScheduleDaysThan(const MScheduleDateRangeInfo &s1, const MScheduleDateRangeInfo &s2)
{
    return s1.bdate.daysTo(s1.edate) > s2.bdate.daysTo(s2.edate);
}
void CMonthSchceduleView::updateData()
{
    //保护数据防止越界
    if (m_data.count() != 42 || m_cNum < 1) return;
    //开始结束时间
    QDate begindate = m_data.begin()->date;
    QDate enddate = m_data[m_data.count() - 1].date;
    m_beginDate = begindate;
    m_endDate  = enddate;

    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        m_weekSchedule[i]->clearItem();
        m_weekSchedule[i]->setHeight(m_ItemHeight,qRound((m_height - m_topMagin - m_buttommagin) / 6.0-27));
        m_weekSchedule[i]->setData(m_data,i*7,7);
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
    return;
}

void CMonthSchceduleView::updateHigh()
{
    for (int j = 0 ; j < m_weekSchedule.size(); ++j) {
        for (int i = 0; i < m_weekSchedule[j]->getScheduleShowItem().count(); i++) {
            m_weekSchedule[j]->getScheduleShowItem().at(i)->update();
        }
    }

}

QVector<QGraphicsRectItem *> CMonthSchceduleView::getScheduleShowItem() const
{
    QVector<QGraphicsRectItem *>   m_scheduleShowItem;
    for (int j = 0 ; j < m_weekSchedule.size(); ++j) {
        for (int i = 0; i < m_weekSchedule[j]->getScheduleShowItem().count(); i++) {
            m_scheduleShowItem.append(m_weekSchedule[j]->getScheduleShowItem().at(i));
        }
    }
    return m_scheduleShowItem;
}

void CMonthSchceduleView::updateDate(const ScheduleDtailInfo &info)
{
    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        if (m_weekSchedule.at(i)->addData(info)) {

        } else {
            m_weekSchedule[i]->clearItem();
            m_weekSchedule[i]->updateSchedule(true);
        };
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
}

void CMonthSchceduleView::changeDate(const ScheduleDtailInfo &info)
{
    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        m_weekSchedule.at(i)->changeDate(info);
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
}

void CMonthSchceduleView::updateDate(const int row, const ScheduleDtailInfo &info)
{
    for (int i = 0 ; i < m_weekSchedule.size(); ++i) {
        if ( row ==i) {
            m_weekSchedule.at(i)->addData(info);
        } else {
            m_weekSchedule[i]->clearItem();
            m_weekSchedule[i]->updateSchedule(true);
        };
        QVector<QVector<MScheduleDateRangeInfo> > mSchedule = m_weekSchedule[i]->getMScheduleInfo();
        updateDateShow(mSchedule,m_weekSchedule[i]->getScheduleShowItem());
    }
}
void CMonthSchceduleView::updateDateShow(QVector<QVector<MScheduleDateRangeInfo> > &vCMDaySchedule, QVector<QGraphicsRectItem *> &schudeleShowItem)
{
    for (int i = 0; i < vCMDaySchedule.count(); i++) {
        for (int j = 0; j < vCMDaySchedule[i].count(); j++) {
            if (vCMDaySchedule[i].at(j).state) {
                createScheduleNumWidget(vCMDaySchedule[i].at(j), i + 1,schudeleShowItem);
            } else {
                createScheduleItemWidget(vCMDaySchedule[i].at(j), i + 1,schudeleShowItem);
            }
        }
    }
}

void CMonthSchceduleView::splitSchedule(MScheduleDateRangeInfo &old, QVector<MScheduleDateRangeInfo> &newData)
{
    int brow = static_cast<int>((m_beginDate.daysTo(old.bdate) + 1) / 7);
    int erow = static_cast<int>((m_beginDate.daysTo(old.edate) + 1) / 7);
    int bcol = (m_beginDate.daysTo(old.bdate) + 1) % 7;
    int ecol = (m_beginDate.daysTo(old.edate) + 1) % 7;
    if (bcol != 0) brow = brow + 1;
    if (ecol != 0) erow = erow + 1;
    if (brow > erow) {
        return;
    }
    if (brow == erow) {
        newData.append(old);
    } else {
        bcol =  bcol ? bcol : 7;
        ecol =  ecol ? ecol : 7;
        //处理开始
        MScheduleDateRangeInfo bfirst = old;
        bfirst.edate = bfirst.bdate.addDays(7 - bcol);
        newData.append(bfirst);
        //处理中间数据
        for (int i = brow + 1; i < erow; i++) {
            MScheduleDateRangeInfo info = old;
            info.bdate = m_beginDate.addDays((i - 1) * 7);
            info.edate = m_beginDate.addDays((i - 1) * 7 + 6);
            newData.append(info);
        }

        //处理结束
        MScheduleDateRangeInfo second = old;
        second.bdate = second.edate.addDays(-ecol + 1);
        newData.append(second);
    }
}

void CMonthSchceduleView::createScheduleItemWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem)
{
    ScheduleDtailInfo gd = info.tData;
    QPoint pos;
    int fw;
    int fh;
    computePos(cnum, info.bdate, info.edate, pos, fw, fh);
    CMonthSchceduleWidgetItem *gwi = new CMonthSchceduleWidgetItem(QRect(pos.x(),pos.y(),fw,fh),nullptr);
    m_Scene->addItem(gwi);

    QFont font;

    gwi->setData(gd);


    QColor TransparentC = "#000000";
    TransparentC.setAlphaF(0.05);
    schudeleShowItem.append(gwi);
}

void CMonthSchceduleView::createScheduleNumWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem)
{
    int type = CScheduleDataManage::getScheduleDataManage()->getTheme();
    CMonthSchceduleNumButton *gwi = new CMonthSchceduleNumButton(nullptr);
    QPoint pos;
    int fw;
    int fh;
    computePos(cnum, info.bdate, info.edate, pos, fw, fh);
    QColor gradientFromC = "#000000";
    gradientFromC.setAlphaF(0.00);
    gwi->setColor(gradientFromC, gradientFromC, true);
    QFont font;
    gwi->setSizeType(DFontSizeManager::T8);
    if (type == 0 || type == 1) {
        QColor tc("#5E5E5E");
        tc.setAlphaF(0.9);
        gwi->setText(tc, font, QPoint(13, 3));
    } else {
        QColor tc("#798190");
        tc.setAlphaF(1);
        gwi->setText(tc, font, QPoint(13, 3));
    }
    m_Scene->addItem(gwi);
    gwi->setRect(pos.x(),
                 pos.y(),
                 fw,
                 fh);
    gwi->setData(info.num);
    gwi->setDate(info.bdate);
    if (m_currentMonth != info.bdate.month() && m_currentMonth != info.edate.month()) {
        QColor TransparentC = "#000000";
        TransparentC.setAlphaF(0.05);
        gwi->setTransparentB(true, TransparentC);
    }
    connect(gwi, &CMonthSchceduleNumButton::signalsCurrentScheduleDate, this, &CMonthSchceduleView::signalsCurrentScheduleDate);
    connect(gwi, &CMonthSchceduleNumButton::signalPressScheduleShow, this, &CMonthSchceduleView::signalPressScheduleShow);

    schudeleShowItem.append(gwi);
}

void CMonthSchceduleView::computePos(int cnum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh)
{
    int brow = static_cast<int>((m_beginDate.daysTo(bgeindate)) / 7);
    int bcol = (m_beginDate.daysTo(bgeindate) ) % 7;
    int ecol = (m_beginDate.daysTo(enddate) ) % 7;

    fw = static_cast<int>((ecol - bcol + 1) * ((m_width - m_leftMagin ) / 7.0) - 11);
    fh = m_ItemHeight;
    int x =static_cast<int>(m_leftMagin + bcol * ((m_width - m_leftMagin )  / 7.0) + 5);
    int y = static_cast<int>(m_topMagin + ((m_height - m_topMagin - m_buttommagin) * brow / 6.0 + 0.5)  + 27 + (cnum - 1) * fh + 2.9);
    pos = QPoint(x, y);
}



CWeekScheduleView::CWeekScheduleView(QObject *parent)
    :QObject (parent),
     m_ScheduleHeight(22),
     m_DayHeight(47)
{
    setMaxNum();
}

CWeekScheduleView::~CWeekScheduleView()
{

}

void CWeekScheduleView::setData(QVector<ScheduleDateRangeInfo> &data, const int position, const int count)
{
    int endPos = position+count;
    Q_ASSERT(!(endPos>data.size()));
    m_ScheduleInfo.clear();
    for (int i = position; i<endPos; ++i) {
        for (int j = 0 ; j < data.at(i).vData.size(); ++j) {
            if (!m_ScheduleInfo.contains(data.at(i).vData.at(j))) {
                m_ScheduleInfo.append(data.at(i).vData.at(j));
            }
        }
    }
    beginDate  = data.at(position).date;
    endDate = data.at(position+count -1).date;
    m_colum = count;
    updateSchedule(true);
}

bool CWeekScheduleView::addData(const ScheduleDtailInfo &info)
{
    if (info.beginDateTime.date().daysTo(endDate)>=0 &&beginDate.daysTo(info.endDateTime.date())>=0) {
        clearItem();
        updateSchedule(false,info);
        return  true;
    }
    return false;
}

void CWeekScheduleView::changeDate(const ScheduleDtailInfo &info)
{
    int index = m_ScheduleInfo.indexOf(info);
    if (index <0) {
        m_ScheduleInfo.append(info);
    } else {
        m_ScheduleInfo[index] = info;
    }
    clearItem();
    updateSchedule(true);
}

void CWeekScheduleView::setHeight(const int ScheduleHeight, const int DayHeigth)
{
    m_ScheduleHeight = ScheduleHeight;
    m_DayHeight = DayHeigth;
    setMaxNum();
}

void CWeekScheduleView::updateSchedule(const bool isNormalDisplay, const ScheduleDtailInfo &info)
{
    QVector<ScheduleDtailInfo> schedulev;
    schedulev.clear();
    schedulev = m_ScheduleInfo;
    if (isNormalDisplay) {
        Q_UNUSED(info);
    } else {
        schedulev.append(info);
    }
    QDate   tbegindate,tenddate;
    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    m_ColumnScheduleCount.clear();
    m_ColumnScheduleCount.fill(0,m_colum);
    for (int i = 0 ; i < schedulev.size(); ++i) {
        //日程时间重新标定
        tbegindate = schedulev.at(i).beginDateTime.date();
        tenddate = schedulev.at(i).endDateTime.date();
//        if (tbegindate.daysTo(tenddate)>0 &&
//                schedulev.at(i).endDateTime.time().hour()<1 &&
//                schedulev.at(i).endDateTime.time().second()<1)
//            tenddate = tenddate.addDays(-1);
        if (tenddate<beginDate ||tbegindate>endDate)
            continue;
        if (tbegindate <  beginDate) tbegindate = beginDate;
        if (tenddate > endDate) tenddate = endDate;
        //日程信息
        MScheduleDateRangeInfo info;
        info.bdate = tbegindate;
        info.edate = tenddate;
        info.tData = schedulev.at(i);
        info.state = false;
        vMDaySchedule.append(info);
        qint64 pos = beginDate.daysTo(info.bdate);
        qint64 count = info.bdate.daysTo(info.edate);
        int j = static_cast<int>(pos);
        for (; j < (pos+ count+1); ++j) {
            ++m_ColumnScheduleCount[j];
        }
    }
    std::sort(vMDaySchedule.begin(),vMDaySchedule.end());
//    qSort(vMDaySchedule.begin(),vMDaySchedule.end());
    sortAndFilter(vMDaySchedule);
}

void CWeekScheduleView::clearItem()
{
    for (int i = 0; i < m_scheduleShowItem.count(); i++) {

        delete m_scheduleShowItem[i];
    }
    m_scheduleShowItem.clear();
}

void CWeekScheduleView::setMaxNum()
{
    m_MaxNum = m_DayHeight/(m_ScheduleHeight+1);
}

void CWeekScheduleView::mScheduleClear()
{
    for (int i = 0; i < m_MScheduleInfo.size(); ++i) {
        m_MScheduleInfo[i].clear();
    }
    m_MScheduleInfo.clear();
}

void CWeekScheduleView::sortAndFilter(QVector<MScheduleDateRangeInfo> &vMDaySchedule)
{
    QVector<QVector<bool> > scheduleFill;
    QVector<bool> scheduf;
    scheduf.fill(false,m_colum);
    scheduleFill.fill(scheduf,m_MaxNum);
    int postion = 0;
    int end = 0;
    mScheduleClear();

    for (int i = 0 ; i < vMDaySchedule.size(); ++i) {
        postion = static_cast<int>(beginDate.daysTo(vMDaySchedule.at(i).bdate));
        end     = static_cast<int>(beginDate.daysTo(vMDaySchedule.at(i).edate));
        int row = 0;
        int pos = postion;
        int count = 0;
        int scheduleRow = row;
        for (; postion<end+1; ++postion) {
            if (row == m_MaxNum) {
                if (m_ColumnScheduleCount[postion] >m_MaxNum) {
                    continue;
                }
                row =0;
                pos = postion;
            }
            while (row<m_MaxNum) {
                if (m_MScheduleInfo.size()<(row+1)) {
                    RowScheduleInfo ms;
                    m_MScheduleInfo.append(ms);
                }
                if (!scheduleFill[row][postion]) {
                    if ((m_ColumnScheduleCount[postion]>m_MaxNum) &&(row>=m_MaxNum-1)) {
                        scheduleFill[row][postion] = true;
                        if (pos !=postion) {
                            MScheduleDateRangeInfo scheduleInfo;
                            scheduleInfo.bdate = beginDate.addDays(pos);
                            scheduleInfo.edate = beginDate.addDays(postion -1);
                            scheduleInfo.state = false;
                            scheduleInfo.tData = vMDaySchedule.at(i).tData;
                            m_MScheduleInfo[row].append(scheduleInfo);
                        }

                        MScheduleDateRangeInfo info;
                        info.bdate = beginDate.addDays(postion);
                        info.edate = info.bdate;
                        info.num = m_ColumnScheduleCount[postion] -m_MaxNum +1;
                        info.state = true;
                        m_MScheduleInfo[row].append(info);

                        pos = postion +1;
                        if (pos<7 && pos <end +1) {
                            if (m_ColumnScheduleCount[pos]<row+1) {
                                row =m_ColumnScheduleCount[pos]-1;
                            }
                        } else {
                            row = 0;
                        }
                        count = 0;
                    } else {
                        scheduleFill[row][postion] = true;
                        ++count;
                        scheduleRow = row;
                    }
                    break;
                } else {
                    ++row;
                }
            }
        }
        if (pos>6||count==0) {

        } else {
            MScheduleDateRangeInfo scheduleInfo;
            scheduleInfo.bdate = beginDate.addDays(pos);
            scheduleInfo.edate = beginDate.addDays(postion -1);
            scheduleInfo.state = false;
            scheduleInfo.tData = vMDaySchedule.at(i).tData;
            m_MScheduleInfo[scheduleRow].append(scheduleInfo);
        }
    }
}
