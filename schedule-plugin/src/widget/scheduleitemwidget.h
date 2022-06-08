/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#ifndef CREATSHCEDULEITEM_H
#define CREATSHCEDULEITEM_H

#include "itemwidget.h"
#include "dschedule.h"

#include <QVBoxLayout>

class scheduleitemdate;
class scheduleitem;
class scheduleitemwidget : public DWidget
{
    Q_OBJECT
public:
    explicit scheduleitemwidget(QWidget *parent = nullptr);
    ~scheduleitemwidget();

    void setScheduleDtailInfo(const DSchedule::List &scheduleInfo);
    void addscheduleitem();
signals:
    void signalItemPress(const DSchedule::Ptr &info);

private:
    void sortScheduleWithTime();

private:
    DSchedule::Ptr m_scheduleDtailInfo;
    DSchedule::List m_scheduleInfo;
};

class scheduleitem : public ItemWidget
{
    Q_OBJECT
public:
    explicit scheduleitem(QWidget *parent = nullptr);

private:
    void DrawItem(QPainter &painter) override;
    void drawScheduleColorLine(QPainter &painter);
    void drawTime(QPainter &painter);
    void drawDetails(QPainter &painter);

protected:
    void mousePressEvent(QMouseEvent *event) override;
signals:
    void signalItemPress(const DSchedule::Ptr &info);

private:
    const int m_timeLeftMargin = 13;
    const int m_timeTopMargin = 9;
    const int m_LineRightMargin = 84;
    const int m_DetailsRightMargin = 7;
    const int m_DetailsTopMargin = 7;
    const int m_DetailsWidth = 256;
    const int m_EllipsisWidth = 8;
};

class scheduleitemdate : public QWidget
{
    Q_OBJECT
public:
    explicit scheduleitemdate(QWidget *parent = nullptr);
    void setScheduleDtailInfo(const DSchedule::Ptr &scheduelDtailInfo);

    QColor DetailsColor();

protected:
    void paintEvent(QPaintEvent *event);

private:
    DSchedule::Ptr m_scheduleDtailInfo;
    QColor m_DateTimeColor;

    const int m_LeftMargin = 13;
};

#endif // CREATSHCEDULEITEM_H
