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
#ifndef MONTHSCHCEDULEVIEW_H
#define MONTHSCHCEDULEVIEW_H

#include "schedulestructs.h"
#include "draginfoitem.h"

#include <DLabel>
#include <DPushButton>
#include <DListWidget>
#include <DFontSizeManager>

#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsScene>

DWIDGET_USE_NAMESPACE
class CMonthSchceduleWidgetItem;
class QVBoxLayout;
class CMonthSchceduleNumButton;
class SchecduleRemindWidget;
class QPropertyAnimation;
class QSequentialAnimationGroup;
class CWeekScheduleView;
class CScheduleDataManage;
class CMonthSchceduleView : public QObject
{
    Q_OBJECT

public:
    CMonthSchceduleView(QWidget *parent,QGraphicsScene *scene);
    ~CMonthSchceduleView() override;
    void setallsize(int w, int h, int left, int top, int buttom, int itemHeight = 22);
    void setData(QVector<ScheduleDateRangeInfo> &data, int currentMonth);
    void setTheMe(int type = 0);
    void updateData();
    void updateHigh();
    QVector<QGraphicsRectItem *> getScheduleShowItem() const;
    /**
     * @brief getSchceduleHeight 获取日程的高度
     * @return 日程高度
     */
    int getSchceduleHeight() const
    {
        return m_ItemHeight;
    }
    void updateDate(const ScheduleDtailInfo &info);
    void changeDate(const ScheduleDtailInfo &info);
    void updateDate(const int row,const ScheduleDtailInfo &info);
signals:
    /**
     * @brief signalsUpdateShcedule 更新日程的信号
     * @param id 日程的id
     */
    void signalsUpdateShcedule(int id = 0);
    /**
     * @brief signalsCurrentScheduleDate 发送当前日程日期的信号
     * @param date 日期
     */
    void signalsCurrentScheduleDate(QDate date);
    /**
     * @brief signalViewtransparentFrame 设置窗口焦点
     * @param type
     */
    void signalViewtransparentFrame(int type);
    /**
     * @brief signalUpdateUI
     * @param type
     */
    void signalUpdateUI(int type);
    /**
     * @brief signalPressScheduleShow 点击展示日程的信号
     * @param isShow 是否展示
     * @param out 日程信息
     */
    void signalPressScheduleShow(const bool isShow, const ScheduleDtailInfo &out = ScheduleDtailInfo());
public slots:
    void slotdeleteitem(CMonthSchceduleWidgetItem *item);
    void slotedititem(CMonthSchceduleWidgetItem *item, int type = 0);
    void slotFontChange();
private:
    void updateDateShow(QVector<QVector<MScheduleDateRangeInfo> > &vCMDaySchedule,QVector<QGraphicsRectItem *> &schudeleShowItem);
    void createScheduleItemWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem);
    void createScheduleNumWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem);
    void computePos(int cnum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh);
private:
    //日程信息
    QVector<ScheduleDateRangeInfo>                  m_data;
    //日程层数
    int                                             m_cNum = 2;
    //父类
    QWidget *m_parernt = nullptr;
    //当天月份
    int                                             m_currentMonth=0;
    //日程开始时间
    QDate                                           m_beginDate;
    //日程结束时间
    QDate                                           m_endDate;
    //日程宽度
    int                                             m_width =0;
    //日程高度
    int                                             m_height=0;
    //左边距
    int                                             m_leftMagin=0;
    //上边距
    int                                             m_topMagin=0;
    //日程间距
    int                                             m_buttommagin=0;
    //以周为单位的日程信息
    QVector<CWeekScheduleView *>                    m_weekSchedule;
    //日程高度
    int                                             m_ItemHeight = 22;
    //日程场景
    QGraphicsScene *m_Scene = nullptr;
};

