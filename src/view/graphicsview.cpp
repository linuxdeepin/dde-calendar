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
#include "graphicsview.h"
#include "scheduleitem.h"
#include "schedulecoormanage.h"
#include "schceduledlg.h"
#include "scheduledatamanage.h"
#include "schcedulectrldlg.h"
#include "myschceduleview.h"
#include "constants.h"

#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>

#include <QMimeData>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QShortcut>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QWheelEvent>

DGUI_USE_NAMESPACE
CGraphicsView::CGraphicsView(QWidget *parent, int viewType)
    : DragInfoGraphicsView(parent)
    , m_viewType(viewType)
{
    m_coorManage = new CScheduleCoorManage;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_dayInterval = width();
    m_timeInterval = height() / 24.0;

    m_Scene->setSceneRect(0, 0, width(), height());
    m_LRPen.setColor(QColor(255, 255, 255));
    m_LRPen.setStyle(Qt::SolidLine);
    m_TBPen.setColor(QColor(255, 255, 255));
    m_TBPen.setStyle(Qt::SolidLine);
    m_LRFlag = true;
    m_TBFlag = true;
    m_margins = QMargins(0, 0, 0, 0);

    setLineWidth(0);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(scrollBarValueChangedSlot()));
    m_timer->start(60000);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarValueChangedSlot()));
    connect(this->verticalScrollBar(),
            &QScrollBar::sliderPressed,
            this,
            &CGraphicsView::slotScrollBar);
}

CGraphicsView::~CGraphicsView()
{
    m_timer->stop();
    m_timer->deleteLater();
    clearSchdule();

    delete m_coorManage;
    m_coorManage = nullptr;
}

void CGraphicsView::setMargins(int left, int top, int right, int bottom)
{
    Q_UNUSED(top)
    m_margins = QMargins(left, 0, right, bottom);
    setViewportMargins(m_margins);
}

void CGraphicsView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_weekcolor = "#00429A";
        m_weekcolor.setAlphaF(0.05);
        QColor linecolor = "#000000";
        linecolor.setAlphaF(0.1);
        m_LRPen.setColor(linecolor);
        m_TBPen.setColor(linecolor);
        m_LRPen.setStyle(Qt::SolidLine);
        m_TBPen.setStyle(Qt::SolidLine);
    } else if (type == 2) {
        m_weekcolor = "#4F9BFF";
        m_weekcolor.setAlphaF(0.1);
        QColor linecolor = "#000000";
        linecolor.setAlphaF(0.1);
        m_LRPen.setColor(linecolor);
        m_TBPen.setColor(linecolor);
    }
    scene()->update();
    update();
}

void CGraphicsView::slotCreate(const QDateTime &date)
{
    emit signalViewtransparentFrame(1);
    CSchceduleDlg dlg(1, this);
    dlg.setDate(date);

    if (dlg.exec() == DDialog::Accepted) {
        emit signalsUpdateShcedule();
    }
    emit signalViewtransparentFrame(0);
}

bool CGraphicsView::MeetCreationConditions(const QDateTime &date)
{
    return  qAbs(date.daysTo(m_PressDate)<7);
}

void CGraphicsView::updateHigh()
{
    scene()->update();
    update();
}

void CGraphicsView::setRange( int w, int h, QDate begindate, QDate enddate, int rightmagin)
{
    m_MoveDate.setDate(begindate.addMonths(-2));
    m_beginDate = begindate;
    m_endDate = enddate;
    w = w - rightmagin -2;
    m_Scene->setSceneRect(0, 0, w, h);
    m_coorManage->setRange(w, h, begindate, enddate, rightmagin);
    m_rightmagin = rightmagin;
    qint64 totalDay = begindate.daysTo(enddate) + 1;
    m_dayInterval = w * 1.0 / totalDay;
    m_timeInterval = h / 24.0;
    m_totalDay = totalDay;

    if (m_viewType ==0) {
        int viewWidth = viewport()->width();
        int viewHeight = viewport()->height();
        QPoint newCenter(viewWidth / 2,  viewHeight / 2 - 2000);

        centerOnScene(mapToScene(newCenter));
    }
}

