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
#include "monthgraphiview.h"
#include "../widget/monthWidget/monthschceduleview.h"
#include "../dialog/schceduledlg.h"
#include "../dialog/schcedulectrldlg.h"
#include "../dialog/myschceduleview.h"
#include "constants.h"

#include <DHiDPIHelper>

#include <QAction>
#include <QMimeData>
#include <QShortcut>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainterPath>

bool CDayGraphicsItem::m_LunarVisible = false;
/**
 * @brief CMonthGraphiview 构造函数
 * @param parent 父类
 */
CMonthGraphiview::CMonthGraphiview(QWidget *parent)
    : DragInfoGraphicsView (parent)
{
    //快捷键
    QShortcut *shortcut = new QShortcut(parent);
    shortcut->setKey(QKeySequence(QLatin1String("Delete")));
    //快捷键对应的功能槽
    connect(shortcut,
            &QShortcut::activated, this,
            &CMonthGraphiview::slotDeleteItem);
    //月视图日程
    m_MonthSchceduleView = new CMonthSchceduleView(this,m_Scene);
    connect(this,&CMonthGraphiview::signalFontChange,
            m_MonthSchceduleView,&CMonthSchceduleView::slotFontChange);
    //每一天一个item
    for (int i = 0; i < DDEMonthCalendar::ItemSizeofMonthDay; ++i) {
        CDayGraphicsItem *item = new CDayGraphicsItem();
        item->setZValue(-1);
        //添加到容器
        m_DayItem.append(item);
        //添加到场景
        m_Scene->addItem(item);
        //设置编号
        item->setitemnum(i);
    }
    updateSize();
}
/**
  * @brief ~CMonthGraphiview 析构函数
  */
CMonthGraphiview::~CMonthGraphiview()
{
    delete m_MonthSchceduleView;

    for (int i = 0; i < m_DayItem.size(); ++i) {
        CDayGraphicsItem *item = m_DayItem.at(i);
        delete item;
        item = nullptr;
    }

    m_DayItem.clear();
}
/**
 * @brief setTheMe 根据系统主题类型设置颜色
 * @param type 系统主题类型
 */
void CMonthGraphiview::setTheMe(int type)
{
    m_themetype = type;
    //设置每个item的系统类型
    for (int i = 0; i < m_DayItem.size(); ++i) {
        m_DayItem.at(i)->setTheMe(type);
    }
    //设置月视图日程的系统类型
    m_MonthSchceduleView->setTheMe(type);
}
/**
 * @brief setDate 设置日期
 * @param date 日期
 */
void CMonthGraphiview::setDate(const QDate date[42])
{
    //设置月份
    if (date[0].day() !=1) {
        //如果左上角的日期不是1号，则为下一个月
        m_currentMonth = date[0].addMonths(1).month();
    } else {
        //本月
        m_currentMonth = date[0].month();
    }
    //设置每天的信息
    for (int i = 0; i < m_DayItem.size(); ++i) {
        //设置时间
        m_DayItem.at(i)->setData(date[i]);
        //是否是当前月
        m_DayItem.at(i)->setCurrentMonth(date[i].month() == m_currentMonth);
        //设置每天标识
        m_DayItem.at(i)->setStatus(CDayGraphicsItem::HolidayStatus(getFestivalInfoByDate(date[i])));
    }
    //更新场景
    this->scene()->update();
}
/**
 * @brief setLunarInfo 设置阴历信息
 * @param lunarCache 日期和阴历信息模板
 */
void CMonthGraphiview::setLunarInfo(QMap<QDate, CaLunarDayInfo> *lunarCache)
{
    m_lunarCache = lunarCache;
    //更新阴历信息
    updateLunar();

}
/**
 * @brief setLunarVisible 设置是否现实阴历信息
 * @param visible 是否现实阴历信息
 */
void CMonthGraphiview::setLunarVisible(bool visible)
{
    CDayGraphicsItem::m_LunarVisible = visible;
}
/**
 * @brief setScheduleInfo 设置日程信息
 * @param info 日程信息
 */
