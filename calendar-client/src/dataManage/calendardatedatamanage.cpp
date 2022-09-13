// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendardatedatamanage.h"

#include <QtMath>

const int MonthMaxDay = 42;
Qt::DayOfWeek     CalendarDateDataManager::m_weekFirstDay = Qt::Sunday;
/**
 * @brief CalendarDateDataManage    构造函数
 * @param parent
 */
CalendarDateDataManager::CalendarDateDataManager()
    : m_currentDateTime(QDateTime::currentDateTime())
    , m_selectDate(m_currentDateTime.date())
    , m_weekDayFormat("ddd")
{
    //设置显示年份,开始结束时间
    setYearBeginAndEndDate(m_selectDate.year());
}
/**
 * @brief setSelectDate     设置选择时间
 * @param selectDate        选择时间
 */
void CalendarDateDataManager::setSelectDate(const QDate &selectDate, const bool isSwitchYear)
{
    m_selectDate = selectDate;
    if (isSwitchYear || m_showDateRange.startDate > m_selectDate || m_showDateRange.stopDate < m_selectDate) {
        //如果选择时间不在显示范围内则修改显示年份,开始和结束时间
        setYearBeginAndEndDate(m_selectDate.year());
    }
}
/**
 * @brief getSelectDate     获取选择时间
 * @return                  返回选择时间
 */
QDate CalendarDateDataManager::getSelectDate() const
{
    return m_selectDate;
}
/**
 * @brief setCurrentDateTime        设置当前时间
 * @param currentDateTime           当前时间
 */
void CalendarDateDataManager::setCurrentDateTime(const QDateTime &currentDateTime)
{
    m_currentDateTime = currentDateTime;
}
/**
 * @brief getCurrentDate    获取当前时间
 * @return                  返回当前时间
 */
QDateTime CalendarDateDataManager::getCurrentDate() const
{
    return  m_currentDateTime;
}
/**
 * @brief getYearDate       获取全年的时间
 * @param year              设置的年份
 * @return                  返回全年的时间，按照月份分组
 */
QMap<int, QVector<QDate> > CalendarDateDataManager::getYearDate()
{
    QMap<int, QVector<QDate> > _resultMap;
    for (int i = 1; i < 13; ++i) {
        _resultMap[i] = getMonthDate(m_showDateRange.showYear, i);
    }
    return _resultMap;
}

/**
 * @brief getMonthDate      获取月份的所有时间
 * @param year              设置的年份
 * @param month             设置的月份
 * @return                  返回当月全部时间
 */
QVector<QDate> CalendarDateDataManager::getMonthDate(const int &year, const int &month)
{
    QVector<QDate> _resultDate;
    //自然月的第一天
    const QDate _monthFirstDay{year, month, 1};
    //获取显示月的第一天
    const QDate _firstShowDayOfMonth = getFirstDayOfWeek(_monthFirstDay);
    //获取该月所有显示时间
    for (int i = 0; i < 42; ++i) {
        _resultDate.append(_firstShowDayOfMonth.addDays(i));
    }
    return _resultDate;
}
/**
 * @brief getWeekDate       获取一周的所有时间
 * @param date              设置的时间
 * @return                  返回这个周全部时间
 */
QVector<QDate> CalendarDateDataManager::getWeekDate(const QDate &date)
{
    QVector<QDate> _resultDate;
    //获取这个周的第一天日期
    const QDate _firstDayofWeek = getFirstDayOfWeek(date);
    //获取该周所有显示时间
    for (int i = 0; i < 7; ++i) {
        _resultDate.append(_firstDayofWeek.addDays(i));
    }
    return _resultDate;
}

/**
 * @brief setWeekFirstDay           设置每周以周几作为每周第一天
 * @param firstDay                  每周第一天
 */
void CalendarDateDataManager::setWeekFirstDay(const Qt::DayOfWeek &firstDay)
{
    m_weekFirstDay = firstDay;
    setYearBeginAndEndDate(m_showDateRange.showYear);
}
/**
 * @brief getWeekFirstDay           获取每周以周几作为每周第一天
 * @return                          每周第一天
 */
Qt::DayOfWeek CalendarDateDataManager::getWeekFirstDay()
{
    return  m_weekFirstDay;
}
/**
 * @brief setWeekDayFormatByID          设置周显示格式
 * @param weekDayFormatID
 *                      0 "dddd"
 *                      1 "ddd"
 */
void CalendarDateDataManager::setWeekDayFormatByID(const int &weekDayFormatID)
{
    switch (weekDayFormatID) {
    case 0:
        m_weekDayFormat = "dddd";
        break;
    default:
        m_weekDayFormat = "ddd";
        break;
    }
}
/**
 * @brief getWeekDayFormat          获取周显示格式
 * @return                          周显示格式
 * "ddd"    周一
 * "dddd"   星期一
 */
