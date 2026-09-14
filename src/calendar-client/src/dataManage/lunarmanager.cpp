// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarmanager.h"
#include "commondef.h"
#include <QFutureWatcher>
#include <QtConcurrent>

static constexpr int MAX_CACHED_RANGES = 10;

namespace {

using DateRange = QPair<QDate, QDate>;
using LunarQueryResult = QPair<bool, QMap<QDate, CaHuangLiDayInfo>>;
using FestivalQueryResult = QPair<bool, QVector<FestivalInfo>>;
using DayQueryResult = QPair<bool, CaHuangLiDayInfo>;

bool isRangeCovered(const QList<DateRange> &ranges, const DateRange &target)
{
    for (const DateRange &range : ranges) {
        if (target.first >= range.first && target.second <= range.second) {
            return true;
        }
    }
    return false;
}

bool isRangeCovered(const QSet<DateRange> &ranges, const DateRange &target)
{
    for (const DateRange &range : ranges) {
        if (target.first >= range.first && target.second <= range.second) {
            return true;
        }
    }
    return false;
}

void rememberRange(QList<DateRange> &ranges, const DateRange &range)
{
    if (ranges.contains(range)) {
        return;
    }
    if (ranges.size() >= MAX_CACHED_RANGES) {
        ranges.removeFirst();
    }
    ranges.append(range);
}

template<typename Value>
void pruneCache(QMap<QDate, Value> &cache, const QList<DateRange> &ranges)
{
    auto it = cache.begin();
    while (it != cache.end()) {
        const DateRange dateRange = qMakePair(it.key(), it.key());
        if (!isRangeCovered(ranges, dateRange)) {
            it = cache.erase(it);
        } else {
            ++it;
        }
    }
}

bool isValidHuangLiDayInfo(const CaHuangLiDayInfo &info)
{
    return !info.mGanZhiYear.isEmpty()
        && !info.mLunarMonthName.isEmpty()
        && !info.mLunarDayName.isEmpty();
}

} // namespace

LunarManager::LunarManager(QObject *parent) : QObject(parent)
  , m_dbusRequest(new DbusHuangLiRequest)
{
    qCDebug(ClientLogger) << "Creating LunarManager";
}

LunarManager* LunarManager::getInstace()
{
    // qCDebug(ClientLogger) << "Getting LunarManager instance";
    static LunarManager lunarManager;
    return &lunarManager;
}

/**
 * @brief LunarManager::getFestivalMonth
 * 按月获取节假日信息
 * @param year 年
 * @param month 月
 * @param festivalInfo 数据保存位置
 * @return
 */
bool LunarManager::getFestivalMonth(quint32 year, quint32 month, FestivalInfo& festivalInfo)
{
    // qCDebug(ClientLogger) << "Getting festival month for year:" << year << "month:" << month;
    bool result = m_dbusRequest->getFestivalMonth(year, month, festivalInfo);
    // qCDebug(ClientLogger) << "Get festival month result:" << result << "with" << festivalInfo.listHoliday.size() << "holidays";
    return result;
}

/**
 * @brief LunarManager::getFestivalMonth
 * 按月获取节假日信息
 * @param date 月所在日期
 * @param festivalInfo 数据储存位置
 * @return  请求成功状态
 */
bool LunarManager::getFestivalMonth(const QDate &date, FestivalInfo& festivalInfo)
{
    // qCDebug(ClientLogger) << "Getting festival month for date:" << date.toString();
    return m_dbusRequest->getFestivalMonth(quint32(date.year()), quint32(date.month()), festivalInfo);
}

/**
 * @brief LunarManager::getHuangLiDay
 * 按天获取黄历信息
 * @param year 年
 * @param month 月
 * @param day 日
 * @param info 数据储存位置
 * @return  请求成功状态
 */
