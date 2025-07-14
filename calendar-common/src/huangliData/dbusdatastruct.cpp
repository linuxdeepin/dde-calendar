// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusdatastruct.h"
#include "commondef.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>

void CaLunarDayInfo::registerMetaType()
{
    qCDebug(CommonLogger) << "Registering CaLunarDayInfo meta type.";
    qRegisterMetaType<CaLunarDayInfo>();
    qDBusRegisterMetaType<CaLunarDayInfo>();
}

QDebug operator<<(QDebug argument, const CaLunarDayInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaLunarDayInfo to QDebug.";
    argument << what.mGanZhiYear << what.mGanZhiMonth << what.mGanZhiDay;
    argument << what.mLunarMonthName << what.mLunarDayName;
    argument << what.mLunarLeapMonth;
    argument << what.mZodiac << what.mTerm;
    argument << what.mSolarFestival << what.mLunarFestival;
    argument << what.mWorktime;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaLunarDayInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaLunarDayInfo to QDBusArgument.";
    argument.beginStructure();
    argument << what.mGanZhiYear << what.mGanZhiMonth << what.mGanZhiDay;
    argument << what.mLunarMonthName << what.mLunarDayName;
    argument << what.mLunarLeapMonth;
    argument << what.mZodiac << what.mTerm;
    argument << what.mSolarFestival << what.mLunarFestival;
    argument << what.mWorktime;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaLunarDayInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaLunarDayInfo from QDBusArgument.";
    argument.beginStructure();
    argument >> what.mGanZhiYear >> what.mGanZhiMonth >> what.mGanZhiDay;
    argument >> what.mLunarMonthName >> what.mLunarDayName;
    argument >> what.mLunarLeapMonth;
    argument >> what.mZodiac >> what.mTerm;
    argument >> what.mSolarFestival >> what.mLunarFestival;
    argument >> what.mWorktime;
    argument.endStructure();

    return argument;
}

void CaLunarMonthInfo::registerMetaType()
{
    // qCDebug(CommonLogger) << "Registering CaLunarMonthInfo meta type.";
    qRegisterMetaType<CaLunarMonthInfo>();
    qDBusRegisterMetaType<CaLunarMonthInfo>();
}

QDebug operator<<(QDebug argument, const CaLunarMonthInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaLunarMonthInfo to QDebug.";
    argument << what.mFirstDayWeek << what.mDays;
    argument << what.mCaLunarDayInfo;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaLunarMonthInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaLunarMonthInfo to QDBusArgument.";
    argument.beginStructure();
    argument << what.mFirstDayWeek << what.mDays;
    argument << what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaLunarMonthInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaLunarMonthInfo from QDBusArgument.";
    argument.beginStructure();
    argument >> what.mFirstDayWeek >> what.mDays;
    argument >> what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}

void CaHuangLiDayInfo::registerMetaType()
{
    // qCDebug(CommonLogger) << "Registering CaHuangLiDayInfo meta type.";
    qRegisterMetaType<CaHuangLiDayInfo>();
    qDBusRegisterMetaType<CaHuangLiDayInfo>();
}

