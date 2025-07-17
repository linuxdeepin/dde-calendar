// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "alldayeventview.h"
#include "schedulecoormanage.h"
#include "schedulectrldlg.h"
#include "scheduledlg.h"
#include "myscheduleview.h"
#include "scheduledatamanage.h"
#include "constants.h"
#include "scheduledaterangeinfo.h"
#include "commondef.h"


#include <DPalette>
#include <DMenu>

#include <QAction>
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

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

void CAllDayEventWeekView::setTheMe(int type)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::setTheMe called with type:" << type;
    CWeekDayGraphicsview::setTheMe(type);
}

void CAllDayEventWeekView::changeEvent(QEvent *event)
{
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::changeEvent called with event type:" << event->type();
    if (event->type() == QEvent::FontChange) {
        qCDebug(ClientLogger) << "Font change event received, updating item height";
        updateItemHeightByFontSize();
        updateInfo();
    }
}

bool CAllDayEventWeekView::MeetCreationConditions(const QDateTime &date)
{
    bool result = qAbs(date.daysTo(m_PressDate)) < 7;
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::MeetCreationConditions called, date:" << date 
    //                      << "m_PressDate:" << m_PressDate << "result:" << result;
    return result;
}

void CAllDayEventWeekView::slotCreate(const QDateTime &date)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::slotCreate called with date:" << date;
    CScheduleDlg dlg(1, this);
    dlg.setDate(date);
    dlg.setAllDay(true);
    if (dlg.exec() == DDialog::Accepted) {
        qCDebug(ClientLogger) << "Schedule creation accepted, updating schedule";
        emit signalsUpdateSchedule();
        slotStateChange(true);
    } else {
        qCDebug(ClientLogger) << "Schedule creation cancelled";
    }
}

bool CAllDayEventWeekView::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
{
    bool result = timeFirst.date() == timeSecond.date();
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::IsEqualtime called, comparing dates:" 
    //                      << timeFirst.date() << "and" << timeSecond.date() << "result:" << result;
    return result;
}

bool CAllDayEventWeekView::JudgeIsCreate(const QPointF &pos)
{
    bool result = qAbs(pos.x() - m_PressPos.x()) > 20 || qAbs(m_PressDate.date().daysTo(m_coorManage->getsDate(mapFrom(this, pos.toPoint())))) > 0;
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::JudgeIsCreate called, result:" << result;
    return result;
}

void CAllDayEventWeekView::RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::RightClickToCreate called at position:" << pos;
    Q_UNUSED(listItem);
    m_rightMenu->clear();
    m_rightMenu->addAction(m_createAction);

    m_createDate.setDate(m_coorManage->getsDate(mapFrom(this, pos)));
    m_createDate.setTime(QTime::currentTime());
    qCDebug(ClientLogger) << "Right click menu prepared with create date:" << m_createDate;
    m_rightMenu->exec(QCursor::pos());
}

void CAllDayEventWeekView::MoveInfoProcess(DSchedule::Ptr &info, const QPointF &pos)
{
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::MoveInfoProcess called";
    Q_UNUSED(pos);
    if (info->allDay()) {
        qint64 offset = m_PressDate.daysTo(m_MoveDate);
        qCDebug(ClientLogger) << "Processing all-day schedule move with offset days:" << offset;
        info->setDtStart(info->dtStart().addDays(offset));
        info->setDtEnd(info->dtEnd().addDays(offset));
    } else {
        qint64 offset = info->dtStart().daysTo(info->dtEnd());
        qCDebug(ClientLogger) << "Converting non-all-day schedule to all-day with day span:" << offset;
        info->setAllDay(true);
        info->setAlarmType(DSchedule::Alarm_15Hour_Front);
        m_DragScheduleInfo->setDtStart(QDateTime(m_MoveDate.date(), QTime(0, 0, 0)));
        m_DragScheduleInfo->setDtEnd(QDateTime(m_MoveDate.addDays(offset).date(), QTime(23, 59, 59)));
    }
    upDateInfoShow(ChangeWhole, info);
}

