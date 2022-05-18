/*
   * Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
   *
   * Author:     chenhaifeng <chenhaifeng@uniontech.com>
   *
   * Maintainer: chenhaifeng <chenhaifeng@uniontech.com>
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
#ifndef CMONTHSCHEDULENUMITEM_H
#define CMONTHSCHEDULENUMITEM_H

#include "cfocusitem.h"
#include "dschedule.h"
#include "scheduledaterangeinfo.h"

#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE
/**
 * @brief The CMonthScheduleNumItem class
 * 月视图 还有N项 标签
 */
class CMonthScheduleNumItem : public CFocusItem
{
    Q_OBJECT
public:
    explicit CMonthScheduleNumItem(QGraphicsItem *parent = nullptr);
    ~CMonthScheduleNumItem() override;
    //背景色
    void setColor(QColor color1, QColor color2);
    //设置字体颜色
    void setText(QColor tColor, QFont font);
    //设置字体大小
    void setSizeType(DFontSizeManager::SizeType sizeType);
    //设置显示多少项
    void setData(int num)
    {
        m_num = num;
    }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QColor m_color1;
    QColor m_color2;
    QColor m_textcolor;
    QFont m_font;
    int m_num;
    DFontSizeManager::SizeType m_SizeType = DFontSizeManager::T8;
};
#endif // CMONTHSCHEDULENUMITEM_H