class CWeekScheduleView :public QObject
{
    Q_OBJECT
    typedef QVector<MScheduleDateRangeInfo> RowScheduleInfo;
public:
    CWeekScheduleView(QObject *parent = nullptr);
    ~CWeekScheduleView() override;
public:
    void setData(QVector<ScheduleDateRangeInfo> &data,const int position,const int count =7);
    bool addData(const ScheduleDtailInfo &info);
    void changeDate(const ScheduleDtailInfo &info);
    void setHeight(const int ScheduleHeight,const int DayHeigth);
    /**
     * @brief getMScheduleInfo 获取一周的日程信息
     * @return  日程信息
     */
    QVector<RowScheduleInfo> getMScheduleInfo() const
    {
        return m_MScheduleInfo;
    }
    /**
     * @brief getScheduleShowItem 获取展示的日程item
     * @return 日程item
     */
    QVector<QGraphicsRectItem *> &getScheduleShowItem()
    {
        return m_scheduleShowItem;
    }
    void updateSchedule(const bool isNormalDisplay,const ScheduleDtailInfo &info = ScheduleDtailInfo());
    void clearItem();
private:
    void setMaxNum();
    void mScheduleClear();
    void sortAndFilter(QVector<MScheduleDateRangeInfo> &vMDaySchedule);
private:
    //日程item
    QVector<QGraphicsRectItem *>            m_scheduleShowItem;
    //一行日程
    QVector<RowScheduleInfo>                m_MScheduleInfo;
    //日程信息
    QVector<ScheduleDtailInfo>              m_ScheduleInfo;
    //日程个数
    QVector<int >                           m_ColumnScheduleCount;
    //日程高度
    int                                     m_ScheduleHeight=0;
    //day rect高度
    int                                     m_DayHeight=0;
    //最大数量
    int                                     m_MaxNum=0;
    //日程开始时间
    QDate                                   beginDate;
    //日程结束时间
    QDate                                   endDate;
    int                                     m_colum=0;
};


class CMonthSchceduleNumButton : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    CMonthSchceduleNumButton(QGraphicsItem *parent = nullptr);
    ~CMonthSchceduleNumButton() override;
    void setColor(QColor color1, QColor color2, bool GradientFlag = false);
    void setText(QColor tcolor, QFont font, QPoint pos);
    void setSizeType(DFontSizeManager::SizeType sizeType);
    void setTransparentB(bool t, QColor tcolor);
    /**
     * @brief setDate 设置日期
     * @param date 日期
     */
    void setDate(QDate date)
    {
        m_date = date;
    }
    /**
     * @brief getDate 获取日期
     * @return  日期
     */
    QDate getDate()const
    {
        return  m_date;
    }
    /**
     * @brief setData 设置未展示的日程数量
     * @param num 日程数量
     */
    void setData(int  num)
    {
        m_num = num;
    }
signals:
    /**
     * @brief signalsCurrentScheduleDate 选择当前日期的信号
     * @param date 日期
     */
    void signalsCurrentScheduleDate(QDate date);
    /**
     * @brief signalPressScheduleShow 点击展示日程的信号
     * @param isShow 是否展示日程
     * @param out 日程信息
     */
    void signalPressScheduleShow(const bool isShow, const ScheduleDtailInfo &out = ScheduleDtailInfo());
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
private:
    bool                  m_GradientFlag;
    //渐变色1
    QColor                m_color1;
    //渐变色2
    QColor                m_color2;
    //文字颜色
    QColor                m_textcolor;
    QFont                 m_font;
    //点击坐标
    QPoint                m_pos;
    int                   m_num;
    QColor                m_transparentcolor;
    bool                  m_transparentf = false;
    //日期
    QDate                 m_date;
    //日程字号
    DFontSizeManager::SizeType m_SizeType = DFontSizeManager::T8;
};

class CMonthSchceduleWidgetItem :public DragInfoItem
{
    Q_OBJECT
public:
    explicit CMonthSchceduleWidgetItem(QRect rect,QGraphicsItem  *parent = nullptr, int edittype = 0);
    ~CMonthSchceduleWidgetItem() override;
    QPixmap getPixmap();
private:
    void paintBackground(QPainter *painter,const QRectF &rect,const int isPixMap = false) override;
private:
    //点击坐标
    QPoint                                  m_pos;
};

#endif // CSHCEDULEDAYVIEW_H
