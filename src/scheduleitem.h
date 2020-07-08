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
#ifndef SCHEDULEITEM_H
#define SCHEDULEITEM_H

#include <QGraphicsItem>
#include <QDateTime>
#include <QRectF>
#include <QPainterPath>

#include "schedulestructs.h"
#include "draginfoitem.h"
class CScheduleCoorManage;
class CScheduleItem :  public DragInfoItem
{
    Q_OBJECT
public:
    CScheduleItem(QRectF rect,QGraphicsItem *parent = nullptr, int type = 0);
    ~CScheduleItem() Q_DECL_OVERRIDE;
    void setData(const ScheduleDtailInfo &info, QDate date,  int totalNum);
    bool hasSelectSchedule(const ScheduleDtailInfo &info);
    int getType()
    {
        return  m_type;
    }
    ScheduleDtailInfo getData() const
    {
        return m_vScheduleInfo;
    }
    QDate getdate()const
    {
        return m_date;
    }
private:
    void splitText(QFont font, int w, int h, QString str, QStringList &liststr, QFontMetrics &fontm);
private:
    void paintBackground(QPainter *painter,const QRectF &rect,const int isPixMap = false) override;
private:
    QDate                               m_date;
    int                                 m_type = 0;
    int                                 m_totalNum;
    QColor                              m_transparentcolor;

};

#endif // SCHEDULEITEM_H
