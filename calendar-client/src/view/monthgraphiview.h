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
#ifndef MONTHGRAPHIVIEW_H
#define MONTHGRAPHIVIEW_H

#include "dschedule.h"
#include "huangliData/lunardatastruct.h"
#include "huangliData/dbusdatastruct.h"
#include "draginfographicsview.h"
#include "view/graphicsItem/cmonthdayitem.h"

#include <DGraphicsView>
#include <DMenu>

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QDate>
#include <QDrag>

#include <dtkwidget_global.h>

DWIDGET_USE_NAMESPACE

class CMonthScheduleView;

class CMonthGraphicsview : public DragInfoGraphicsView
{
    Q_OBJECT
public:
    explicit CMonthGraphicsview(QWidget *parent = nullptr);
    ~CMonthGraphicsview() override;
    void setTheMe(int type = 0) override;
public:
    void setDate(const QVector<QDate> &showDate);
    //设置班休信息
    void setFestival(const QMap<QDate, int> &festivalInfo);
    //设置农历信息
    void setLunarInfo(const QMap<QDate, CaHuangLiDayInfo> &lunarCache);
    //设置是否显示农历信息
    void setLunarVisible(bool visible);
    //设置日程信息
    void setScheduleInfo(const QMap<QDate, QVector<DSchedule>> &info);
    //设置搜索选中日程
    void setSelectSearchSchedule(const DSchedule &scheduleInfo) override;
    //设置搜索日程信息
    void setSearchScheduleInfo(const QVector<DSchedule> &searchScheduleInfo);

private:
    void updateSize();
    void updateLunar();
    //更新日程数据显示
    void updateInfo() override;
    QPointF getItemPos(const QPoint &p, const QRectF &itemRect);
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *) override;
    //更新背景上显示的item
    void updateBackgroundShowItem() override;

protected:
    void setSceneCurrentItemFocus(const QDate &focusDate) override;

private:
    void setDragPixmap(QDrag *drag, DragInfoItem *item) override;
    bool MeetCreationConditions(const QDateTime &date) override;
    //判断时间是否相等
    bool IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond) override;
    //根据鼠标移动的距离判断是否创建日程
    bool JudgeIsCreate(const QPointF &pos)  override;
    void RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos) override;
    void MoveInfoProcess(DSchedule &info, const QPointF &pos) override;
    QDateTime getDragScheduleInfoBeginTime(const QDateTime &moveDateTime) override;
    QDateTime getDragScheduleInfoEndTime(const QDateTime &moveDateTime) override;
    PosInItem getPosInItem(const QPoint &p, const QRectF &itemRect)override;
    QDateTime getPosDate(const QPoint &p)override;
    void upDateInfoShow(const DragStatus &status = NONE, const DSchedule &info = DSchedule()) override;
    /**
     * @brief slideEvent            触摸滑动事件处理
     * @param startPoint            触摸开始坐标
     * @param stopPort              触摸结束坐标
     */
    void slideEvent(QPointF &startPoint, QPointF &stopPort) override;
signals:
    void signalFontChange();
    void signalsViewSelectDate(QDate date);
    void signalsScheduleUpdate(const int id = 0);
public slots:
    void slotCreate(const QDateTime &date) override;
private:
    QVector<CMonthDayItem *> m_DayItem;
    QMap<QDate, CaHuangLiDayInfo> m_lunarCache;
    QMap<QDate, int>                    m_festivallist;
    int                                 m_currentMonth;
    CMonthScheduleView                  *m_MonthScheduleView = nullptr;
    QMap<QDate, QVector<DSchedule>> m_schedulelistdata;
};
#endif // MONTHGRAPHIVIEW_H