void CMonthGraphiview::setScheduleInfo(const QVector<ScheduleDateRangeInfo> &info)
{
    m_shceludelistdata = info;
    //更新日程信息
    updateInfo();
}
/**
 * @brief setSelectSchedule 设置选择的日程
 * @param scheduleInfo 日程信息
 */
void CMonthGraphiview::setSelectSchedule(const ScheduleDtailInfo &scheduleInfo)
{
    //获取日程item
    QVector<QGraphicsRectItem *> mscheduleShowBtn = m_MonthSchceduleView->getScheduleShowItem();
    //遍历日程item
    for (int i = 0; i < mscheduleShowBtn.size(); ++i) {
        CMonthSchceduleWidgetItem *item = dynamic_cast<CMonthSchceduleWidgetItem *>(mscheduleShowBtn.at(i));
        //如果item为空，继续遍历
        if (item == nullptr) continue;
        //日程信息匹配
        if (scheduleInfo == item->getData()) {
            item->setStartValue(0);
            item->setEndValue(4);
            //开始动画
            item->startAnimation();
        }
    }
}
/**
 * @brief updateSize 更新日期所在矩形的大小
 */
void CMonthGraphiview::updateSize()
{
    //设置场景
    m_Scene->setSceneRect(this->viewport()->rect());
    qreal w = m_Scene->width() / DDEMonthCalendar::AFewDaysofWeek;
    qreal h = m_Scene->height() / DDEMonthCalendar::LinesNumofMonth;
    QRectF rect ;
    int w_offset = 0;
    int h_offset = 0;
    //设置每天的矩形大小
    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        h_offset = i / DDEMonthCalendar::AFewDaysofWeek;
        w_offset = i % DDEMonthCalendar::AFewDaysofWeek;
        rect.setRect(w*w_offset,
                     h*h_offset,
                     w,
                     h);
        m_DayItem.at(i)->setRect(rect);
    }
}
/**
 * @brief updateLunar 更新阴历信息
 */
void CMonthGraphiview::updateLunar()
{
    QDate date;
    CaLunarDayInfo info;
    QString lunarStr("");

    for (int i = 0 ; i < m_DayItem.size(); ++i) {
        //每天的日期
        date = m_DayItem.at(i)->getDate();
        if (m_lunarCache->contains(date)) {
            info = m_lunarCache->value(date);
            //如果是阴历初一，则设置为月份
            if (info.mLunarDayName == "初一") {
                info.mLunarDayName = info.mLunarMonthName + info.mLunarDayName;
            }

            if (info.mTerm.isEmpty()) {
                lunarStr = info.mLunarDayName;
            } else {
                lunarStr = info.mTerm;
            }
        } else {
            lunarStr = "";
        }
        m_DayItem.at(i)->setLunar(lunarStr);
    }
}
/**
 * @brief updateInfo 更新信息
 */
void CMonthGraphiview::updateInfo()
{
    int h = m_MonthSchceduleView->getSchceduleHeight();
    m_MonthSchceduleView->setallsize(this->viewport()->width(),
                                     this->viewport()->height(),
                                     0, 0, 0, h);
    m_MonthSchceduleView->setData(m_shceludelistdata, 1);
    //判断拖拽的类型
    switch (m_DragStatus) {
    case IsCreate:
        //新建日程
        upDateInfoShow(IsCreate,m_DragScheduleInfo);
        break;
    case ChangeWhole:
        //修改日程
        upDateInfoShow(ChangeWhole,m_DragScheduleInfo);
        break;
    default:

        break;
    }
    update();
}
/**
 * @brief getFestivalInfoByDate 通过日期获取节日信息
 * @param date 日期
 * @return 节日字符串
 */
char CMonthGraphiview::getFestivalInfoByDate(const QDate &date)
{
    for (int i = 0; i < m_festivallist.count(); i++) {
        for (int j = 0; j < m_festivallist[i].listHoliday.count(); j++) {
            if (m_festivallist[i].listHoliday[j].date == date) {
                return m_festivallist[i].listHoliday[j].status;
            }
        }
    }
    return 0;
}
/**
 * @brief getItemPos 获取鼠标点击的那个点坐标
 * @param p 鼠标点击的那个点
 * @param itemRect item所在的矩形
 * @return 点坐标
 */
