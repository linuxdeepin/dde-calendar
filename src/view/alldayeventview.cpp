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
#include "alldayeventview.h"
#include "schedulecoormanage.h"
#include "schcedulectrldlg.h"
#include "schceduledlg.h"
#include "myschceduleview.h"
#include "scheduledatamanage.h"
#include "constants.h"

#include <DMessageBox>
#include <DPushButton>
#include <DHiDPIHelper>
#include <DPalette>

#include <QAction>
#include <QListWidget>
#include <QLabel>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include <QMimeData>
#include <QDrag>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGraphicsOpacityEffect>
#include <QWheelEvent>

DGUI_USE_NAMESPACE

CAllDayEventWidgetItem::CAllDayEventWidgetItem(QRectF rect, QGraphicsItem *parent, int edittype)
    : DragInfoItem(rect, parent)
{
    Q_UNUSED(edittype);
}

bool CAllDayEventWidgetItem::hasSelectSchedule(const ScheduleDtailInfo &info)
{
    return info == m_vScheduleInfo;
}

void CAllDayEventWidgetItem::paintBackground(QPainter *painter, const QRectF &rect, const int isPixMap)
{
    Q_UNUSED(isPixMap);
    m_font = DFontSizeManager::instance()->get(m_sizeType, m_font);
    painter->setRenderHints(QPainter::Antialiasing);
    CSchedulesColor gdcolor = CScheduleDataManage::getScheduleDataManage()->getScheduleColorByType(m_vScheduleInfo.type.ID);
    QRectF drawrect = rect;
    QLinearGradient linearGradient(drawrect.topLeft().x(), 0, drawrect.topRight().x(), 0);

    QColor color1 = gdcolor.gradientFromC;
    QColor color2 = gdcolor.gradientToC;
    QColor textcolor = gdcolor.textColor;

    m_vHighflag = CScheduleDataManage::getScheduleDataManage()->getSearchResult(m_vScheduleInfo);

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
    int themetype = CScheduleDataManage::getScheduleDataManage()->getTheme();

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
    QRectF fillRect = QRectF(drawrect.x(),
                             drawrect.y(),
                             drawrect.width(),
                             drawrect.height() - 2);
    //将直线开始点设为0，终点设为1，然后分段设置颜色
    painter->setBrush(linearGradient);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(fillRect, rect.height() / 3, rect.height() / 3);
    painter->setFont(m_font);
    painter->setPen(textcolor);
    QFontMetrics fm = painter->fontMetrics();
    QString tStitlename = m_vScheduleInfo.titleName;
    tStitlename.replace("\n", "");
    QString str = tStitlename;
    QString tstr;
    for (int i = 0; i < str.count(); i++) {
        tstr.append(str.at(i));
        int widthT = fm.width(tstr) + 5;
        if (widthT >= fillRect.width() - 13) {
            tstr.chop(2);
            break;
        }
    }
    if (tstr != str) {
        tstr = tstr + "...";
    }
    painter->drawText(QRectF(fillRect.topLeft().x() + 13, fillRect.y(), fillRect.width(), fillRect.height()),
                      Qt::AlignLeft | Qt::AlignVCenter, tstr);
    if (m_vHoverflag && !m_vSelectflag) {
        QRectF trect = QRectF(fillRect.x() + 0.5, fillRect.y() + 0.5, fillRect.width() - 1, fillRect.height() - 1);
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

void CAllDayEventWeekView::setTheMe(int type)
{
    m_themetype=type;
    if (type == 0 || type == 1) {
        m_weekColor = "#00429A";
        m_weekColor.setAlphaF(0.05);
    } else {
        m_weekColor = "#4F9BFF";
        m_weekColor.setAlphaF(0.1);
    }
    updateDateShow();
}

void CAllDayEventWeekView::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        updateItemHeightByFontSize();
        updateInfo();
    }
}

bool CAllDayEventWeekView::MeetCreationConditions(const QDateTime &date)
{
    return  qAbs(date.daysTo(m_PressDate)<7);
}

void CAllDayEventWeekView::slotCreate(const QDateTime &date)
{
    emit signalViewtransparentFrame(1);
    CSchceduleDlg dlg(1, this);
    dlg.setDate(date);
    dlg.setAllDay(true);
    if (dlg.exec() == DDialog::Accepted) {
        emit signalsUpdateShcedule();
    }
    emit signalViewtransparentFrame(0);
}