QDateTime CAllDayEventWeekView::getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
{
    QDateTime result = moveDateTime.daysTo(m_InfoEndTime) < 0 ? QDateTime(m_InfoEndTime.date(), QTime(0, 0, 0)) : QDateTime(moveDateTime.date(), QTime(0, 0, 0));
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::getDragScheduleInfoBeginTime called, result:" << result;
    return result;
}

QDateTime CAllDayEventWeekView::getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
{
    QDateTime result = m_InfoBeginTime.daysTo(m_MoveDate) < 0 ? QDateTime(m_InfoBeginTime.date(), QTime(23, 59, 0)) : QDateTime(moveDateTime.date(), QTime(23, 59, 0));
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::getDragScheduleInfoEndTime called, result:" << result;
    return result;
}

void CAllDayEventWeekView::updateHeight()
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::updateHeight called for" << m_baseShowItem.count() << "items";
    for (int i = 0; i < m_baseShowItem.count(); i++) {
        m_baseShowItem.at(i)->update();
    }
}

/**
 * @brief CAllDayEventWeekView::setSelectSearchSchedule     设置搜索选中日程
 * @param info
 */
void CAllDayEventWeekView::setSelectSearchSchedule(const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::setSelectSearchSchedule called for schedule";
    DragInfoGraphicsView::setSelectSearchSchedule(info);
    for (int i = 0; i < m_baseShowItem.size(); ++i) {
        CAllDayScheduleItem *item = m_baseShowItem.at(i);
        if (item->hasSelectSchedule(info)) {
            qCDebug(ClientLogger) << "Found matching schedule item, starting animation";
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
    qCDebug(ClientLogger) << "CAllDayEventWeekView::setMargins called with:" << left << top << right << bottom;
    setViewportMargins(QMargins(left, top, right, bottom));
}

/**
 * @brief CAllDayEventWeekView::updateInfo  更新日程显示
 */
void CAllDayEventWeekView::updateInfo()
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::updateInfo called, drag status:" << m_DragStatus;
    DragInfoGraphicsView::updateInfo();
    switch (m_DragStatus) {
    case IsCreate:
        qCDebug(ClientLogger) << "Updating info in IsCreate mode";
        upDateInfoShow(IsCreate, m_DragScheduleInfo);
        break;
    default:
        qCDebug(ClientLogger) << "Updating info in default mode";
        upDateInfoShow();
        break;
    }
}

void CAllDayEventWeekView::upDateInfoShow(const DragStatus &status, const DSchedule::Ptr &info)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::upDateInfoShow called with status:" << status;
    DSchedule::List vListData;
    vListData = m_scheduleInfo;
    switch (status) {
    case NONE:
        qCDebug(ClientLogger) << "Status: NONE";
        Q_UNUSED(info);
        break;
    case ChangeBegin:
    case ChangeEnd: {
        qCDebug(ClientLogger) << "Status: ChangeBegin/ChangeEnd for schedule";
        int index = vListData.indexOf(info);
        if (index >= 0)
            vListData[index] = info;
    } break;
    case ChangeWhole:
        qCDebug(ClientLogger) << "Status: ChangeWhole, adding schedule to list";
        vListData.append(info);
        break;
    case IsCreate:
        qCDebug(ClientLogger) << "Status: IsCreate, adding schedule to list";
        vListData.append(info);
        break;
    }

    std::sort(vListData.begin(), vListData.end());

    QVector<MScheduleDateRangeInfo> vMDaySchedule;
    for (int i = 0; i < vListData.count(); i++) {
        DSchedule::Ptr ptr = vListData.at(i);
        if (ptr.isNull()) {
            continue;
        }

        QDate tbegindate = ptr->dtStart().date();
        QDate tenddate = ptr->dtEnd().date();
        if (tbegindate < m_beginDate)
            tbegindate = m_beginDate;
        if (tenddate > m_endDate)
            tenddate = m_endDate;
        MScheduleDateRangeInfo sinfo;
        sinfo.bdate = tbegindate;
        sinfo.edate = tenddate;
        sinfo.tData = ptr;
        sinfo.state = false;
        vMDaySchedule.append(sinfo);
    }
    QVector<QVector<int>> vCfillSchedule;
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
    QVector<DSchedule::List> vResultData;
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
        DSchedule::List tData;
        for (int j = 0; j < vId.count(); j++) {
            tData.append(vMDaySchedule[vId[j]].tData);
        }
        if (!tData.isEmpty())
            vResultData.append(tData);
    }

    int m_topMagin;
    if (vResultData.count() < 2) {
        m_topMagin = 32;
    } else if (vResultData.count() < 6) {
        m_topMagin = 31 + (vResultData.count() - 1) * (itemHeight + 1);
    } else {
        m_topMagin = 123;
    }
    qCDebug(ClientLogger) << "Setting view height based on result data count:" << vResultData.count() << "top margin:" << m_topMagin;
    setFixedHeight(m_topMagin - 3);
    setDayData(vResultData);
    update();
    emit signalUpdatePaint(m_topMagin);
}

