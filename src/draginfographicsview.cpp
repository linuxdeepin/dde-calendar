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

#include <DMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

#include "schceduledlg.h"
#include "schcedulectrldlg.h"
#include "myschceduleview.h"

DragInfoGraphicsView::DragInfoGraphicsView(DWidget *parent)
    :DGraphicsView (parent),
     m_Scene(new QGraphicsScene(this)),
     m_rightMenu(new DMenu(this)),
     m_MoveDate(QDateTime::currentDateTime())
{
    setFrameShape(QFrame::NoFrame);
    setScene(m_Scene);
    setContentsMargins(0, 0, 0, 0);
    QPalette pal = palette();
    pal.setColor(QPalette::Light, QColor(0, 0, 0, 0));
    pal.setColor(QPalette::Dark, QColor(0, 0, 0, 0));
    pal.setColor(QPalette::Base, QColor(0, 0, 0, 0));
    setPalette(pal);

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
}

DragInfoGraphicsView::~DragInfoGraphicsView()
{

}


void DragInfoGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
//        m_press = false;
//        m_DragStatus =NONE;
//        emit signalScheduleShow(false);
        return;
    }

    setPressSelectInfo(ScheduleDtailInfo());
    QGraphicsItem *listItem =itemAt(event->localPos().toPoint());
    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(listItem);
    if (infoitem != nullptr) {
        setPressSelectInfo(infoitem->getData());
        m_press = true;
        DragInfoItem::setPressFlag(true);
    } else {
        emit signalScheduleShow(false);
    }
    DragPressEvent(event->pos(),infoitem);
    update();
}

void DragInfoGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() ==Qt::RightButton) {
        return;
    }
    mouseReleaseScheduleUpdate();
}


void DragInfoGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    DGraphicsView::mouseMoveEvent(event);
    if (m_press) {
        emit signalScheduleShow(false);
        m_press = false;
        DragInfoItem::setPressFlag(false);
    }
    DragInfoItem *item = dynamic_cast<DragInfoItem *>(itemAt(event->pos()));
    if (item != nullptr) {
        if (item->getData().type.ID != 4) {
            if (m_DragStatus == NONE) {
                switch (getPosInItem(event->pos(),item->rect())) {
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
            if (!IsEqualtime(m_MoveDate,gDate)) {
                m_MoveDate = gDate;
                m_DragScheduleInfo = getScheduleInfo(m_PressDate,m_MoveDate);
                upDateInfoShow(IsCreate,m_DragScheduleInfo);
                setPressSelectInfo(m_DragScheduleInfo);
            }
        }
        break;
    case ChangeBegin:
        if (!IsEqualtime(m_MoveDate,gDate)) {
            m_MoveDate = gDate;
            m_DragScheduleInfo.beginDateTime =
                getDragScheduleInfoBeginTime(m_MoveDate);
            m_DragScheduleInfo.endDateTime = m_InfoEndTime;
            upDateInfoShow(ChangeBegin,m_DragScheduleInfo);
        }
        break;
    case ChangeEnd:
        if (!IsEqualtime(m_MoveDate,gDate)) {
            m_MoveDate = gDate;
            m_DragScheduleInfo.endDateTime =
                getDragScheduleInfoEndTime(m_MoveDate);
            m_DragScheduleInfo.beginDateTime =m_InfoBeginTime;
            upDateInfoShow(ChangeEnd,m_DragScheduleInfo);
        }
        break;
    case ChangeWhole: {
        if (!m_PressRect.contains(event->pos())) {
            Qt::DropAction dropaction = m_Drag->exec( Qt::MoveAction);
            Q_UNUSED(dropaction);
            m_Drag = nullptr;
            m_DragStatus = NONE;
            setCursor(Qt::ArrowCursor);
            m_DragScheduleInfo.IsMoveInfo = false;
            setPressSelectInfo(m_DragScheduleInfo);
            emit signalsUpdateShcedule();
        }
    }
    break;
    default:
        break;
    }
}

void DragInfoGraphicsView::wheelEvent(QWheelEvent *event)
{
    DGraphicsView::wheelEvent(event);
}

void DragInfoGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    DGraphicsView::contextMenuEvent(event);
    if (m_DragStatus ==IsCreate) {
        return;
    }
    emit signalScheduleShow(false);
    m_press = false;
    m_DragStatus =NONE;
    QGraphicsItem *listItem = itemAt(event->pos());

    DragInfoItem *infoitem = dynamic_cast<DragInfoItem *>(listItem);
    if (infoitem !=nullptr) {
        if (infoitem->getData().type.ID !=4) {
            m_rightMenu->clear();
            m_rightMenu->addAction(m_editAction);
            m_rightMenu->addAction(m_deleteAction);
            QAction *action_t = m_rightMenu->exec(QCursor::pos());
            if (action_t == m_editAction) {
                emit signalViewtransparentFrame(1);
                CSchceduleDlg dlg(0, this);
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
            CMySchceduleView dlg(infoitem->getData(), this);
            dlg.exec();
            emit signalViewtransparentFrame(0);

//            qDebug()<<"setFocus";
//            emit signalViewtransparentFrame(1);
//            this->setFocus(Qt::MouseFocusReason);
        }
    } else {
        RightClickToCreate(listItem,event->pos());
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
        ScheduleDtailInfo info =
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->JsonObjectToInfo(rootobj);
        if ((event->source() !=this && info.rpeat >0) ||
                info.type.ID ==4) {
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
    if (!IsEqualtime(m_MoveDate,gDate)) {
        m_MoveDate = gDate;
        QJsonObject rootobj = jsonDoc.object();
        m_DragScheduleInfo =
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->JsonObjectToInfo(rootobj);

        MoveInfoProcess(m_DragScheduleInfo,event->posF());
        setPressSelectInfo(m_DragScheduleInfo);
    }
}

void DragInfoGraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("Info")) {
        if (event->source()!=this || m_MoveDate !=m_PressDate) {
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
    if (e->type() ==QEvent::Leave) {
        if (m_DragStatus ==IsCreate ||
                m_DragStatus == ChangeBegin ||
                m_DragStatus == ChangeEnd)
            mouseReleaseScheduleUpdate();
    }
    return DGraphicsView::event(e);
}

void DragInfoGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left || event->key() ==Qt::Key_Right)
        return;
    DGraphicsView::keyPressEvent(event);
}

void DragInfoGraphicsView::slotCreate()
{
    slotCreate(m_createDate);
}

void DragInfoGraphicsView::setPressSelectInfo(const ScheduleDtailInfo &info)
{
    CScheduleDataManage::getScheduleDataManage()->setPressSelectInfo(info);
}

void DragInfoGraphicsView::updateScheduleInfo(const ScheduleDtailInfo &info)
{
    emit signalViewtransparentFrame(1);
    if (info.rpeat >0) {
        CSchceduleDlg::ChangeRecurInfo(this,info,
                                       m_PressScheduleInfo,m_themetype);
    } else {
        CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(
            info);
    }
    emit signalViewtransparentFrame(0);
}

void DragInfoGraphicsView::DragPressEvent(const QPoint &pos, DragInfoItem *item)
{
    m_PressPos = pos;
    m_PressDate = getPosDate(pos);

    m_MoveDate = m_PressDate.addMonths(-2);
    if (item != nullptr) {
        PosInItem mpressstatus = getPosInItem(pos,item->boundingRect());
        if (mpressstatus != MIDDLE && item->getData().type.ID == 4) {
            return;
        }
        m_DragScheduleInfo = item->getData();
        m_PressScheduleInfo = item->getData();
        m_InfoBeginTime = m_DragScheduleInfo.beginDateTime;
        m_InfoEndTime = m_DragScheduleInfo.endDateTime;
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
            mimeData->setText(m_DragScheduleInfo.titleName);
            mimeData->setData("Info",
                              CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->InfoToJson(m_DragScheduleInfo).toUtf8());

            if (m_Drag ==nullptr) {
                m_Drag = new QDrag(this);
            }
            m_Drag->setMimeData(mimeData);
            setDragPixmap(m_Drag,item);
            QPointF itemPos = QPointF(pos.x()-item->boundingRect().x(),
                                      pos.y()-item->boundingRect().y());
            m_Drag->setHotSpot(itemPos.toPoint());
            break;
        }
    } else {
        m_DragStatus = IsCreate;
        m_isCreate = false;
    }
}

void DragInfoGraphicsView::mouseReleaseScheduleUpdate()
{
    setCursor(Qt::ArrowCursor);
    m_press = false;
    DragInfoItem::setPressFlag(false);
    bool isUpdateInfo = false;
    switch (m_DragStatus) {
    case IsCreate:
        if (MeetCreationConditions(m_MoveDate)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;

            emit signalViewtransparentFrame(1);
            CSchceduleDlg dlg(1, this);
            dlg.setData(m_DragScheduleInfo);
            if (dlg.exec() == DDialog::Accepted) {
            } else {
                setPressSelectInfo(ScheduleDtailInfo());
            }
            emit signalViewtransparentFrame(0);
            isUpdateInfo = true;
        }
        break;
    case ChangeBegin:
        if (!IsEqualtime(m_MoveDate,m_InfoBeginTime)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            updateScheduleInfo(m_DragScheduleInfo);
            isUpdateInfo = true;
        }
        break;
    case ChangeEnd:
        if (!IsEqualtime(m_MoveDate,m_InfoEndTime)) {
            //如果不添加会进入leaveEvent事件内的条件
            m_DragStatus = NONE;
            updateScheduleInfo(m_DragScheduleInfo);
            isUpdateInfo = true;
        }
        break;
    default:
        break;
    }
    m_DragStatus = NONE;
    if (isUpdateInfo) {
        emit signalsUpdateShcedule();
    }
    update();
}

