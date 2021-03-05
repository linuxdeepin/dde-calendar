/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#include "draginfographicsview.h"
#include "scheduledlg.h"
#include "schedulectrldlg.h"
#include "myscheduleview.h"
#include "constants.h"
#include "cscheduleoperation.h"

#include <DMenu>

#include <QMimeData>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QtMath>
#include <QPainter>
#include <QPainterPath>

//定义拖拽日程
ScheduleDataInfo DragInfoGraphicsView::m_DragScheduleInfo;
bool DragInfoGraphicsView::m_hasUpdateMark = false;

DragInfoGraphicsView::DragInfoGraphicsView(DWidget *parent)
    : DGraphicsView(parent)
    , m_Scene(new CGraphicsScene(this))
    , m_rightMenu(new DMenu(this))
    , m_MoveDate(QDateTime::currentDateTime())
{
    setFrameShape(QFrame::NoFrame);
    setScene(m_Scene);
    setContentsMargins(0, 0, 0, 0);

    m_editAction = new QAction(tr("Edit"), this);
    m_deleteAction = new QAction(tr("Delete"), this);
    m_createAction = new QAction(tr("New event"), this);
    connect(m_createAction, &QAction::triggered, this,
            static_cast<void (DragInfoGraphicsView::*)()>(&DragInfoGraphicsView::slotCreate));
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setViewportMargins(0, 0, 0, 0);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_Drag = new QDrag(this);
    //设置接受触摸事件
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::SwipeGesture);

    m_touchAnimation = new QPropertyAnimation(this, "touchSlidePos", this);
    //设置动画时间
    m_touchAnimation->setDuration(1000);
    //设置动画曲线
    m_touchAnimation->setEasingCurve(QEasingCurve::OutQuart);

    connect(m_Scene, &CGraphicsScene::signalSwitchPrePage, this, &DragInfoGraphicsView::slotSwitchPrePage);
    connect(m_Scene, &CGraphicsScene::signalSwitchNextPage, this, &DragInfoGraphicsView::slotSwitchNextPage);
    connect(m_Scene, &CGraphicsScene::signalGotoDayView, this, &DragInfoGraphicsView::signalGotoDayView);
    connect(m_Scene, &CGraphicsScene::signalContextMenu, this, &DragInfoGraphicsView::slotContextMenu);
}

DragInfoGraphicsView::~DragInfoGraphicsView()
{
}

void DragInfoGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }
    stopTouchAnimation();
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸点击则记录相关状态并改变触摸状态
        DGraphicsView::mousePressEvent(event);
        m_TouchBeginPoint = event->pos();
        m_TouchBeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        m_touchState = TS_PRESS;
        return;
    }
    mousePress(event->pos());
    //更新其它view中item状态显示
    emit signalSceneUpdate();
}

void DragInfoGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        return;
    }
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸点击状态则调用左击事件处理
        if (m_touchState == TS_PRESS) {
            mousePress(m_TouchBeginPoint.toPoint());
        }
        if (m_touchState == TS_SLIDE) {
            stopTouchAnimation();
            const qint64 timeOffset = QDateTime::currentDateTime().toMSecsSinceEpoch() - m_TouchBeginTime;
            //如果为快速滑动则开启滑动动画效果
            if (timeOffset < 150) {
                m_touchAnimation->setStartValue(verticalScrollBar()->sliderPosition());
                m_touchAnimation->setEndValue(verticalScrollBar()->minimum());
                switch (m_touchMovingDir) {
                case touchGestureOperation::T_TOP: {
                    //如果手势往上
                    m_touchAnimation->setStartValue(verticalScrollBar()->sliderPosition());
                    m_touchAnimation->setEndValue(verticalScrollBar()->maximum());
                    m_touchAnimation->start();
                    break;
                }
                case touchGestureOperation::T_BOTTOM: {
                    //如果手势往下
                    m_touchAnimation->setStartValue(verticalScrollBar()->sliderPosition());
                    m_touchAnimation->setEndValue(verticalScrollBar()->minimum());
                    m_touchAnimation->start();
                    break;
                }
                default:

                    break;
                }
            }
        }
    }
    m_touchState = TS_NONE;
    mouseReleaseScheduleUpdate();
    //update scene
    this->scene()->update();
}


void DragInfoGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    //移动偏移
    const int lengthOffset = 5;
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        m_touchMovingDir = touchGestureOperation::T_MOVE_NONE;
        switch (m_touchState) {
        case TS_NONE: {
            break;
        }
        case TS_PRESS: {
            //1 点击
            qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            qint64 timeOffset = currentTime - m_TouchBeginTime;
            //获取移动距离
            qreal movingLength = QLineF(m_TouchBeginPoint, event->pos()).length();
            //如果移动距离小于5且点击时间大于250毫秒小于900毫秒则为拖拽移动状态
            if (movingLength < lengthOffset && (timeOffset > 250 && timeOffset < 900)) {
                m_touchState = TS_DRAG_MOVE;
                m_touchDragMoveState = 1;
            }
            //如果移动距离大于5则为滑动状态
            if (movingLength > lengthOffset) {
                m_touchState = TS_SLIDE;
            }
            break;
        }
        case TS_DRAG_MOVE: {
            //2 拖拽移动
            qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            qint64 timeOffset = currentTime - m_TouchBeginTime;
            qreal movingLength = QLineF(m_TouchBeginPoint, event->pos()).length();
            //如果移动距离小于5且点击时间大于900毫秒则为长按状态
            if (movingLength < lengthOffset && (timeOffset > 900)) {
                m_touchState = TS_LONG_PRESS;
            }
            if (movingLength > lengthOffset) {
                if (m_touchDragMoveState == 1) {
                    mousePress(m_TouchBeginPoint.toPoint());
                }
                m_touchDragMoveState = 2;
            }
            break;
        }
        case TS_SLIDE: {
            //3 滑动
            QPointF _currentPoint = event->pos();
            slideEvent(m_TouchBeginPoint, _currentPoint);
            break;
        }
        default:
            break;
        }
    }

    if (m_press) {
        emit signalScheduleShow(false);
        m_press = false;
        DragInfoItem::setPressFlag(false);
    }

    DragInfoItem *item = dynamic_cast<DragInfoItem *>(itemAt(event->pos()));

    if (item != nullptr) {
        if (item->getData().getType() != DDECalendar::FestivalTypeID) {
            if (m_DragStatus == NONE) {
                switch (getPosInItem(event->pos(), item->rect())) {
                case LEFT:
                case RIGHT:
                    setCursor(Qt::SplitHCursor);
                    break;
                case TOP:
                case BOTTOM:
                    setCursor(Qt::SplitVCursor);
                    break;
                default:
                    setCursor(Qt::ArrowCursor);
                    break;
                }
            }
        }
    } else {
        if (m_DragStatus == NONE) {
            setCursor(Qt::ArrowCursor);
        }
    }
    QDateTime gDate =  getPosDate(event->pos());
    switch (m_DragStatus) {
    case IsCreate:
        m_isCreate = JudgeIsCreate(event->pos());
        if (m_isCreate) {
            if (!IsEqualtime(m_MoveDate, gDate)) {
                m_MoveDate = gDate;
                m_DragScheduleInfo = getScheduleInfo(m_PressDate, m_MoveDate);
                upDateInfoShow(IsCreate, m_DragScheduleInfo);
                setPressSelectInfo(m_DragScheduleInfo);
            }
        }
        break;
    case ChangeBegin:
        if (!IsEqualtime(m_MoveDate, gDate)) {
            m_MoveDate = gDate;
            //获取日程开始时间
            QDateTime _beginTime = getDragScheduleInfoBeginTime(m_MoveDate);
            m_DragScheduleInfo.setBeginDateTime(_beginTime);
            m_DragScheduleInfo.setEndDateTime(m_InfoEndTime);
            upDateInfoShow(ChangeBegin, m_DragScheduleInfo);
        }
        break;
    case ChangeEnd:
        if (!IsEqualtime(m_MoveDate, gDate)) {
            m_MoveDate = gDate;
            m_DragScheduleInfo.setBeginDateTime(m_InfoBeginTime);
            //获取结束时间
            QDateTime _endTime = getDragScheduleInfoEndTime(m_MoveDate);
            m_DragScheduleInfo.setEndDateTime(_endTime);
            upDateInfoShow(ChangeEnd, m_DragScheduleInfo);
        }
        break;
    case ChangeWhole: {
        if (!m_PressRect.contains(event->pos())) {
            //拖拽前设置是否已经更新日程界面标志为否
            m_hasUpdateMark = false;
            //TODO 拖拽结束后还会返回到鼠标移动事件???
            m_Drag->exec(Qt::MoveAction);
            //TODO 调试发现exec后代码有时候会执行2遍，先标记下，以后研究,现判断m_Drag是否为nullptr，若为空指针则表示执行过一遍直接退出
            if (m_Drag == nullptr)
                return;
            m_Drag = nullptr;
            m_DragStatus = NONE;
            setCursor(Qt::ArrowCursor);
            //如果拖拽结束后没有修改日程则更新下界面日程显示
            if (!m_hasUpdateMark) {
                updateInfo();
            }
        }
    }
    break;
    default:
        break;
    }
    DGraphicsView::mouseMoveEvent(event);
}

