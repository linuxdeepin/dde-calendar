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
#include "calendarmanage.h"
#include "constants.h"
#include <QLocale>
#include <QPainter>

CWeekWidget::CWeekWidget(QWidget *parent) : QPushButton(parent)
  , m_firstDay(CalendarManager::getInstance()->getFirstDayOfWeek())
{
    setMinimumHeight(10);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::NoFocus);
}

void CWeekWidget::setFirstDay(Qt::DayOfWeek first)
{
    m_firstDay = first;
    setAutoFirstDay(true);
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

    QFont font;

    if (m_autoFontSizeByWindow) {
        //字体跟随界面大小
        qreal w = this->width() / 7;
        qreal h = this->height();
        qreal r = w > h ? h : w ;

        if (QLocale::system().language() == QLocale::English) {
            r*=0.8;
        }

        //根据高度和宽度设置时间字体的大小

        font.setPixelSize(int(r/20.0*12));

    } else {
        font.setPixelSize(DDECalendar::FontSizeTwelve);
    }

    painter.setFont(font);

    QLocale locale;
    qreal setp = (width())/7.0;
    //获取一周首日
    int firstDay = m_firstDay;
    if (m_autoFirstDay) {
        firstDay = CalendarManager::getInstance()->getFirstDayOfWeek();
    }

    QStringList weekStr;
    weekStr << tr("Sun") << tr("Mon") << tr("Tue") << tr("Wed") << tr("Thu") << tr("Fri") << tr("Sat");

    //绘制周一到周日
    for (int i = Qt::Monday; i <= Qt::Sunday; ++i) {
        int index = (firstDay + i - Qt::Monday) % Qt::Sunday;

        QString text = weekStr[index];
        QRectF rect((i-Qt::Monday)*setp, 0, setp, height());
        painter.drawText(rect, Qt::AlignCenter, text);
    }
}
