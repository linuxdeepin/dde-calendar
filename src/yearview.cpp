
/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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

#include "calendardbus.h"

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include "yearview.h"
#include <QSpacerItem>
#include <DPalette>
#include <QLocale>
#include <DHiDPIHelper>
#include "scheduledatamanage.h"
#include "yearschceduleview.h"
#include <QDesktopWidget>
#include <DApplication>

#include <QMouseEvent>

#include <DArrowRectangle>
DGUI_USE_NAMESPACE
CYearSchceduleOutView      *CYearView::m_Scheduleview = nullptr;
CMonthDayRect               *CMonthDayRect::m_CurrentRect =nullptr;
qreal                       CMonthDayRect::m_DevicePixelRatio = 0;
QColor                      CMonthDayRect::m_ceventColor("#FF5D00");
QColor                      CMonthDayRect::m_notCurrentTextColor = "#b2b2b2";
CMonthDayRect::CellColor    CMonthDayRect::m_currentColor;
QColor                      CMonthDayRect::m_defaultTextColor;
QColor                      CMonthDayRect::m_selectedTextColor;


CYearView::CYearView(QWidget *parent) : CustomFrame(parent)
{
//    m_dayNumFont.setFamily("Helvetica");

    setMouseTracking(true);
    //m_dayNumFont.setWeight(QFont::Light);

    //add separator line
    m_currentMouth = new CustomFrame();
    m_currentMouth->setFixedHeight(24);
    m_currentMouth->setContentsMargins(0, 0, 0, 0);
    //m_currentMouth->setMinimumWidth(100);
    m_currentMouth->setTextAlign(Qt::AlignLeft);
    //m_currentMouth->setStyleSheet("border: 1px solid rgba(0, 0, 0, 0.05);");

//    m_momthFont.setFamily("SourceHanSansSC");
    m_momthFont.setWeight(QFont::Medium);
    m_momthFont.setPixelSize(16);
    m_currentMouth->setTextFont(m_momthFont);

    QHBoxLayout *separatorLineLayout = new QHBoxLayout;
    separatorLineLayout->setMargin(0);
    separatorLineLayout->setSpacing(0);
    separatorLineLayout->setContentsMargins(0, 0, 0, 0);
    //separatorLineLayout->addSpacing(13);
    separatorLineLayout->addWidget(m_currentMouth);
    separatorLineLayout->addStretch();
    // QSpacerItem *t_spaceitem = new QSpacerItem(30, 32, QSizePolicy::Expanding, QSizePolicy::Fixed);
    //separatorLineLayout->addSpacerItem(t_spaceitem);

    m_currentMouth->show();
    m_currentMouth->installEventFilter(this);


    m_monthView = new CYearMonthView(this);
    connect(m_monthView,
            &CYearMonthView::signalPressDate,
            this,
            &CYearView::slotPressClickDate);
    connect(m_monthView,
            &CYearMonthView::signalDoubleClickDate,
            this,
            &CYearView::slotDoubleClickDate);

    m_hhLayout = new QVBoxLayout;
    m_hhLayout->addLayout(separatorLineLayout);
    m_hhLayout->addWidget(m_monthView);
    m_hhLayout->setMargin(0);
    m_hhLayout->setSpacing(0);
    m_hhLayout->setContentsMargins(13, 10, 10, 10);

    setLayout(m_hhLayout);
    createYearSchceduleView(parent);
}

void CYearView::updateInfoWIndow(bool flag)
{
    if (flag) {
        m_Scheduleview->show(0, 0);
    } else {
        m_Scheduleview->hide();
    }
}

void CYearView::slotupdateSchedule(const int id)
{
    setCurrentDate(m_currentDate, id);
}

void CYearView::slotDoubleClickDate(const QDate &date)
{
    if (date.year()<1900)
        return;
    SchceduleViewHide();
    emit signaldoubleclickDate(date);
}