void DragInfoGraphicsView::wheelEvent(QWheelEvent *event)
{
    stopTouchAnimation();
    DGraphicsView::wheelEvent(event);
}

void DragInfoGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    DGraphicsView::contextMenuEvent(event);

    if (m_DragStatus == IsCreate) {
        return;
    }
    emit signalScheduleShow(false);
    m_press = false;
    m_DragStatus = NONE;
    QGraphicsItem *listItem = itemAt(event->pos());
    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(listItem);

    if (infoitem != nullptr) {
        if (infoitem->getData().getType() != DDECalendar::FestivalTypeID) {
            m_rightMenu->clear();
            m_rightMenu->addAction(m_editAction);
            m_rightMenu->addAction(m_deleteAction);
            QAction *action_t = m_rightMenu->exec(QCursor::pos());

            if (action_t == m_editAction) {
                emit signalViewtransparentFrame(1);
                CScheduleDlg dlg(0, this);
                dlg.setData(infoitem->getData());
                if (dlg.exec() == DDialog::Accepted) {
                    emit signalsUpdateShcedule();
                }
                emit signalViewtransparentFrame(0);
            } else if (action_t == m_deleteAction) {
                DeleteItem(infoitem->getData());
            }
        } else {
            emit signalViewtransparentFrame(1);
            CMyScheduleView dlg(infoitem->getData(), this);
            dlg.exec();
            emit signalViewtransparentFrame(0);
        }
    } else {
        RightClickToCreate(listItem, event->pos());
    }
}

void DragInfoGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("Info")) {
        QJsonParseError json_error;
        QString str = event->mimeData()->data("Info");
        QJsonDocument jsonDoc(QJsonDocument::fromJson(str.toLocal8Bit(), &json_error));

        if (json_error.error != QJsonParseError::NoError) {
            event->ignore();
        }
        QJsonObject rootobj = jsonDoc.object();
        ScheduleDataInfo info = ScheduleDataInfo::JsonToSchedule(rootobj);

        if ((event->source() != this && info.getRepetitionRule().getRuleId() > 0) || info.getType() == DDECalendar::FestivalTypeID) {
            event->ignore();
        } else {
            event->accept();
        }
    } else {
        event->ignore();
    }

}

void DragInfoGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    upDateInfoShow();
    m_MoveDate = m_MoveDate.addMonths(-2);
}

void DragInfoGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    QString str = event->mimeData()->data("Info");
    QDateTime gDate =  getPosDate(event->pos());
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(str.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return;
    }

    if (!IsEqualtime(m_MoveDate, gDate)) {
        m_MoveDate = gDate;
        QJsonObject rootobj = jsonDoc.object();
        m_DragScheduleInfo = ScheduleDataInfo::JsonToSchedule(rootobj);
        m_DragScheduleInfo.setIsMoveInfo(true);
        MoveInfoProcess(m_DragScheduleInfo, event->posF());
        DragInfoItem::setPressSchedule(m_DragScheduleInfo);
    }
}

void DragInfoGraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("Info")) {
        if (event->source() != this || m_MoveDate != m_PressDate) {
            updateScheduleInfo(m_DragScheduleInfo);
        } else {
            emit signalsUpdateShcedule();
        }
        m_DragStatus = NONE;
        m_MoveDate = m_MoveDate.addMonths(-2);
    }
}

bool DragInfoGraphicsView::event(QEvent *e)
{
    if (e->type() == QEvent::Leave) {
        if (m_DragStatus == IsCreate ||
                m_DragStatus == ChangeBegin ||
                m_DragStatus == ChangeEnd)
            mouseReleaseScheduleUpdate();
    }
    return DGraphicsView::event(e);
}

void DragInfoGraphicsView::keyPressEvent(QKeyEvent *event)
{
    DGraphicsView::keyPressEvent(event);
}

void DragInfoGraphicsView::paintEvent(QPaintEvent *event)
{
    DGraphicsView::paintEvent(event);
    //绘制圆角效果
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_outerBorderColor);
    //左下角绘制圆角补角,颜色与外框背景色一致
    if (m_leftShowRadius) {
        QPainterPath _leftPath;
        _leftPath.moveTo(0, this->height() - m_radius);
        _leftPath.arcTo(0, this->height() - m_radius * 2, m_radius * 2, m_radius * 2, 180, 90);
        _leftPath.lineTo(0, this->height());
        _leftPath.lineTo(0, this->height() - m_radius);
        painter.drawPath(_leftPath);
    }
    //右下角绘制圆角补角
    if (m_rightShowRadius) {
        QPainterPath _rightPath;
        _rightPath.moveTo(this->width() - m_radius, this->height());
        _rightPath.arcTo(this->width() - m_radius * 2, this->height() - m_radius * 2, m_radius * 2, m_radius * 2, 270, 90);
        _rightPath.lineTo(this->width(), this->height());
        _rightPath.lineTo(this->width() - m_radius, this->height());
        painter.drawPath(_rightPath);
        painter.end();
    }
}

void DragInfoGraphicsView::showEvent(QShowEvent *event)
{
    //显示时重置大小
    resize(this->width(), this->height());
    DGraphicsView::showEvent(event);
}

void DragInfoGraphicsView::slotCreate()
{
    slotCreate(m_createDate);
}

/**
 * @brief DragInfoGraphicsView::setPressSelectInfo      设置点击选中日程
 * @param info
 */
void DragInfoGraphicsView::setPressSelectInfo(const ScheduleDataInfo &info)
{
    DragInfoItem::setPressSchedule(info);
}

/**
 * @brief DragInfoGraphicsView::updateScheduleInfo      拖拽更新日程信息
 * @param info
 */
void DragInfoGraphicsView::updateScheduleInfo(const ScheduleDataInfo &info)
{
    emit signalViewtransparentFrame(1);
    CScheduleOperation _scheduleOperation(this);
    if (_scheduleOperation.changeSchedule(info, m_PressScheduleInfo)) {
        //如果日程修改成功则更新更新标志
        m_hasUpdateMark = true;
    } else {
        //如果取消更新则主动更新显示
        updateInfo();
    }
    emit signalViewtransparentFrame(0);
}