void CGraphicsView::setRange(QDate begin, QDate end)
{
    m_MoveDate.setDate(begin.addMonths(-2));
    m_beginDate = begin;
    m_endDate = end;
    getCoorManage()->setDateRange(begin, end);
}

void CGraphicsView::setInfo(const QVector<ScheduleDtailInfo> &info)
{
    m_scheduleInfo = info;
}

bool MScheduleTimeThan(const ScheduleDtailInfo &s1, const ScheduleDtailInfo &s2)
{
    if (s1.beginDateTime.date().daysTo(s1.endDateTime.date())==
            s2.beginDateTime.date().daysTo(s2.endDateTime.date())) {
        if (s1.beginDateTime == s2.beginDateTime) {
            return s1.beginDateTime.secsTo(s1.endDateTime) > s2.beginDateTime.secsTo(s2.endDateTime);
        } else {
            return s1.beginDateTime < s2.beginDateTime;
        }
    } else {
        return s1.beginDateTime.date().daysTo(s1.endDateTime.date())>s2.beginDateTime.date().daysTo(s2.endDateTime.date());
    }
}

void CGraphicsView::upDateInfoShow(const CGraphicsView::DragStatus &status, const ScheduleDtailInfo &info)
{
    clearSchdule();
    QVector<ScheduleDtailInfo> vListData;
    vListData = m_scheduleInfo;

    switch (status) {
    case NONE:
        Q_UNUSED(info);
        break;
    case ChangeBegin:
    case ChangeEnd: {
        int index = vListData.indexOf(info);
        if (index >=0)
            vListData[index] = info;
    }
    break;
    case ChangeWhole:
        vListData.append(info);
        break;
    case IsCreate:
        vListData.append(info);
        break;
    }
    QMap<QDate,QVector<ScheduleDtailInfo> > m_InfoMap;
    QDate currentDate;
    qint64 count = m_beginDate.daysTo(m_endDate);
    qint64 beginoffset = 0, endoffset = 0;
    QVector<ScheduleDtailInfo> currentInfo;

    for (int i = 0; i <= count; ++i) {
        currentDate = m_beginDate.addDays(i);
        currentInfo.clear();

        for (int j = 0; j <vListData.size(); ++j) {
            beginoffset = vListData.at(j).beginDateTime.date().daysTo(currentDate);
            endoffset = currentDate.daysTo(vListData.at(j).endDateTime.date());

            if (beginoffset<0 || endoffset <0) {
                continue;
            }
            if (vListData.at(j).endDateTime.date() == currentDate &&
                    vListData.at(j).beginDateTime.daysTo(vListData.at(j).endDateTime)>0 &&
                    vListData.at(j).endDateTime.time() ==QTime(0,0,0)) {
                continue;
            }
            currentInfo.append(vListData.at(j));
        }
        std::sort(currentInfo.begin(), currentInfo.end(), MScheduleTimeThan);
        if (currentInfo.size()>0) {
            m_InfoMap[currentDate] = currentInfo;
            QList<ScheduleclassificationInfo> info;
            scheduleClassificationType(currentInfo,info);

            for (int m = 0; m < info.count(); m++) {
                int tnum = info.at(m).vData.count();

                if (m_viewType == 0) {
                    if (tnum > m_sMaxNum) {
                        tnum = m_sMaxNum;
                        for (int n = 0; n < tnum - 1; n++) {
                            addSchduleItem(info.at(m).vData.at(n), currentDate, n + 1,
                                           tnum, 0, m_viewType, m_sMaxNum);
                        }
                        ScheduleDtailInfo tdetaliinfo = info.at(m).vData.at(tnum - 2);
                        tdetaliinfo.titleName = "...";
                        tdetaliinfo.type.ID = 3;
                        addSchduleItem(tdetaliinfo, currentDate, tnum, tnum, 1,
                                       m_viewType, m_sMaxNum);
                    } else {
                        for (int n = 0; n < tnum; n++) {
                            addSchduleItem(info.at(m).vData.at(n), currentDate, n + 1,
                                           tnum, 0, m_viewType, m_sMaxNum);
                        }
                    }
                } else {
                    for (int n = 0; n < tnum; n++) {
                        addSchduleItem(info.at(m).vData.at(n), currentDate, n + 1,
                                       tnum, 0, m_viewType, m_sMaxNum);
                    }
                }
            }
        }
    }
}

