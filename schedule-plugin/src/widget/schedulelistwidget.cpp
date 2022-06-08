/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#include "schedulelistwidget.h"
#include <QVBoxLayout>

#include "modifyscheduleitem.h"
#include "opencalendarwidget.h"
#include "../globaldef.h"

scheduleListWidget::scheduleListWidget(QWidget *parent)
    : IconDFrame(parent)
{
}

void scheduleListWidget::setScheduleInfoVector(const DSchedule::List &ScheduleInfoVector)
{
    m_ScheduleInfoVector = ScheduleInfoVector;
    updateUI();
}

void scheduleListWidget::updateUI()
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(1);
    bool showOpenWidget = m_ScheduleInfoVector.size() > ITEM_SHOW_NUM;
    const int showcount = showOpenWidget ? ITEM_SHOW_NUM : m_ScheduleInfoVector.size();
    for (int i = 0; i < showcount; ++i) {
        modifyScheduleItem *item = new modifyScheduleItem();
        if (i == 0) {
            item->setPositon(ItemWidget::ItemTop);
        } else if (i == showcount - 1) {
            item->setPositon(ItemWidget::ItemBottom);
        } else {
            item->setPositon(ItemWidget::ItemMiddle);
        }
        item->setIndex(i + 1);
        item->setScheduleInfo(m_ScheduleInfoVector[i]);
        connect(item, &modifyScheduleItem::signalSendSelectScheduleIndex,
                this, &scheduleListWidget::signalSelectScheduleIndex);
        layout->addWidget(item);
    }

    if (showOpenWidget) {
        OpenCalendarWidget *openWidget = new OpenCalendarWidget();
        openWidget->setScheduleCount(m_ScheduleInfoVector.size());
        layout->addSpacing(6);
        layout->addWidget(openWidget);
    }
    this->setCenterLayout(layout);
}