bool CAllDayEventWeekView::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
{
    return timeFirst.date() == timeSecond.date();
}

bool CAllDayEventWeekView::JudgeIsCreate(const QPointF &pos)
{
    return  qAbs(pos.x()-m_PressPos.x())>20 ||
            qAbs(m_PressDate.date().daysTo(m_coorManage->getsDate(mapFrom(this,pos.toPoint()))))>0;
}

void CAllDayEventWeekView::RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos)
{
    Q_UNUSED(listItem);
    m_rightMenu->clear();
    m_rightMenu->addAction(m_createAction);

    m_createDate.setDate(m_coorManage->getsDate(mapFrom(this, pos)));
    m_createDate.setTime(QTime::currentTime());
    m_rightMenu->exec(QCursor::pos());

}

void CAllDayEventWeekView::MoveInfoProcess(ScheduleDtailInfo &info, const QPointF &pos)
{
    Q_UNUSED(pos);
    if (info.allday) {
        qint64 offset = m_PressDate.daysTo(m_MoveDate);
        info.beginDateTime = info.beginDateTime.addDays(offset);
        info.endDateTime    = info.endDateTime.addDays(offset);
    } else {
        qint64 offset = info.beginDateTime.daysTo(info.endDateTime);
        info.allday = true;
        info.remind = true;
        info.remindData.time = QTime(9, 0);
        info.remindData.n = 1;
        m_DragScheduleInfo.beginDateTime = QDateTime(m_MoveDate.date(),QTime(0,0,0));
        m_DragScheduleInfo.endDateTime = QDateTime(m_MoveDate.addDays(offset).date(),QTime(23,59,59));
    }
    info.IsMoveInfo = true;
    upDateInfoShow(ChangeWhole,info);
}

QDateTime CAllDayEventWeekView::getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
{
    return moveDateTime.daysTo(m_InfoEndTime)<0 ?
           QDateTime(m_InfoEndTime.date(),QTime(0,0,0)):
           QDateTime(moveDateTime.date(),QTime(0,0,0));
}

QDateTime CAllDayEventWeekView::getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
{
    return  m_InfoBeginTime.daysTo(m_MoveDate)<0?
            QDateTime(m_InfoBeginTime.date(),QTime(23,59,0)):
            QDateTime(moveDateTime.date(),QTime(23,59,0));
}

void CAllDayEventWeekView::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
{
    m_MoveDate.setDate(begindate.addMonths(-2));
    m_beginDate = begindate;
    m_endDate = enddate;
    w -=2;
    m_coorManage->setRange(w, h, begindate, enddate, rightmagin);
    m_Scene->setSceneRect(0, 0, w, h);
    m_rightmagin = rightmagin;
    updateDateShow();
}

void CAllDayEventWeekView::setRange(QDate begin, QDate end)
{
    m_MoveDate.setDate(begin.addMonths(-2));
    m_beginDate = begin;
    m_endDate = end;
    getCoorManage()->setDateRange(begin, end);
}

void CAllDayEventWeekView::updateHigh()
{
    for (int i = 0; i < m_baseShowItem.count(); i++) {
        m_baseShowItem.at(i)->update();
    }
}

void CAllDayEventWeekView::setSelectSchedule(const ScheduleDtailInfo &info)
{
    for (int i = 0; i < m_baseShowItem.size(); ++i) {
        CAllDayEventWidgetItem *item = m_baseShowItem.at(i);
        if (item->hasSelectSchedule(info)) {
            QRectF rect = item->rect();
            centerOn(0, rect.y());
            setTransformationAnchor(QGraphicsView::AnchorViewCenter);
            item->setStartValue(0);
            item->setEndValue(4);
            item->startAnimation();
        }
    }
}

void CAllDayEventWeekView::setMargins(int left, int top, int right, int bottom)
{
    setViewportMargins(QMargins(left, top, right, bottom));
}

void CAllDayEventWeekView::updateInfo()
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

