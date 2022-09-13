// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CREATSHCEDULEITEM_H
#define CREATSHCEDULEITEM_H

#include "itemwidget.h"
#include "../data/schedulestructs.h"
#include "../data/schedulecolourmanage.h"
#include <QVBoxLayout>

class scheduleitemdate;
class scheduleitem;
class scheduleitemwidget : public DWidget
{
    Q_OBJECT
public:
    explicit scheduleitemwidget(QWidget *parent = nullptr);
    ~scheduleitemwidget();

    void setScheduleDtailInfo(QVector<ScheduleDtailInfo> &scheduleInfo);
    void addscheduleitem();
signals:
    void signalItemPress(const ScheduleDtailInfo &info);

private:
    void sortScheduleWithTime();

private:
    ScheduleDtailInfo m_scheduleDtailInfo;
    QVector<ScheduleDtailInfo> m_scheduleInfo;
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
    void signalItemPress(const ScheduleDtailInfo &info);

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
    void setScheduleDtailInfo(ScheduleDtailInfo &scheduelDtailInfo);

    QColor DetailsColor();

protected:
    void paintEvent(QPaintEvent *event);

private:
    ScheduleDtailInfo m_scheduleDtailInfo;
    QColor m_DateTimeColor;

    const int m_LeftMargin = 13;
};

#endif // CREATSHCEDULEITEM_H
