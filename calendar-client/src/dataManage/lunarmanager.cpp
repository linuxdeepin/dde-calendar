// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarmanager.h"
#include "commondef.h"
#include <QFutureWatcher>
#include <QtConcurrent>

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
    
    QFutureWatcher<QMap<QDate, CaHuangLiDayInfo>> *w = new QFutureWatcher<QMap<QDate, CaHuangLiDayInfo>>(this);
    QFuture<QMap<QDate, CaHuangLiDayInfo>> future = QtConcurrent::run([offsetMonth, startDate]() -> QMap<QDate, CaHuangLiDayInfo> {
        auto dbus = new DbusHuangLiRequest();
        QMap<QDate, CaHuangLiDayInfo> lunarInfoMap;
        CaHuangLiMonthInfo monthInfo;
        //获取开始时间至结束时间所在月的农历和节假日信息
        for (int i = 0; i <= offsetMonth; ++i) {
            monthInfo.clear();
            QDate beginDate = startDate.addMonths(i);
            dbus->getHuangLiMonth(beginDate.year(), beginDate.month(), false, monthInfo);

            QDate getDate(beginDate.year(), beginDate.month(), 1);
            for (int j = 0; j < monthInfo.mDays; ++j) {
                lunarInfoMap[getDate.addDays(j)] = monthInfo.mCaLunarDayInfo.at(j);
            }
        }
        delete dbus;
        return lunarInfoMap;
    });
    connect(w, &QFutureWatcher<QMap<QDate, CaHuangLiDayInfo>>::finished, this, [this, w]() {
        m_lunarInfoMap = w->result();
        qCDebug(ClientLogger) << "Lunar info query completed with" << m_lunarInfoMap.size() << "days";
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
    
    QFutureWatcher<QVector<FestivalInfo>> *w = new QFutureWatcher<QVector<FestivalInfo>>(this);
    QFuture<QVector<FestivalInfo>> future = QtConcurrent::run([offsetMonth, startDate]() -> QVector<FestivalInfo> {
        auto dbus = new DbusHuangLiRequest();
        QVector<FestivalInfo> festivallist{};
        for (int i = 0; i <= offsetMonth; ++i) {
            FestivalInfo info;
            QDate beginDate = startDate.addMonths(i);
            if (dbus->getFestivalMonth(quint32(beginDate.year()), quint32(beginDate.month()), info)) {
                festivallist.push_back(info);
            }
        }
        delete dbus;
        return festivallist;
    });
    connect(w, &QFutureWatcher<QVector<FestivalInfo>>::finished, this, [this, w]() {
        auto festivallist = w->result();
        m_festivalDateMap.clear();
        for (const FestivalInfo &info : festivallist) {
            for (const HolidayInfo &h : info.listHoliday) {
                m_festivalDateMap[h.date] = h.status;
            }
        }
        qCDebug(ClientLogger) << "Festival date map updated with" << m_festivalDateMap.size() << "days";
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
    if (m_lunarInfoMap.contains(date)) {
        qCDebug(ClientLogger) << "Found HuangLi day info in cache";
        info = m_lunarInfoMap[date];
    } else {
        qCDebug(ClientLogger) << "HuangLi day info not in cache, fetching via dbus";
        getHuangLiDay(date, info);
    }
    return info;
}

/**
 * @brief LunarManager::getHuangLiDayAsync
 * 异步获取农历信息，避免阻塞 UI
 * @param date 获取日期
 */
void LunarManager::getHuangLiDayAsync(const QDate &date)
{
    qCDebug(ClientLogger) << "Getting HuangLi day info async for date:" << date.toString();
    if (m_lunarInfoMap.contains(date)) {
        qCDebug(ClientLogger) << "Found HuangLi day info in cache, emitting directly";
        emit huangLiDayReady(date, m_lunarInfoMap[date]);
        return;
    }
    
    //异步获取农历数据
    QFutureWatcher<CaHuangLiDayInfo> *w = new QFutureWatcher<CaHuangLiDayInfo>(this);
    QFuture<CaHuangLiDayInfo> future = QtConcurrent::run([date]() -> CaHuangLiDayInfo {
        auto dbus = new DbusHuangLiRequest();
        CaHuangLiDayInfo info;
        dbus->getHuangLiDay(date.year(), date.month(), date.day(), info);
        delete dbus;
        return info;
    });
    connect(w, &QFutureWatcher<CaHuangLiDayInfo>::finished, this, [this, w, date]() {
        CaHuangLiDayInfo info = w->result();
        //缓存结果
        m_lunarInfoMap[date] = info;
        qCDebug(ClientLogger) << "Async HuangLi day info ready for date:" << date.toString();
        emit huangLiDayReady(date, info);
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
        if (iterator.key() >= startDate || iterator.key() <= stopDate) {
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
        if (iterator.key() >= startDate || iterator.key() <= stopDate) {
            iterator.value();
            festivalDateMap[iterator.key()] = iterator.value();
        }
        iterator++;
    }
    qCDebug(ClientLogger) << "Festival date map contains" << festivalDateMap.size() << "days";
    return festivalDateMap;
}