void CAllDayEventWeekView::upDateInfoShow(const DragStatus &status, const ScheduleDtailInfo &info)
{
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

    std::sort(vListData.begin(), vListData.end());

    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    for (int i = 0; i < vListData.count(); i++) {
        QDate tbegindate = vListData.at(i).beginDateTime.date();
        QDate tenddate = vListData.at(i).endDateTime.date();
        if (tbegindate < m_beginDate)
            tbegindate = m_beginDate;
        if (tenddate > m_endDate)
            tenddate = m_endDate;
        MScheduleDateRangeInfo sinfo;
        sinfo.bdate = tbegindate;
        sinfo.edate = tenddate;
        sinfo.tData = vListData.at(i);
        sinfo.state = false;
        vMDaySchedule.append(sinfo);
    }
    QVector<QVector<int> > vCfillSchedule;
    vCfillSchedule.resize(vListData.count());
    int tNum = static_cast<int>(m_beginDate.daysTo(m_endDate) + 1);
    for (int i = 0; i < vListData.count(); i++) {
        vCfillSchedule[i].resize(tNum);
        vCfillSchedule[i].fill(-1);
    }
    //首先填充跨天日程
    for (int i = 0; i < vMDaySchedule.count(); i++) {
        if (vMDaySchedule[i].state)
            continue;
        int bindex = static_cast<int>(m_beginDate.daysTo(vMDaySchedule[i].bdate));
        int eindex = static_cast<int>(m_beginDate.daysTo(vMDaySchedule[i].edate));
        int c = -1;
        for (int k = 0; k < vListData.count(); k++) {
            int t = 0;
            for (t = bindex; t <= eindex; t++) {
                if (vCfillSchedule[k][t] != -1) {
                    break;
                }
            }
            if (t == eindex + 1) {
                c = k;
                break;
            }
        }
        if (c == -1)
            continue;

        bool flag = false;
        for (int sd = bindex; sd <= eindex; sd++) {
            if (vCfillSchedule[c][sd] != -1)
                continue;
            vCfillSchedule[c][sd] = i;
            flag = true;
        }
        if (flag)
            vMDaySchedule[i].state = true;
    }
    QVector<QVector<ScheduleDtailInfo> > vResultData;
    for (int i = 0; i < vListData.count(); i++) {
        QVector<int> vId;
        for (int j = 0; j < tNum; j++) {
            if (vCfillSchedule[i][j] != -1) {
                int k = 0;
                for (; k < vId.count(); k++) {
                    if (vId[k] == vCfillSchedule[i][j])
                        break;
                }
                if (k == vId.count())
                    vId.append(vCfillSchedule[i][j]);
            }
        }
        QVector<ScheduleDtailInfo> tData;
        for (int j = 0; j < vId.count(); j++) {
            tData.append(vMDaySchedule[vId[j]].tData);
        }
        if (!tData.isEmpty())
            vResultData.append(tData);
    }

    int m_topMagin;
    if (vResultData.count() < 2) {
        m_topMagin = 32;
    } else if (vResultData.count()  < 6) {
        m_topMagin = 31 + (vResultData.count()  - 1) * (itemHeight+1);

    } else {
        m_topMagin = 123;
    }
    setFixedHeight(m_topMagin - 3);
    setDayData(vResultData);
    update();
    emit signalUpdatePaint(m_topMagin);
}

CAllDayEventWeekView::CAllDayEventWeekView(QWidget *parent, int edittype)
    : DragInfoGraphicsView (parent)
{
    m_editType = edittype;
    updateItemHeightByFontSize();
    m_coorManage = new CScheduleCoorManage;
}

CAllDayEventWeekView::~CAllDayEventWeekView()
{
    delete m_coorManage;
    m_coorManage = nullptr;
}

void CAllDayEventWeekView::setDayData(const QVector<QVector<ScheduleDtailInfo> > &vlistData)
{
    m_vlistData = vlistData;
    updateDateShow();
}

void CAllDayEventWeekView::setInfo(const QVector<ScheduleDtailInfo> &info)
{
    m_scheduleInfo = info;
}

void CAllDayEventWeekView::slotDoubleEvent()
{
    m_updateDflag  = true;
    emit signalsUpdateShcedule();
}

