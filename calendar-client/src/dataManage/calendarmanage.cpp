// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarmanage.h"
#include "cschedulebasewidget.h"
#include "scheduledatamanage.h"
#include "commondef.h"

#include <QDBusConnection>

CalendarManager *CalendarManager::m_scheduleManager = nullptr;
CalendarManager *CalendarManager::getInstance()
{
    // qCDebug(ClientLogger) << "Getting CalendarManager instance";
    CaHuangLiDayInfo::registerMetaType();
    if (m_scheduleManager == nullptr) {
        m_scheduleManager = new  CalendarManager;
        qRegisterMetaType<QMap<QDate, bool>>("QMap<QDate,bool>");
        qRegisterMetaType<QMap<QDate, CaHuangLiDayInfo> >("QMap<QDate, CaHuangLiDayInfo>");
        qRegisterMetaType<QMap<QDate, int>>("QMap<QDate, int>");
    }
    return m_scheduleManager;
}

void CalendarManager::releaseInstance()
{
    // qCDebug(ClientLogger) << "Releasing CalendarManager instance";
    if (m_scheduleManager != nullptr) {
        delete m_scheduleManager;
        m_scheduleManager = nullptr;
    }
}

//设置选择时间
void CalendarManager::setSelectDate(const QDate &selectDate, bool isSwitchYear)
{
    qCDebug(ClientLogger) << "Setting select date to:" << selectDate.toString() << "switch year:" << isSwitchYear;
    m_selectDate = selectDate;
    if (isSwitchYear || m_showDateRange.startDate > m_selectDate || m_showDateRange.stopDate < m_selectDate) {
        qCDebug(ClientLogger) << "Selected date outside display range, updating year to:" << selectDate.year();
        //如果选择时间不在显示范围内则修改显示年份,开始和结束时间
        setYearBeginAndEndDate(m_selectDate.year());
    }
}
//获取选择时间
QDate CalendarManager::getSelectDate() const
{
    // qCDebug(ClientLogger) << "Getting select date:" << m_selectDate.toString();
    return m_selectDate;
}
//设置当前时间
void CalendarManager::setCurrentDateTime(const QDateTime &currentDateTime)
{
    // qCDebug(ClientLogger) << "Setting current date time to:" << currentDateTime.toString();
    m_currentDateTime = currentDateTime;
}
//获取当前时间
QDateTime CalendarManager::getCurrentDate() const
{
    // qCDebug(ClientLogger) << "Getting current date time:" << m_currentDateTime.toString();
    return m_currentDateTime;
}

