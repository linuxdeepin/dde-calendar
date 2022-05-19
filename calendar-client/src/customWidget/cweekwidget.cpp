/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#include "cweekwidget.h"
#include "configsettings.h"
#include "constants.h"
#include <QLocale>
#include <QPainter>

CWeekWidget::CWeekWidget(QWidget *parent) : QWidget(parent)
  , m_firstDay(gSetting->getFirstDayOfWeek())
{

}

void CWeekWidget::setFirstDay(Qt::DayOfWeek first)
{
    m_firstDay = first;
    setAutoFirstDay(false);
}

void CWeekWidget::setAutoFirstDay(bool is)
{
    m_autoFirstDay = is;
}

void CWeekWidget::setAutoFontSizeByWindow(bool is)
{
    m_autoFontSizeByWindow = is;
}

void CWeekWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_autoFontSizeByWindow) {
        //字体跟随界面大小
        qreal w = this->width() / 7;
        qreal h = this->height();
        const qreal r = w > h ? h * 0.7 : w * 0.7;
        //根据高度和宽度设置时间字体的大小
        QFont font;
        font.setPixelSize(qRound(DDECalendar::FontSizeTwelve + (r - 18) * 6 / 17.0));
        painter.setFont(font);

    }

    painter.setPen(Qt::SolidLine);
    QLocale locale;
    int setp = (width())/7;
    //获取一周首日
    int firstDay = m_firstDay;
    if (m_autoFirstDay) {
        firstDay = gSetting->getFirstDayOfWeek();
    }
    //绘制周一到周日
    for (int i = Qt::Monday; i <= Qt::Sunday; ++i) {
        int index = (firstDay + i - Qt::Monday) % Qt::Sunday;
        if (index == 0) {
            index = Qt::Sunday;
        }
        QString text = locale.dayName(index, QLocale::ShortFormat).right(1);
        QRect rect((i-Qt::Monday)*setp, 0, setp, height());
        painter.drawText(rect, Qt::AlignCenter, text);
    }
}
