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
    connect(gAccountManager, &AccountManager::signalScheduleUpdate, this, &ScheduleManager::slotScheduleUpdate);
}

/**
 * @brief ScheduleManager::resetSchedule
 * 重新读取日程数据
 * @param year 年
 */
void ScheduleManager::resetSchedule(int year)
{
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        p->querySchedulesWithParameter(year);
    }
}

void ScheduleManager::resetSchedule(const QDateTime& start, const QDateTime& end)
{
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        p->querySchedulesWithParameter(start, end);
    }
}

/**
 * @brief ScheduleManager::updateSchedule
 * 更新日程数据
 */
void ScheduleManager::updateSchedule()
{
    m_scheduleMap.clear();
    if (nullptr != gAccountManager->getLocalAccountItem()) {
        m_scheduleMap = gAccountManager->getLocalAccountItem()->getScheduleMap();
    }

    if (nullptr != gAccountManager->getUnionAccountItem()) {
        QMap<QDate, DSchedule::List> scheduleMap = gAccountManager->getUnionAccountItem()->getScheduleMap();
        if (m_scheduleMap.size() == 0) {
            m_scheduleMap = scheduleMap;
        } else {
            auto iterator = scheduleMap.begin();
            while(iterator != scheduleMap.end()) {
                DSchedule::List list = m_scheduleMap[iterator.key()];
                list.append(iterator.value());
                m_scheduleMap[iterator.key()] = list;
                iterator++;
            }
        }
    }
    emit signalScheduleUpdate();
}

/**
 * @brief ScheduleManager::updateSearchSchedule
 * 更新被搜索的日程数据
 */
void ScheduleManager::updateSearchSchedule()
{
    m_searchScheduleMap.clear();
    if (nullptr != gAccountManager->getLocalAccountItem()) {
        m_searchScheduleMap = gAccountManager->getLocalAccountItem()->getScheduleMap();
    }

    if (nullptr != gAccountManager->getUnionAccountItem()) {
        QMap<QDate, DSchedule::List> scheduleMap = gAccountManager->getUnionAccountItem()->getScheduleMap();
        if (m_searchScheduleMap.size() == 0) {
            m_searchScheduleMap = scheduleMap;
        } else {
            auto iterator = scheduleMap.begin();
            while(iterator != scheduleMap.end()) {
                DSchedule::List list = m_searchScheduleMap[iterator.key()];
                list.append(iterator.value());
                m_searchScheduleMap[iterator.key()] = list;
                iterator++;
            }
        }
    }
    emit signalSearchScheduleUpdate();
}

/**
 * @brief ScheduleManager::slotScheduleUpdate
 * 日程数据更新事件
 */
void ScheduleManager::slotScheduleUpdate()
{
    updateSchedule();
}

/**
 * @brief ScheduleManager::getAllScheduleMap
 * 获取所有的日程数据
 * @return
 */
QMap<QDate, DSchedule::List> ScheduleManager::getAllScheduleMap()
{
    return m_scheduleMap;
}

/**
 * @brief ScheduleManager::getScheduleMap
 * 获取一定时间范围内的日程
 * @param startDate 开始时间
 * @param stopDate 结束时间
 * @return
 */
QMap<QDate, DSchedule::List> ScheduleManager::getScheduleMap(const QDate &startDate, const QDate &stopDate) const
{
    QMap<QDate, DSchedule::List> map;
    QDate date = startDate;
    while(date != stopDate) {
        if (m_scheduleMap.contains(date)) {
            map[date] = m_scheduleMap[date];
        }
        date = date.addDays(1);
    }

    if (m_scheduleMap.contains(date)) {
        map[date] = m_scheduleMap[date];
    }
    return map;
}

/**
 * @brief ScheduleManager::getAllSearchedScheduleMap
 * 获取所有的被搜索的日程数据
 * @return
 */
QMap<QDate, DSchedule::List> ScheduleManager::getAllSearchedScheduleMap()
{
    return m_searchScheduleMap;
}

/**
 * @brief ScheduleManager::getAllSearchedScheduleList
 * 获取所有的被搜索的日程数据
 * @return
 */
DSchedule::List ScheduleManager::getAllSearchedScheduleList()
{
    DSchedule::List list;
    for (DSchedule::List l : m_scheduleMap.values()) {
        list.append(l);
    }
    return list;
}

/**
 * @brief ScheduleManager::getAllScheduleDate
 * 获取所有的有日程的时间
 * @return
 */
QSet<QDate> ScheduleManager::getAllScheduleDate()
{
    QSet<QDate> set;
    for (QDate date : m_scheduleMap.keys()) {
        set.insert(date);
    }
    return set;
}

/**
 * @brief ScheduleManager::getAllSearchedScheduleDate
 * 获取所有有被搜索日程的时间
 * @return
 */
QSet<QDate> ScheduleManager::getAllSearchedScheduleDate()
{
    QSet<QDate> set;
    for (QDate date : m_searchScheduleMap.keys()) {
        set.insert(date);
    }
    return set;
}

/**
 * @brief ScheduleManager::getScheduleByDay
 * 获取某天的日程
 * @param date 需要获取日程的日期
 * @return
 */
DSchedule::List ScheduleManager::getScheduleByDay(QDate date)
{
    if (m_scheduleMap.contains(date)) {
        return m_scheduleMap[date];
    }
    return DSchedule::List();
}

/**
 * @brief ScheduleManager::getScheduleTypeByScheduleId
 * 根据日程类型id获取日程类型
 * @param id
 * @return
 */
DScheduleType::Ptr ScheduleManager::getScheduleTypeByScheduleId(const QString& id)
{
    DScheduleType::Ptr type = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        type = p->getScheduleTypeByID(id);
        if (nullptr != type) {
            break;
        }
    }
    return type;
}

/**
 * @brief ScheduleManager::searchSchedule
 * 搜索日程
 * @param key   搜索关键字
 * @param startTime 开始时间
 * @param endTime   结束时间
 */
void ScheduleManager::searchSchedule(const QString &key, const QDateTime &startTime, const QDateTime &endTime)
{
    static int count = 0;
    count = 0;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        count ++;
        p->querySchedulesWithParameter(key, startTime, endTime, [&](CallMessge) {
            count--;
            if (count == 0) {
                this->updateSearchSchedule();
            }
        });
    }
}

/**
 * @brief ScheduleManager::clearSearchSchedule
 * 情况已搜索的如此数据
 */
void ScheduleManager::clearSearchSchedule()
{
    m_searchScheduleMap.clear();
    emit signalSearchScheduleUpdate();
}
