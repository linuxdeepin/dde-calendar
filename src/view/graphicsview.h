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
#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include "schedulestructs.h"
#include "draginfographicsview.h"

#include <DGraphicsView>
#include <DMenu>

#include <QGraphicsScene>
#include <QAction>
#include <QMutex>
#include <QDrag>

DWIDGET_USE_NAMESPACE

typedef struct _tagScheduleclassificationInfo {
    QDateTime begindate;
    QDateTime enddate;
    QVector<ScheduleDtailInfo> vData;

} ScheduleclassificationInfo;

class CScheduleCoorManage;
class CScheduleItem;
class CGraphicsView : public DragInfoGraphicsView
{
    Q_OBJECT
public:
    CGraphicsView(QWidget *parent, int viewType = 0);
    ~CGraphicsView() override;
    void setMargins(int left, int top, int right, int bottom);
    QMargins getMargins()
    {
        return m_margins;
    }
    void updateHigh();
    void setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin);
    void setRange(QDate begin, QDate end);

    CScheduleCoorManage *getCoorManage()
    {
        return m_coorManage;
    }
    void setInfo(const QVector<ScheduleDtailInfo> &info);
    void  addSchduleItem(const ScheduleDtailInfo &info, QDate date, int index, int totalNum, int type, int viewtype, int maxnum);
    void  deleteSchduleItem(CScheduleItem *item);
    void  setSelectSchedule(const ScheduleDtailInfo &info);
    void  clearSchdule();

    void setMinTime(const int &minTime)
    {
        m_minTime = minTime;
    }
    void setMaxNum(const int maxnum)
    {
        m_sMaxNum = maxnum;
    }
    void keepCenterOnScene();

    void scheduleClassificationType(QVector<ScheduleDtailInfo> &scheduleInfolist,
                                    QList<ScheduleclassificationInfo> &info);
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

#ifndef QT_NO_WHEELEVENT
    void wheelEvent(QWheelEvent *event) override;
#endif

    /************************************************************************
    Function:       resizeEvent()
    Description:    窗口大小改变事件
    Input:          event 窗口大小改变事件
    Output:         无
    Return:         无
    Others:         无
    ************************************************************************/
    void resizeEvent(QResizeEvent *event) override;

    /************************************************************************
    Function:       getLargeScaleFlag()
    Description:    获取大刻度显示
    Input:          LRFlag 水平刻度，TBFlag 垂直刻度
    Output:         无
    Return:         无
    Others:         无
    ************************************************************************/
    void getLargeScaleFlag(bool &LRFlag, bool &TBFlag);
    void setFirstWeekday(int weekday);
    void setTime(QTime time);
    void updateInfo();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    /**
     * @brief paintBackground 绘制背景--周试图周六周天的背景色和每天的分割线
     * @param painter painter对象
     */
    void paintBackground(QPainter &painter);
public slots:
    void scrollBarValueChangedSlot();
    void slotDoubleEvent(int type);
    void slotDeleteItem();
    void slotScrollBar();
    void slotUpdateScene();
signals:
    void signalsPosHours(QVector<int> vPos, QVector<int> vHours, int cuttrnttimetype = 0);
    void signalsCurrentScheduleDate(QDate date);
    void signalSceneUpdate();
private:
    int checkDay(int weekday);
    void TimeRound(QDateTime &dtime);
    QDateTime TimeRounding(const QDateTime &time);
    void centerOnScene(const QPointF &pos);
    void setSceneHeightScale(const QPointF &pos);
public:
    void setTheMe(int type = 0) override;
protected:
    void slotCreate(const QDateTime &date) override;
    bool MeetCreationConditions(const QDateTime &date) override;
    void upDateInfoShow(const DragStatus &status = NONE,const ScheduleDtailInfo &info =ScheduleDtailInfo())override;
    QDateTime getPosDate(const QPoint &p) override;
    void ShowSchedule(DragInfoItem *infoitem) override;
    void MoveInfoProcess(ScheduleDtailInfo &info,const QPointF &pos) override;
    PosInItem getPosInItem(const QPoint &p,const QRectF &itemRect) override;
    ScheduleDtailInfo getScheduleInfo(const QDateTime &beginDate,const QDateTime &endDate) override;
    bool IsEqualtime(const QDateTime &timeFirst,const QDateTime &timeSecond) override;
    bool JudgeIsCreate(const QPointF &pos) override;
    void RightClickToCreate(QGraphicsItem *listItem,const QPoint &pos) override;
    QDateTime getDragScheduleInfoBeginTime(const QDateTime &moveDateTime) override;
    QDateTime getDragScheduleInfoEndTime(const QDateTime &moveDateTime) override;
private:
    CScheduleCoorManage *m_coorManage = nullptr;
    QVector<CScheduleItem *>        m_vScheduleItem;
    QMargins                        m_margins;                     //四周空白
    bool                            m_LRFlag;          //水平线
    QPen                            m_LRPen;           //水平线画笔
    bool                            m_TBFlag;          //垂直线
    QPen                            m_TBPen;           //垂直线画笔
    QVector<int>                    m_vLRLarge;        //大刻度像素位置
    QVector<int>                    m_vTBLarge;        //大刻度像素位置
    qreal                           m_dayInterval;
    qreal                           m_timeInterval;
    int                             m_firstWeekDay;
    qint64 m_totalDay;
    qreal                           m_sceneHeightScale =0;

    QColor                          m_weekcolor = "#4F9BFF";
    QColor                          m_currenttimecolor = "#F74444";
    int                             m_cuttrnttimetype = 0;
    QTimer *m_timer = nullptr;
    QMutex                          m_Mutex;
    int                             m_viewType = 0;
    bool                            m_updateDflag  = false;
    int                             m_rightmagin = 0;

    QVector<ScheduleDtailInfo>      m_scheduleInfo;
    QDate                           m_beginDate;
    QDate                           m_endDate;
    int                             m_minTime;      //最小高度对应的最小时间
    int                             m_sMaxNum = 4;
};

#endif // GRAPHICSVIEW_H
