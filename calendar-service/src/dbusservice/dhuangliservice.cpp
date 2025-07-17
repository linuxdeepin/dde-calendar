// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhuangliservice.h"
#include "commondef.h"

#include "calendarprogramexitcontrol.h"
#include "units.h"

DHuangliService::DHuangliService(QObject *parent)
    : DServiceBase(serviceBasePath + "/HuangLi", serviceBaseName + ".HuangLi", parent)
    , m_huangli(new CalendarHuangLi(this))
{
    qCDebug(ServiceLogger) << "Initializing HuangliService with path:" << serviceBasePath + "/HuangLi";
    CaLunarDayInfo::registerMetaType();
    CaLunarMonthInfo::registerMetaType();
    CaHuangLiDayInfo::registerMetaType();
    CaHuangLiMonthInfo::registerMetaType();
    qCDebug(ServiceLogger) << "Registered all HuangLi meta types";
    qCDebug(ServiceLogger) << "HuangliService initialization completed";
}

// 获取指定公历月的假日信息
// !这个接口 dde-daemon 在使用，不能变动!
QString DHuangliService::getFestivalMonth(quint32 year, quint32 month)
{
    qCDebug(ServiceLogger) << "Getting festival month data for year:" << year << "month:" << month;
    DServiceExitControl exitControl;
    auto list = m_huangli->getFestivalMonth(year, month);
    qCDebug(ServiceLogger) << "Retrieved" << list.size() << "festival entries";
    // 保持接口返回值兼容
    QJsonArray result;
    if (!list.empty()) {
        qCDebug(ServiceLogger) << "Creating JSON response with first entry ID:" << list.at(0)["date"];
        QJsonObject obj;
        obj.insert("id", list.at(0)["date"]);
        obj.insert("description", "");
        obj.insert("list", list);
        result.push_back(obj);
    } else {
        qCDebug(ServiceLogger) << "No festival data found, returning empty result";
    }
    QJsonDocument doc;
    doc.setArray(result);
    QString jsonResult = doc.toJson(QJsonDocument::Compact);
    qCDebug(ServiceLogger) << "Returning festival month JSON with length:" << jsonResult.length();
    return jsonResult;
}

// 获取指定公历日的黄历信息
QString DHuangliService::getHuangLiDay(quint32 year, quint32 month, quint32 day)
{
    qCDebug(ServiceLogger) << "Getting HuangLi day data for date:" << year << "-" << month << "-" << day;
    DServiceExitControl exitControl;
    QString huangliInfo = m_huangli->getHuangLiDay(year, month, day);
    qCDebug(ServiceLogger) << "Retrieved HuangLi day data with length:" << huangliInfo.length();
    return huangliInfo;
}

// 获取指定公历月的黄历信息
QString DHuangliService::getHuangLiMonth(quint32 year, quint32 month, bool fill)
{
    qCDebug(ServiceLogger) << "Getting HuangLi month data for year:" << year << "month:" << month << "fill:" << fill;
    DServiceExitControl exitControl;
    QString huangliInfo = m_huangli->getHuangLiMonth(year, month, fill);
    qCDebug(ServiceLogger) << "Retrieved HuangLi month data with length:" << huangliInfo.length();
    return huangliInfo;
}

// 通过公历获取阴历信息
CaLunarDayInfo DHuangliService::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
{
    qCDebug(ServiceLogger) << "Getting lunar info by solar date:" << year << "-" << month << "-" << day;
    DServiceExitControl exitControl;
    CaLunarDayInfo huangliInfo = m_huangli->getLunarInfoBySolar(year, month, day);
    qCDebug(ServiceLogger) << "Retrieved lunar day info for date:" << year << "-" << month << "-" << day;
    return huangliInfo;
}

// 获取阴历月信息
CaLunarMonthInfo DHuangliService::getLunarMonthCalendar(quint32 year, quint32 month, bool fill)
{
    qCDebug(ServiceLogger) << "Getting lunar month calendar for year:" << year << "month:" << month << "fill:" << fill;
    DServiceExitControl exitControl;
    CaLunarMonthInfo huangliInfo = m_huangli->getLunarCalendarMonth(year, month, fill);
    qCDebug(ServiceLogger) << "Retrieved lunar month calendar for year:" << year << "month:" << month;
    return huangliInfo;
}
