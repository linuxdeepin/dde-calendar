// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cschedulebasewidget.h"
CalendarManager *CScheduleBaseWidget::m_calendarManager = nullptr;
CScheduleBaseWidget::CScheduleBaseWidget(QWidget *parent)
    : QWidget(parent)
{
    if (m_calendarManager == nullptr) {
        m_calendarManager = CalendarManager::getInstance();
        //获取一年的日程信息
        updateDBusData();
    }
    m_calendarManager->addShowWidget(this);
}

CScheduleBaseWidget::~CScheduleBaseWidget()
{
    m_calendarManager->removeShowWidget(this);
    if (m_calendarManager->getShowWidgetSize() == 0) {
        CalendarManager::releaseInstance();
        m_calendarManager = nullptr;
    }
}

/**
 * @brief CScheduleBaseWidget::setSelectDate    设置选择时间
 * @param selectDate
 */
bool CScheduleBaseWidget::setSelectDate(const QDate &selectDate, const bool isSwitchYear)
{
    bool _result = false;    //选择时间必须大于等于1900年
    if (selectDate.year() >= 1900) {
        m_calendarManager->getCalendarDateDataManage()->setSelectDate(selectDate, isSwitchYear);
        _result = true;
        //更新其它视图界面显示
        for (int i = 0; i < m_calendarManager->getShowWidgetSize(); ++i) {
            //如果为当前视图则不更新
            if (m_calendarManager->getShowWidget(i) == this || m_calendarManager->getShowWidget(i) == nullptr)
                continue;
            m_calendarManager->getShowWidget(i)->setYearData();
            m_calendarManager->getShowWidget(i)->updateShowDate();
        }
        updateDBusData();
    }
    return _result;
}

/**
 * @brief CScheduleBaseWidget::getSelectDate    获取选择时间
 * @return
 */
QDate CScheduleBaseWidget::getSelectDate() const
{
    return m_calendarManager->getCalendarDateDataManage()->getSelectDate();
}

/**
 * @brief CScheduleBaseWidget::setCurrentDateTime   设置当前时间
 * @param currentDate
 */
void CScheduleBaseWidget::setCurrentDateTime(const QDateTime &currentDate)
{
    m_calendarManager->getCalendarDateDataManage()->setCurrentDateTime(currentDate);
}

/**
 * @brief CScheduleBaseWidget::getCurrendDateTime   获取当前时间
 * @return
 */
QDateTime CScheduleBaseWidget::getCurrendDateTime() const
{
    return m_calendarManager->getCalendarDateDataManage()->getCurrentDate();
}

/**
 * @brief CScheduleBaseWidget::getShowLunar     获取是否显示农历信息
 * @return
 */
bool CScheduleBaseWidget::getShowLunar()
{
    return m_calendarManager->getShowLunar();
}

/**
 * @brief CScheduleBaseWidget::updateData       更新数据显示
 */
void CScheduleBaseWidget::updateData()
{
    updateShowDate();
    updateShowSchedule();
    updateShowLunar();
    updateSearchScheduleInfo();
}

/**
 * @brief CScheduleBaseWidget::updateDBusData           更新一年的dbus数据
 */
void CScheduleBaseWidget::updateDBusData()
{
    ShowDateRange _showDateRange = m_calendarManager->getCalendarDateDataManage()->getShowDateRange();
    //如果缓存中不包含开始或结束时间则更新dbus数据
    if (!m_calendarManager->getScheduleTask()->hasScheduleInfo(_showDateRange.startDate, _showDateRange.stopDate)) {
        //获取日程开始和结束时间，考虑切换日视图会显示显示时间之外的时间所以前后多获取2个月日程数据。
        QDate _startDate = _showDateRange.startDate.addDays(-42);
        QDate _stopDate = _showDateRange.stopDate.addDays(42); //getShowLunar()
        //获取日程任务
        CScheduleTask *_task = m_calendarManager->getScheduleTask();
        if (_startDate.isValid() && _stopDate.isValid()) {
            //更新日程信息
            _task->updateInfo(_startDate, _stopDate, getShowLunar());
        } else {
            qWarning() << "startDate or stopDate Err!";
        }
    }
}

/**
 * @brief CScheduleBaseWidget::updateSearchScheduleInfo     更新搜索日程
 *  由子类完成
 */
void CScheduleBaseWidget::updateSearchScheduleInfo()
{

}

void CScheduleBaseWidget::deleteselectSchedule()
{

}

/**
 * @brief CScheduleBaseWidget::getLunarInfo 获取选择时间的农历信息
 * @return
 */
CaHuangLiDayInfo CScheduleBaseWidget::getLunarInfo()
{
    QMap<QDate, CaHuangLiDayInfo> _huangLiInfo = m_calendarManager->getScheduleTask()->getHuangLiInfo(getSelectDate(), getSelectDate());
    QMap<QDate, CaHuangLiDayInfo>::const_iterator interator = _huangLiInfo.constBegin();
    if (interator == _huangLiInfo.constEnd())
        return CaHuangLiDayInfo();
    m_lunarYear = QString("-%0%1年-").arg(interator.value().mGanZhiYear).arg(interator.value().mZodiac);
    m_lunarDay = QString("-农历%0%1-").arg(interator.value().mLunarMonthName).arg(interator.value().mLunarDayName);
    return interator.value();
}