QPointF CMonthGraphiview::getItemPos(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    return  QPointF(scenePos.x()-itemRect.x(),
                    scenePos.y()-itemRect.y());
}
/**
 * @brief getPosInItem 判断是从日程的哪个位置开始拖拽的
 * @param p 鼠标点击的点
 * @param itemRect 日程所在天的矩形
 * @return 鼠标位置
 */
CMonthGraphiview::PosInItem CMonthGraphiview::getPosInItem(const QPoint &p, const QRectF &itemRect)
{
    QPointF scenePos = this->mapToScene(p);
    //鼠标点击的点
    QPointF itemPos = QPointF(scenePos.x()-itemRect.x(),
                              scenePos.y()-itemRect.y());
    qreal bottomy = itemRect.width()- itemPos.x();
    //左边界
    if (itemPos.x()<5) {
        return LEFT;
    }
    //右边界
    if (bottomy <5) {
        return RIGHT;
    }
    //中间
    return MIDDLE;
}
/**
 * @brief getPosDate 获取点击位置的时间
 * @param p 点击的点
 * @return 时间
 */
QDateTime CMonthGraphiview::getPosDate(const QPoint &p)
{
    if (!this->sceneRect().contains(p))
        return m_MoveDate;
    QRectF rect = this->sceneRect();
    qreal x =0;
    qreal y = 0;
    //保证左右拖拽不会超出边界
    if (p.x()<0) {
        x =0;
    } else if (p.x()>(rect.width()-10)) {
        x = rect.width()-10;
    } else {
        x = p.x();
    }
    //保证上下拖拽不会超出边界
    if (p.y()<0) {
        y =0;
    } else if (p.y()>(rect.height()-10)) {
        y = rect.height()-10;
    } else {
        y = p.y();
    }
    //日程所在列数
    int xoffset = qFloor(x / (rect.width() / DDEMonthCalendar::AFewDaysofWeek)) % DDEMonthCalendar::AFewDaysofWeek;
    //日程所在行数
    int yoffset = qFloor(y / (rect.height() / DDEMonthCalendar::LinesNumofMonth)) % DDEMonthCalendar::LinesNumofMonth;
    //返回日期
    return  QDateTime(m_DayItem[xoffset+yoffset*7]->getDate(),
                      QTime(0,0,0));
}
/**
 * @brief upDateInfoShow 更新日程信息展示
 * @param status 鼠标状态
 * @param info 日程信息
 */
void CMonthGraphiview::upDateInfoShow(const CMonthGraphiview::DragStatus &status, const ScheduleDtailInfo &info)
{
    switch (status) {
    case NONE:
        Q_UNUSED(info);
        break;
    case ChangeBegin:
    case ChangeEnd: {
        //鼠标移动结束，更新日程信息
        m_MonthSchceduleView->changeDate(info);
    }
    break;
    case ChangeWhole: {
    }
    break;
    case IsCreate:
        //新建日程，
        m_MonthSchceduleView->updateDate(info);
        break;
    }
}
/**
 * @brief updateScheduleInfo 更新日程信息
 * @param info 日程信息
 */
void CMonthGraphiview::updateScheduleInfo(const ScheduleDtailInfo &info)
{
    if (info.rpeat >0) {
        CSchceduleDlg::ChangeRecurInfo(this,info,
                                       m_PressScheduleInfo,m_themetype);
    } else {
        CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(
            info);
    }
}
/**
 * @brief DeleteItem 删除日程
 * @param info 日程信息
 */
