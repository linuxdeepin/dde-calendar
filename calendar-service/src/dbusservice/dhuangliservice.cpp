/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#include "dhuangliservice.h"
#include "units.h"

#include "calendarprogramexitcontrol.h"

DHuangliService::DHuangliService(QObject *parent)
    : DServiceBase(serviceBasePath + "/HuangLi", serviceBaseName + ".HuangLi", parent)
    , m_huangli(new CalendarHuangLi(this))
{
    CaLunarDayInfo::registerMetaType();
    CaLunarMonthInfo::registerMetaType();
    CaHuangLiDayInfo::registerMetaType();
    CaHuangLiMonthInfo::registerMetaType();
}

//获取指定公历月的假日信息
QString DHuangliService::getFestivalMonth(quint32 year, quint32 month)
{
    CalendarProgramExitControl::getProgramExitControl()->addExc();
    QString festivalInfo = m_huangli->getFestivalMonth(year, month);
    CalendarProgramExitControl::getProgramExitControl()->reduce();
    return festivalInfo;
}

//获取指定公历日的黄历信息
QString DHuangliService::getHuangLiDay(quint32 year, quint32 month, quint32 day)
{
    CalendarProgramExitControl::getProgramExitControl()->addExc();
    QString huangliInfo = m_huangli->getHuangLiDay(year, month, day);
    CalendarProgramExitControl::getProgramExitControl()->reduce();
    return huangliInfo;
}

//获取指定公历月的黄历信息
QString DHuangliService::getHuangLiMonth(quint32 year, quint32 month, bool fill)
{
    CalendarProgramExitControl::getProgramExitControl()->addExc();
    QString huangliInfo = m_huangli->getHuangLiMonth(year, month, fill);
    CalendarProgramExitControl::getProgramExitControl()->reduce();
    return huangliInfo;
}

//通过公历获取阴历信息
CaLunarDayInfo DHuangliService::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
{
    CalendarProgramExitControl::getProgramExitControl()->addExc();
    CaLunarDayInfo huangliInfo = m_huangli->getLunarInfoBySolar(year, month, day);
    CalendarProgramExitControl::getProgramExitControl()->reduce();
    return huangliInfo;
}

//获取阴历月信息
CaLunarMonthInfo DHuangliService::getLunarMonthCalendar(quint32 year, quint32 month, bool fill)
{
    CalendarProgramExitControl::getProgramExitControl()->addExc();
    CaLunarMonthInfo huangliInfo = m_huangli->getLunarCalendarMonth(year, month, fill);
    CalendarProgramExitControl::getProgramExitControl()->reduce();
    return huangliInfo;
}