QDateTime CGraphicsView::getPosDate(const QPoint &p)
{
    return TimeRounding(m_coorManage->getDate(mapToScene(p)));
}

void CGraphicsView::ShowSchedule(DragInfoItem *infoitem)
{
    CScheduleItem *scheduleitem = dynamic_cast<CScheduleItem *>(infoitem);
    if (scheduleitem->getType() == 1)
        return;
    DragInfoGraphicsView::ShowSchedule(infoitem);
}

void CGraphicsView::MoveInfoProcess(ScheduleDtailInfo &info, const QPointF &pos)
{
    Q_UNUSED(pos);

    if (!info.allday) {
        qint64 offset = m_PressDate.secsTo(m_MoveDate);
        info.beginDateTime = info.beginDateTime.addSecs(offset);
        info.endDateTime    = info.endDateTime.addSecs(offset);
    } else {
        info.allday = false;
        info.remind = false;
        info.beginDateTime = m_MoveDate;
        info.endDateTime = m_MoveDate.addSecs(3600);
    }
    info.IsMoveInfo = true;
    upDateInfoShow(ChangeWhole,info);
}

void CGraphicsView::addSchduleItem( const ScheduleDtailInfo &info, QDate date, int index, int totalNum, int type, int viewtype, int maxnum)
{
    CScheduleItem *item = new CScheduleItem(
        m_coorManage->getDrawRegion(date, info.beginDateTime,
                                    info.endDateTime, index, totalNum, maxnum,
                                    viewtype), nullptr, type);
    m_Scene->addItem(item);
    item->setData(info, date,  totalNum);
    m_vScheduleItem.append(item);

}

void CGraphicsView::deleteSchduleItem( CScheduleItem *item )
{
    int id = item->getData().id;

    for (int i = 0; i < m_vScheduleItem.size(); i++) {
        if (m_vScheduleItem[i]->getData().id == id) {
            m_vScheduleItem.remove(i);
            m_Scene->removeItem(item);
            delete m_vScheduleItem[i];
            m_vScheduleItem[i] = nullptr;
            i--;
        }
    }
}

void CGraphicsView::setSelectSchedule(const ScheduleDtailInfo &info)
{
    setTime(info.beginDateTime.time());

    for (int i = 0 ; i < m_vScheduleItem.size(); ++i) {
        if (m_vScheduleItem.at(i)->getType() == 1)
            continue;
        if (m_vScheduleItem.at(i)->hasSelectSchedule(info)) {
            m_vScheduleItem.at(i)->setStartValue(0);
            m_vScheduleItem.at(i)->setEndValue(10);
            m_vScheduleItem.at(i)->startAnimation();
        }
    }
}

void CGraphicsView::clearSchdule()
{
    for (int i = 0; i < m_vScheduleItem.size(); i++) {
        m_Scene->removeItem(m_vScheduleItem.at(i));
        delete m_vScheduleItem[i];
        m_vScheduleItem[i] = nullptr;
    }
    m_vScheduleItem.clear();
    m_updateDflag = true;
}

