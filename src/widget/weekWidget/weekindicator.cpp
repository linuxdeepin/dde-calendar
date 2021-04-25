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

#include "weekindicator.h"

#include <QDebug>
#include <QDate>

WeekIndicator::WeekIndicator(QWidget *parent)
    : DWidget(parent)
{
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    setLayout(m_mainLayout);
}

void WeekIndicator::setList(int weekday)
{
    QLayoutItem *child;

    while ((child = m_mainLayout->takeAt(0)) != nullptr) {
        if (child->widget() != nullptr) {
            delete child->widget();
        }
        delete child;
    }

    QLocale locale;

    for (int i = 0; i != DDEWeekCalendar::AFewDaysofWeek; ++i) {
        int d = checkDay(i - weekday);
        DLabel *label = new DLabel(locale.dayName(d ? d : DDEWeekCalendar::AFewDaysofWeek, QLocale::ShortFormat));

        if ((i == weekday - 1 && weekday != 0) || i == weekday || (weekday == 0 && i == 6)) {
            label->setObjectName("CalendarHeaderWeekend");
        } else {
            label->setObjectName("CalendarHeaderWeekday");
        }

        label->setAlignment(Qt::AlignCenter);
        label->setFixedSize(DDECalendar::CellWidth, DDECalendar::HeaderItemHeight);
        m_mainLayout->addWidget(label, 0, Qt::AlignCenter);
    }
}

int WeekIndicator::checkDay(int weekday)
{
    // check the week, calculate the correct order in the custom.
    if (weekday <= 0)
        return weekday += DDEWeekCalendar::AFewDaysofWeek;

    if (weekday > DDEWeekCalendar::AFewDaysofWeek)
        return weekday -= DDEWeekCalendar::AFewDaysofWeek;

    return weekday;
}