CAllDayEventWeekView::CAllDayEventWeekView(QWidget *parent, ViewPosition type)
    : CWeekDayGraphicsview(parent, type, ViewType::ALLDayView)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView constructor called with view position type:" << type;
    updateItemHeightByFontSize();
    connect(this, &CAllDayEventWeekView::sigStateChange, this, &CAllDayEventWeekView::slotStateChange, Qt::DirectConnection);
}

CAllDayEventWeekView::~CAllDayEventWeekView()
{
    // qCDebug(ClientLogger) << "CAllDayEventWeekView destructor called";
}

void CAllDayEventWeekView::setDayData(const QVector<DSchedule::List> &vlistData)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::setDayData called with" << vlistData.size() << "schedule lists";
    m_vlistData = vlistData;
    updateDateShow();
}

void CAllDayEventWeekView::setInfo(const DSchedule::List &info)
{
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::setInfo called with" << info.size() << "schedules";
    m_scheduleInfo = info;
}

void CAllDayEventWeekView::slotDoubleEvent()
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::slotDoubleEvent called";
    m_updateDflag = true;
    emit signalsUpdateSchedule();
}

void CAllDayEventWeekView::mouseDoubleClickEvent(QMouseEvent *event)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::mouseDoubleClickEvent at position:" << event->pos();
    if (event->button() == Qt::RightButton) {
        qCDebug(ClientLogger) << "Right button double click ignored";
        return;
    }
    emit signalScheduleShow(false);
    DGraphicsView::mouseDoubleClickEvent(event);
    CAllDayScheduleItem *item = dynamic_cast<CAllDayScheduleItem *>(itemAt(event->pos()));
    if (item == nullptr) {
        qCDebug(ClientLogger) << "No item at position, creating new schedule";
        m_createDate.setDate(m_coorManage->getsDate(mapFrom(this, event->pos())));
        m_createDate.setTime(QTime::currentTime());
        slotCreate(m_createDate);
    } else {
        qCDebug(ClientLogger) << "Item found, showing schedule view dialog";
        m_updateDflag = false;
        CMyScheduleView dlg(item->getData(), this);
        connect(&dlg, &CMyScheduleView::signalsEditorDelete, this, &CAllDayEventWeekView::slotDoubleEvent);
        if (dlg.exec() == DDialog::Accepted){
            qCDebug(ClientLogger) << "Schedule view dialog accepted";
            slotStateChange(true);
        } else {
            qCDebug(ClientLogger) << "Schedule view dialog cancelled";
        }
        disconnect(&dlg, &CMyScheduleView::signalsEditorDelete, this, &CAllDayEventWeekView::slotDoubleEvent);
    }
}

void CAllDayEventWeekView::wheelEvent(QWheelEvent *event)
{
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::wheelEvent with angle delta:" << event->angleDelta();
    //若滚轮事件为左右方向则退出
    if (event->angleDelta().x() != 0 ) {
        qCDebug(ClientLogger) << "Horizontal wheel event, ignoring";
        return;
    }
    emit signalScheduleShow(false);
    DGraphicsView::wheelEvent(event);
}

