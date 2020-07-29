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
#ifndef WEEKVIEW_H
#define WEEKVIEW_H

#include <QObject>
#include <QWidget>
#include <QDate>
class CWeekView : public QWidget
{
    Q_OBJECT
public:
    explicit CWeekView(QWidget *parent = nullptr);
    ~CWeekView();
    void setFirstWeekDay(int weekday);
    void setCurrentDate(const QDate date);
    void setTheMe(int type = 0);
//    void setwindowFixw(int w, int rw);
    void setsearchfalg(bool flag);
public slots:
    void cellClicked(QWidget *cell);
    void slotprev();
    void slotnext();
signals:
    void signalsSelectDate(QDate date, QDate currentDate);
    void signalIsDragging(bool &isDragging);
protected:
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);
private:
    void paintCell(QWidget *cell);
    bool eventFilter(QObject *o, QEvent *e);
    void setSelectedCell(int index);
    void updateDate();
private:
    QList<QWidget *> m_cellList;
    QDate m_selectDate;
    QDate m_days[10];
    bool m_cellSelectable = true;
    int m_selectedCell = 0;
    QFont m_dayNumFont;

    QColor m_defaultTextColor = Qt::black;
    QColor m_backgrounddefaultColor = Qt::white;
    QColor m_currentDayTextColor = Qt::white;
    QColor m_backgroundcurrentDayColor = "#0081FF";
    QColor m_fillColor = Qt::white;
    int m_firstWeekDay;
    int m_weekAddDay = 0;
    int m_themetype = 1;
    int m_fixwidth = 200;
    int m_realwidth = 100;
    bool m_searchfalg = false;
};

#endif // MONTDAYVIEW_H
