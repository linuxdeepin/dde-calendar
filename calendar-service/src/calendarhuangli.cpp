// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calendarhuangli.h"
#include "lunarandfestival/lunarmanager.h"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CalendarHuangLiLog, "calendar.huangli")

CalendarHuangLi::CalendarHuangLi(QObject *parent)
    : QObject(parent)
    , m_database(new DHuangLiDataBase(this))
{
    qCDebug(CalendarHuangLiLog) << "Initializing CalendarHuangLi";
}

//获取指定公历月的假日信息
QJsonArray CalendarHuangLi::getFestivalMonth(quint32 year, quint32 month)
{
    qCDebug(CalendarHuangLiLog) << "Getting festival info for year:" << year << "month:" << month;
    QJsonArray result = m_database->queryFestivalList(year, month);
    qCDebug(CalendarHuangLiLog) << "Found" << result.size() << "festivals";
    return result;
}

QString CalendarHuangLi::getHuangLiDay(quint32 year, quint32 month, quint32 day)
{
    qCDebug(CalendarHuangLiLog) << "Getting HuangLi info for date:" << year << month << day;
    stDay viewday{static_cast<int>(year), static_cast<int>(month), static_cast<int>(day)};
    QList<stDay> viewdate{viewday};
    CaHuangLiDayInfo hldayinfo;
    //获取阴历信息
    stLunarDayInfo lunardayinfo = SolarToLunar(static_cast<qint32>(year), static_cast<qint32>(month), static_cast<qint32>(day));
    //获取宜忌信息
    QList<stHuangLi> hllist = m_database->queryHuangLiByDays(viewdate);
    if (hllist.isEmpty()) {
        qCWarning(CalendarHuangLiLog) << "No HuangLi data found for date:" << year << month << day;
        return QString();
    }
    //将黄历信息保存到CaHuangLiDayInfo
    hldayinfo.mSuit = hllist.begin()->Suit;
    hldayinfo.mAvoid = hllist.begin()->Avoid;
    hldayinfo.mGanZhiYear = lunardayinfo.GanZhiYear;
    hldayinfo.mGanZhiMonth = lunardayinfo.GanZhiMonth;
    hldayinfo.mGanZhiDay = lunardayinfo.GanZhiDay;
    hldayinfo.mLunarDayName = lunardayinfo.LunarDayName;
    hldayinfo.mLunarFestival = lunardayinfo.LunarFestival;
    hldayinfo.mLunarLeapMonth = lunardayinfo.LunarLeapMonth;
    hldayinfo.mLunarMonthName = lunardayinfo.LunarMonthName;
    hldayinfo.mSolarFestival = lunardayinfo.SolarFestival;
    hldayinfo.mTerm = lunardayinfo.Term;
    hldayinfo.mZodiac = lunardayinfo.Zodiac;
    hldayinfo.mWorktime = lunardayinfo.Worktime;
    //返回json
    QString result = hldayinfo.toJson();
    qCDebug(CalendarHuangLiLog) << "HuangLi info retrieved successfully";
    return result;
}

QString CalendarHuangLi::getHuangLiMonth(quint32 year, quint32 month, bool fill)
{
    qCDebug(CalendarHuangLiLog) << "Getting HuangLi month info for year:" << year << "month:" << month << "fill:" << fill;
    CaHuangLiMonthInfo monthinfo;
    SolarMonthInfo solarmonth = GetSolarMonthCalendar(static_cast<qint32>(year), static_cast<qint32>(month), fill);
    LunarMonthInfo lunarmonth = GetLunarMonthCalendar(static_cast<qint32>(year), static_cast<qint32>(month), fill);
    monthinfo.mFirstDayWeek = lunarmonth.FirstDayWeek;
    monthinfo.mDays = lunarmonth.Days;
    QList<stHuangLi> hllist = m_database->queryHuangLiByDays(solarmonth.Datas);
    if (hllist.isEmpty()) {
        qCWarning(CalendarHuangLiLog) << "No HuangLi data found for month:" << year << month;
        return QString();
    }
    for (int i = 0; i < lunarmonth.Datas.size(); ++i) {
        CaHuangLiDayInfo hldayinfo;
        hldayinfo.mAvoid = hllist.at(i).Avoid;
        hldayinfo.mSuit = hllist.at(i).Suit;
        stLunarDayInfo lunardayinfo = lunarmonth.Datas.at(i);
        hldayinfo.mGanZhiYear = lunardayinfo.GanZhiYear;
        hldayinfo.mGanZhiMonth = lunardayinfo.GanZhiMonth;
        hldayinfo.mGanZhiDay = lunardayinfo.GanZhiDay;
        hldayinfo.mLunarDayName = lunardayinfo.LunarDayName;
        hldayinfo.mLunarFestival = lunardayinfo.LunarFestival;
        hldayinfo.mLunarLeapMonth = lunardayinfo.LunarLeapMonth;
        hldayinfo.mLunarMonthName = lunardayinfo.LunarMonthName;
        hldayinfo.mSolarFestival = lunardayinfo.SolarFestival;
        hldayinfo.mTerm = lunardayinfo.Term;
        hldayinfo.mZodiac = lunardayinfo.Zodiac;
        hldayinfo.mWorktime = lunardayinfo.Worktime;
        monthinfo.mCaLunarDayInfo.append(hldayinfo);
//        qCDebug(ServiceLogger) << hldayinfo.mGanZhiYear << hldayinfo.mGanZhiMonth << hldayinfo.mGanZhiDay
//                 << hldayinfo.mLunarDayName << hldayinfo.mLunarFestival << hldayinfo.mLunarFestival
//                 << hldayinfo.mLunarLeapMonth << hldayinfo.mLunarMonthName << hldayinfo.mSolarFestival
//                 << hldayinfo.mTerm << hldayinfo.mZodiac << hldayinfo.mWorktime;
    }
    QString result = monthinfo.toJson();
    qCDebug(CalendarHuangLiLog) << "HuangLi month info retrieved successfully with" << monthinfo.mCaLunarDayInfo.size() << "days";
    return result;
}