QString CaHuangLiDayInfo::toJson()
{
    // qCDebug(CommonLogger) << "Converting CaHuangLiDayInfo to JSON string.";
    QJsonDocument doc;
    QJsonObject obj;

    obj.insert("Suit", mSuit);
    obj.insert("Avoid", mAvoid);
    obj.insert("Worktime", mWorktime);
    obj.insert("LunarFestival", mLunarFestival);
    obj.insert("SolarFestival", mSolarFestival);
    obj.insert("Term", mTerm);
    obj.insert("Zodiac", mZodiac);
    obj.insert("LunarLeapMonth", mLunarLeapMonth);
    obj.insert("LunarDayName", mLunarDayName);
    obj.insert("LunarMonthName", mLunarMonthName);
    obj.insert("GanZhiDay", mGanZhiDay);
    obj.insert("GanZhiMonth", mGanZhiMonth);
    obj.insert("GanZhiYear", mGanZhiYear);

    doc.setObject(obj);
    QString strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void CaHuangLiDayInfo::strJsonToInfo(const QString &strJson, bool &isVaild)
{
    // qCDebug(CommonLogger) << "Parsing CaHuangLiDayInfo from JSON string.";
    isVaild = true;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(strJson.toLocal8Bit(), &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        qCDebug(CommonLogger) << "JSON parse error:" << json_error.errorString();
        isVaild = false;
        return ;
    }
    QJsonObject rootObj = jsonDoc.object();
    jsonObjectToInfo(rootObj);
}

void CaHuangLiDayInfo::jsonObjectToInfo(const QJsonObject &jsonObject)
{
    // qCDebug(CommonLogger) << "Parsing CaHuangLiDayInfo from JSON object.";
    //因为是预先定义好的JSON数据格式，所以这里可以这样读取int
    if (jsonObject.contains("Suit")) {
        // qCDebug(CommonLogger) << "Found and setting Suit.";
        this->mSuit = jsonObject.value("Suit").toString();
    }
    if (jsonObject.contains("Avoid")) {
        // qCDebug(CommonLogger) << "Found and setting Avoid.";
        this->mAvoid = jsonObject.value("Avoid").toString();
    }
    if (jsonObject.contains("Worktime")) {
        // qCDebug(CommonLogger) << "Found and setting Worktime.";
        this->mWorktime = jsonObject.value("Worktime").toInt();
    }
    if (jsonObject.contains("LunarFestival")) {
        // qCDebug(CommonLogger) << "Found and setting LunarFestival.";
        this->mLunarFestival = jsonObject.value("LunarFestival").toString();
    }
    if (jsonObject.contains("SolarFestival")) {
        // qCDebug(CommonLogger) << "Found and setting SolarFestival.";
        this->mSolarFestival = jsonObject.value("SolarFestival").toString();
    }
    if (jsonObject.contains("Term")) {
        // qCDebug(CommonLogger) << "Found and setting Term.";
        this->mTerm = jsonObject.value("Term").toString();
    }
    if (jsonObject.contains("Zodiac")) {
        // qCDebug(CommonLogger) << "Found and setting Zodiac.";
        this->mZodiac = jsonObject.value("Zodiac").toString();
    }
    if (jsonObject.contains("LunarLeapMonth")) {
        // qCDebug(CommonLogger) << "Found and setting LunarLeapMonth.";
        this->mLunarLeapMonth = jsonObject.value("LunarLeapMonth").toInt();
    }
    if (jsonObject.contains("LunarDayName")) {
        // qCDebug(CommonLogger) << "Found and setting LunarDayName.";
        this->mLunarDayName = jsonObject.value("LunarDayName").toString();
    }
    if (jsonObject.contains("LunarMonthName")) {
        // qCDebug(CommonLogger) << "Found and setting LunarMonthName.";
        this->mLunarMonthName = jsonObject.value("LunarMonthName").toString();
    }
    if (jsonObject.contains("GanZhiDay")) {
        // qCDebug(CommonLogger) << "Found and setting GanZhiDay.";
        this->mGanZhiDay = jsonObject.value("GanZhiDay").toString();
    }
    if (jsonObject.contains("GanZhiMonth")) {
        // qCDebug(CommonLogger) << "Found and setting GanZhiMonth.";
        this->mGanZhiMonth = jsonObject.value("GanZhiMonth").toString();
    }
    if (jsonObject.contains("GanZhiYear")) {
        // qCDebug(CommonLogger) << "Found and setting GanZhiYear.";
        this->mGanZhiYear = jsonObject.value("GanZhiYear").toString();
    }
}

QDebug operator<<(QDebug argument, const CaHuangLiDayInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaHuangLiDayInfo to QDebug.";
    argument << what.mSuit << what.mAvoid;
    argument << what.mWorktime;
    argument << what.mLunarFestival << what.mSolarFestival;
    argument << what.mTerm << what.mZodiac;
    argument << what.mLunarLeapMonth;
    argument << what.mLunarDayName << what.mLunarMonthName;
    argument << what.mGanZhiDay << what.mGanZhiMonth << what.mGanZhiYear;
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaHuangLiDayInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaHuangLiDayInfo to QDBusArgument.";
    argument.beginStructure();
    argument << what.mSuit << what.mAvoid;
    argument << what.mWorktime;
    argument << what.mLunarFestival << what.mSolarFestival;
    argument << what.mTerm << what.mZodiac;
    argument << what.mLunarLeapMonth;
    argument << what.mLunarDayName << what.mLunarMonthName;
    argument << what.mGanZhiDay << what.mGanZhiMonth << what.mGanZhiYear;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaHuangLiDayInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaHuangLiDayInfo from QDBusArgument.";
    argument.beginStructure();
    argument >> what.mSuit >> what.mAvoid;
    argument >> what.mWorktime;
    argument >> what.mLunarFestival >> what.mSolarFestival;
    argument >> what.mTerm >> what.mZodiac;
    argument >> what.mLunarLeapMonth;
    argument >> what.mLunarDayName >> what.mLunarMonthName;
    argument >> what.mGanZhiDay >> what.mGanZhiMonth >> what.mGanZhiYear;
    argument.endStructure();
    return argument;
}

void CaHuangLiMonthInfo::registerMetaType()
{
    // qCDebug(CommonLogger) << "Registering CaHuangLiMonthInfo meta type.";
    qRegisterMetaType<CaHuangLiMonthInfo>();
    qDBusRegisterMetaType<CaHuangLiMonthInfo>();
}

QString CaHuangLiMonthInfo::toJson()
{
    // qCDebug(CommonLogger) << "Converting CaHuangLiMonthInfo to JSON string.";
    QJsonArray daysarr;
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("Days", mDays);
    obj.insert("FirstDayWeek", mFirstDayWeek);
    foreach (CaHuangLiDayInfo dayinfo, mCaLunarDayInfo) {
        // qCDebug(CommonLogger) << "Processing a day's info for JSON conversion.";
        QJsonObject dayobj;
        dayobj.insert("Suit", dayinfo.mSuit);
        dayobj.insert("Avoid", dayinfo.mAvoid);
        dayobj.insert("Worktime", dayinfo.mWorktime);
        dayobj.insert("LunarFestival", dayinfo.mLunarFestival);
        dayobj.insert("SolarFestival", dayinfo.mSolarFestival);
        dayobj.insert("Term", dayinfo.mTerm);
        dayobj.insert("Zodiac", dayinfo.mZodiac);
        dayobj.insert("LunarLeapMonth", dayinfo.mLunarLeapMonth);
        dayobj.insert("LunarDayName", dayinfo.mLunarDayName);
        dayobj.insert("LunarMonthName", dayinfo.mLunarMonthName);
        dayobj.insert("GanZhiDay", dayinfo.mGanZhiDay);
        dayobj.insert("GanZhiMonth", dayinfo.mGanZhiMonth);
        dayobj.insert("GanZhiYear", dayinfo.mGanZhiYear);
        daysarr.append(dayobj);
    }
    obj.insert("Datas", daysarr);
    doc.setObject(obj);
    QString strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void CaHuangLiMonthInfo::strJsonToInfo(const QString &strJson, bool &isVaild)
{
    // qCDebug(CommonLogger) << "Parsing CaHuangLiMonthInfo from JSON string.";
    isVaild = true;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(strJson.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        // qCDebug(CommonLogger) << "JSON parse error:" << json_error.errorString();
        isVaild = false;
        return;
    }

    QJsonObject rootObj = jsonDoc.object();

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if (rootObj.contains("Days")) {
        // qCDebug(CommonLogger) << "Found and setting Days.";
        this->mDays = rootObj.value("Days").toInt();
    }
    if (rootObj.contains("FirstDayWeek")) {
        // qCDebug(CommonLogger) << "Found and setting FirstDayWeek.";
        this->mFirstDayWeek = rootObj.value("FirstDayWeek").toInt();
    }
    if (rootObj.contains("Datas")) {
        // qCDebug(CommonLogger) << "Found and processing Datas array.";
        QJsonArray subArray = rootObj.value("Datas").toArray();
        for (int i = 0; i < subArray.size(); i++) {
            // qCDebug(CommonLogger) << "Processing array item at index" << i;
            QJsonObject subObj = subArray.at(i).toObject();
            CaHuangLiDayInfo huangliday;
            huangliday.jsonObjectToInfo(subObj);
            this->mCaLunarDayInfo.append(huangliday);
        }
    }
}

void CaHuangLiMonthInfo::clear()
{
    // qCDebug(CommonLogger) << "Clearing CaHuangLiMonthInfo.";
    this->mDays = 0;
    this->mCaLunarDayInfo.clear();
}

QDebug operator<<(QDebug argument, const CaHuangLiMonthInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaHuangLiMonthInfo to QDebug.";
    argument << what.mDays << what.mFirstDayWeek;
    argument << what.mCaLunarDayInfo;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaHuangLiMonthInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaHuangLiMonthInfo to QDBusArgument.";
    argument.beginStructure();
    argument << what.mDays << what.mFirstDayWeek;
    argument << what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaHuangLiMonthInfo &what)
{
    // qCDebug(CommonLogger) << "Streaming CaHuangLiMonthInfo from QDBusArgument.";
    argument.beginStructure();
    argument >> what.mDays >> what.mFirstDayWeek;
    argument >> what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}