void DragInfoGraphicsView::DragPressEvent(const QPoint &pos, DragInfoItem *item)
{
    m_PressPos = pos;
    m_PressDate = getPosDate(pos);
    m_MoveDate = m_PressDate.addMonths(-2);

    if (item != nullptr) {
        PosInItem mpressstatus = getPosInItem(pos, item->boundingRect());
        if (mpressstatus != MIDDLE && item->getData().getType() == 4) {
            return;
        }
        m_DragScheduleInfo = item->getData();
        m_PressScheduleInfo = item->getData();
        m_InfoBeginTime = m_DragScheduleInfo.getBeginDateTime();
        m_InfoEndTime = m_DragScheduleInfo.getEndDateTime();
        switch (mpressstatus) {
        case TOP:
            m_DragStatus = ChangeBegin;
            setCursor(Qt::SplitVCursor);
            break;
        case BOTTOM:
            m_DragStatus = ChangeEnd;
            setCursor(Qt::SplitVCursor);
            break;
        case LEFT:
            m_DragStatus = ChangeBegin;
            setCursor(Qt::SplitHCursor);
            break;
        case RIGHT:
            m_DragStatus = ChangeEnd;
            setCursor(Qt::SplitHCursor);
            break;
        default:
            ShowSchedule(item);
            m_DragStatus = ChangeWhole;
            QMimeData *mimeData = new QMimeData();
            mimeData->setText(m_DragScheduleInfo.getTitleName());
            mimeData->setData("Info", ScheduleDataInfo::ScheduleToJsonStr(m_DragScheduleInfo).toUtf8());

            if (m_Drag == nullptr) {
                m_Drag = new QDrag(this);
            }
            m_Drag->setMimeData(mimeData);
            QPointF itemPos = QPointF(pos.x() - item->boundingRect().x(),
                                      pos.y() - item->boundingRect().y());
            m_Drag->setHotSpot(itemPos.toPoint());
            setDragPixmap(m_Drag, item);
            break;
        }
    } else {
        m_DragStatus = IsCreate;
        m_isCreate = false;
    }
}

/**
 * @brief DragInfoGraphicsView::mouseReleaseScheduleUpdate      鼠标左击释放数据处理
 */
void DragInfoGraphicsView::mouseReleaseScheduleUpdate()
{
    setCursor(Qt::ArrowCursor);
    m_press = false;
    DragInfoItem::setPressFlag(false);

    switch (m_DragStatus) {
    case IsCreate:
        if (MeetCreationConditions(m_MoveDate)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            emit signalViewtransparentFrame(1);
            CScheduleDlg dlg(1, this);
            dlg.setData(m_DragScheduleInfo);
            //如果取消新建则主动刷新日程信息
            dlg.exec();
            //因dtk override了exec函数，这里使用result判断返回值类型，如果不为Accepted则刷新界面
            if (dlg.result() != DDialog::Accepted) {
                updateInfo();
            }
            //设置选中日程为无效日程
            setPressSelectInfo(ScheduleDataInfo());
            emit signalViewtransparentFrame(0);
        }
        break;
    case ChangeBegin:
        if (!IsEqualtime(m_MoveDate, m_InfoBeginTime)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            updateScheduleInfo(m_DragScheduleInfo);
        }
        break;
    case ChangeEnd:
        if (!IsEqualtime(m_MoveDate, m_InfoEndTime)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            updateScheduleInfo(m_DragScheduleInfo);
        }
        break;
    default:
        break;
    }
    m_DragStatus = NONE;
    update();
}

void DragInfoGraphicsView::mousePress(const QPoint &point)
{
    setPressSelectInfo(ScheduleDataInfo());
    //设置拖拽日程为无效日程
    m_DragScheduleInfo = ScheduleDataInfo();
    QGraphicsItem *listItem = itemAt(point);
    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(listItem);

    if (infoitem != nullptr) {
        setPressSelectInfo(infoitem->getData());
        m_press = true;
        DragInfoItem::setPressFlag(true);
    } else {
        emit signalScheduleShow(false);
    }
    DragPressEvent(point, infoitem);
    this->scene()->update();
    update();
}

int DragInfoGraphicsView::getSlidePos() const
{
    return m_touchSlidePos;
}

void DragInfoGraphicsView::setSlidePos(int pos)
{
    m_touchSlidePos = pos;
    verticalScrollBar()->setValue(m_touchSlidePos);
}

void DragInfoGraphicsView::stopTouchAnimation()
{
    m_touchAnimation->stop();
}

/**
 * @brief DragInfoGraphicsView::DeleteItem      删除日程
 * @param info
 */
void DragInfoGraphicsView::DeleteItem(const ScheduleDataInfo &info)
{
    emit signalViewtransparentFrame(1);
    //删除日程
    CScheduleOperation _scheduleOperation(this);
    _scheduleOperation.deleteSchedule(info);
    emit signalViewtransparentFrame(0);
}

