// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbushuanglirequest.h"

#include "commondef.h"

#include <QDebug>

DbusHuangLiRequest::DbusHuangLiRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/HuangLi",
                      "com.deepin.dataserver.Calendar.HuangLi",
                      QDBusConnection::sessionBus(),
                      parent)
{
}

/**
 * @brief DbusHuangLiRequest::getFestivalMonth
 * 按月获取节假日信息
 * @param year
 * @param month
 */
bool DbusHuangLiRequest::getFestivalMonth(quint32 year, quint32 month, FestivalInfo &festivalInfo)
{
    qCDebug(ClientLogger) << "Requesting festival info for" << year << "month" << month;
    QDBusPendingReply<QString> reply = call("getFestivalMonth", QVariant(year), QVariant(month));

    if (reply.isError()) {
        qCWarning(ClientLogger) << "Failed to get festival month info:" << reply.error().message();
        return false;
    }

    QString json = reply.argumentAt<0>();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        qCWarning(ClientLogger) << "Failed to parse festival month JSON:" << json_error.errorString();
        return false;
    }
    // 解析数据
    QJsonArray rootarry = jsonDoc.array();
    for (int i = 0; i < rootarry.size(); i++) {
        QJsonObject hsubObj = rootarry.at(i).toObject();

        QJsonArray subOnjArry = hsubObj.value("list").toArray();

        // 遍历内部的节日列表数组
        for (int j = 0; j < subOnjArry.size(); j++) {
            QJsonObject dayObj = subOnjArry.at(j).toObject();

            HolidayInfo dayinfo;
            if (dayObj.contains("status")) {
                dayinfo.status = static_cast<char>(dayObj.value("status").toInt());
            }
            if (dayObj.contains("date")) {
                dayinfo.date = QDate::fromString(dayObj.value("date").toString(), "yyyy-MM-dd");
            }
            festivalInfo.listHoliday.append(dayinfo);
        }
    }
    return true;
}

/**
 * @brief DbusHuangLiRequest::getHuangLiDay
 * 按天获取黄历信息
 * @param year
 * @param month
 * @param day
 */
bool DbusHuangLiRequest::getHuangLiDay(quint32 year,
                                       quint32 month,
                                       quint32 day,
                                       CaHuangLiDayInfo &info)
{
    qCDebug(ClientLogger) << "Requesting HuangLi day info for" << year << month << day;
    QDBusPendingReply<QString> reply =
        call("getHuangLiDay", QVariant(year), QVariant(month), QVariant(day));

    if (reply.isError()) {
        qCWarning(ClientLogger) << "Failed to get HuangLi day info:" << reply.error().message();
        return false;
    }

    QString json = reply.argumentAt<0>();
    bool isVoild;
    info.strJsonToInfo(json, isVoild);
    return isVoild;
}

/**
 * @brief DbusHuangLiRequest::getHuangLiMonth
 * 按月获取黄历信息
 * @param year
 * @param month
 * @param fill
 */
bool DbusHuangLiRequest::getHuangLiMonth(quint32 year,
                                         quint32 month,
                                         bool fill,
                                         CaHuangLiMonthInfo &info)
{
    qCDebug(ClientLogger) << "Requesting HuangLi month info for" << year << month << "fill:" << fill;
    QDBusPendingReply<QString> reply =
        call("getHuangLiMonth", QVariant(year), QVariant(month), QVariant(fill));
    if (reply.isError()) {
        qCWarning(ClientLogger) << "Failed to get HuangLi month info:" << reply.error().message();
        return false;
    }
    QString json = reply.argumentAt<0>();

    bool infoIsVaild;
    info.strJsonToInfo(json, infoIsVaild);
    return infoIsVaild;
}

/**
 * @brief DbusHuangLiRequest::getLunarInfoBySolar
 * 获取农历信息
 * @param year
 * @param month
 * @param day
 */
void DbusHuangLiRequest::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
{
    qCDebug(ClientLogger) << "Requesting lunar info for solar date" << year << month << day;
    asyncCall("getLunarInfoBySolar", {QVariant(year), QVariant(month), QVariant(day)});
}

/**
 * @brief DbusHuangLiRequest::getLunarMonthCalendar
 * 获取农历月日程
 * @param year
 * @param month
 * @param fill
 */
void DbusHuangLiRequest::getLunarMonthCalendar(quint32 year, quint32 month, bool fill)
{
    qCDebug(ClientLogger) << "Requesting lunar month calendar for" << year << month << "fill:" << fill;
    asyncCall("getLunarMonthCalendar", {QVariant(year), QVariant(month), QVariant(fill)});
}

void DbusHuangLiRequest::slotCallFinished(CDBusPendingCallWatcher *call)
{
    if (call->isError()) {
        qCWarning(ClientLogger) << "Async call failed:" << call->reply().member() << call->error().message();
        return;
    }
    qCDebug(ClientLogger) << "Async call completed successfully:" << call->reply().member();
    call->deleteLater();
}
