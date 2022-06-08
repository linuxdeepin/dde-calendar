/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#include "viewschedulewidget.h"
#include "opencalendarwidget.h"

#include "dscheduledatamanager.h"
#include "../globaldef.h"

#include <QVBoxLayout>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>

viewschedulewidget::viewschedulewidget(QWidget *parent)
    : IconDFrame(parent)
{
}

viewschedulewidget::~viewschedulewidget()
{
}

void viewschedulewidget::setScheduleDateRangeInfo(const DSchedule::Map &scheduleDateInfo)
{
    m_scheduleDateInfo = scheduleDateInfo;
}

void viewschedulewidget::viewScheduleInfoShow(const DSchedule::Map &showSchedule)
{
    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->setSpacing(10);
    int scheduleNum = 0;
    DSchedule::List scheduleInfo;
    DSchedule::Map::const_iterator iter = showSchedule.constBegin();
    for (; iter != showSchedule.constEnd(); ++iter) {
        for (int i = 0; i < iter.value().size(); ++i) {
            scheduleInfo.append(iter.value().at(i));
            scheduleNum++;
            if (scheduleNum == 10)
                break;
        }
        scheduleitemwidget *item = new scheduleitemwidget();
        connect(item, &scheduleitemwidget::signalItemPress, this, &viewschedulewidget::slotItemPress);
        item->setScheduleDtailInfo(scheduleInfo);
        item->addscheduleitem();
        mainlayout->addWidget(item);
        scheduleInfo.clear();

        if (scheduleNum == 10)
            break;
    }

    //
    int scheduleCount = getScheduleNum(showSchedule);
    if (scheduleCount > 10) {
        OpenCalendarWidget *openWidget = new OpenCalendarWidget();
        openWidget->setScheduleCount(scheduleCount);
        mainlayout->addWidget(openWidget);
    }

    setCenterLayout(mainlayout);
}

int viewschedulewidget::getScheduleNum(DSchedule::Map scheduleList)
{
    int scheduleTotalNum = 0;
    DSchedule::Map::const_iterator iter = scheduleList.constBegin();
    for (; iter != scheduleList.constEnd(); ++iter) {
        scheduleTotalNum += iter.value().size();
    }
    return scheduleTotalNum;
}

void viewschedulewidget::setQueryBeginDateTime(QDateTime begindatetime)
{
    m_beginDateTime = begindatetime;
}

void viewschedulewidget::setQueryEndDateTime(QDateTime enddatetime)
{
    m_endDateTime = enddatetime;
}

void viewschedulewidget::slotItemPress(const DSchedule::Ptr &info)
{
    QProcess proc;
    proc.startDetached(PROCESS_OPEN_CALENDAR);
    QThread::msleep(750);
    QString schedulestr;
    DSchedule::toJsonString(info, schedulestr);
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_CALENDAR_SERVICE,
                                                          DBUS_CALENDAR_PATCH,
                                                          DBUS_CALENDAR_INTFACE,
                                                          "OpenSchedule");
    message << schedulestr;
    //发送消息
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
}