void DragInfoGraphicsView::DeleteItem(const ScheduleDtailInfo &info)
{
    emit signalViewtransparentFrame(1);
    if (info.rpeat == 0) {
        CSchceduleCtrlDlg msgBox(this);
        msgBox.setText(tr("You are deleting an event."));
        msgBox.setInformativeText(tr("Are you sure you want to delete this event?"));
        /*QAbstractButton *noButton = */msgBox.addPushButton(tr("Cancel"), true);
        /*QAbstractButton *yesButton = */msgBox.addWaringButton(tr("Delete"), true);
//        msgBox.updatesize();
//        DPalette pa = yesButton->palette();
//        if (m_themetype == 0 || m_themetype == 1) {
//            pa.setColor(DPalette::ButtonText, Qt::red);
//        } else {
//            pa.setColor(DPalette::ButtonText, "#FF5736");
//        }
//        yesButton->setPalette(pa);
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            emit signalViewtransparentFrame(0);
            return;
        } else if (msgBox.clickButton() == 1) {
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(info.id);
        }
    } else {
        if (info.RecurID == 0) {
            CSchceduleCtrlDlg msgBox(this);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete all occurrences of this event, or only the selected occurrence?"));
            /*QAbstractButton *noButton = */msgBox.addPushButton(tr("Cancel"));
            /*QAbstractButton *yesallbutton = */msgBox.addPushButton(tr("Delete All"));
            /*QAbstractButton *yesButton = */msgBox.addsuggestButton(tr("Delete Only This Event"));
//            msgBox.updatesize();
//            DPalette pa = yesButton->palette();
//            if (m_themetype == 0 || m_themetype == 1) {
//                pa.setColor(DPalette::ButtonText, Qt::white);
//                pa.setColor(DPalette::Dark, QColor("#25B7FF"));
//                pa.setColor(DPalette::Light, QColor("#0098FF"));
//            } else {
//                pa.setColor(DPalette::ButtonText, "#B8D3FF");
//                pa.setColor(DPalette::Dark, QColor("#0056C1"));
//                pa.setColor(DPalette::Light, QColor("#004C9C"));
//            }
//            yesButton->setPalette(pa);
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                emit signalViewtransparentFrame(0);
                return;
            } else if (msgBox.clickButton() == 1) {
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(info.id);
            } else if (msgBox.clickButton() == 2) {

                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(info.id, newschedule);
                newschedule.ignore.append(info.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            }
        } else {
            CSchceduleCtrlDlg msgBox(this);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete this and all future occurrences of this event, or only the selected occurrence?"));
            /*QAbstractButton *noButton = */msgBox.addPushButton(tr("Cancel"));
            /*QAbstractButton *yesallbutton = */msgBox.addPushButton(tr("Delete All Future Events"));
            /*QAbstractButton *yesButton = */msgBox.addsuggestButton(tr("Delete Only This Event"));
//            msgBox.updatesize();
//            DPalette pa = yesButton->palette();
//            if (m_themetype == 0 || m_themetype == 1) {
//                pa.setColor(DPalette::ButtonText, Qt::white);
//                pa.setColor(DPalette::Dark, QColor("#25B7FF"));
//                pa.setColor(DPalette::Light, QColor("#0098FF"));
//            } else {
//                pa.setColor(DPalette::ButtonText, "#B8D3FF");
//                pa.setColor(DPalette::Dark, QColor("#0056C1"));
//                pa.setColor(DPalette::Light, QColor("#004C9C"));
//            }
//            yesButton->setPalette(pa);
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                emit signalViewtransparentFrame(0);
                return;
            } else if (msgBox.clickButton() == 1) {
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(info.id, newschedule);
                newschedule.enddata.type = 2;
                newschedule.enddata.date = info.beginDateTime.addDays(-1);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);

            } else if (msgBox.clickButton() == 2) {
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(info.id, newschedule);
                newschedule.ignore.append(info.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            }
        }
    }
    emit signalsUpdateShcedule();
    emit signalViewtransparentFrame(0);
}

void DragInfoGraphicsView::setDragPixmap(QDrag *drag, DragInfoItem *item)
{
    Q_UNUSED(drag);
    Q_UNUSED(item);
}

void DragInfoGraphicsView::slotCreate(const QDateTime &date)
{
    emit signalViewtransparentFrame(1);
    CSchceduleDlg dlg(1, this);
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

ScheduleDtailInfo DragInfoGraphicsView::getScheduleInfo(const QDateTime &beginDate, const QDateTime &endDate)
{
    ScheduleDtailInfo info;
    if (beginDate.daysTo(endDate)>0) {
        info.beginDateTime = QDateTime(beginDate.date(),QTime(0,0,0));
        info.endDateTime = QDateTime(endDate.date(),QTime(23,59,59));
    } else {
        info.beginDateTime = QDateTime(endDate.date(),QTime(0,0,0));
        info.endDateTime = QDateTime(beginDate.date(),QTime(23,59,00));
    }
    info.titleName = tr("New Event");
    info.allday = true;
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

void DragInfoGraphicsView::ShowSchedule(DragInfoItem *infoitem)
{
    if (infoitem ==nullptr)
        return;
    emit signalScheduleShow(true, infoitem->getData());

}

int DragInfoGraphicsView::getDragStatus() const
{
    return m_DragStatus;
}