QVector<QDate> CalendarManager::getMonthDate(const int &year, const int &month)
{
    qCDebug(ClientLogger) << "Getting month dates for year:" << year << "month:" << month;
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

QVector<QDate> CalendarManager::getWeekDate(const QDate &date)
{
    qCDebug(ClientLogger) << "Getting week dates starting from:" << date.toString();
    QVector<QDate> _resultDate;
    //获取这个周的第一天日期
    const QDate _firstDayofWeek = getFirstDayOfWeek(date);
    //获取该周所有显示时间
    for (int i = 0; i < 7; ++i) {
        _resultDate.append(_firstDayofWeek.addDays(i));
    }
    return _resultDate;
}

//设置周显示格式
void CalendarManager::setWeekDayFormatByID(const int &weekDayFormatID)
{
    qCDebug(ClientLogger) << "Setting week day format to ID:" << weekDayFormatID;
    switch (weekDayFormatID) {
    case 0:
        m_weekDayFormat = "dddd";
        break;
    default:
        m_weekDayFormat = "ddd";
        break;
    }
}
//获取周显示格式
QString CalendarManager::getWeekDayFormat() const
{
    // qCDebug(ClientLogger) << "Getting week day format:" << m_weekDayFormat;
    return m_weekDayFormat;
}

//返回显示的年份,开始和结束时间
ShowDateRange CalendarManager::getShowDateRange() const
{
    // qCDebug(ClientLogger) << "Getting show date range for year:" << m_showDateRange.showYear 
    //                       << "from" << m_showDateRange.startDate.toString() 
    //                       << "to" << m_showDateRange.stopDate.toString();
    return m_showDateRange;
}

//根据日期获取当前周第一天的日期
QDate CalendarManager::getFirstDayOfWeek(const QDate &date)
{
    qCDebug(ClientLogger) << "Calculating first day of week for date:" << date.toString();
    //根据选择时间周工作日和每周第一天的周工作日得到偏移量
    int _offset = date.dayOfWeek() - getFirstDayOfWeek();
    //根据偏移量获取需要添加还有减去的偏移天数
    const int _offsetDay = (_offset + 7) % 7;
    //返回这周第一天的日期
    QDate firstDay = date.addDays(-_offsetDay);
    qCDebug(ClientLogger) << "First day of week is:" << firstDay.toString();
    return firstDay;
}

//根据日期获取该日期处于该年第多少周
int CalendarManager::getWeekNumOfYear(const QDate &date)
{
    qCDebug(ClientLogger) << "Calculating week number of year for date:" << date.toString();
    int _weekNum {0};
    //获取选择时间所在周的最后一天
    const QDate _laseDateInWeekBySelectDate = getInstance()->getFirstDayOfWeek(date).addDays(6);
    //该年第一天
    const QDate _firstDayOfYear{_laseDateInWeekBySelectDate.year(), 1, 1};
    //该年显示的第一天日期
    const QDate _firstShowDayOfYear = getInstance()->getFirstDayOfWeek(_firstDayOfYear);
    //处于该年显示第多少天,0为第一天
    const qint64  _dayOfShowYear = _firstShowDayOfYear.daysTo(_firstDayOfYear) + _laseDateInWeekBySelectDate.dayOfYear() - 1;
    _weekNum = qFloor(_dayOfShowYear / 7) + 1;
    qCDebug(ClientLogger) << "Week number is:" << _weekNum;
    return  _weekNum;
}

void CalendarManager::setTimeFormatChanged(int value)
{
    qCDebug(ClientLogger) << "Setting time format to value:" << value;
    // value = 0/1,对应的时间格式不对
    if (value == 0) {
        m_timeFormat = "h:mm";
    } else {
        m_timeFormat = "hh:mm";
    }
    updateData();
}

void CalendarManager::setDateFormatChanged(int value)
{
    qCDebug(ClientLogger) << "Setting date format to value:" << value;
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

QString CalendarManager::getTimeFormat() const
{
    // qCDebug(ClientLogger) << "Getting time format:" << m_timeFormat;
    return m_timeFormat;
}

void CalendarManager::setTimeShowType(int value, bool update)
{
    qCDebug(ClientLogger) << "Setting time show type to:" << value << "update:" << update;
    m_timeShowType = value;
    if (update) {
        updateData();
    }
}

int CalendarManager::getTimeShowType() const
{
    // qCDebug(ClientLogger) << "Getting time show type:" << m_timeShowType;
    return m_timeShowType;
}

QString CalendarManager::getDateFormat() const
{
    // qCDebug(ClientLogger) << "Getting date format:" << m_dateFormat;
    return m_dateFormat;
}

void CalendarManager::setYearBeginAndEndDate(const int year)
{
    qCDebug(ClientLogger) << "Setting year begin and end dates for year:" << year;
    m_showDateRange.showYear = year;
    QDate _firstDayOfJan(year, 1, 1);
    m_showDateRange.startDate = getFirstDayOfWeek(_firstDayOfJan);
    QDate _firstDayOfDec(year, 12, 1);
    m_showDateRange.stopDate = getFirstDayOfWeek(_firstDayOfDec).addDays(42 - 1);

    qCDebug(ClientLogger) << "Date range set to:" << m_showDateRange.startDate.toString() << "to" << m_showDateRange.stopDate.toString();

    //更新日程
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    // Qt5.12+: QDate 有 startOfDay() 方法
    gScheduleManager->resetSchedule(m_showDateRange.startDate.startOfDay(), m_showDateRange.stopDate.startOfDay());
#else
    // Qt5.11.3 兼容：使用 QDateTime 构造函数将 QDate 转为 QDateTime（时间设为 00:00:00）
    gScheduleManager->resetSchedule(QDateTime(m_showDateRange.startDate), QDateTime(m_showDateRange.stopDate));
#endif
    if (m_showLunar) {
        qCDebug(ClientLogger) << "Updating lunar and festival info for new date range";
        //刷新农历和节假日信息
        gLunarManager->queryLunarInfo(m_showDateRange.startDate, m_showDateRange.stopDate);
        gLunarManager->queryFestivalInfo(m_showDateRange.startDate, m_showDateRange.stopDate);
    }
}

/**
 * @brief CalendarManager::addShowWidget    添加显示界面
 * @param showWidget
 */
void CalendarManager::addShowWidget(CScheduleBaseWidget *showWidget)
{
    // qCDebug(ClientLogger) << "Adding show widget:" << showWidget;
    m_showWidget.append(showWidget);
}

/**
 * @brief CalendarManager::removeShowWidget 移除显示界面
 * @param showWidget
 */
void CalendarManager::removeShowWidget(CScheduleBaseWidget *showWidget)
{
    // qCDebug(ClientLogger) << "Removing show widget:" << showWidget;
    m_showWidget.removeOne(showWidget);
}

/**
 * @brief CalendarManager::getShowWidget    根据编号获取显示界面
 * @param index
 * @return
 */
CScheduleBaseWidget *CalendarManager::getShowWidget(const int index)
{
    qCDebug(ClientLogger) << "Getting show widget at index:" << index;
    if (index < 0 || index >= m_showWidget.size()) {
        qCDebug(ClientLogger) << "Invalid index, returning nullptr";
        return nullptr;
    }
    return m_showWidget.at(index);
}

/**
 * @brief CalendarManager::getShowWidgetSize        获取显示窗口的数目
 * @return
 */
int CalendarManager::getShowWidgetSize()
{
    // qCDebug(ClientLogger) << "Getting show widget size:" << m_showWidget.size();
    return m_showWidget.size();
}

/**
 * @brief CalendarManager::getShowLunar     获取农历信息
 * @return
 */
bool CalendarManager::getShowLunar() const
{
    // qCDebug(ClientLogger) << "Getting show lunar flag:" << m_showLunar;
    return m_showLunar;
}

/**
 * @brief getFirstDayOfWeek
 * @return 一周首日
 */
Qt::DayOfWeek CalendarManager::getFirstDayOfWeek()
{
    // qCDebug(ClientLogger) << "Getting first day of week:" << m_firstDayOfWeek;
    return Qt::DayOfWeek(m_firstDayOfWeek);
}

void CalendarManager::setFirstDayOfWeek(int day, bool update)
{
    qCDebug(ClientLogger) << "Setting first day of week to:" << day << "update:" << update;
    m_firstDayOfWeek = day;
    //更新显示界面
    if (update) {
        updateData();
    }
}

void CalendarManager::updateData()
{
    qCDebug(ClientLogger) << "Updating calendar data for" << m_showWidget.size() << "widgets";
    // 为了跟之前的代码一致，在这里发送信号出来；
    emit sigNotifySidebarFirstDayChanged(getFirstDayOfWeek());
    for (int i = 0; i < m_showWidget.size(); ++i) {
        m_showWidget.at(i)->updateData();
    }
}

CalendarManager::CalendarManager(QObject *parent)
    : QObject(parent)
    , m_timeDateDbus(new DaemonTimeDate(this))
    , m_currentDateTime(QDateTime::currentDateTime())
    , m_selectDate(m_currentDateTime.date())
    , m_weekDayFormat("ddd")
{
    qCDebug(ClientLogger) << "Creating CalendarManager";
    initConnection();
    initData();
}

CalendarManager::~CalendarManager()
{
    qCDebug(ClientLogger) << "Destroying CalendarManager";
}

/**
 * @brief CalendarManager::initData 初始化数据
 */
void CalendarManager::initData()
{
    qCDebug(ClientLogger) << "Initializing calendar data";
    //获取本地语言判断是否为中文
    m_showLunar = QLocale::system().language() == QLocale::Chinese;
    qCDebug(ClientLogger) << "Show lunar set to:" << m_showLunar << "based on system language";
    //获取时间日期格式
    const int _timeFormat = m_timeDateDbus->shortTimeFormat();
    const int _dateFormat = m_timeDateDbus->shortDateFormat();
    qCDebug(ClientLogger) << "System time format:" << _timeFormat << "date format:" << _dateFormat;
    setYearBeginAndEndDate(m_selectDate.year());
    //设置时间日期格式
    setTimeFormatChanged(_timeFormat);
    setDateFormatChanged(_dateFormat);
    slotGeneralSettingsUpdate();
}

/**
 * @brief CalendarManager::initConnection   初始化关联
 */
void CalendarManager::initConnection()
{
    qCDebug(ClientLogger) << "Initializing calendar connections";
    connect(gAccountManager, &AccountManager::signalGeneralSettingsUpdate, this, &CalendarManager::slotGeneralSettingsUpdate);
    connect(m_timeDateDbus, &DaemonTimeDate::ShortTimeFormatChanged, this, &CalendarManager::signalTimeFormatChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::ShortTimeFormatChanged, this, &CalendarManager::slotTimeFormatChanged);  // add time format slot.
    connect(m_timeDateDbus, &DaemonTimeDate::ShortDateFormatChanged, this, &CalendarManager::slotDateFormatChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::ShortDateFormatChanged, this, &CalendarManager::signalDateFormatChanged);
}

/**
 * @brief CalendarManager::WeekBeginsChanged    关联dbus信号，每周首日改变事触发
 * @param value
 */
void CalendarManager::weekBeginsChanged(int value)
{
    qCDebug(ClientLogger) << "Week begins changed to:" << value;
    setFirstDayOfWeek(value);
}

/**
 * @brief CalendarManager::slotGetLunarSuccess          农历更新成功刷新界面
 */
void CalendarManager::slotGetLunarSuccess()
{
    qCDebug(ClientLogger) << "Lunar data updated successfully";
    //更新显示界面
    for (int i = 0; i < m_showWidget.size(); ++i) {
        m_showWidget.at(i)->updateShowLunar();
    }
}

void CalendarManager::slotGeneralSettingsUpdate()
{
    qCDebug(ClientLogger) << "Updating from general settings";
    DCalendarGeneralSettings::Ptr setting = gAccountManager->getGeneralSettings();
    if (!setting) {
        qCWarning(ClientLogger) << "Failed to get general settings";
        return;
    }
    qCDebug(ClientLogger) << "Setting first day of week to:" << setting->firstDayOfWeek() << "and time show type to:" << setting->timeShowType();
    setFirstDayOfWeek(setting->firstDayOfWeek());
    setTimeShowType(setting->timeShowType());
}

/**
 * @brief CalendarManager::slotDateFormatChanged 更新日期显示格式
 * @param value
 */
void CalendarManager::slotDateFormatChanged(int value)
{
    qCDebug(ClientLogger) << "Date format changed to:" << value;
    setDateFormatChanged(value);
}

void CalendarManager::slotTimeFormatChanged(int value)
{
    qCDebug(ClientLogger) << "Time format changed to:" << value;
    setTimeFormatChanged(value);
}
