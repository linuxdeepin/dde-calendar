// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cweekwidget.h"
#include "calendarmanage.h"
#include "constants.h"
#include "commondef.h"
#include <QLocale>
#include <QPainter>
#include <QDate>

CWeekWidget::CWeekWidget(QWidget *parent) : QPushButton(parent)
  , m_firstDay(CalendarManager::getInstance()->getFirstDayOfWeek())
{
    // qCDebug(ClientLogger) << "CWeekWidget constructor";
    setMinimumHeight(10);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::NoFocus);
}

void CWeekWidget::setFirstDay(Qt::DayOfWeek first)
{
    // qCDebug(ClientLogger) << "CWeekWidget::setFirstDay first:" << first;
    m_firstDay = first;
    setAutoFirstDay(true);
}

void CWeekWidget::setAutoFirstDay(bool is)
{
    // qCDebug(ClientLogger) << "CWeekWidget::setAutoFirstDay is:" << is;
    m_autoFirstDay = is;
}

void CWeekWidget::setAutoFontSizeByWindow(bool is)
{
    // qCDebug(ClientLogger) << "CWeekWidget::setAutoFontSizeByWindow is:" << is;
    m_autoFontSizeByWindow = is;
}

void CWeekWidget::paintEvent(QPaintEvent *event)
{
    // qCDebug(ClientLogger) << "CWeekWidget::paintEvent";
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font;

    if (m_autoFontSizeByWindow) {
        // qCDebug(ClientLogger) << "CWeekWidget::paintEvent - Auto font size by window";
        //字体跟随界面大小
        qreal w = this->width() / 7;
        qreal h = this->height();
        qreal r = w > h ? h : w ;

        if (QLocale::system().language() == QLocale::English) {
            // qCDebug(ClientLogger) << "CWeekWidget::paintEvent - English locale, reducing font size";
            r*=0.8;
        }

        //根据高度和宽度设置时间字体的大小

        font.setPixelSize(int(r/20.0*12));

    } else {
        // qCDebug(ClientLogger) << "CWeekWidget::paintEvent - Fixed font size";
        font.setPixelSize(DDECalendar::FontSizeTwelve);
    }

    painter.setFont(font);

    QLocale locale;
    qreal setp = (width())/7.0;
    //获取一周首日
    int firstDay = m_firstDay;
    if (m_autoFirstDay) {
        // qCDebug(ClientLogger) << "CWeekWidget::paintEvent - Auto first day";
        firstDay = CalendarManager::getInstance()->getFirstDayOfWeek();
    }

    QStringList weekStr;
    for (auto i : { 7, 1, 2, 3, 4, 5, 6 }) {
        QString weekDayName = locale.dayName(i, QLocale::NarrowFormat);
        weekStr << weekDayName;
    }

    //绘制周一到周日
    for (int i = Qt::Monday; i <= Qt::Sunday; ++i) {
        int index = (firstDay + i - Qt::Monday) % Qt::Sunday;

        QString text = weekStr[index];
        QRectF rect((i-Qt::Monday)*setp, 0, setp, height());
        painter.drawText(rect, Qt::AlignCenter, text);
    }
}