bool LunarManager::getHuangLiDay(quint32 year, quint32 month, quint32 day, CaHuangLiDayInfo &info)
{
    // qCDebug(ClientLogger) << "Getting HuangLi day for year:" << year << "month:" << month << "day:" << day;
    bool result = m_dbusRequest->getHuangLiDay(year, month, day, info);
    // qCDebug(ClientLogger) << "Get HuangLi day result:" << result;
    return result;
}

/**
 * @brief LunarManager::getHuangLiDay
 * 按天获取农历信息
 * @param date 请求日期
 * @param info 数据储存位置
 * @return 请求成功状态
 */
bool LunarManager::getHuangLiDay(const QDate &date, CaHuangLiDayInfo &info)
{
    // qCDebug(ClientLogger) << "Getting HuangLi day for date:" << date.toString();
    return getHuangLiDay(quint32(date.year()), quint32(date.month()), quint32(date.day()), info);
}

/**
 * @brief LunarManager::getHuangLiMonth
 * 按月获取农历信息
 * @param year 年
 * @param month 月
 * @param info 日
 * @param fill
 * @return  请求成功状态
 */
bool LunarManager::getHuangLiMonth(quint32 year, quint32 month, CaHuangLiMonthInfo &info, bool fill)
{
    // qCDebug(ClientLogger) << "Getting HuangLi month for year:" << year << "month:" << month << "fill:" << fill;
    bool result = m_dbusRequest->getHuangLiMonth(year, month, fill, info);
    // qCDebug(ClientLogger) << "Get HuangLi month result:" << result << "with" << info.mDays << "days";
    return result;
}

/**
 * @brief LunarManager::getHuangLiMonth
 * 按月获取农历信息
 * @param date 请求日期
 * @param info 数据储存位置
 * @return 请求成功状态
 */
bool LunarManager::getHuangLiMonth(const QDate &date, CaHuangLiMonthInfo &info, bool fill)
{
    // qCDebug(ClientLogger) << "Getting HuangLi month for date:" << date.toString() << "fill:" << fill;
    return getHuangLiMonth(quint32(date.year()), quint32(date.month()), info, fill);
}

/**
 * @brief LunarManager::getHuangLiShortName
 * 获取当天的农历月日期和日日期名
 * @param date 请求日期
 * @return 农历名
 */
QString LunarManager::getHuangLiShortName(const QDate &date)
{
    qCDebug(ClientLogger) << "Getting HuangLi short name for date:" << date.toString();
    CaHuangLiDayInfo info = getHuangLiDay(date);
    QString shortName = info.mLunarMonthName + info.mLunarDayName;
    qCDebug(ClientLogger) << "HuangLi short name:" << shortName;
    return shortName;
}

/**
 * @brief LunarManager::queryLunarInfo
 * 查询农历信息
 * @param startDate 开始时间
 * @param stopDate 结束时间
 */
