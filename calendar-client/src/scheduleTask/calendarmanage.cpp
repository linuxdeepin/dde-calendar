// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarmanage.h"
#include "cschedulebasewidget.h"
#include "scheduledatamanage.h"

#include <QDBusConnection>

const QString DBus_TimeDate_Name = "com.deepin.daemon.Timedate";
const QString DBus_TimeDate_Path = "/com/deepin/daemon/Timedate";

CalendarManager *CalendarManager::m_scheduleManager = nullptr;
CalendarManager *CalendarManager::getInstance()
{
    ScheduleDataInfo::registerMetaType();
    CaHuangLiDayInfo::registerMetaType();
    if (m_scheduleManager == nullptr) {
        m_scheduleManager = new  CalendarManager;
        qRegisterMetaType<QMap<QDate, QVector<ScheduleDataInfo> > >("QMap<QDate,QVector<ScheduleDataInfo> >");
        qRegisterMetaType<QMap<QDate, bool>>("QMap<QDate,bool>");
        qRegisterMetaType<QMap<QDate, CaHuangLiDayInfo> >("QMap<QDate, CaHuangLiDayInfo>");
        qRegisterMetaType<QMap<QDate, int>>("QMap<QDate, int>");
    }
    return m_scheduleManager;
}

void CalendarManager::releaseInstance()
{
    if (m_scheduleManager != nullptr) {
        delete m_scheduleManager;
        m_scheduleManager = nullptr;
    }
}

/**
 * @brief CalendarManager::addShowWidget    添加显示界面
 * @param showWidget
 */
void CalendarManager::addShowWidget(CScheduleBaseWidget *showWidget)
{
    m_showWidget.append(showWidget);
}

/**
 * @brief CalendarManager::removeShowWidget 移除显示界面
 * @param showWidget
 */
void CalendarManager::removeShowWidget(CScheduleBaseWidget *showWidget)
{
    m_showWidget.removeOne(showWidget);
}

/**
 * @brief CalendarManager::getShowWidget    根据编号获取显示界面
 * @param index
 * @return
 */