void CYearView::slotPressClickDate(const QDate &date)
{
    if (date.year()<1900)
        return;
    emit signalcurrentDateChanged(date);
    emit signalHideInfo();
    emit signalSelectInfo(true);
    m_Scheduleview->setCurrentDate(date);
    for (int i = 0; i < m_DateRangeInfo.size(); ++i) {
        if (m_DateRangeInfo.at(i).date ==date) {
            m_Scheduleview->setData(m_DateRangeInfo[i].vData);
        }
    }
    QPoint pos22 = QCursor::pos();
    QDesktopWidget *w = QApplication::desktop();
    m_Scheduleview->showWindow();

    if (pos22.x() + 10 + m_Scheduleview->width() < w->width()) {
        m_Scheduleview->setArrowDirection(DArrowRectangle::ArrowLeft);
        m_Scheduleview->show(pos22.x() + 10, pos22.y());
    } else {
        m_Scheduleview->adjustPosition(true);
        m_Scheduleview->setArrowDirection(DArrowRectangle::ArrowRight);
        m_Scheduleview->show(pos22.x() - 10, pos22.y());
    }
}

void CYearView::setFirstWeekday(int weekday)
{
    m_firstWeekDay = weekday;
    updateDate();
}


void CYearView::setTheMe(int type)
{
    m_themetype = type;
    if (type == 0 || type == 1) {
        DPalette bpa = palette();
        bpa.setColor(DPalette::Background, Qt::white);
        setPalette(bpa);
        setBackgroundRole(DPalette::Background);
        m_currentMouth->setTextColor( QColor("#CF0059"));

        m_bnormalColor = "#FFFFFF";
        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0081FF";
        m_currentDayTextColor = "#0081FF";
        m_weekendsTextColor = Qt::black;
        m_festivalTextColor = Qt::black;

    } else if (type == 2) {
        m_bnormalColor = "#FFFFFF";
        m_bnormalColor.setAlphaF(0.05);
        m_currentMouth->setTextColor( QColor("#BF1D63"));

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0059D2";


        m_currentDayTextColor = "#0059D2";
        m_weekendsTextColor = Qt::black;

        m_festivalTextColor = Qt::black;

    }
    m_monthView->setTheMe(type);
    QColor monthcolor = Qt::white;
    monthcolor.setAlphaF(0);
    m_currentMouth->setBColor(monthcolor);
    m_Scheduleview->setTheMe(type);
}


void CYearView::SchceduleViewHide()
{
    m_Scheduleview->hide();
    m_Scheduleview->clearData();
}

void CYearView::getInfoAndSetLineFlag()
{
    CScheduleDataManage *tdataManage = CScheduleDataManage::getScheduleDataManage();
    m_vlineflag.resize(42);
    m_vlineflag.fill(false);
    if (tdataManage->getGetAllYearScheduleInfo()->m_monthInfo.size()>0) {
        m_DateRangeInfo = tdataManage->getGetAllYearScheduleInfo()->m_monthInfo[m_currentDate.month()];
        if (m_DateRangeInfo.count() == 42) {
            for (int i = 0; i < 42; i++) {
                if (!m_DateRangeInfo.at(i).vData.isEmpty()) {
                    m_vlineflag[i] = true;
                }
            }
        }
    }
    m_monthView->setLintFlag(m_vlineflag);
    m_monthView->update();
}

void CYearView::setCurrentDate(const QDate date, int type)
{
    Q_UNUSED(type);
    qDebug() << "set current date " << date;

    //if (date == m_currentDate) {
    //   return;
    // }

    m_currentDate = date;
    QLocale locale;
    m_currentMouth->setTextStr(locale.monthName(date.month(), QLocale::ShortFormat));
    updateDate();
    m_monthView->setDate(m_days);
//    getInfoAndSetLineFlag();
}

void CYearView::setCellSelectable(bool selectable)
{
    if (selectable == m_cellSelectable)
        return;
    m_cellSelectable = selectable;
}

int CYearView::getDateIndex(const QDate &date) const
{
    for (int i = 0; i != 42; ++i)
        if (m_days[i] == date)
            return i;

    return 0;
}
bool CYearView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);
    if (cell == m_currentMouth) {
        if (e->type() == QEvent::MouseButtonDblClick) {
            emit signalselectMonth(m_currentDate);
        }
    }
    return false;
}
void CYearView::updateDate()
{
    const QDate firstDay(m_currentDate.year(), m_currentDate.month(), 1);
    int offset = firstDay.dayOfWeek() % 7 - m_firstWeekDay ;

    const int day = offset <0 ?offset +7:offset;
    const int currentIndex = day + m_currentDate.day() - 1;

    if (currentIndex < 0) {
        return;
    }
    for (int i(0); i != 42; ++i) {
        m_days[i] = firstDay.addDays(i - day);
    }
    update();
}