void LunarManager::queryLunarInfo(const QDate &startDate, const QDate &stopDate)
{
    qCDebug(ClientLogger) << "Querying lunar info from" << startDate.toString() << "to" << stopDate.toString();
    const int offsetMonth = (stopDate.year() - startDate.year()) * 12 + stopDate.month() - startDate.month();

    QFutureWatcher<LunarQueryResult> *w = new QFutureWatcher<LunarQueryResult>(this);
    QFuture<LunarQueryResult> future = QtConcurrent::run([offsetMonth, startDate]() -> LunarQueryResult {
        auto dbus = new DbusHuangLiRequest();
        QMap<QDate, CaHuangLiDayInfo> lunarInfoMap;
        CaHuangLiMonthInfo monthInfo;
        bool success = true;
        //获取开始时间至结束时间所在月的农历和节假日信息
        for (int i = 0; i <= offsetMonth; ++i) {
            monthInfo.clear();
            QDate beginDate = startDate.addMonths(i);
            if (!dbus->getHuangLiMonth(beginDate.year(), beginDate.month(), false, monthInfo)
                || monthInfo.mDays <= 0
                || monthInfo.mDays > monthInfo.mCaLunarDayInfo.size()) {
                success = false;
                continue;
            }

            QDate getDate(beginDate.year(), beginDate.month(), 1);
            for (int j = 0; j < monthInfo.mDays; ++j) {
                const CaHuangLiDayInfo &dayInfo = monthInfo.mCaLunarDayInfo.at(j);
                if (!isValidHuangLiDayInfo(dayInfo)) {
                    success = false;
                    continue;
                }
                lunarInfoMap[getDate.addDays(j)] = dayInfo;
            }
        }
        delete dbus;
        return qMakePair(success, lunarInfoMap);
    });
    connect(w, &QFutureWatcher<LunarQueryResult>::finished, this, [this, w, startDate, stopDate]() {
        const LunarQueryResult queryResult = w->result();
        const QMap<QDate, CaHuangLiDayInfo> &result = queryResult.second;
        bool success = queryResult.first;
        if (success) {
            const int expectedDays = startDate.daysTo(stopDate) + 1;
            for (int i = 0; i < expectedDays; ++i) {
                if (!result.contains(startDate.addDays(i))) {
                    success = false;
                    break;
                }
            }
        }

        m_pendingLunarQueries.remove(qMakePair(startDate, stopDate));
        // Keep every valid day even when the requested range is incomplete.
        // Do not remember an incomplete range, so the missing days can retry.
        for (auto it = result.constBegin(); it != result.constEnd(); ++it) {
            m_lunarInfoMap[it.key()] = it.value();
        }
        if (success) {
            rememberRange(m_queriedRanges, qMakePair(startDate, stopDate));
            pruneCache(m_lunarInfoMap, m_queriedRanges);
            qCDebug(ClientLogger) << "Lunar info query completed, total cached:" << m_lunarInfoMap.size() << "days";
            emit lunarInfoReady(startDate, stopDate);
        } else {
            qCWarning(ClientLogger) << "Lunar info query returned incomplete data for"
                                    << startDate.toString() << "to" << stopDate.toString()
                                    << ", kept" << result.size() << "valid days for the cache";
        }
        w->deleteLater();
    });
    w->setFuture(future);
}

/**
 * @brief LunarManager::queryFestivalInfo
 * 查询节假日信息（异步）
 * @param startDate 开始时间
 * @param stopDate 结束时间
 */
void LunarManager::queryFestivalInfo(const QDate &startDate, const QDate &stopDate)
{
    qCDebug(ClientLogger) << "Querying festival info from" << startDate.toString() << "to" << stopDate.toString();
    const int offsetMonth = (stopDate.year() - startDate.year()) * 12 + stopDate.month() - startDate.month();

    QFutureWatcher<FestivalQueryResult> *w = new QFutureWatcher<FestivalQueryResult>(this);
    QFuture<FestivalQueryResult> future = QtConcurrent::run([offsetMonth, startDate]() -> FestivalQueryResult {
        auto dbus = new DbusHuangLiRequest();
        QVector<FestivalInfo> festivallist{};
        bool success = true;
        for (int i = 0; i <= offsetMonth; ++i) {
            FestivalInfo info;
            QDate beginDate = startDate.addMonths(i);
            if (!dbus->getFestivalMonth(quint32(beginDate.year()), quint32(beginDate.month()), info)) {
                success = false;
                break;
            }
            festivallist.push_back(info);
        }
        delete dbus;
        return qMakePair(success, festivallist);
    });
    connect(w, &QFutureWatcher<FestivalQueryResult>::finished, this, [this, w, startDate, stopDate]() {
        const FestivalQueryResult queryResult = w->result();
        m_pendingFestivalQueries.remove(qMakePair(startDate, stopDate));
        if (queryResult.first) {
            auto oldIt = m_festivalDateMap.lowerBound(startDate);
            while (oldIt != m_festivalDateMap.end() && oldIt.key() <= stopDate) {
                oldIt = m_festivalDateMap.erase(oldIt);
            }
            for (const FestivalInfo &info : queryResult.second) {
                for (const HolidayInfo &h : info.listHoliday) {
                    if (h.date.isValid()) {
                        m_festivalDateMap[h.date] = h.status;
                    }
                }
            }
            rememberRange(m_queriedFestivalRanges, qMakePair(startDate, stopDate));
            pruneCache(m_festivalDateMap, m_queriedFestivalRanges);
            qCDebug(ClientLogger) << "Festival date map updated with" << m_festivalDateMap.size() << "days";
            emit festivalInfoReady();
        } else {
            qCWarning(ClientLogger) << "Festival info query failed for"
                                    << startDate.toString() << "to" << stopDate.toString();
        }
        w->deleteLater();
    });
    w->setFuture(future);
}

