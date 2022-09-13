// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARDATEDATAMANAGE_H
#define CALENDARDATEDATAMANAGE_H

#include <QDateTime>
#include <QVector>
#include <QMap>

struct ShowDateRange {
    int showYear{0};
    QDate startDate;
    QDate stopDate;
};
/**
 * @brief The CalendarDateDataManager class
 * 日历时间管理类
 */
class CalendarDateDataManager
{
public:
    explicit CalendarDateDataManager();
    //设置选择时间
    void setSelectDate(const QDate &selectDate, const bool isSwitchYear = false);
    //获取选择时间
    QDate getSelectDate() const;
    //设置当前时间
    void setCurrentDateTime(const QDateTime &currentDateTime = QDateTime::currentDateTime());
    //获取当前时间
    QDateTime getCurrentDate() const;
    //获取全年的时间
    QMap<int, QVector<QDate> > getYearDate();
    //获取月份的所有时间
    static QVector<QDate> getMonthDate(const int &year, const int &month);
    //获取一周的所有时间
    static QVector<QDate> getWeekDate(const QDate &date);
    //设置每周以周几作为每周第一天
    void setWeekFirstDay(const Qt::DayOfWeek &firstDay);
    //获取每周以周几作为每周第一天
    static Qt::DayOfWeek getWeekFirstDay();
    //设置周显示格式
    void setWeekDayFormatByID(const int &weekDayFormatID);
    //获取周显示格式
    QString getWeekDayFormat() const;
    //返回显示的年份,开始和结束时间
    ShowDateRange getShowDateRange() const;
    //根据日期获取当前周第一天的日期
    static QDate getFirstDayOfWeek(const QDate &date);
    //根据日期获取该日期处于该年第多少周
    static int getWeekNumOfYear(const QDate &date);

    void setTimeFormatChanged(int value);
    void setDateFormatChanged(int value);
    QString getTimeFormat() const;
    int getTimeFormatValue() const;
    QString getDateFormat() const;

private:
    void setYearBeginAndEndDate(const int year);
private:
    /**
     * @brief m_currentDateTime     当前时间
     */
    QDateTime   m_currentDateTime;
    /**
     * @brief m_selectDate          设置选择时间
     */
    QDate m_selectDate;
    /**
     * @brief m_firstDay          每周第一天
     * 1  周一
     * 2  周二
     * 3  周三
     * 4  周四
     * 5  周五
     * 6  周六
     * 7  周日
     */
    static Qt::DayOfWeek     m_weekFirstDay;
    /**
     * @brief m_weekDayFormat          周显示格式
     * ddd  周一
     * dddd  星期一
     */
    QString                 m_weekDayFormat{"ddd"};
    ShowDateRange           m_showDateRange;
    QString                 m_timeFormat = "h:mm";
    QString                 m_dateFormat = "yyyy-MM-dd";
    int                     m_timeFormatValue = 0;
};

#endif // CALENDARDATEDATAMANAGE_H
