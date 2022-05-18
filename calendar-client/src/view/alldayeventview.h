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
#ifndef ALLDAYEVENTVIEW_H
#define ALLDAYEVENTVIEW_H

#include "graphicsItem/calldayscheduleitem.h"
#include "graphicsItem/cweekdaybackgrounditem.h"
#include "cweekdaygraphicsview.h"

#include <DFontSizeManager>

#include <QGraphicsScene>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class CScheduleCoorManage;

class CAllDayEventWeekView : public CWeekDayGraphicsview
{
    Q_OBJECT
public:
    CAllDayEventWeekView(QWidget *parent = nullptr, ViewPosition type = WeekPos);
    ~CAllDayEventWeekView() override;
    void setDayData(const QVector<QVector<DSchedule>> &vlistData);
    void setInfo(const QVector<DSchedule> &info);
    QVector<QVector<DSchedule>> &getListData()
    {
        return m_vlistData;
    }
    void updateHeight();
    //获取搜索选中日程
    void setSelectSearchSchedule(const DSchedule &info) override;
    void setMargins(int left, int top, int right, int bottom);
    //更新日程显示
    void updateInfo() override;
signals:
    void signalUpdatePaint(const int topM);
    void signalSceneUpdate();
public slots:
    void slotUpdateScene();
private slots:
    void slotDoubleEvent();
public:
    void setTheMe(int type = 0) override;
private:
    void changeEvent(QEvent *event) override;
    bool MeetCreationConditions(const QDateTime &date) override;
    void slotCreate(const QDateTime &date) override;
    //判断时间是否相等
    bool IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond) override;
    //根据鼠标移动的距离判断是否创建日程
    bool JudgeIsCreate(const QPointF &pos) override;
    void RightClickToCreate(QGraphicsItem *listItem, const QPoint &pos) override;
    void MoveInfoProcess(DSchedule &info, const QPointF &pos) override;
    QDateTime getDragScheduleInfoBeginTime(const QDateTime &moveDateTime) override;
    QDateTime getDragScheduleInfoEndTime(const QDateTime &moveDateTime) override;
    PosInItem getPosInItem(const QPoint &p, const QRectF &itemRect) override;
    QDateTime getPosDate(const QPoint &p) override;
    void upDateInfoShow(const DragStatus &status = NONE, const DSchedule &info = DSchedule()) override;

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private:
    void updateDateShow();
    void createItemWidget(int index, bool average = false);
    void updateItemHeightByFontSize();
private:
    int itemHeight = 22;
    QVector<QVector<DSchedule>> m_vlistData;
    QVector<DSchedule> m_scheduleInfo;
    QVector<CAllDayScheduleItem *> m_baseShowItem;
    bool m_updateDflag = false;
};

#endif // CSCHEDULEDAYVIEW_H
