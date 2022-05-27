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
#include "schedulemanager.h"

ScheduleManager::ScheduleManager(QObject *parent) : QObject(parent)
{
    initconnect();
}

ScheduleManager* ScheduleManager::getInstace()
{
    static ScheduleManager manager;
    return &manager;
}

void ScheduleManager::initconnect()
{
    connect(gAccounManager, &AccountManager::signalScheduleUpdate, this, &ScheduleManager::slotScheduleUpdate);
}

void ScheduleManager::resetSchedule(int year)
{
    for (AccountItem::Ptr p : gAccounManager->getAccountList()) {
        p->querySchedulesWithParameter(year);
    }
}

void ScheduleManager::updateDate()
{
    m_scheduleMap.clear();
    if (nullptr != gAccounManager->getLocalAccountItem()) {
        m_scheduleMap = gAccounManager->getLocalAccountItem()->getScheduleMap();
    }

    if (nullptr != gAccounManager->getUnionAccountItem()) {
        QMap<QDate, DSchedule::List> scheduleMap = gAccounManager->getUnionAccountItem()->getScheduleMap();
        if (m_scheduleMap.size() == 0) {
            m_scheduleMap = scheduleMap;
        } else {
            auto iterator = scheduleMap.begin();
            while(iterator != scheduleMap.end()) {
                DSchedule::List list = m_scheduleMap[iterator.key()];
                for (DSchedule :: Ptr p : iterator.value()) {
                    list.append(p);
                }
                m_scheduleMap[iterator.key()] = list;
                iterator++;
            }
        }
    }
    emit signalScheduleUpdate();
}

void ScheduleManager::slotScheduleUpdate()
{
    updateDate();
}

QMap<QDate, DSchedule::List> ScheduleManager::getAllSchedule()
{
    return m_scheduleMap;
}

QMap<QDate, bool> ScheduleManager::getAllScheduleDate()
{
    QMap<QDate, bool> map;
    for (QDate date : m_scheduleMap.keys()) {
        map[date] = true;
    }
    return map;
}