QString CalendarDateDataManager::getWeekDayFormat() const
{
    return m_weekDayFormat;
}

/**
 * @brief CalendarDateDataManager::getShowDateRange     获取显示年,开始结束时间
 * @return
 */
ShowDateRange CalendarDateDataManager::getShowDateRange() const
{
    return m_showDateRange;
}
/**
 * @brief getFirstDayOfWeek         根据日期获取当前周第一天的日期
 * @param date                      选择的日期
 * @return                          当前周第一天的日期
 */
QDate CalendarDateDataManager::getFirstDayOfWeek(const QDate &date)
{
    //根据选择时间周工作日和每周第一天的周工作日得到偏移量
    int _offset = date.dayOfWeek() - m_weekFirstDay;
    //根据偏移量获取需要添加还有减去的偏移天数
    const int _offsetDay = _offset < 0 ? _offset + 7 : _offset;
    //返回这周第一天的日期
    return date.addDays(0 - _offsetDay);
}
/**
 * @brief getWeekNumOfYear      根据日期获取该日期处于该年第多少周
 * @param date                  选择的日期
 * @return                      处于当年第多少周
 */
int CalendarDateDataManager::getWeekNumOfYear(const QDate &date)
{
    int _weekNum {0};
    //获取选择时间所在周的最后一天
    const QDate _laseDateInWeekBySelectDate = getFirstDayOfWeek(date).addDays(6);
    //该年第一天
    const QDate _firstDayOfYear{_laseDateInWeekBySelectDate.year(), 1, 1};
    //该年显示的第一天日期
    const QDate _firstShowDayOfYear = getFirstDayOfWeek(_firstDayOfYear);
    //处于该年显示第多少天,0为第一天
    const qint64  _dayOfShowYear = _firstShowDayOfYear.daysTo(_firstDayOfYear) + _laseDateInWeekBySelectDate.dayOfYear() - 1;
    _weekNum = qFloor(_dayOfShowYear / 7) + 1;
    return  _weekNum;
}

/**
 * @brief CalendarDateDataManager::setTimeFormatChanged 根据value设置短时间显示格式
 * @param value
 */
void CalendarDateDataManager::setTimeFormatChanged(int value)
{
    m_timeFormatValue = value;
    if (value) {
        m_timeFormat = "hh:mm";
    } else {
        m_timeFormat = "h:mm";
    }
}

/**
 * @brief CalendarDateDataManager::setDateFormatChanged 根据value设置短日期显示格式
 * @param value
 */
void CalendarDateDataManager::setDateFormatChanged(int value)
{
    switch (value) {
    case 0: {
        m_dateFormat = "yyyy/M/d";
    } break;
    case 1: {
        m_dateFormat = "yyyy-M-d";
    } break;
    case 2: {
        m_dateFormat = "yyyy.M.d";
    } break;
    case 3: {
        m_dateFormat = "yyyy/MM/dd";
    } break;
    case 4: {
        m_dateFormat = "yyyy-MM-dd";
    } break;
    case 5: {
        m_dateFormat = "yyyy.MM.dd";
    } break;
    case 6: {
        m_dateFormat = "yy/M/d";
    } break;
    case 7: {
        m_dateFormat = "yy-M-d";
    } break;
    case 8: {
        m_dateFormat = "yy.M.d";
    } break;
    default: {
        m_dateFormat = "yyyy-MM-dd";
    } break;
    }
}

/**
 * @brief CalendarDateDataManager::getTimeFormat 获取短时间格式
 */
QString CalendarDateDataManager::getTimeFormat() const
{
    return m_timeFormat;
}

/**
 * @brief CalendarDateDataManager::getTimeFormatValue 获取时间格式的value
 */
int CalendarDateDataManager::getTimeFormatValue() const
{
    return m_timeFormatValue;
}

/**
 * @brief CalendarDateDataManager::getDateFormat 获取短日期格式
 */
QString CalendarDateDataManager::getDateFormat() const
{
    return m_dateFormat;
}

void CalendarDateDataManager::setYearBeginAndEndDate(const int year)
{
    m_showDateRange.showYear = year;
    QDate _firstDayOfJan(year, 1, 1);
    m_showDateRange.startDate = getFirstDayOfWeek(_firstDayOfJan);
    QDate _firstDayOfDec(year, 12, 1);
    m_showDateRange.stopDate = getFirstDayOfWeek(_firstDayOfDec).addDays(MonthMaxDay - 1);
}