void CMonthGraphiview::DeleteItem(const ScheduleDtailInfo &info)
{
    emit signalViewtransparentFrame(1);

    if (info.rpeat == 0) {
        //非重复日程
        CSchceduleCtrlDlg msgBox(this);
        msgBox.setText(tr("You are deleting an event."));
        msgBox.setInformativeText(tr("Are you sure you want to delete this event?"));
        msgBox.addPushButton(tr("Cancel"), true);
        msgBox.addWaringButton(tr("Delete"), true);
        msgBox.exec();

        if (msgBox.clickButton() == 0) {
            //取消按钮
            emit signalViewtransparentFrame(0);
            return;
        } else if (msgBox.clickButton() == 1) {
            //删除按钮
            CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(info.id);
        }
    } else {
        //重复日程
        if (info.RecurID == 0) {
            //重复日程的第一个日程
            CSchceduleCtrlDlg msgBox(this);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete all occurrences of this event, or only the selected occurrence?"));
            msgBox.addPushButton(tr("Cancel"));
            msgBox.addPushButton(tr("Delete All"));
            msgBox.addWaringButton(tr("Delete Only This Event"));
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                //取消按钮
                emit signalViewtransparentFrame(0);
                return;
            } else if (msgBox.clickButton() == 1) {
                //删除所有按钮
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->deleteScheduleInfoById(info.id);
            } else if (msgBox.clickButton() == 2) {
                //仅删除此日程按钮
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(info.id, newschedule);
                newschedule.ignore.append(info.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            }
        } else {
            //重复日程的中间日程
            CSchceduleCtrlDlg msgBox(this);
            msgBox.setText(tr("You are deleting an event."));
            msgBox.setInformativeText(tr("Do you want to delete this and all future occurrences of this event, or only the selected occurrence?"));
            msgBox.addPushButton(tr("Cancel"));
            msgBox.addPushButton(tr("Delete All Future Events"));
            msgBox.addWaringButton(tr("Delete Only This Event"));
            msgBox.exec();

            if (msgBox.clickButton() == 0) {
                //取消按钮
                emit signalViewtransparentFrame(0);
                return;
            } else if (msgBox.clickButton() == 1) {
                //删除未来所有日程按钮
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(info.id, newschedule);
                newschedule.enddata.type = 2;
                newschedule.enddata.date = info.beginDateTime.addDays(-1);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            } else if (msgBox.clickButton() == 2) {
                //仅删除此日程按钮
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(info.id, newschedule);
                newschedule.ignore.append(info.beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            }
        }
    }
    //发送更新日程信号
    emit signalsUpdateShcedule();
    emit signalViewtransparentFrame(0);
}
/**
 * @brief setPressSelectInfo 设置鼠标选中的日程信息
 * @param info 日程信息
 */
void CMonthGraphiview::setPressSelectInfo(const ScheduleDtailInfo &info)
{
    CScheduleDataManage::getScheduleDataManage()->setPressSelectInfo(info);
}
/**
 * @brief mouseDoubleClickEvent 鼠标双击事件
 * @param event 鼠标事件
 */
void CMonthGraphiview::mouseDoubleClickEvent(QMouseEvent *event)
{
    //不是鼠标左键，退出
    if (event->button() != Qt::LeftButton) {
        return;
    }
    //获取点击的item
    QGraphicsItem *listItem =itemAt(event->pos());
    //还有n项item
    CMonthSchceduleNumButton *item = dynamic_cast<CMonthSchceduleNumButton *>(listItem);

    if (item!= nullptr) {
        //双击切换视图
        if (item->getDate().year() > DDECalendar::QueryEarliestYear) {
            emit signalsViewSelectDate(item->getDate());
        }
        return;
    }
    //日程信息item
    CMonthSchceduleWidgetItem *infoitem = dynamic_cast<CMonthSchceduleWidgetItem *>(listItem);
    //展示日程详情
    if (infoitem != nullptr) {
        //我的日程
        CMySchceduleView dlg(infoitem->getData(), this);
        connect(&dlg, &CMySchceduleView::signalsEditorDelete, this, &CMonthGraphiview::signalsUpdateShcedule);
        connect(&dlg, &CMySchceduleView::signalViewtransparentFrame,
                this, &CMonthGraphiview::signalViewtransparentFrame);
        dlg.exec();
        return;
    }
    //day视图item
    CDayGraphicsItem *Dayitem = dynamic_cast<CDayGraphicsItem *>(listItem);
    //双击位置不同，动作不同
    if (Dayitem !=nullptr) {
        //鼠标点击位置
        QPointF point = getItemPos(event->pos(),Dayitem->rect());
        if (point.y()<38) {
            //双击切换视图
            if (Dayitem->getDate().year() > DDECalendar::QueryEarliestYear) {
                emit signalsViewSelectDate(Dayitem->getDate());
            }
        } else {
            //双击新建日程
            slotCreate(QDateTime(Dayitem->getDate(),QTime(0,0,0)));
        }
    }
}
/**
 * @brief resizeEvent 窗口大小调整
 * @param event 窗口大小调整事件
 */
void CMonthGraphiview::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    //更新大小
    updateSize();
    //更新信息
    updateInfo();
}
/**
 * @brief changeEvent 字体大小改变
 * @param event QT事件
 */
