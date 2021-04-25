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

#ifndef MONTHWEEKVIEW_H
#define MONTHWEEKVIEW_H

#include "constants.h"
#include "customframe.h"

#include <DWidget>
#include <DLabel>
#include <DHorizontalLine>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE
/**
 * @brief The CMonthWeekView class
 * 月视图日期上面的周
 *
 * 将周末和工作日文字颜色设置不同，以做区分
 */
class CMonthWeekView : public DWidget
{
    Q_OBJECT
public:
    explicit CMonthWeekView(QWidget *parent = nullptr);
    void setList(int weekday);
    void setTheMe(int type = 0);
    void updateWeek();
private:
    int checkDay(int weekday);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    //整体布局
    QHBoxLayout *m_mainLayout = nullptr;
    /**
     * @brief m_weekData 数据容器
     *
     * 将数据都放到容器里面
     *
     * QPair类是存储一对项的模板类。
     *
     * T1值为：frame，用以显示周数
     *
     * T2值为：int，用以区分周末(1)和工作日(0)
     *
     * 将周数和类型放到一起，可以根据类型对周数做不同操作
     *
     */
    QVector<QPair<CustomFrame *, int>> m_weekData;
    QVector<DHorizontalLine *>             m_vline;
};

#endif // WEEKINDICATOR_H