void CYearView::createYearSchceduleView(QWidget *parent)
{
    if (m_Scheduleview == nullptr) {
        m_Scheduleview = new CYearSchceduleOutView(parent);
        connect(m_Scheduleview, &CYearSchceduleOutView::signalsViewSelectDate, this, &CYearView::signalselectWeekwindow);
        connect(m_Scheduleview, &CYearSchceduleOutView::signalupdateschcedule, this, &CYearView::signalupdateschcedule);
    }
}

void CYearView::resizeEvent(QResizeEvent *event)
{
    int leftmagin = qFloor(width() * 0.06435 + 0.5);
    int rightmagin = leftmagin;
    int topmagin = qFloor(height() * 0.02955 + 0.5);
    int buttonmagin = qFloor(height() * 0.044 + 0.5);

    m_hhLayout->setContentsMargins(leftmagin, topmagin, rightmagin, buttonmagin);

    m_momthFont.setPixelSize(qFloor(16 + (height() - 159) / 16.75));
    m_currentMouth->setTextFont(m_momthFont);
    m_currentMouth->setFixedHeight(24 + (height() - 159) / 12);
    m_currentMouth->update();

    QFrame::resizeEvent(event);
}

void CYearView::mousePressEvent(QMouseEvent *event)
{
    if (m_selectFlag) return;
    if (event->button() == Qt::LeftButton) {
        emit signalHideInfo();
    }
    CustomFrame::mousePressEvent(event);
}


void CYearView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width() - 2 * m_borderframew;
    int labelheight = height() - 2 * m_borderframew;

    QPainter painter(this);
    if (1) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        painter.setBrush(QBrush(m_bnormalColor));
        painter.setPen(Qt::NoPen);
        QPainterPath painterPath;
        painterPath.moveTo(m_radius, m_borderframew);
        painterPath.arcTo(QRect(m_borderframew, m_borderframew, m_radius * 2, m_radius * 2), 90, 90);

        painterPath.lineTo(0, labelheight - m_radius);
        painterPath.arcTo(QRect(m_borderframew, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 180, 90);

        painterPath.lineTo(labelwidth - m_radius, labelheight);
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);

        painterPath.lineTo(labelwidth, m_radius);

        painterPath.arcTo(QRect(labelwidth - m_radius * 2, m_borderframew, m_radius * 2, m_radius * 2), 0, 90);

        painterPath.lineTo(m_radius, m_borderframew);
        painterPath.closeSubpath();
        painter.drawPath(painterPath);
        painter.restore();
    }
}

CYearMonthView::CYearMonthView(DWidget *parent)
    :DWidget (parent)
{
    for (int i = 0; i < 42; ++i) {
        CMonthDayRect *item = new CMonthDayRect();
        m_DayItem.append(item);
    }
    setMouseTracking(true);
    CMonthDayRect::setDevicePixelRatio(devicePixelRatioF());
}

CYearMonthView::~CYearMonthView()
{
    for (int i = 0; i <m_DayItem.size(); ++i) {
        delete m_DayItem[i];
    }
    m_DayItem.clear();
}

void CYearMonthView::setDate(const QDate date[42])
{
    if (date[0].day() !=1) {
        m_currentMonth = date[0].addMonths(1).month();
    } else {
        m_currentMonth = date[0].month();
    }
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setDate(date[i]);
        m_DayItem.at(i)->setIsCurrentMonth(date[i].month() == m_currentMonth);
    }

}

void CYearMonthView::setTheMe(int type)
{
    CMonthDayRect::setTheMe(type);
}

void CYearMonthView::setLintFlag(const QVector<bool> &lineFlag)
{
    m_vlineflag = lineFlag;
    if (m_vlineflag.size() == 42) {
        for (int i = 0; i < 42; i++) {
            m_DayItem.at(i)->setLineFlag(m_vlineflag.at(i));
        }
    }
}

void CYearMonthView::updateSize()
{
    qreal w = width()/7;
    qreal h = height()/6;
    QRectF rect ;
    int w_offset = 0;
    int h_offset = 0;
    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        h_offset = i / 7;
        w_offset = i % 7;
        rect.setRect(w*w_offset,
                     h*h_offset,
                     w,
                     h);
        m_DayItem.at(i)->setRect(rect);
    }
    update();
}