void CMonthGraphiview::changeEvent(QEvent *event)
{
    if (event->type() ==QEvent::FontChange) {
        emit signalFontChange();
    }
}
/**
 * @brief wheelEvent 鼠标滑轮事件
 */
void CMonthGraphiview::wheelEvent(QWheelEvent *e)
{
    Q_UNUSED(e);
}
/**
 * @brief setDragPixmap 设置拖拽的像素图
 * @param drag 拖拽
 * @param item 日程
 */
void CMonthGraphiview::setDragPixmap(QDrag *drag, DragInfoItem *item)
{
    CMonthSchceduleWidgetItem *infoitem = dynamic_cast<CMonthSchceduleWidgetItem *>(item);
    //设置拖拽的图像
    drag->setPixmap(infoitem->getPixmap());
}
/**
 * @brief MeetCreationConditions 拖拽日期在当前页面有效
 * @param date 移动日程的日期
 * @return 移动日期和点击日期的差值小于43
 */
bool CMonthGraphiview::MeetCreationConditions(const QDateTime &date)
{
    return  qAbs(date.daysTo(m_PressDate)<43);
}
/**
 * @brief IsEqualtime 判断时间是否相等
 * @param timeFirst 拖拽前的日期
 * @param timeSecond 拖拽后的日程
 * @return 前后日期是否相等
 */
bool CMonthGraphiview::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
{
    return timeFirst.date()==timeSecond.date();
}
/**
 * @brief JudgeIsCreate 根据鼠标移动的距离判断是否创建日程
 * @param pos 鼠标点击的点的坐标
 * @return 是否创建日程
 */
bool CMonthGraphiview::JudgeIsCreate(const QPointF &pos)
{
    return qAbs(pos.x()-m_PressPos.x())>20 ||qAbs(m_PressDate.daysTo(getPosDate(pos.toPoint())))>0;
}
/**
 * @brief RightClickToCreate 右键点击新建日程
 * @param listItem 这个天所在的item
 * @param pos 点击的点
 */
void CMonthGraphiview::RightClickToCreate(QGraphicsItem *listItem,const QPoint &pos)
{
    Q_UNUSED(pos);
    CDayGraphicsItem *Dayitem = dynamic_cast<CDayGraphicsItem *>(listItem);

    if (Dayitem != nullptr) {
        m_rightMenu->clear();
        //添加新建动作
        m_rightMenu->addAction(m_createAction);
        //设置点击的日期
        m_createDate.setDate(Dayitem->getDate());
        m_rightMenu->exec(QCursor::pos());
    }
}
/**
 * @brief MoveInfoProcess 拖拽过程
 * @param info 日程信息
 * @param pos 鼠标移动点的坐标
 */
