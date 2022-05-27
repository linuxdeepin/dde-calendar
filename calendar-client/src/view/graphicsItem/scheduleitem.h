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

#include "dschedule.h"
#include "draginfoitem.h"

#include <QGraphicsItem>
#include <QDateTime>
#include <QRectF>
#include <QPainterPath>

class CScheduleCoorManage;
class CScheduleItem : public DragInfoItem
{
    Q_OBJECT
public:
    CScheduleItem(QRectF rect, QGraphicsItem *parent = nullptr, int type = 0);
    ~CScheduleItem() override;
    //设置显示数据
    void setData(const DSchedule::Ptr &info, QDate date, int totalNum);
    //是否含有选中日程
    bool hasSelectSchedule(const DSchedule::Ptr &info);
    int getType()
    {
        return m_type;
    }
    DSchedule::Ptr getData() const
    {
        return m_vScheduleInfo;
    }
private:
    //根据字体大小,宽度和高度将标题切换为多行
    void splitText(QFont font, int w, int h, QString str, QStringList &listStr, QFontMetrics &fontM);
private slots:
    void timeFormatChanged(int value);
private:
    //绘制item显示效果
    void paintBackground(QPainter *painter, const QRectF &rect, const bool isPixMap = false) override;

private:
    int m_type = 0;
    int m_totalNum;
    QColor m_transparentcolor;
    QString m_timeFormat = "h:mm";
};

#endif // SCHEDULEITEM_H
