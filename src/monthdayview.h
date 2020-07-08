/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
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
#ifndef MONTHDAYVIEW_H
#define MONTHDAYVIEW_H

#include <QObject>
#include <DFrame>
#include <QDate>

DWIDGET_USE_NAMESPACE

class CMonthWidget;
class CMonthDayView : public DFrame
{
    Q_OBJECT
public:
    explicit CMonthDayView(QWidget *parent = nullptr);
    ~CMonthDayView();
    void setCurrentDate(const QDate date);
    void setRCurrentDate(const QDate date);
    void setTheMe(int type = 0);
    void setwindowFixw(int w, int rw);
    void setsearchfalg(bool flag);
signals:
    void signalsSelectDate(QDate date);
    void signalsCurrentDate(QDate date);
private:
    CMonthWidget                *m_monthWidget;
    QDate                       m_selectDate;
    QDate                       m_days[12];

    int                         m_fixwidth = 200;
    int                         m_realwidth = 100;
    bool                        m_searchfalg = false;
    int                         m_radius = 8;
};

class CMonthRect;
class CMonthWidget : public QWidget
{
    Q_OBJECT
public:
    CMonthWidget(QWidget *parent = nullptr);
    ~CMonthWidget() override;
    void setDate(const QDate date[12]);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void updateSize();
    int getMousePosItem(const QPointF &pos);
signals:
    void signalsSelectDate(QDate date);
private:
    QVector<CMonthRect *>  m_MonthItem;
    QDate                       m_days[12];

};

class CMonthRect
{
public:
    CMonthRect();
    void setDate(const QDate &date);
    QDate getDate()const;
    QRectF rect() const;
    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h);
    void paintItem(QPainter *painter, const QRectF &rect);
    void setDevicePixelRatio(const qreal pixel);
    static void setTheMe(int type);
    static void setSelectRect(CMonthRect *selectRect);
private:
    QRectF                              m_rect;
    QDate                               m_Date;
    static int                          m_themetype ;
    static qreal                        m_DevicePixelRatio;

    static QColor                       m_defaultTextColor;
    static QColor                       m_backgrounddefaultColor;
    static QColor                       m_currentDayTextColor;
    static QColor                       m_backgroundcurrentDayColor;
    static QColor                       m_fillColor;
    static QFont                        m_dayNumFont;
    static CMonthRect                  *m_SelectRect;
    QColor                              m_selectColor;
};

#endif // MONTDAYVIEW_H
