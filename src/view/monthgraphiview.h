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
#include "calendardbus.h"
#include "schedulestructs.h"
#include "draginfographicsview.h"

#include <DGraphicsView>
#include <DMenu>

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QDate>
#include <QDrag>

#include <dtkwidget_global.h>

DWIDGET_USE_NAMESPACE

class CDayGraphicsItem;
class CMonthSchceduleWidgetItem;
class CMonthSchceduleView;
/**
 * @brief The CMonthGraphiview class
 * 月视图显示一个月的日期所在的视图
 */
class CMonthGraphiview : public DragInfoGraphicsView
{
    Q_OBJECT
public:
    explicit CMonthGraphiview(QWidget *parent = nullptr);
    ~CMonthGraphiview() override;
    void setTheMe(int type = 0) override;
public:
    void setDate(const QDate date[42]);
    /**
     * @brief setFestivalInfo 设置节日信息
     * @param info 节日信息
     */
    void setFestivalInfo(const QVector<FestivalInfo> &info)
    {
        m_festivallist = info;
    }
    void setLunarInfo(QMap<QDate, CaLunarDayInfo> *lunarCache);
    void setLunarVisible(bool visible);
    void setScheduleInfo(const QVector<ScheduleDateRangeInfo> &info);
    void setSelectSchedule(const ScheduleDtailInfo &scheduleInfo);
private:
    void updateSize();
    void updateLunar();
    void updateInfo();
    char getFestivalInfoByDate(const QDate &date);
    QPointF getItemPos(const QPoint &p, const QRectF &itemRect);
    void updateScheduleInfo(const ScheduleDtailInfo &info);
    void DeleteItem(const ScheduleDtailInfo &info);
    void setPressSelectInfo(const ScheduleDtailInfo &info);
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *) override;
private:
    void setDragPixmap(QDrag *drag,DragInfoItem *item) override;
    bool MeetCreationConditions(const QDateTime &date) override;
    bool IsEqualtime(const QDateTime &timeFirst,const QDateTime &timeSecond) override;
    bool JudgeIsCreate(const QPointF &pos)  override;
    void RightClickToCreate(QGraphicsItem *listItem,const QPoint &pos) override;
    void MoveInfoProcess(ScheduleDtailInfo &info,const QPointF &pos) override;
    QDateTime getDragScheduleInfoBeginTime(const QDateTime &moveDateTime) override;
    QDateTime getDragScheduleInfoEndTime(const QDateTime &moveDateTime) override;
    PosInItem getPosInItem(const QPoint &p,const QRectF &itemRect)override;
    QDateTime getPosDate(const QPoint &p)override;
    void upDateInfoShow(const DragStatus &status = NONE,const ScheduleDtailInfo &info =ScheduleDtailInfo())override;
signals:
    /**
     * @brief signalFontChange 字体大小改变的信号
     */
    void signalFontChange();
    /**
     * @brief signalsViewSelectDate 选择日程的信号
     * @param date 选择的日期
     */
    void signalsViewSelectDate(QDate date);
    /**
     * @brief signalsSchceduleUpdate 更新日程的信号
     * @param id 日程id
     */
    void signalsSchceduleUpdate(const int id = 0);
public slots:
    void slotCreate(const QDateTime &date) override;
    void slotdelete(const int id =0);
    void slotDeleteItem();
private:
    /**
     * @brief m_DayItem item容器
     *
     * 存放每天的item
     * 构成一个月的view
     */
    QVector<CDayGraphicsItem *>         m_DayItem;
    /**
     * @brief m_lunarCache 字典
     *
     * 存储日期和阴历信息
     * 是日期和阴历信息形成一一对应的关系
     */
    QMap<QDate, CaLunarDayInfo> *m_lunarCache = nullptr;
    /**
     * @brief m_festivallist 容器
     *
     * 存放节假日的信息
     */
    QVector<FestivalInfo>               m_festivallist;
    //当前月
    int                                 m_currentMonth;
    /**
     * @brief m_MonthSchceduleView view
     *
     * 月视图的日程view
     */
    CMonthSchceduleView *m_MonthSchceduleView = nullptr;
    //日程信息
    QVector<ScheduleDateRangeInfo>      m_shceludelistdata;
};
/**
 * @brief The CDayGraphicsItem class
 * 每一天所在的item
 */
class CDayGraphicsItem :public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum HolidayStatus {H_NONE = 0x00,H_REST = 0x01,H_WORK = 0x02};
    explicit CDayGraphicsItem( QGraphicsItem *parent = nullptr);
    ~CDayGraphicsItem() override;
    void setData(const QDate &date);
    void setLunar(const QString &lunar);
    void setStatus(const HolidayStatus &status);
    /**
     * @brief setCurrentMonth 设置当前月
     * @param isCurrent 是否是当前月
     */
    void setCurrentMonth(bool isCurrent)
    {
        m_IsCurrentMonth = isCurrent;
    }
    void setTheMe(int type = 0);
    /**
     * @brief getDate 获取日期
     * @return 日期
     */
    QDate getDate()const
    {
        return m_Date;
    }
    void setitemnum(int num);
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
private:
    //每天日期
    QDate                   m_Date;
    //阴历
    QString                 m_DayLunar;
    //每天的类型
    HolidayStatus           m_DayStatus;
    //日期的文字字体
    QFont                   m_dayNumFont;
    //日期的文字颜色
    QColor                  m_dayNumColor;
    //今天的文字颜色
    QColor                  m_dayNumCurrentColor;
    //农历的字体
    QFont                   m_LunerFont;
    //农历的颜色
    QColor                  m_LunerColor;
    //背景色
    QColor                  m_BorderColor;
    //是否是今天
    bool                    m_IsCurrentMonth = false;
    //一般日期的背景色
    QColor                  m_fillColor = Qt::white;
    //补班日期的背景色
    QColor                  m_banColor = "#FBE9B7";
    //节假日休息的背景色
    QColor                  m_xiuColor = "#D4FFB3";
    //系统主题类型
    int                     m_themetype = 0;
    //今天的颜色
    QColor                  m_currentColor;
    //item数量
    int m_itemnum = 0;
    //圆角半径
    const int m_radius = 18;

public:
    //是否显示阴历
    static bool             m_LunarVisible;
};

#endif // MONTHGRAPHIVIEW_H