void CMonthGraphiview::MoveInfoProcess(ScheduleDtailInfo &info, const QPointF &pos)
{
    //鼠标点击和移动的时间差
    qint64 offset       = m_PressDate.daysTo(m_MoveDate);
    //日程开始时间
    info.beginDateTime  = info.beginDateTime.addDays(offset);
    //日程结束时间
    info.endDateTime    = info.endDateTime.addDays(offset);
    qreal y = 0;
    //一个月的视图所在矩形
    QRectF rect = this->sceneRect();
    //保证日程移动在视图的矩形内
    if (pos.y()<0) {
        y =0;
    } else if (pos.y()>rect.height()) {
        y = rect.height();
    } else {
        y = pos.y();
    }
    //该日程在第几行
    int yoffset = qFloor(y / (rect.height() / DDEMonthCalendar::LinesNumofMonth)) % DDEMonthCalendar::LinesNumofMonth;
    info.IsMoveInfo = true;
    //更新日程
    m_MonthSchceduleView->updateDate(yoffset,info);
}
/**
 * @brief getDragScheduleInfoBeginTime 获取拖拽日程的开始时间
 * @param moveDateTime 拖拽的日期
 * @return 日程的开始时间
 */
QDateTime CMonthGraphiview::getDragScheduleInfoBeginTime(const QDateTime &moveDateTime)
{
    return moveDateTime.daysTo(m_InfoEndTime)<0 ?
           QDateTime(m_InfoEndTime.date(),m_InfoBeginTime.time()):
           QDateTime(moveDateTime.date(),m_InfoBeginTime.time());
}
/**
 * @brief getDragScheduleInfoEndTime 获取拖拽日程的结束时间
 * @param moveDateTime 拖拽的日期
 * @return 日程的结束时间
 */
QDateTime CMonthGraphiview::getDragScheduleInfoEndTime(const QDateTime &moveDateTime)
{
    return m_InfoBeginTime.daysTo(moveDateTime)<0 ?
           QDateTime(m_InfoBeginTime.date(),m_InfoEndTime.time()):
           QDateTime(moveDateTime.date(),m_InfoEndTime.time());
}
/**
 * @brief slotCreate 新建日程
 * @param date 日期
 */
void CMonthGraphiview::slotCreate(const QDateTime &date)
{
    emit signalViewtransparentFrame(1);
    CSchceduleDlg dlg(1, this);
    QDateTime tDatatime;
    tDatatime.setDate(date.date());
    //根据是否是今天，设置不同的时间
    if (date.date() == QDate::currentDate()) {
        tDatatime.setTime(QTime::currentTime());
    } else {
        tDatatime.setTime(QTime(8, 0));
    }
    //设置时间
    dlg.setDate(tDatatime);
    dlg.setAllDay(true);

    if (dlg.exec() == DDialog::Accepted) {
        emit signalsUpdateShcedule();
        emit signalsSchceduleUpdate(0);
    }
    emit signalViewtransparentFrame(0);
}
/**
 * @brief slotdelete 发送更新日程的信号
 * @param id
 */
void CMonthGraphiview::slotdelete(const int id)
{
    Q_UNUSED(id);
    emit signalsSchceduleUpdate(0);
}
/**
 * @brief slotDeleteItem 删除日程
 */
void CMonthGraphiview::slotDeleteItem()
{
    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().type.ID <0) {
        return;
    }
    //只要不是节假日，就可以删除日程
    if (CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo().type.ID != DDECalendar::FestivalTypeID) {
        DeleteItem(CScheduleDataManage::getScheduleDataManage()->getPressSelectInfo());
    }
    //设置点击选择的日程信息
    CScheduleDataManage::getScheduleDataManage()->setPressSelectInfo(ScheduleDtailInfo());
}
/**
 * @brief CDayGraphicsItem 构造函数
 * @param parent 父类
 */
CDayGraphicsItem::CDayGraphicsItem(QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , m_Date(QDate::currentDate())
    , m_DayLunar("")
    , m_DayStatus(H_NONE)
{
    //设置day日期的字体大小
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwentyfour);
    //设置字体权重
    m_dayNumFont.setWeight(QFont::Light);
    //设置阴历day的字体大小
    m_LunerFont.setPixelSize(DDECalendar::FontSizeTwelve);
    //设置字体权重
    m_LunerFont.setWeight(QFont::Normal);
}
/**
  * @brief ~CDayGraphicsItem 析构函数
  */
CDayGraphicsItem::~CDayGraphicsItem()
{

}
/**
 * @brief setData 设置日期
 * @param date 日期
 */