int CYearMonthView::getMousePosItem(const QPointF &pos)
{
    int res =-1;
    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        if (m_DayItem.at(i)->rect().contains(pos)) {
            res = i;
            break;
        }
    }
    return res;
}

void CYearMonthView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateSize();
}

void CYearMonthView::mousePressEvent(QMouseEvent *event)
{
    int itemindex = getMousePosItem(event->pos());
    if (!(itemindex<0)) {
        m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::CellPress);
        m_press = true;
        if ( event->button() ==Qt::LeftButton) {
            m_pressIndex = itemindex;
            emit signalPressDate(m_DayItem.at(itemindex)->getDate());
        }
    }
    update();
}

void CYearMonthView::mouseDoubleClickEvent(QMouseEvent *event)
{
    int itemindex = getMousePosItem(event->pos());
    if (!(itemindex<0)) {
        if (m_pressIndex == itemindex) {
            m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::CellPress);
            m_press = true;
            if ( event->button() ==Qt::LeftButton) {
                emit signalDoubleClickDate(m_DayItem.at(itemindex)->getDate());
            }
        }
    }
}

void CYearMonthView::mouseReleaseEvent(QMouseEvent *event)
{
    m_press = false;
    mouseMoveEvent(event);
}

void CYearMonthView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_press) {
        int itemindex = getMousePosItem(event->pos());
        if (!(itemindex<0)) {
            m_DayItem.at(itemindex)->setCellEvent(CMonthDayRect::Cellhover);
        }
        update();
    }

}

void CYearMonthView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing |QPainter::SmoothPixmapTransform);
    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        m_DayItem[i]->paintItem(&painter,m_DayItem[i]->rect());
    }
    painter.end();
}

void CYearMonthView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    CMonthDayRect::setCurrentRect(nullptr);
    update();
}


CMonthDayRect::CMonthDayRect()
    :m_rect(0,0,0,0)
{
    m_dayNumFont.setPixelSize(12);
    m_hightFont.setPixelSize(12);
}

void CMonthDayRect::setTheMe(int type)
{
    if (type ==2) {
        m_currentColor.hoverColor = "#FFFFFF";
        m_currentColor.hoverColor.setAlphaF(0.1);
        m_currentColor.pressColor = "#252525";
        m_currentColor.pressColor.setAlphaF(1);
        m_currentColor.normalColor = "#000000";
        m_currentColor.normalColor.setAlphaF(0.00);
        m_ceventColor = QColor(204, 77, 3);

        m_notCurrentTextColor = "#C0C6D4";
        m_notCurrentTextColor.setAlphaF(0.5);
        m_defaultTextColor = "#C0C6D4";
        m_selectedTextColor = Qt::white;

    } else if (type == 0 || type == 1) {
        m_currentColor.hoverColor = "#000000";
        m_currentColor.hoverColor.setAlphaF(0.05);
        m_currentColor.pressColor = "#000000";
        m_currentColor.pressColor.setAlphaF(0.2);
        m_currentColor.normalColor = "#FFFFFF";
        m_currentColor.normalColor.setAlphaF(1);
        m_ceventColor = QColor(255, 93, 0);

        m_selectedTextColor = Qt::white;
        m_defaultTextColor = Qt::black;
        m_notCurrentTextColor = "#b2b2b2";
    }


}

CMonthDayRect::~CMonthDayRect()
{

}

void CMonthDayRect::setDate(const QDate &date)
{
    m_date = date;
}

QDate CMonthDayRect::getDate() const
{
    return  m_date;
}

void CMonthDayRect::setCellEvent(const CMonthDayRect::CellEventType &type)
{
    m_cellEventType = type;
    m_CurrentRect = this;
}

void CMonthDayRect::setIsCurrentMonth(const bool isCurrMonth)
{
    m_isCurrentMonth = isCurrMonth;
}

QRectF CMonthDayRect::rect() const
{
    return  m_rect;
}

void CMonthDayRect::setRect(const QRectF &rect)
{
    m_rect = rect;
}

void CMonthDayRect::setRect(qreal x, qreal y, qreal w, qreal h)
{
    m_rect.setRect(x,y,w,h);
}