CaLunarDayInfo CalendarHuangLi::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
{
    qCDebug(CalendarHuangLiLog) << "Getting lunar info for solar date:" << year << month << day;
    CaLunarDayInfo lunardayinfo;
    //获取阴历信息
    stLunarDayInfo m_lunardayinfo = SolarToLunar(static_cast<qint32>(year), static_cast<qint32>(month), static_cast<qint32>(day));
    //将阴历信息保存到CaLunarDayInfo
    lunardayinfo.mGanZhiYear = m_lunardayinfo.GanZhiYear;
    lunardayinfo.mGanZhiMonth = m_lunardayinfo.GanZhiMonth;
    lunardayinfo.mGanZhiDay = m_lunardayinfo.GanZhiDay;
    lunardayinfo.mLunarDayName = m_lunardayinfo.LunarDayName;
    lunardayinfo.mLunarFestival = m_lunardayinfo.LunarFestival;
    lunardayinfo.mLunarLeapMonth = m_lunardayinfo.LunarLeapMonth;
    lunardayinfo.mLunarMonthName = m_lunardayinfo.LunarMonthName;
    lunardayinfo.mSolarFestival = m_lunardayinfo.SolarFestival;
    lunardayinfo.mTerm = m_lunardayinfo.Term;
    lunardayinfo.mZodiac = m_lunardayinfo.Zodiac;
    lunardayinfo.mWorktime = m_lunardayinfo.Worktime;
    //返回CaLunarDayInfo
    qCDebug(CalendarHuangLiLog) << "Lunar info retrieved successfully";
    return lunardayinfo;
}

CaLunarMonthInfo CalendarHuangLi::getLunarCalendarMonth(quint32 year, quint32 month, bool fill)
{
    qCDebug(CalendarHuangLiLog) << "Getting lunar calendar month for year:" << year << "month:" << month << "fill:" << fill;
    CaLunarMonthInfo lunarmonthinfo;
    //获取阴历月信息
    LunarMonthInfo lunarmonth = GetLunarMonthCalendar(static_cast<qint32>(year), static_cast<qint32>(month), fill);
    //将阴历月信息保存到CaLunarMonthInfo
    lunarmonthinfo.mDays = lunarmonth.Days;
    lunarmonthinfo.mFirstDayWeek = lunarmonth.FirstDayWeek;
    for (int i = 0; i < lunarmonth.Datas.size(); ++i) {
        CaLunarDayInfo m_lunardayinfo;
        stLunarDayInfo lunardayinfo = lunarmonth.Datas.at(i);
        m_lunardayinfo.mGanZhiYear = lunardayinfo.GanZhiYear;
        m_lunardayinfo.mGanZhiMonth = lunardayinfo.GanZhiMonth;
        m_lunardayinfo.mGanZhiDay = lunardayinfo.GanZhiDay;
        m_lunardayinfo.mLunarDayName = lunardayinfo.LunarDayName;
        m_lunardayinfo.mLunarFestival = lunardayinfo.LunarFestival;
        m_lunardayinfo.mLunarLeapMonth = lunardayinfo.LunarLeapMonth;
        m_lunardayinfo.mLunarMonthName = lunardayinfo.LunarMonthName;
        m_lunardayinfo.mSolarFestival = lunardayinfo.SolarFestival;
        m_lunardayinfo.mTerm = lunardayinfo.Term;
        m_lunardayinfo.mZodiac = lunardayinfo.Zodiac;
        m_lunardayinfo.mWorktime = lunardayinfo.Worktime;
        lunarmonthinfo.mCaLunarDayInfo.append(m_lunardayinfo);
    }
    //返回CaLunarMonthInfo
    qCDebug(CalendarHuangLiLog) << "Lunar calendar month retrieved successfully with" << lunarmonthinfo.mCaLunarDayInfo.size() << "days";
    return lunarmonthinfo;
}
