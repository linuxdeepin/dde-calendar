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
#include "schedulestructs.h"
#include <DListWidget>
#include "scheduledatamanage.h"
DWIDGET_USE_NAMESPACE
class CMonthSchceduleWidgetItem;
class QVBoxLayout;
class CMonthSchceduleNumButton;
class SchecduleRemindWidget;
class QPropertyAnimation;
class QSequentialAnimationGroup;
class CMonthSchceduleView : public QObject
{
    Q_OBJECT

public:
    CMonthSchceduleView(QWidget *parent);
    ~CMonthSchceduleView();
    void setallsize(int w, int h, int left, int top, int buttom);
    void setData(QVector<ScheduleDateRangeInfo> &data, int currentMonth);
    void setTheMe(int type = 0);
    void updateData();
    void updateHigh();
    QVector<DPushButton *> getScheduleShowItem() const;
signals:
    void signalsUpdateShcedule(int id = 0);
    void signalsCurrentScheduleDate(QDate date);
    void signalViewtransparentFrame(int type);
    void signalUpdateUI(int type);
    void signalPressScheduleShow(const bool isShow, const int id);
public slots:
    void slotdeleteitem(CMonthSchceduleWidgetItem *item);
    void slotedititem(CMonthSchceduleWidgetItem *item, int type = 0);
    void slotupdateItem(CMonthSchceduleWidgetItem *item);
    void slotDeleteItem();
    void slotGetScheduleChange(int h);
private:

    void updateDateShow(QVector<QVector<MScheduleDateRangeInfo> > &vCMDaySchedule);
    void splitSchedule(MScheduleDateRangeInfo &old, QVector<MScheduleDateRangeInfo> &newData);
    void createScheduleItemWidget(MScheduleDateRangeInfo info, int cnum);
    void createScheduleNumWidget(MScheduleDateRangeInfo info, int cnum);
    void computePos(int cnum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh);
private:
    QVector<DPushButton *>         m_scheduleShowItem;
    QVector<ScheduleDateRangeInfo>               m_data;
    int                                          m_cNum = 2;//日程层数
    QWidget                                     *m_parernt;
    int                                          m_currentMonth;
    QDate                                        m_beginDate;
    QDate                                        m_endDate;
    int                                          m_width;
    int                                          m_height;
    int                                          m_leftMagin;
    int                                          m_topMagin;
    int                                          m_buttommagin;
    CMonthSchceduleWidgetItem                   *m_currentitem = nullptr;
    int hieght = 22;
};

class CMonthSchceduleNumButton : public DPushButton
{
    Q_OBJECT

public:
    CMonthSchceduleNumButton(QWidget *parent = nullptr);
    ~CMonthSchceduleNumButton();
    void setColor(QColor color1, QColor color2, bool GradientFlag = false);
    void setText(QColor tcolor, QFont font, QPoint pos);
    void setTransparentB(bool t, QColor tcolor);
    void setDate(QDate date)
    {
        m_date = date;
    }
    void setData(int  num)
    {
        m_num = num;
    }
signals:
    void signalsCurrentScheduleDate(QDate date);
protected:
    void paintEvent(QPaintEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *event);
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
};

class CMonthSchceduleWidgetItem : public DPushButton
{
    Q_OBJECT
    Q_PROPERTY(int setRectOffset WRITE setRectOffset)
public:
    explicit CMonthSchceduleWidgetItem(QWidget *parent = nullptr, int edittype = 0);
    ~CMonthSchceduleWidgetItem() Q_DECL_OVERRIDE;
    void setColor(QColor color1, QColor color2, bool GradientFlag = false);
    void setText(QColor tcolor, QFont font, QPoint pos);
    void getColor(QColor &color1, QColor &color2, bool &GradientFlag);
    void getText(QColor &tcolor, QFont &font, QPoint &pos);
    void setTransparentB(bool t, QColor tcolor);
    void setTransparentB(bool t);
    void setData(ScheduleDtailInfo  vScheduleInfo);
    const ScheduleDtailInfo &getData() const
    {
        return m_ScheduleInfo;
    }
    void setRectOffset(int offset);
    void startAnimation();
    void setStartValue(int offset);
    void setEndValue(int offset);
signals:
    void signalsDelete(CMonthSchceduleWidgetItem *item);
    void signalsEdit(CMonthSchceduleWidgetItem *item, int type = 0);
    void signalsPress(CMonthSchceduleWidgetItem *item);
    void signalViewtransparentFrame(int type);
    void signalUpdateUI(int type);
    void signalPressScheduleShow(const bool isShow, const int id);
    void signalScheduleChange(int h);
public slots:
    void slotEdit();
    void slotDelete();
    void slotDoubleEvent(int type = 0);
    void slotPress();
    void slotScheduleChange(int h);
protected:
    void paintEvent ( QPaintEvent *e) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent (QMouseEvent *event ) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event)Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
private:
    ScheduleDtailInfo     m_ScheduleInfo;
    QAction              *m_editAction;
    QAction              *m_deleteAction;
    bool                  m_GradientFlag;
    QColor                m_color1;
    QColor                m_color2;
    QColor                m_textcolor;
    QFont                 m_font;
    QPoint                m_pos;
    QColor                m_transparentcolor;
    bool                  m_transparentf = false; //按下
    bool                  m_selectflag = false;
    bool                  m_hoverflag = false;
    bool                  m_highflag = false;
    int                   m_editType = 0;
    bool                    m_pressMove = false;
    CSchedulesColor gdcolor;
    QPropertyAnimation *m_properAnimationFirst = nullptr;
    QPropertyAnimation *m_properAnimationSecond = nullptr;
    QSequentialAnimationGroup *m_Group = nullptr;
    QRect              m_rect;
    int hight = 0;

};

#endif // CSHCEDULEDAYVIEW_H