void DragInfoGraphicsView::setBuildName(const QString &buildName)
{
    m_buildName = buildName;
}

QString DragInfoGraphicsView::getBuildName() const
{
    return m_buildName;
}

/**
 * @brief DragInfoGraphicsView::setSelectSearchSchedule     设置选中搜索日程
 * @param scheduleInfo
 */
void DragInfoGraphicsView::setSelectSearchSchedule(const ScheduleDataInfo &scheduleInfo)
{
    setPressSelectInfo(scheduleInfo);
}

void DragInfoGraphicsView::setDragPixmap(QDrag *drag, DragInfoItem *item)
{
    Q_UNUSED(item);
    //设置一个1*1的透明图片，要不然关闭窗口特效会有一个小黑点
    QPixmap pixmap(1, 1);
    pixmap.fill(Qt::transparent);
    drag->setPixmap(pixmap);
    //设置图标位置为鼠标位置
    drag->setHotSpot(QPoint(0, 0));
}

void DragInfoGraphicsView::slotCreate(const QDateTime &date)
{
    emit signalViewtransparentFrame(1);
    CScheduleDlg dlg(1, this);
    QDateTime tDatatime;
    tDatatime.setDate(date.date());

    if (date.date() == QDate::currentDate()) {
        tDatatime.setTime(QTime::currentTime());
    } else {
        tDatatime.setTime(QTime(8, 0));
    }
    dlg.setDate(tDatatime);
    dlg.setAllDay(true);

    if (dlg.exec() == DDialog::Accepted) {
        emit signalsUpdateShcedule();
    }
    emit signalViewtransparentFrame(0);
}

ScheduleDataInfo DragInfoGraphicsView::getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate)
{
    ScheduleDataInfo info;

    if (beginDate.daysTo(endDate) > 0) {
        info.setBeginDateTime(QDateTime(beginDate.date(), QTime(0, 0, 0)));
        info.setEndDateTime(QDateTime(endDate.date(), QTime(23, 59, 59)));
    } else {
        info.setBeginDateTime(QDateTime(endDate.date(), QTime(0, 0, 0)));
        info.setEndDateTime(QDateTime(beginDate.date(), QTime(23, 59, 00)));
    }
    info.setTitleName(tr("New Event"));
    info.setAllDay(true);
    info.setRemindData(RemindData(1, QTime(9, 0)));
    info.setID(0);
    info.setRecurID(0);
    return info;
}

void DragInfoGraphicsView::ShowSchedule(DragInfoItem *infoitem)
{
    if (infoitem == nullptr)
        return;
    emit signalScheduleShow(true, infoitem->getData());

}

void DragInfoGraphicsView::setTheMe(int type)
{
    Q_UNUSED(type);
    DPalette _painte;
    //获取外框背景色
    m_outerBorderColor = _painte.color(QPalette::Active, QPalette::Window);
}

void DragInfoGraphicsView::slideEvent(QPointF &startPoint, QPointF &stopPort)
{
    qreal _movingLine {0};
    //获取滑动方向
    touchGestureOperation::TouchMovingDirection _touchMovingDir =
        touchGestureOperation::getTouchMovingDir(startPoint, stopPort, _movingLine);
    m_touchMovingDir = _touchMovingDir;
    //切换标志 0 不切换  1 下一页  -1 上一页
    int delta {0};
    //移动偏移 25则切换
    const int moveOffset = 25;
    switch (_touchMovingDir) {
    case touchGestureOperation::T_TOP:
    case touchGestureOperation::T_BOTTOM: {
        const int pos_Diff_Y = qFloor(stopPort.y() - startPoint.y());
        verticalScrollBar()->setValue(verticalScrollBar()->sliderPosition() - pos_Diff_Y);
        startPoint = stopPort;
        break;
    }
    case touchGestureOperation::T_LEFT: {
        if (_movingLine > moveOffset) {
            delta = 1;
            startPoint = stopPort;
        }
        break;
    }
    case touchGestureOperation::T_RIGHT: {
        if (_movingLine > moveOffset) {
            delta = -1;
            startPoint = stopPort;
        }
        break;
    }
    default:
        break;
    }
    if (delta != 0) {
        emit signalAngleDelta(delta);
    }
}

