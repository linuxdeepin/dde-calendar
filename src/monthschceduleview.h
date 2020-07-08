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

#include <DLabel>
#include <QMouseEvent>
#include <DPushButton>
#include <DListWidget>
#include <DFontSizeManager>
#include <QGraphicsRectItem>
#include <QGraphicsScene>

#include "schedulestructs.h"
#include "scheduledatamanage.h"
#include "draginfoitem.h"

DWIDGET_USE_NAMESPACE
class CMonthSchceduleWidgetItem;
class QVBoxLayout;
class CMonthSchceduleNumButton;
class SchecduleRemindWidget;
class QPropertyAnimation;
class QSequentialAnimationGroup;
class CWeekScheduleView;
class CMonthSchceduleView : public QObject
{
    Q_OBJECT

public:
    CMonthSchceduleView(QWidget *parent,QGraphicsScene *scene);
    ~CMonthSchceduleView();
    void setallsize(int w, int h, int left, int top, int buttom, int itemHeight = 22);
    void setData(QVector<ScheduleDateRangeInfo> &data, int currentMonth);
    void setTheMe(int type = 0);
    void updateData();
    void updateHigh();
    QVector<QGraphicsRectItem *> getScheduleShowItem() const;
    int getSchceduleHeight() const
    {
        return m_ItemHeight;
    }
    void updateDate(const ScheduleDtailInfo &info);
    void changeDate(const ScheduleDtailInfo &info);
    void updateDate(const int row,const ScheduleDtailInfo &info);
signals:
    void signalsUpdateShcedule(int id = 0);
    void signalsCurrentScheduleDate(QDate date);
    void signalViewtransparentFrame(int type);
    void signalUpdateUI(int type);
    void signalPressScheduleShow(const bool isShow, const ScheduleDtailInfo &out = ScheduleDtailInfo());
public slots:
    void slotdeleteitem(CMonthSchceduleWidgetItem *item);
    void slotedititem(CMonthSchceduleWidgetItem *item, int type = 0);
    void slotFontChange();
private:
    void updateDateShow(QVector<QVector<MScheduleDateRangeInfo> > &vCMDaySchedule,QVector<QGraphicsRectItem *> &schudeleShowItem);
    void splitSchedule(MScheduleDateRangeInfo &old, QVector<MScheduleDateRangeInfo> &newData);
    void createScheduleItemWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem);
    void createScheduleNumWidget(MScheduleDateRangeInfo info, int cnum,QVector<QGraphicsRectItem *> &schudeleShowItem);
    void computePos(int cnum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh);
private:
    QVector<ScheduleDateRangeInfo>                  m_data;
    int                                             m_cNum = 2;//日程层数
    QWidget                                         *m_parernt;
    int                                             m_currentMonth=0;
    QDate                                           m_beginDate;
    QDate                                           m_endDate;
    int                                             m_width =0;
    int                                             m_height=0;
    int                                             m_leftMagin=0;
    int                                             m_topMagin=0;
    int                                             m_buttommagin=0;
    QVector<CWeekScheduleView *>                    m_weekSchedule;
    int                                             m_ItemHeight = 22;
    QGraphicsScene                                  *m_Scene;

};

class CWeekScheduleView :public QObject
{
    Q_OBJECT
    typedef QVector<MScheduleDateRangeInfo> RowScheduleInfo;
public:
    CWeekScheduleView(QObject *parent = nullptr);
    ~CWeekScheduleView();
public:
    void setData(QVector<ScheduleDateRangeInfo> &data,const int position,const int count =7);
    bool addData(const ScheduleDtailInfo &info);
    void changeDate(const ScheduleDtailInfo &info);
    void setHeight(const int ScheduleHeight,const int DayHeigth);
    QVector<RowScheduleInfo> getMScheduleInfo() const
    {
        return m_MScheduleInfo;
    }
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
    QVector<QGraphicsRectItem *>            m_scheduleShowItem;
    QVector<RowScheduleInfo>                m_MScheduleInfo;
    QVector<ScheduleDtailInfo>              m_ScheduleInfo;
    QVector<int >                           m_ColumnScheduleCount;
    int                                     m_ScheduleHeight=0;
    int                                     m_DayHeight=0;
    int                                     m_MaxNum=0;
    QDate                                   beginDate;
    QDate                                   endDate;
    int                                     m_colum=0;
};


class CMonthSchceduleNumButton : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    CMonthSchceduleNumButton(QGraphicsItem *parent = nullptr);
    ~CMonthSchceduleNumButton() Q_DECL_OVERRIDE;
    void setColor(QColor color1, QColor color2, bool GradientFlag = false);
    void setText(QColor tcolor, QFont font, QPoint pos);
    void setSizeType(DFontSizeManager::SizeType sizeType);
    void setTransparentB(bool t, QColor tcolor);
    void setDate(QDate date)
    {
        m_date = date;
    }
    QDate getDate()const
    {
        return  m_date;
    }
    void setData(int  num)
    {
        m_num = num;
    }
signals:
    void signalsCurrentScheduleDate(QDate date);
    void signalPressScheduleShow(const bool isShow, const ScheduleDtailInfo &out = ScheduleDtailInfo());
protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
private:
    bool                  m_GradientFlag;
    QColor                m_color1;
    QColor                m_color2;
    QColor                m_textcolor;
    QFont                 m_font;
    QPoint                m_pos;
    int                   m_num;
    QColor                m_transparentcolor;
    bool                  m_transparentf = false;
    QDate                 m_date;
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
    QPoint                                  m_pos;
};

#endif // CSHCEDULEDAYVIEW_H