void CAllDayEventWeekView::updateDateShow()
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::updateDateShow called";
    qreal sceneHeight;
    qreal itemsHeight = (itemHeight + 1) * m_vlistData.size();
    if (itemsHeight < 32) {
        sceneHeight = 29;
    } else {
        sceneHeight = itemsHeight + 6;
    }
    //如果设置场景的高度小于viewport的高度则设置场景的高度为viewport的高度
    sceneHeight = sceneHeight < viewport()->height() ? viewport()->height() : sceneHeight;
    qCDebug(ClientLogger) << "Setting scene rect with height:" << sceneHeight;
    setSceneRect(0, 0, m_Scene->width(), sceneHeight);

    for (int i = 0; i < m_baseShowItem.count(); i++) {
        delete m_baseShowItem[i];
    }
    m_baseShowItem.clear();
    qCDebug(ClientLogger) << "Creating" << m_vlistData.size() << "item widgets";
    for (int i = 0; i < m_vlistData.size(); ++i) {
        createItemWidget(i);
    }
    //更新每个背景上的日程标签
    updateBackgroundShowItem();
}

void CAllDayEventWeekView::createItemWidget(int index, bool average)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::createItemWidget called for index:" << index;
    Q_UNUSED(average)
    for (int i = 0; i < m_vlistData[index].size(); ++i) {
        const DSchedule::Ptr &info = m_vlistData[index].at(i);
        QRectF drawrect = m_coorManage->getAllDayDrawRegion(info->dtStart().date(), info->dtEnd().date());
        drawrect.setY(2 + (itemHeight + 1) * index);
        drawrect.setHeight(itemHeight);

        CAllDayScheduleItem *gwi = new CAllDayScheduleItem(drawrect, nullptr);
        gwi->setData(m_vlistData[index].at(i));
        m_Scene->addItem(gwi);
        m_baseShowItem.append(gwi);
    }
    qCDebug(ClientLogger) << "Created" << m_vlistData[index].size() << "schedule items for index" << index;
}

void CAllDayEventWeekView::slotStateChange(bool bState)
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::slotStateChange called with state:" << bState;
    if(bState) {
        qCDebug(ClientLogger) << "Hiding" << m_baseShowItem.count() << "schedule items";
        for (int i = 0; i < m_baseShowItem.count(); i++) {
            m_baseShowItem[i]->setVisible(false);
        }
    }
}

void CAllDayEventWeekView::updateItemHeightByFontSize()
{
    qCDebug(ClientLogger) << "CAllDayEventWeekView::updateItemHeightByFontSize called";
    QFont font;
    DFontSizeManager::instance()->setFontGenericPixelSize(
        static_cast<quint16>(DFontSizeManager::instance()->fontPixelSize(qGuiApp->font())));
    font = DFontSizeManager::instance()->t8(font);
    QFontMetrics fm(font);
    int h = fm.height();
    if (itemHeight != h) {
        qCDebug(ClientLogger) << "Item height updated from" << itemHeight << "to" << h;
        itemHeight = h;
    }
}

CAllDayEventWeekView::PosInItem CAllDayEventWeekView::getPosInItem(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    QPointF itemPos = QPointF(scenePos.x() - itemRect.x(),
                              scenePos.y() - itemRect.y());
    double bottomy = itemRect.width() - itemPos.x();
    PosInItem result;
    if (itemPos.x() < 5) {
        result = LEFT;
    } else if (bottomy < 5) {
        result = RIGHT;
    } else {
        result = MIDDLE;
    }
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::getPosInItem result:" << result;
    return result;
}

QDateTime CAllDayEventWeekView::getPosDate(const QPoint &p)
{
    QDateTime result = QDateTime(m_coorManage->getsDate(mapFrom(this, p)), QTime(0, 0, 0));
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::getPosDate called, result:" << result;
    return result;
}

void CAllDayEventWeekView::slotUpdateScene()
{
    // qCDebug(ClientLogger) << "CAllDayEventWeekView::slotUpdateScene called";
    this->scene()->update();
}
