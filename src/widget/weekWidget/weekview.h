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
/**
 * @brief The CWeekView class
 */
class CWeekView : public QWidget
{
    Q_OBJECT
public:
    explicit CWeekView(QWidget *parent = nullptr);
    ~CWeekView();
    void setFirstWeekDay(int weekday);
    void setCurrentDate(const QDate date);
    void setTheMe(int type = 0);
    void setsearchfalg(bool flag);
public slots:
    void cellClicked(QWidget *cell);
    void slotprev();
    void slotnext();
signals:
    /**
     * @brief signalsSelectDate 选择日期的信号
     * @param date 选择的日期
     * @param currentDate 今天的时间
     */
    void signalsSelectDate(QDate date, QDate currentDate);
    /**
     * @brief signalIsDragging 判断是否是拖拽状态
     * @param isDragging 是否是拖拽状态
     */
    void signalIsDragging(bool &isDragging);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private:
    void paintCell(QWidget *cell);
    bool eventFilter(QObject *o, QEvent *e) override;
    void setSelectedCell(int index);
    void updateDate();
private:
    /**
     * @brief m_cellList list
     *
     * 周数list
     */
    QList<QWidget *> m_cellList;
    //选择的日期
    QDate m_selectDate;
    //周数对应的日期
    QDate m_days[10];
    //是否被选择
    bool m_cellSelectable = true;
    //选择的索引
    int m_selectedCell = 0;
    //字体
    QFont m_dayNumFont;
    //默认字体颜色
    QColor m_defaultTextColor = Qt::black;
    //默认背景色
    QColor m_backgrounddefaultColor = Qt::white;
    //当天的字体颜色
    QColor m_currentDayTextColor = Qt::white;
    //今天的背景色
    QColor m_backgroundcurrentDayColor = "#0081FF";
    //背景色
    QColor m_fillColor = Qt::white;
    //每周起始的周数
    int m_firstWeekDay;
    int m_weekAddDay = 0;
    //系统主题类型
    int m_themetype = 1;
    //选择标志
    bool m_searchfalg = false;
};

#endif // MONTDAYVIEW_H
