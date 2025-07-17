// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulemanager.h"
#include "commondef.h"

ScheduleManager::ScheduleManager(QObject *parent) : QObject(parent)
{
    qCDebug(ClientLogger) << "ScheduleManager constructor";
    initconnect();
}

ScheduleManager *ScheduleManager::getInstace()
{
    qCDebug(ClientLogger) << "Getting ScheduleManager instance";
    static ScheduleManager manager;
    return &manager;
}

void ScheduleManager::initconnect()
{
    qCDebug(ClientLogger) << "Initializing ScheduleManager connections";
    connect(gAccountManager, &AccountManager::signalScheduleUpdate, this, &ScheduleManager::slotScheduleUpdate);
    connect(gAccountManager, &AccountManager::signalSearchScheduleUpdate, this, &ScheduleManager::slotSearchUpdate);
}

/**
 * @brief ScheduleManager::resetSchedule
 * 重新读取日程数据
 * @param year 年
 */
void ScheduleManager::resetSchedule(int year)
{
    qCDebug(ClientLogger) << "Resetting schedule for year:" << year;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        p->querySchedulesWithParameter(year);
    }
}

void ScheduleManager::resetSchedule(const QDateTime &start, const QDateTime &end)
{
    qCDebug(ClientLogger) << "Resetting schedule from" << start << "to" << end;
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
    qCDebug(ClientLogger) << "Updating schedule data";
    m_scheduleMap.clear();
    if (nullptr != gAccountManager->getLocalAccountItem()) {
        qCDebug(ClientLogger) << "Getting schedule map from local account";
        m_scheduleMap = gAccountManager->getLocalAccountItem()->getScheduleMap();
    }

    if (nullptr != gAccountManager->getUnionAccountItem()) {
        qCDebug(ClientLogger) << "Getting schedule map from union account";
        QMap<QDate, DSchedule::List> scheduleMap = gAccountManager->getUnionAccountItem()->getScheduleMap();
        if (m_scheduleMap.size() == 0) {
            m_scheduleMap = scheduleMap;
        } else {
            auto iterator = scheduleMap.begin();
            while (iterator != scheduleMap.end()) {
                DSchedule::List list = m_scheduleMap[iterator.key()];
                list.append(iterator.value());
                m_scheduleMap[iterator.key()] = list;
                iterator++;
            }
        }
    }
    qCDebug(ClientLogger) << "Schedule update complete with" << m_scheduleMap.size() << "dates";
    emit signalScheduleUpdate();
}

/**
 * @brief ScheduleManager::updateSearchSchedule
 * 更新被搜索的日程数据
 */
void ScheduleManager::updateSearchSchedule()
{
    qCDebug(ClientLogger) << "Updating search schedule data";
    m_searchScheduleMap.clear();
    if (nullptr != gLocalAccountItem) {
        qCDebug(ClientLogger) << "Getting search schedule map from local account";
        m_searchScheduleMap = gLocalAccountItem->getSearchScheduleMap();
    }
    if (nullptr != gUosAccountItem) {
        qCDebug(ClientLogger) << "Getting search schedule map from UOS account";
        QMap<QDate, DSchedule::List> scheduleMap = gUosAccountItem->getSearchScheduleMap();
        if (m_searchScheduleMap.size() == 0) {
            m_searchScheduleMap = scheduleMap;
        } else {
            auto iterator = scheduleMap.begin();
            while (iterator != scheduleMap.end()) {
                DSchedule::List list = m_searchScheduleMap[iterator.key()];
                list.append(iterator.value());
                m_searchScheduleMap[iterator.key()] = list;
                iterator++;
            }
        }
    }
    qCDebug(ClientLogger) << "Search schedule update complete with" << m_searchScheduleMap.size() << "dates";
    emit signalSearchScheduleUpdate();
}

/**
 * @brief ScheduleManager::slotScheduleUpdate
 * 日程数据更新事件
 */
void ScheduleManager::slotScheduleUpdate()
{
    qCDebug(ClientLogger) << "Schedule update slot triggered";
    updateSchedule();
}

void ScheduleManager::slotSearchUpdate()
{
    qCDebug(ClientLogger) << "Search update slot triggered";
    updateSearchSchedule();
}

/**
 * @brief ScheduleManager::getAllScheduleMap
 * 获取所有的日程数据
 * @return
 */