void CMonthDayRect::paintItem(QPainter *painter, const QRectF &rect)
{
    m_highColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    if (m_date.year()<1900)
        return;
    const bool isCurrentDay = m_date == QDate::currentDate() && m_isCurrentMonth;
    bool isSelectedCell  = false;
    if (isCurrentDay) {
        isSelectedCell = true;
    }


    const qreal r = rect.width() > rect.height() ? rect.height() * 0.9 : rect.width() * 0.9 ;
    int fontsize =  qRound(12 + (r - 18) *6/ 17);
    if (fontsize <12) {
        fontsize = 12;
    }
    m_dayNumFont.setPixelSize(fontsize);
    m_hightFont.setPixelSize(fontsize);
    const qreal x = rect.x()+(rect.width() - r) / 2;
    const qreal y = rect.y()+(rect.height() - r) / 2;
    QRectF fillRect = QRectF(x, y, r, r);

    QColor m_cellBackgroundColor;
    if (m_CurrentRect !=this) {
        m_cellEventType = CellNormal;
    }
    if (m_cellEventType == CellPress) {
        m_cellBackgroundColor = m_currentColor.pressColor;
    } else if (m_cellEventType == Cellhover) {
        m_cellBackgroundColor = m_currentColor.hoverColor;
    } else {
        m_cellBackgroundColor = m_currentColor.normalColor;
    }
    painter->setBrush(m_cellBackgroundColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(fillRect);

    bool highflag = false;
    if (m_isCurrentMonth) {
        highflag = CScheduleDataManage::getScheduleDataManage()->getSearchResult(m_date);
    }
    if (highflag) {
        painter->setBrush(QBrush(m_highColor));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(fillRect);
        painter->setPen(Qt::SolidLine);
        painter->setPen(m_highTextColor);
        painter->setFont(m_hightFont);
        painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_date.day()));

    } else {
        // draw selected cell background circle
        if (isSelectedCell) {
//            QRectF pixmapRect(QRectF(fillRect.x()-3,
//                                     fillRect.y(),
//                                     fillRect.width()+6,
//                                     fillRect.height()+6));
//            QPixmap pixmap;
//            if (m_themetype == 2)
//                pixmap = DHiDPIHelper::loadNxPixmap(":/resources/icon/darkchoose30X30_checked .svg");
//            else {
//                pixmap = DHiDPIHelper::loadNxPixmap(":/resources/icon/choose30X30_checked .svg");
//            }
//            pixmap.setDevicePixelRatio(m_DevicePixelRatio);
//            painter->save();
//            painter->setRenderHint(QPainter::Antialiasing);
//            painter->setRenderHint(QPainter::HighQualityAntialiasing);
//            painter->setRenderHint(QPainter::SmoothPixmapTransform);
//            painter->drawPixmap(pixmapRect.toRect(), pixmap);
//            painter->restore();
            painter->setBrush(QBrush(m_highColor));
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(fillRect);
        }
        painter->setPen(Qt::SolidLine);

        // draw text of day
        if (isSelectedCell) {
            painter->setPen(m_selectedTextColor);
        } else if (isCurrentDay) {
            painter->setPen(m_currentDayTextColor);
        } else {
            if (m_isCurrentMonth)
                painter->setPen(m_defaultTextColor);
            else
                painter->setPen(m_notCurrentTextColor);
        }
        painter->setFont(m_dayNumFont);
        painter->drawText(fillRect, Qt::AlignCenter, QString::number(m_date.day()));
        if (m_vlineflag) {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::HighQualityAntialiasing);
            painter->setRenderHint(QPainter::SmoothPixmapTransform);
            QPen pen;
            pen.setWidth(2);
            pen.setColor(m_ceventColor);
            painter->setPen(pen);
            painter->setBrush(QBrush(m_ceventColor));
            painter->setPen(Qt::NoPen);
            qreal r = rect.width() * (4 / 25);
            if (r < 4) {
                r = 4;
            } else if ( r > 7) {
                r = 7;
            }
            painter->drawEllipse(QRectF(rect.width() - r+rect.x(),
                                        rect.y(),
                                        r,
                                        r));
            painter->restore();
        }
    }
}

void CMonthDayRect::setLineFlag(const bool flag)
{
    m_vlineflag = flag;
}

void CMonthDayRect::setDevicePixelRatio(const qreal pixel)
{
    m_DevicePixelRatio = pixel;
}

void CMonthDayRect::setCurrentRect(CMonthDayRect *currrect)
{
    m_CurrentRect = currrect;
}