CScheduleBaseWidget *CalendarManager::getShowWidget(const int index)
{
    if (index < 0 || index >= m_showWidget.size()) {
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
    return m_showWidget.size();
}

/**
 * @brief CalendarManager::getCalendarDateDataManage        获取日历时间数据管理
 * @return
 */
CalendarDateDataManager *CalendarManager::getCalendarDateDataManage() const
{
    return m_dateManage;
}

/**
 * @brief CalendarManager::getScheduleTask      获取日程任务
 * @return
 */
CScheduleTask *CalendarManager::getScheduleTask() const
{
    return m_scheduleTask;
}

/**
 * @brief CalendarManager::getShowLunar     获取农历信息
 * @return
 */
bool CalendarManager::getShowLunar() const
{
    return m_showLunar;
}

CalendarManager::CalendarManager(QObject *parent)
    : QObject(parent)
    , m_dateManage(new CalendarDateDataManager)
    , m_timeDateDbus(new DaemonTimeDate(DBus_TimeDate_Name, DBus_TimeDate_Path, QDBusConnection::sessionBus(), this))
    , m_scheduleTask(new CScheduleTask(this))
{
    initData();
    initConnection();
    //更新日程类型和颜色
    updateJobTypeColor();
}

CalendarManager::~CalendarManager()
{
    delete m_dateManage;
}

/**
 * @brief CalendarManager::initData 初始化数据
 */
void CalendarManager::initData()
{
    //获取本地语言判断是否为中文
    m_showLunar = QLocale::system().language() == QLocale::Chinese;
    //获取每周第一天
    const int _weekFirstDay = m_timeDateDbus->weekBegins();
    //获取时间日期格式
    const int _timeFormat = m_timeDateDbus->shortTimeFormat();
    const int _dateFormat = m_timeDateDbus->shortDateFormat();
    //设置时间日期格式
    m_dateManage->setTimeFormatChanged(_timeFormat);
    m_dateManage->setDateFormatChanged(_dateFormat);
    setWeekBegins(_weekFirstDay);
}

/**
 * @brief CalendarManager::initConnection   初始化关联
 */
void CalendarManager::initConnection()
{
    connect(m_timeDateDbus, &DaemonTimeDate::WeekBeginsChanged, this, &CalendarManager::WeekBeginsChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::ShortTimeFormatChanged, this, &CalendarManager::slotTimeFormatChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::ShortTimeFormatChanged, this, &CalendarManager::signalTimeFormatChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::ShortDateFormatChanged, this, &CalendarManager::slotDateFormatChanged);
    connect(m_timeDateDbus, &DaemonTimeDate::ShortDateFormatChanged, this, &CalendarManager::signalDateFormatChanged);
    connect(m_scheduleTask, &CScheduleTask::signalUpdateScheduleShow, this, &CalendarManager::slotGetScheduleSuccess);
    connect(m_scheduleTask, &CScheduleTask::signalLunarGetSuccess, this, &CalendarManager::slotGetLunarSuccess);
    connect(m_scheduleTask, &CScheduleTask::jobsUpdate, this, &CalendarManager::slotJobsUpdated);
    connect(m_scheduleTask, &CScheduleTask::jobsTypeOrColorUpdate, this, &CalendarManager::slotUpdateJobTypeColor);
    connect(m_scheduleTask, &CScheduleTask::signalUpdateSearchSchedule, this, &CalendarManager::slotUpdateSearchSchedule);
}

/**
 * @brief CalendarManager::setWeekBegins    设置每周首日
 * @param value
 */
void CalendarManager::setWeekBegins(const int value)
{
    //将从TimeDate DBus获取到的数字与周对应
    QMap<int, Qt::DayOfWeek> WeekBeginMap{
        {0, Qt::DayOfWeek::Monday},
        {1, Qt::DayOfWeek::Tuesday},
        {2, Qt::DayOfWeek::Wednesday},
        {3, Qt::DayOfWeek::Thursday},
        {4, Qt::DayOfWeek::Friday},
        {5, Qt::DayOfWeek::Saturday},
        {6, Qt::DayOfWeek::Sunday}
    };
    if (value >= 0 && value < 7) {
        //设置每周第一天
        m_dateManage->setWeekFirstDay(WeekBeginMap[value]);
    } else {
        qWarning() << "setWeekBegins err ,set value:" << value;
    }
}

/**
 * @brief CalendarManager::updateJobs       更新日程信息
 */
void CalendarManager::updateJobs()
{
    //更新dbus数据
    ShowDateRange _showDateRange = m_dateManage->getShowDateRange();
    //获取显示年份前后一个月数据
    m_scheduleTask->updateInfo(_showDateRange.startDate.addDays(-42), _showDateRange.stopDate.addDays(42), m_showLunar);
}

void CalendarManager::updateJobTypeColor()
{
    JobTypeInfoManager::instance()->updateInfo();
}

/**
 * @brief CalendarManager::WeekBeginsChanged    关联dbus信号，每周首日改变事触发
 * @param value
 */
void CalendarManager::WeekBeginsChanged(int value)
{
    //设置每周首日
    setWeekBegins(value);
    //更新显示界面
    for (int i = 0; i < m_showWidget.size(); ++i) {
        m_showWidget.at(i)->updateData();
    }
    //更新dbus数据
    updateJobs();
}

/**
 * @brief CalendarManager::slotGetScheduleSuccess       日程更新成功刷新界面
 */
void CalendarManager::slotGetScheduleSuccess()
{
    //更新显示界面
    for (int i = 0; i < m_showWidget.size(); ++i) {
        m_showWidget.at(i)->updateShowSchedule();
    }
}

/**
 * @brief CalendarManager::slotGetLunarSuccess          农历更新成功刷新界面
 */
void CalendarManager::slotGetLunarSuccess()
{
    //更新显示界面
    for (int i = 0; i < m_showWidget.size(); ++i) {
        m_showWidget.at(i)->updateShowLunar();
    }
}

/**
 * @brief CalendarManager::slotJobsUpdated  接收dbus信号更新日程日程信息
 * @param Ids
 */
void CalendarManager::slotJobsUpdated()
{
    updateJobs();
}

void CalendarManager::slotUpdateSearchSchedule()
{
    //更新显示界面
    for (int i = 0; i < m_showWidget.size(); ++i) {
        m_showWidget.at(i)->updateSearchScheduleInfo();
    }
}

void CalendarManager::slotUpdateJobTypeColor()
{
    updateJobTypeColor();
}

/**
 * @brief CalendarManager::slotTimeFormatChanged 更新时间显示格式
 * @param value
 */
void CalendarManager::slotTimeFormatChanged(int value)
{
    QString timeFormat;
    m_dateManage->setTimeFormatChanged(value);
    //更新显示界面
    for (int i = 0; i < m_showWidget.size(); ++i) {
        m_showWidget.at(i)->updateData();
    }
}

/**
 * @brief CalendarManager::slotDateFormatChanged 更新日期显示格式
 * @param value
 */
void CalendarManager::slotDateFormatChanged(int value)
{
    m_dateManage->setDateFormatChanged(value);
}