QMap<QDate, DSchedule::List> ScheduleManager::getAllScheduleMap()
{
    qCDebug(ClientLogger) << "Getting all schedule map with" << m_scheduleMap.size() << "dates";
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
    qCDebug(ClientLogger) << "Getting schedule map from" << startDate << "to" << stopDate;
    QMap<QDate, DSchedule::List> map;
    QDate date = startDate;
    while (date != stopDate) {
        if (m_scheduleMap.contains(date)) {
            map[date] = m_scheduleMap[date];
        }
        date = date.addDays(1);
    }

    if (m_scheduleMap.contains(date)) {
        map[date] = m_scheduleMap[date];
    }
    qCDebug(ClientLogger) << "Found schedules for" << map.size() << "dates in the requested range";
    return map;
}

/**
 * @brief ScheduleManager::getAllSearchedScheduleMap
 * 获取所有的被搜索的日程数据
 * @return
 */
QMap<QDate, DSchedule::List> ScheduleManager::getAllSearchedScheduleMap()
{
    qCDebug(ClientLogger) << "Getting all searched schedule map with" << m_searchScheduleMap.size() << "dates";
    return m_searchScheduleMap;
}

/**
 * @brief ScheduleManager::getAllSearchedScheduleList
 * 获取所有的被搜索的日程数据
 * @return
 */
DSchedule::List ScheduleManager::getAllSearchedScheduleList()
{
    qCDebug(ClientLogger) << "Getting all searched schedule list";
    DSchedule::List list;
    for (DSchedule::List l : m_searchScheduleMap.values()) {
        list.append(l);
    }
    qCDebug(ClientLogger) << "Found" << list.size() << "searched schedules";
    return list;
}

/**
 * @brief ScheduleManager::getAllScheduleDate
 * 获取所有的有日程的时间
 * @return
 */
QSet<QDate> ScheduleManager::getAllScheduleDate()
{
    qCDebug(ClientLogger) << "Getting all schedule dates";
    QSet<QDate> set;
    for (QDate date : m_scheduleMap.keys()) {
        set.insert(date);
    }
    qCDebug(ClientLogger) << "Found" << set.size() << "dates with schedules";
    return set;
}

/**
 * @brief ScheduleManager::getAllSearchedScheduleDate
 * 获取所有有被搜索日程的时间
 * @return
 */
QSet<QDate> ScheduleManager::getAllSearchedScheduleDate()
{
    qCDebug(ClientLogger) << "Getting all searched schedule dates";
    QSet<QDate> set;
    for (QDate date : m_searchScheduleMap.keys()) {
        set.insert(date);
    }
    qCDebug(ClientLogger) << "Found" << set.size() << "dates with searched schedules";
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
    qCDebug(ClientLogger) << "Getting schedules for date:" << date;
    if (m_scheduleMap.contains(date)) {
        qCDebug(ClientLogger) << "Found" << m_scheduleMap[date].size() << "schedules for" << date;
        return m_scheduleMap[date];
    }
    qCDebug(ClientLogger) << "No schedules found for" << date;
    return DSchedule::List();
}

/**
 * @brief ScheduleManager::getScheduleTypeByScheduleId
 * 根据日程类型id获取日程类型
 * @param id
 * @return
 */
DScheduleType::Ptr ScheduleManager::getScheduleTypeByScheduleId(const QString &id)
{
    qCDebug(ClientLogger) << "Getting schedule type by schedule ID:" << id;
    DScheduleType::Ptr type = nullptr;
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        type = p->getScheduleTypeByID(id);
        if (nullptr != type) {
            qCDebug(ClientLogger) << "Found schedule type for ID:" << id;
            break;
        }
    }
    if (type == nullptr) {
        qCDebug(ClientLogger) << "No schedule type found for ID:" << id;
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
    qCDebug(ClientLogger) << "Searching schedules with key:" << key << "from" << startTime << "to" << endTime;
    m_searchScheduleMap.clear();
    static int count = 0;
    count = 0;

    m_searchQuery.reset(new DScheduleQueryPar);
    m_searchQuery->setKey(key);
    m_searchQuery->setDtStart(startTime);
    m_searchQuery->setDtEnd(endTime);
    for (AccountItem::Ptr p : gAccountManager->getAccountList()) {
        count++;
        // qCDebug(ClientLogger) << "Querying account for schedules";
        p->querySchedulesWithParameter(m_searchQuery, [&](CallMessge) {
            count--;
            if (count == 0) {
                // qCDebug(ClientLogger) << "All account queries completed, updating search schedule";
                updateSearchSchedule();
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
    qCDebug(ClientLogger) << "Clearing search schedule";
    m_searchScheduleMap.clear();
    m_searchQuery.reset(nullptr);
    emit signalSearchScheduleUpdate();
}