/**
 * @brief LunarManager::getHuangLiDay
 * 获取农历信息
 * @param date 获取日期
 * @return
 */
CaHuangLiDayInfo LunarManager::getHuangLiDay(const QDate &date)
{
    qCDebug(ClientLogger) << "Getting HuangLi day info for date:" << date.toString();
    //首先在缓存中查找是否存在该日期的农历信息，没有则通过dbus获取
    CaHuangLiDayInfo info;
    if (hasHuangLiDay(date)) {
        qCDebug(ClientLogger) << "Found HuangLi day info in cache";
        info = m_lunarInfoMap.value(date);
    } else {
        qCDebug(ClientLogger) << "HuangLi day info not in cache, fetching via dbus";
        getHuangLiDay(date, info);
    }
    return info;
}

bool LunarManager::hasHuangLiDay(const QDate &date) const
{
    const auto it = m_lunarInfoMap.constFind(date);
    return it != m_lunarInfoMap.constEnd() && isValidHuangLiDayInfo(it.value());
}

bool LunarManager::hasHuangLiRange(const QDate &startDate, const QDate &endDate) const
{
    if (!startDate.isValid() || !endDate.isValid() || startDate > endDate) {
        return false;
    }

    if (isRangeCovered(m_queriedRanges, qMakePair(startDate, endDate))) {
        return true;
    }

    const int expectedDays = startDate.daysTo(endDate) + 1;
    for (int i = 0; i < expectedDays; ++i) {
        if (!hasHuangLiDay(startDate.addDays(i))) {
            return false;
        }
    }
    return true;
}

/**
 * @brief LunarManager::getHuangLiDayAsync
 * 异步获取农历信息，避免阻塞 UI
 * @param date 获取日期
 */
void LunarManager::getHuangLiDayAsync(const QDate &date)
{
    qCDebug(ClientLogger) << "Getting HuangLi day info async for date:" << date.toString();
    if (hasHuangLiDay(date)) {
        qCDebug(ClientLogger) << "Found HuangLi day info in cache, emitting directly";
        emit huangLiDayReady(date, m_lunarInfoMap.value(date));
        return;
    }

    const DateRange target = qMakePair(date, date);
    if (isRangeCovered(m_pendingLunarQueries, target)) {
        // A range request will publish the same date through lunarInfoReady.
        return;
    }
    if (m_pendingDayQueries.contains(date)) {
        return;
    }
    m_pendingDayQueries.insert(date);

    //异步获取农历数据
    QFutureWatcher<DayQueryResult> *w = new QFutureWatcher<DayQueryResult>(this);
    QFuture<DayQueryResult> future = QtConcurrent::run([date]() -> DayQueryResult {
        auto dbus = new DbusHuangLiRequest();
        CaHuangLiDayInfo info;
        const bool success = dbus->getHuangLiDay(date.year(), date.month(), date.day(), info)
            && isValidHuangLiDayInfo(info);
        delete dbus;
        return qMakePair(success, info);
    });
    connect(w, &QFutureWatcher<DayQueryResult>::finished, this, [this, w, date]() {
        const DayQueryResult queryResult = w->result();
        m_pendingDayQueries.remove(date);
        if (queryResult.first) {
            // Do not let a slower single-day request overwrite a valid range result.
            if (!hasHuangLiDay(date)) {
                m_lunarInfoMap[date] = queryResult.second;
            }
            qCDebug(ClientLogger) << "Async HuangLi day info ready for date:" << date.toString();
            emit huangLiDayReady(date, m_lunarInfoMap.value(date));
        } else {
            qCWarning(ClientLogger) << "Async HuangLi day query failed for date:" << date.toString();
        }
        w->deleteLater();
    });
    w->setFuture(future);
}