/**
 * @brief DragInfoGraphicsView::updateInfo      更新日程数据显示
 */
void DragInfoGraphicsView::updateInfo()
{
    //如果拖拽日程有效则更新为不是移动日程
    if (m_DragScheduleInfo.isValid() && m_DragScheduleInfo.getID() != 0) {
        m_DragScheduleInfo.setIsMoveInfo(false);
        //设置选择日程状态
        setPressSelectInfo(m_DragScheduleInfo);
    }
}

int DragInfoGraphicsView::getDragStatus() const
{
    return m_DragStatus;
}

void DragInfoGraphicsView::setShowRadius(bool leftShow, bool rightShow)
{
    m_leftShowRadius = leftShow;
    m_rightShowRadius = rightShow;
}

/**
 * @brief DragInfoGraphicsView::slotDeleteItem      删除日程
 */
void DragInfoGraphicsView::slotDeleteItem()
{
    //获取选中日程
    ScheduleDataInfo _pressSchedule = DragInfoItem::getPressSchedule();
    //判断是否有效,如果为有效日程且日程类型不为节日或纪念日则删除
    if (_pressSchedule.isValid() && _pressSchedule.getType() != 4) {
        CScheduleOperation _scheduleOperation(this);
        _scheduleOperation.deleteSchedule(_pressSchedule);
        //设置选择日程为无效日程
        setPressSelectInfo(ScheduleDataInfo());
        //设置拖拽日程为无效日程
        m_DragScheduleInfo = DragInfoItem::getPressSchedule();
    }
}

void DragInfoGraphicsView::slotSwitchPrePage(const QDate &focusDate, bool isSwitchView)
{
    emit signalSwitchPrePage();
    if (isSwitchView) {
        m_Scene->signalSwitchView(focusDate, true);
    } else {
        setSceneCurrentItemFocus(focusDate);
    }
}

void DragInfoGraphicsView::slotSwitchNextPage(const QDate &focusDate, bool isSwitchView)
{
    emit signalSwitchNextPage();
    //如果需要切换视图则
    if (isSwitchView) {
        m_Scene->signalSwitchView(focusDate, true);
    } else {
        setSceneCurrentItemFocus(focusDate);
    }
}

void DragInfoGraphicsView::slotContextMenu(CFocusItem *item)
{
    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(item);
    if (infoitem != nullptr) {
        //如果为节假日则退出不展示右击菜单
        if (infoitem->getData().getType() == 4)
            return;
        //快捷键调出右击菜单
        m_Scene->setIsContextMenu(true);
        m_rightMenu->clear();
        m_rightMenu->addAction(m_editAction);
        m_rightMenu->addAction(m_deleteAction);
        QPointF itemPos = QPointF(infoitem->rect().x() + infoitem->rect().width() / 2, infoitem->rect().y() + infoitem->rect().height() / 2);
        QPointF scene_pos = infoitem->mapToScene(itemPos);
        QPointF view_pos = mapFromScene(scene_pos);
        QPoint screen_pos = mapToGlobal(view_pos.toPoint());
        QAction *action_t = m_rightMenu->exec(screen_pos);

        if (action_t == m_editAction) {
            emit signalViewtransparentFrame(1);
            CScheduleDlg dlg(0, this);
            dlg.setData(infoitem->getData());
            if (dlg.exec() == DDialog::Accepted) {
                emit signalsUpdateShcedule();
            }
            emit signalViewtransparentFrame(0);
        } else if (action_t == m_deleteAction) {
            DeleteItem(infoitem->getData());
        }
        m_Scene->setIsContextMenu(false);
        m_Scene->currentFocusItemUpdate();
    }
}

void DragInfoGraphicsView::setSceneCurrentItemFocus(const QDate &focusDate)
{
    Q_UNUSED(focusDate);
}