void CDayGraphicsItem::setData(const QDate &date)
{
    m_Date = date;
    setLunar("");
}
/**
 * @brief setLunar 设置阴历信息
 * @param lunar 阴历信息
 */
void CDayGraphicsItem::setLunar(const QString &lunar)
{
    m_DayLunar = lunar;
}
/**
 * @brief setStatus 设置班休
 * @param status 班休
 */
void CDayGraphicsItem::setStatus(const CDayGraphicsItem::HolidayStatus &status)
{
    m_DayStatus = status;
}
/**
 * @brief setTheMe 根据系统类型设置颜色
 * @param type 系统类型
 */
void CDayGraphicsItem::setTheMe(int type)
{
    m_themetype = type;
    //根据系统主题类型设置颜色
    if (type == 0 || type == 1) {
        //浅色主题或跟随系统
        //设置day字体的颜色
        m_dayNumColor = "#000000";
        //设置今天day字体的颜色
        m_dayNumCurrentColor = "#FFFFFF";
        //设置阴历day的颜色
        m_LunerColor = "#5E5E5E";
        //设置阴历day颜色的透明度
        m_LunerColor.setAlphaF(0.5);

        m_fillColor = Qt::white;
        //设置上班的颜色
        m_banColor = "#FF7171";
        //设置上班颜色的透明度
        m_banColor.setAlphaF(0.1);
        //设置休息的颜色
        m_xiuColor = "#ADFF71";
        //设置休息颜色的透明度
        m_xiuColor.setAlphaF(0.1);
        //设置边框颜色
        m_BorderColor = "#000000";
        //设置边框颜色的透明度
        m_BorderColor.setAlphaF(0.05);
    } else if (type == 2) {
        //深色系统主题
        m_dayNumColor = "#C0C6D4";
        m_dayNumCurrentColor = "#B8D3FF";

        m_LunerColor = "#ABDAFF";
        m_LunerColor.setAlphaF(0.5);

        m_fillColor = "#000000";
        m_fillColor.setAlphaF(0.05);
        m_banColor = "#FF7171";
        m_banColor.setAlphaF(0.1);
        m_xiuColor = "#ADFF71";
        m_xiuColor.setAlphaF(0.1);

        m_BorderColor = "#000000";
        m_BorderColor.setAlphaF(0.05);
    }
    update();
}
/**
 * @brief setitemnum 为每天的矩形设置标号
 * @param num 标号
 */
void CDayGraphicsItem::setitemnum(int num)
{
    m_itemnum = num;
}
/**
 * @brief paint 绘图事件
 * @param painter 绘图
 * @param option
 * @param widget
 */
void CDayGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    const int hh = 36;
    //设置今天日期背景色为系统活动色
    m_currentColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    painter->setRenderHints(QPainter::Antialiasing);
    //绘制背景
    if (m_LunarVisible) {
        switch (m_DayStatus) {
        case H_WORK:
            //补班
            painter->setBrush(QBrush(m_banColor));
            break;
        case H_REST:
            //休息
            painter->setBrush(QBrush(m_xiuColor));
            break;
        default:
            //正常
            painter->setBrush(QBrush(m_fillColor));
            break;
        }
    } else {
        painter->setBrush(QBrush(m_fillColor));
    }

    if (!m_IsCurrentMonth) {
        painter->setOpacity(0.4);
    }
    QPen pen;
    pen.setWidth(1);
    pen.setColor(m_BorderColor);
    painter->setPen(pen);
    //绘制圆角
    if (m_itemnum == 35) {
        //绘制左下角圆角
        QPainterPath painterpath;
        painterpath.moveTo(0, this->rect().y());
        painterpath.lineTo(0, this->rect().y() + this->rect().height() - m_radius);
        painterpath.arcTo(QRectF(0,
                                 this->rect().y() + this->rect().height() - m_radius * 2,
                                 m_radius * 2,
                                 m_radius * 2),
                          180, 90);
        painterpath.lineTo(this->rect().width(), this->rect().y() + this->rect().height());
        painterpath.lineTo(this->rect().width(), this->rect().y());
        painterpath.lineTo(0, this->rect().y());
        painter->drawPath(painterpath);
    } else if (m_itemnum == 41) {
        //绘制右下角圆角
        QPainterPath painterpath;
        painterpath.moveTo(this->rect().x(), this->rect().y());
        painterpath.lineTo(this->rect().x(), this->rect().y() + this->rect().height());
        painterpath.lineTo(this->rect().x() + this->rect().width() - m_radius, this->rect().y() + this->rect().height());
        painterpath.arcTo(QRectF(this->rect().x() + this->rect().width() - m_radius * 2,
                                 this->rect().y() + this->rect().height() - m_radius * 2,
                                 m_radius * 2,
                                 m_radius * 2),
                          270, 90);
        painterpath.lineTo(this->rect().x() + this->rect().width(), this->rect().y());
        painterpath.lineTo(this->rect().y() + this->rect().height(), this->rect().y());
        painter->drawPath(painterpath);
    } else {
        painter->drawRect(this->rect());
    }

    painter->save();
    painter->restore();
    //绘制日期
    painter->setFont(m_dayNumFont);
    QRectF fillRect;
    //根据是否有阴历信息，设置矩形大小
    if (m_LunarVisible) {
        fillRect.setRect(this->rect().x()+3,this->rect().y()+4, hh, hh);
    } else {
        fillRect.setRect(this->rect().x(),this->rect().y()+4, this->rect().width(), hh);
    }

    if (m_Date ==QDate::currentDate()) {
        //今天
        QFont tfont = m_dayNumFont;
        tfont.setPixelSize(DDECalendar::FontSizeTwenty);
        painter->setFont(tfont);
        painter->setPen(m_dayNumCurrentColor);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(QBrush(m_currentColor));
        painter->setPen(Qt::NoPen);
        //根据是否有阴历信息，在不同的位置绘制今天的背景圆
        if (m_LunarVisible)
            painter->drawEllipse(QRectF(this->rect().x() + 6, this->rect().y() + 4, hh - 8, hh - 8));
        else
            painter->drawEllipse(QRectF((this->rect().width() - hh + 8) / 2 + this->rect().x(), this->rect().y() + 4, hh - 8, hh - 8));
        painter->restore();
    } else {
        painter->setPen(m_dayNumColor);
    }

    fillRect.setY(fillRect.y()-10);
    fillRect.setX(fillRect.x()-1);
    painter->drawText(fillRect,
                      Qt::AlignCenter,
                      QString::number(m_Date.day()));

    painter->save();
    painter->restore();
    //绘制农历
    if (m_LunarVisible) {
        QFontMetrics metrics(m_LunerFont);
        int Lunarwidth = metrics.width(m_DayLunar);
        qreal filleRectX = this->rect().width()-12-3-(58 +Lunarwidth)/2;
        QRectF fillRect(this->rect().x()+filleRectX,
                        this->rect().y()+9,
                        12,
                        12);

        if (filleRectX>hh) {
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setRenderHint(QPainter::HighQualityAntialiasing);
            painter->setRenderHint(QPainter::SmoothPixmapTransform);
            switch (m_DayStatus) {
            case H_WORK: {
                //补班
                QPixmap  pixmap = DHiDPIHelper::loadNxPixmap(":/resources/icon/ban.svg");
                //绘制补班的图像
                painter->drawPixmap(fillRect.toRect(), pixmap);
            }
            break;
            case H_REST: {
                //假期休息
                QPixmap pixmap = DHiDPIHelper::loadNxPixmap(":/resources/icon/xiu.svg");
                //绘制假期休息的图像
                painter->drawPixmap(fillRect.toRect(), pixmap);
            }
            break;
            default:
                break;
            }
        }
        painter->setFont(m_LunerFont);
        painter->setPen(m_LunerColor);
        //绘制阴历信息
        painter->drawText(QRectF(this->rect().x()+this->rect().width() - 58,
                                 this->rect().y()+6, 58, 18), Qt::AlignCenter, m_DayLunar);
    }
}