void CGraphicsView::scheduleClassificationType(QVector<ScheduleDtailInfo> &scheduleInfolist, QList<ScheduleclassificationInfo> &info)
{
    QVector<ScheduleDtailInfo> schedulelist = scheduleInfolist;
    if (schedulelist.isEmpty())
        return;

    info.clear();
    std::sort(schedulelist.begin(), schedulelist.end(), MScheduleTimeThan);
    QVector<int> containIndex;

    for (int k = 0; k < schedulelist.count(); k++) {
        QDateTime endTime = schedulelist.at(k).endDateTime;
        QDateTime begTime = schedulelist.at(k).beginDateTime;

        if (begTime.date().daysTo(endTime.date())==0 && begTime.time().secsTo(endTime.time())<m_minTime) {
            endTime = begTime.addSecs(m_minTime);
        }
        if (endTime.time().hour()==0&&
                endTime.time().second()==0) {
            endTime = endTime.addSecs(-1);
        }
        containIndex.clear();

        for (int i = 0; i < info.count(); i++) {
            if ((schedulelist.at(k).beginDateTime >= info.at(i).begindate &&
                    schedulelist.at(k).beginDateTime <= info.at(i).enddate) ||
                    (endTime >= info.at(i).begindate &&
                     endTime <= info.at(i).enddate) ) {
                containIndex.append(i);
            }
        }
        if (containIndex.count()==0) {
            ScheduleclassificationInfo firstschedule;
            firstschedule.begindate = schedulelist.at(k).beginDateTime;
            firstschedule.enddate = endTime;
            firstschedule.vData.append(schedulelist.at(k));
            info.append(firstschedule);
        } else {
            ScheduleclassificationInfo &scheduleInfo = info[containIndex.at(0)];
            int index = 0;

            for (int i = 1; i < containIndex.count(); ++i) {
                index = containIndex.at(i);
                if (info.at(index).begindate < scheduleInfo.begindate)
                    scheduleInfo.begindate = info.at(index).begindate;
                if (info.at(index).enddate>scheduleInfo.enddate)
                    scheduleInfo.enddate = info.at(index).enddate;
                scheduleInfo.vData.append(info.at(index).vData);
            }
            for (int i = containIndex.count() -1; i >0; --i) {
                info.removeAt(containIndex.at(i));
            }
            if (schedulelist.at(k).beginDateTime < scheduleInfo.begindate)
                scheduleInfo.begindate = schedulelist.at(k).beginDateTime;
            if (endTime>scheduleInfo.enddate)
                scheduleInfo.enddate = endTime;
            scheduleInfo.vData.append(schedulelist.at(k));
        }
    }
}

void CGraphicsView::mouseDoubleClickEvent( QMouseEvent *event )
{
    emit signalScheduleShow(false);
    DGraphicsView::mouseDoubleClickEvent(event);
    CScheduleItem *item = dynamic_cast<CScheduleItem *>(itemAt(event->pos()));

    if (item == nullptr) {
        QPointF senceposs = mapToScene(event->pos());
        emit signalViewtransparentFrame(1);
        CSchceduleDlg dlg(1, this);
        QDateTime tDatatime = m_coorManage->getDate(senceposs);
        dlg.setDate(tDatatime);
        if (dlg.exec() == DDialog::Accepted) {
            emit signalsUpdateShcedule();
        }
        emit signalViewtransparentFrame(0);
        return;
    }
    if (item->getType() == 1) {
        emit signalsCurrentScheduleDate(item->getdate());
        return;
    }
    emit signalViewtransparentFrame(1);
    m_updateDflag  = false;
    CMySchceduleView dlg(item->getData(), this);
    connect(&dlg, &CMySchceduleView::signalsEditorDelete, this, &CGraphicsView::slotDoubleEvent);
    dlg.exec();
    emit signalViewtransparentFrame(0);
    disconnect(&dlg, &CMySchceduleView::signalsEditorDelete, this, &CGraphicsView::slotDoubleEvent);

    if (item == nullptr) {
        return;
    }
}

void CGraphicsView::mousePressEvent(QMouseEvent *event)
{
    CScheduleItem *item = dynamic_cast<CScheduleItem *>(itemAt(event->pos()));

    if (item !=nullptr &&item->getType() ==1) {
        emit signalScheduleShow(false);
        return;
    }
    DragInfoGraphicsView::mousePressEvent(event);
}

void CGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    CScheduleItem *item = dynamic_cast<CScheduleItem *>(itemAt(event->pos()));

    if (item !=nullptr &&item->getType() ==1) {
        setCursor(Qt::ArrowCursor);
        DGraphicsView::mouseMoveEvent(event);
        return;
    }
    DragInfoGraphicsView::mouseMoveEvent(event);
}
void CGraphicsView::slotDoubleEvent(int type)
{
    Q_UNUSED(type);
    m_updateDflag  = true;
    emit signalsUpdateShcedule();
}

void CGraphicsView::slotDeleteItem()
{
    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().type.ID <0) {
        return;
    }
    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().type.ID != DDECalendar::FestivalTypeID) {
        DeleteItem(CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo());
    }
    CScheduleDataManage::getScheduleDataManage()->setPressSelectInfo(ScheduleDtailInfo());
}


void CGraphicsView::slotScrollBar()
{
    emit signalScheduleShow(false);
}

void CGraphicsView::slotUpdateScene()
{
    this->scene()->update();
}

#ifndef QT_NO_WHEELEVENT
/************************************************************************
Function:       wheelEvent()
Description:    鼠标滚轮事件
Input:          event 滚轮事件
Output:         无
Return:         无
Others:         无
************************************************************************/
void CGraphicsView::wheelEvent( QWheelEvent *event )
{
    emit signalScheduleShow(false);
    int test = event -> delta();
    int viewWidth = viewport()->width();
    int viewHeight = viewport()->height();
    QPoint newCenter(viewWidth / 2,  viewHeight / 2 - test);
    QPointF centerpos = mapToScene(newCenter);
    centerOnScene(centerpos);
}
#endif


/************************************************************************
Function:       resizeEvent()
Description:    窗口大小改变事件
Input:          event 窗口大小改变事件
Output:         无
Return:         无
Others:         无
************************************************************************/
void CGraphicsView::resizeEvent( QResizeEvent *event )
{
    scrollBarValueChangedSlot();
    QGraphicsView::resizeEvent(event);
    viewport()->update();
    update();
}

/************************************************************************
Function:       paintEvent()
Description:    绘制事件
Input:          event 事件
Output:         无
Return:         无
Others:         无
************************************************************************/
void CGraphicsView::paintEvent(QPaintEvent *event)
{
    QPainter t_painter(viewport());
    //绘制背景
    paintBackground(t_painter);
    int t_width = viewport()->width()  + 2;
    //绘制水平线
    if (m_LRFlag) {
        if (m_cuttrnttimetype == 0) {
            t_painter.save();
            t_painter.setPen(m_LRPen);

            for (int i = 0; i < m_vLRLarge.size(); ++i)
                t_painter.drawLine(QPoint(0, m_vLRLarge[i] - 1), QPoint(t_width, m_vLRLarge[i] - 1));
            t_painter.restore();
        } else {
            t_painter.save();
            t_painter.setPen(m_LRPen);

            for (int i = 0; i < m_vLRLarge.size() - 1; ++i)
                t_painter.drawLine(QPoint(0, m_vLRLarge[i] - 1), QPoint(t_width, m_vLRLarge[i] - 1));
            t_painter.restore();

            if (m_viewType == 1 && m_coorManage->getBegindate() == QDate::currentDate()) {
                t_painter.save();
                QPen pen = m_LRPen;
                pen.setColor(m_currenttimecolor);
                t_painter.setPen(pen);
                int index = m_vLRLarge.count() - 1;
                t_painter.drawLine(QPoint(0, m_vLRLarge[index] - 1), QPoint(t_width, m_vLRLarge[index] - 1));
                t_painter.restore();
            }

        }
    }
    QGraphicsView::paintEvent(event);
}