/**
 * @brief LunarManager::getHuangLiDayMap
 * 获取一定时间范围内的农历数据
 * @param startDate 开始时间
 * @param stopDate 结束时间
 * @return 农历信息
 */
QMap<QDate, CaHuangLiDayInfo> LunarManager::getHuangLiDayMap(const QDate &startDate, const QDate &stopDate)
{
    qCDebug(ClientLogger) << "Getting HuangLi day map from" << startDate.toString() << "to" << stopDate.toString();
    QMap<QDate, CaHuangLiDayInfo> lunarInfoMap;
    auto iterator = m_lunarInfoMap.begin();
    while(iterator != m_lunarInfoMap.end()) {
        if (iterator.key() >= startDate && iterator.key() <= stopDate) {
            iterator.value();
            lunarInfoMap[iterator.key()] = iterator.value();
        }
        iterator++;
    }
    qCDebug(ClientLogger) << "HuangLi day map contains" << lunarInfoMap.size() << "days";
    return lunarInfoMap;
}

/**
 * @brief LunarManager::getFestivalInfoDateMap
 * 获取节假日日期信息
 * @param startDate 开始时间
 * @param stopDate 结束时间
 * @return 节假日日期信息
 */
QMap<QDate, int> LunarManager::getFestivalInfoDateMap(const QDate &startDate, const QDate &stopDate)
{
    qCDebug(ClientLogger) << "Getting festival info date map from" << startDate.toString() << "to" << stopDate.toString();
    QMap<QDate, int> festivalDateMap;
    auto iterator = m_festivalDateMap.begin();
    while(iterator != m_festivalDateMap.end()) {
        if (iterator.key() >= startDate && iterator.key() <= stopDate) {
            iterator.value();
            festivalDateMap[iterator.key()] = iterator.value();
        }
        iterator++;
    }
    qCDebug(ClientLogger) << "Festival date map contains" << festivalDateMap.size() << "days";
    return festivalDateMap;
}

/**
 * @brief LunarManager::ensureLunarDataLoaded
 * 确保农历数据已加载（带缓存机制，避免重复查询）
 * @param startDate 开始日期
 * @param endDate 结束日期
 */
void LunarManager::ensureLunarDataLoaded(const QDate &startDate, const QDate &endDate)
{
    // Input validation
    if (!startDate.isValid() || !endDate.isValid() || startDate > endDate) {
        qCWarning(ClientLogger) << "Invalid date range:" << startDate.toString() << "to" << endDate.toString();
        return;
    }

    // Prevent re-entrant queries for the same range while DBus call is in flight
    auto key = qMakePair(startDate, endDate);
    // Check the lunar range and the festival range independently. They are
    // two asynchronous requests and must not clear one shared pending flag.
    const bool lunarCached = isRangeCovered(m_queriedRanges, key);
    const bool festivalCached = isRangeCovered(m_queriedFestivalRanges, key);

    if (!lunarCached) {
        if (hasHuangLiRange(startDate, endDate)) {
            rememberRange(m_queriedRanges, key);
            pruneCache(m_lunarInfoMap, m_queriedRanges);
        } else if (!isRangeCovered(m_pendingLunarQueries, key)) {
            qCDebug(ClientLogger) << "Querying incomplete lunar cache for range"
                                   << startDate.toString() << "to" << endDate.toString();
            m_pendingLunarQueries.insert(key);
            queryLunarInfo(startDate, endDate);
        }
    }

    if (!festivalCached && !isRangeCovered(m_pendingFestivalQueries, key)) {
        m_pendingFestivalQueries.insert(key);
        queryFestivalInfo(startDate, endDate);
    }
}
