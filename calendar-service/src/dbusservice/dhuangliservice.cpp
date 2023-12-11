// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
    DServiceExitControl exitControl;
    QString festivalInfo = m_huangli->getFestivalMonth(year, month);
    return festivalInfo;
}

//获取指定公历日的黄历信息
QString DHuangliService::getHuangLiDay(quint32 year, quint32 month, quint32 day)
{

    if( 0 >= year || 0 >= month || 0 >= day)
        return "";
    DServiceExitControl exitControl;
    QString huangliInfo = m_huangli->getHuangLiDay(year, month, day);
    return huangliInfo;
}

//获取指定公历月的黄历信息
QString DHuangliService::getHuangLiMonth(quint32 year, quint32 month, bool fill)
{
    DServiceExitControl exitControl;
    QString huangliInfo = m_huangli->getHuangLiMonth(year, month, fill);
    return huangliInfo;
}

//通过公历获取阴历信息
CaLunarDayInfo DHuangliService::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
{
    DServiceExitControl exitControl;
    CaLunarDayInfo huangliInfo = m_huangli->getLunarInfoBySolar(year, month, day);
    return huangliInfo;
}

//获取阴历月信息
CaLunarMonthInfo DHuangliService::getLunarMonthCalendar(quint32 year, quint32 month, bool fill)
{
    DServiceExitControl exitControl;
    CaLunarMonthInfo huangliInfo = m_huangli->getLunarCalendarMonth(year, month, fill);
    return huangliInfo;
}