void CGraphicsView::paintBackground(QPainter &painter)
{
    // 绘制Rect的宽度
    const int t_width = viewport()->width() - 2;
    // 需要处理的天数
    const qint64 m_TotalDay = m_beginDate.daysTo(m_endDate) + 1;
    // 左边距
    const int m_leftMagin = 0;
    // 每天的宽度
    const qreal intenval = 1.0 * (t_width - m_leftMagin) / m_TotalDay;
    // 每天X坐标点偏移
    const qreal XPointOffset = 1.5;
    // 分割线颜色
    QColor m_linecolor = "#000000";
    m_linecolor.setAlphaF(0.1);
    if (m_TotalDay > 1) {
        painter.save();
        painter.setPen(Qt::SolidLine);
        painter.setPen(m_linecolor);
        //绘制分割线
        for (int i = 1; i < 7; ++i) {
            painter.drawLine(QPointF(i * intenval + m_leftMagin + XPointOffset, 1),
                             QPointF(i * intenval + m_leftMagin + XPointOffset, this->height()));
        }
        painter.restore();
        painter.save();
        //绘制周六周日背景色
        painter.setBrush(m_weekcolor);
        painter.setPen(Qt::NoPen);
        painter.setRenderHint(QPainter::Antialiasing);
        for (int i = 0; i != 7; ++i) {
            int d = m_beginDate.addDays(i).dayOfWeek();
            if (d == 7 || d == 6) {
                painter.drawRect(
                    QRectF(m_leftMagin + i * intenval + XPointOffset, 0, intenval, this->height()));
            }
        }
        painter.restore();
    }
}

void CGraphicsView::scrollBarValueChangedSlot()
{
    emit signalScheduleShow(false);
    QMutexLocker locker(&m_Mutex);
    int viewHeight = viewport()->height();
    m_vLRLarge.clear();
    m_vTBLarge.clear();
    QPointF leftToprealPos = mapToScene(QPoint(0, 0));
    QPointF leftBttomrealPos = mapToScene(QPoint(0, viewHeight));

    for (qreal i = m_dayInterval; i < scene()->width(); i = i + m_dayInterval) {
        m_vTBLarge.append(qFloor(i));
    }

    qreal beginpos = static_cast<qreal>(qFloor(leftToprealPos.y() / m_timeInterval) * m_timeInterval);

    if (beginpos < leftToprealPos.y()) {
        beginpos = (beginpos / m_timeInterval + 1) * m_timeInterval ;
    }

    QVector<int> vHours;

    for (qreal i = beginpos; i < leftBttomrealPos.y(); i = i + m_timeInterval) {
        QPoint point = mapFromScene(leftBttomrealPos.x(), i);
        m_vLRLarge.append(point.y());
        vHours.append(qFloor(i / m_timeInterval + 0.5));
    }

    qreal currentTime =  static_cast<qreal>(m_coorManage->getHeight(QTime::currentTime()));

    if (currentTime > beginpos && currentTime < leftBttomrealPos.y()) {
        m_cuttrnttimetype = 1;
        QPoint point = mapFromScene(leftBttomrealPos.x(), currentTime);
        m_vLRLarge.append(point.y());
        vHours.append(qFloor(currentTime / m_timeInterval + 0.5));
        emit signalsPosHours(m_vLRLarge, vHours, m_cuttrnttimetype);
    } else {
        m_cuttrnttimetype = 0;
        emit signalsPosHours(m_vLRLarge, vHours, m_cuttrnttimetype);
    }
    scene()->update();
    update();
}

int CGraphicsView::checkDay(int weekday)
{
    if (weekday <= 0)
        return weekday += 7;

    if (weekday > 7)
        return weekday -= 7;

    return weekday;
}


CGraphicsView::PosInItem CGraphicsView::getPosInItem(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    QPointF itemPos = QPointF(scenePos.x()-itemRect.x(),
                              scenePos.y()-itemRect.y());
    qreal bottomy = itemRect.height()- itemPos.y();

    if (itemPos.y()<5) {
        return TOP;
    }
    if (bottomy <5) {
        return BOTTOM;
    }

    return MIDDLE;
}