void CAllDayEventWeekView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }
    emit signalScheduleShow(false);
    DGraphicsView::mouseDoubleClickEvent(event);
    CAllDayEventWidgetItem *item = dynamic_cast<CAllDayEventWidgetItem *>(itemAt(event->pos()));
    if (item == nullptr) {
        m_createDate.setDate(m_coorManage->getsDate(mapFrom(this, event->pos())));
        m_createDate.setTime(QTime::currentTime());
        slotCreate(m_createDate);
    } else {
        emit signalViewtransparentFrame(1);
        m_updateDflag  = false;
        CMySchceduleView dlg(item->getData(), this);
        connect(&dlg, &CMySchceduleView::signalsEditorDelete, this, &CAllDayEventWeekView::slotDoubleEvent);
        dlg.exec();
        emit signalViewtransparentFrame(0);
        disconnect(&dlg, &CMySchceduleView::signalsEditorDelete, this, &CAllDayEventWeekView::slotDoubleEvent);
        if (item == nullptr) {
            return;
        }
    }
}

void CAllDayEventWeekView::wheelEvent(QWheelEvent *event)
{
    //若滚轮事件为左右方向则退出
    if(event->orientation() == Qt::Orientation::Horizontal){
        return;
    }
    emit signalScheduleShow(false);
    DGraphicsView::wheelEvent(event);
}

void CAllDayEventWeekView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    //绘制背景
    paintBackground(painter);
    painter.end();
    QGraphicsView::paintEvent(event);
}

void CAllDayEventWeekView::updateDateShow()
{
    m_Scene->setSceneRect(0,
                          0,
                          m_Scene->width(),
                          (itemHeight + 1)*m_vlistData.size());

    for (int i = 0; i < m_baseShowItem.count(); i++) {
        delete  m_baseShowItem[i];
    }
    m_baseShowItem.clear();
    for (int i = 0; i < m_vlistData.size(); ++i) {
        createItemWidget(i);
    }
}

void CAllDayEventWeekView::createItemWidget(int index, bool average)
{
    Q_UNUSED(average)
    for (int i = 0; i < m_vlistData[index].size(); ++i) {
        const ScheduleDtailInfo &info = m_vlistData[index].at(i);
        QRectF drawrect = m_coorManage->getAllDayDrawRegion(info.beginDateTime.date(), info.endDateTime.date());
        drawrect.setY(2 + (itemHeight + 1) * index);
        drawrect.setHeight(itemHeight);

        CAllDayEventWidgetItem *gwi = new CAllDayEventWidgetItem(drawrect, nullptr, m_editType);
        gwi->setData(m_vlistData[index].at(i));
        m_Scene->addItem(gwi);
        m_baseShowItem.append(gwi);
    }
}

void CAllDayEventWeekView::updateItemHeightByFontSize()
{
    QFont font;
    DFontSizeManager::instance()->setFontGenericPixelSize(
        static_cast<quint16>(DFontSizeManager::instance()->fontPixelSize(qGuiApp->font())));
    font = DFontSizeManager::instance()->t8(font);
    QFontMetrics fm(font);
    int h = fm.height();
    if (itemHeight != h) {
        itemHeight = h;
    }
}

void CAllDayEventWeekView::paintBackground(QPainter &painter)
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
        painter.setBrush(m_weekColor);
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

CAllDayEventWeekView::PosInItem CAllDayEventWeekView::getPosInItem(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    QPointF itemPos = QPointF(scenePos.x()-itemRect.x(),
                              scenePos.y()-itemRect.y());
    double bottomy = itemRect.width()- itemPos.x();
    if (itemPos.x()<5) {
        return LEFT;
    }
    if (bottomy <5) {
        return RIGHT;
    }
    return MIDDLE;
}

QDateTime CAllDayEventWeekView::getPosDate(const QPoint &p)
{
    return QDateTime(m_coorManage->getsDate(mapFrom(this, p)),
                     QTime(0,0,0));
}

void CAllDayEventWeekView::slotDeleteItem()
{
    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().type.ID <0) {
        return;
    }
    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().type.ID != DDECalendar::FestivalTypeID) {
        DeleteItem(CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo());
    }
    CScheduleDataManage::getScheduleDataManage()->setPressSelectInfo(ScheduleDtailInfo());
}

void CAllDayEventWeekView::slotUpdateScene()
{
    this->scene()->update();
}