ScheduleDtailInfo CGraphicsView::getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate)
{
    ScheduleDtailInfo info;

    if (beginDate.secsTo(endDate)>0) {
        info.beginDateTime = beginDate;

        if (beginDate.secsTo(endDate) < DDECalendar::ThirtyMinutesWithSec) {
            info.endDateTime = beginDate.addSecs(DDECalendar::ThirtyMinutesWithSec);
        } else {
            info.endDateTime = endDate;
        }
    } else {
        if (endDate.secsTo(beginDate) < DDECalendar::ThirtyMinutesWithSec) {
            info.beginDateTime = beginDate.addSecs(-DDECalendar::ThirtyMinutesWithSec);
        } else {
            info.beginDateTime = endDate;
        }
        info.endDateTime = beginDate;
    }
    info.titleName = tr("New Event");
    info.allday = false;
    info.remind = true;
    info.id = 0;
    info.remindData.n = 1;
    info.remindData.time = QTime(9, 0);
    info.RecurID = 0;
    CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->GetType(
        1, info.type);
    info.rpeat = 0;

    return info;
}

bool CGraphicsView::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
{
    return !(qAbs(timeFirst.secsTo(timeSecond))>100);
}

bool CGraphicsView::JudgeIsCreate(const QPointF &pos)
{
    return qAbs(pos.x()-m_PressPos.x())>20 ||
           qAbs(m_PressDate.secsTo(m_coorManage->getDate(mapToScene(pos.toPoint()))))>300;
}

void CGraphicsView::RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos)
{
    Q_UNUSED(listItem);
    m_rightMenu->clear();
    m_rightMenu->addAction(m_createAction);
    QPointF senceposs = mapToScene(pos);
    m_createDate =m_coorManage->getDate(senceposs);
    m_rightMenu->exec(QCursor::pos());
}

QDateTime CGraphicsView::getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
{
    return moveDateTime.secsTo(m_InfoEndTime) < DDECalendar::ThirtyMinutesWithSec ? m_InfoEndTime.addSecs(-DDECalendar::ThirtyMinutesWithSec) : moveDateTime;
}

QDateTime CGraphicsView::getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
{
    return m_InfoBeginTime.secsTo(moveDateTime) < DDECalendar::ThirtyMinutesWithSec ? m_InfoBeginTime.addSecs(DDECalendar::ThirtyMinutesWithSec) : moveDateTime;
}

QDateTime CGraphicsView::TimeRounding(const QDateTime &time)
{
    int hours = time.time().hour();
    int minnutes = 0;
    minnutes = time.time().minute() / 15;

    return QDateTime(time.date(),QTime(hours,minnutes*15,0));
}

void CGraphicsView::centerOnScene(const QPointF &pos)
{
    // view 根据鼠标下的点作为锚点来定位 scene
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    centerOn(pos);
    // scene 在 view 的中心点作为锚点
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    scrollBarValueChangedSlot();
    setSceneHeightScale(pos);
}

void CGraphicsView::setSceneHeightScale(const QPointF &pos)
{
    m_sceneHeightScale = pos.y()/this->scene()->height();
}

void CGraphicsView::keepCenterOnScene()
{
    QPointF pos;
    pos.setX(this->viewport()->width()/2);
    pos.setY(this->scene()->height()*m_sceneHeightScale);
    centerOnScene(pos);
}

void CGraphicsView::getLargeScaleFlag(bool &LRFlag, bool &TBFlag)
{
    LRFlag = m_LRFlag;
    TBFlag = m_TBFlag;
}

void CGraphicsView::setFirstWeekday(int weekday)
{
    m_firstWeekDay = weekday;
}

void CGraphicsView::setTime(QTime time)
{
    int viewWidth = viewport()->width();
    int viewHeight = viewport()->height();

    QPoint newCenter(viewWidth / 2,  viewHeight / 2);
    QPointF centerpos = mapToScene(newCenter);
    centerpos = QPointF(centerpos.x(), static_cast<qreal>(m_coorManage->getHeight(time)));
    centerOnScene(centerpos);
}

void CGraphicsView::updateInfo()
{
    switch (m_DragStatus) {
    case IsCreate:
        upDateInfoShow(IsCreate,m_DragScheduleInfo);
        break;
    default:
        upDateInfoShow();
        break;
    }
}
