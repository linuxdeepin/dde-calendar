// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscheduledbus.h"

#include <QDebug>
#include <QDBusReply>
#include <QJsonDocument>


CScheduleDBus *CScheduleDBus::m_scheduleDBus = nullptr;
CScheduleDBus::CScheduleDBus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    //关联后端dbus触发信号
    if (!QDBusConnection::sessionBus().connect(this->service(), this->path(), staticInterfaceName(), "", this, SLOT(propertyChanged(QDBusMessage)))) {
        qWarning() << "the connection was fail!";
    };
}

CScheduleDBus *CScheduleDBus::getInstance()
{
    if (m_scheduleDBus == nullptr) {
        m_scheduleDBus = new CScheduleDBus(DBUS_NAME, DBUS_PATH, QDBusConnection::sessionBus());
    }
    return m_scheduleDBus;
}

void CScheduleDBus::releaseInstance()
{
    delete m_scheduleDBus;
    m_scheduleDBus = nullptr;
}

CScheduleDBus::~CScheduleDBus()
{

}

/**
 * @brief CScheduleDBus::CreateJob  创建日程
 * @param info                      日程信息
 * @return          -1表示失败， 创建日程ID
 */
qint64 CScheduleDBus::CreateJob(const ScheduleDataInfo &info)
{
    if (!info.isValid()) {
        qWarning() << "this is not a valid info :" << info << "  createJob Err";
        return -1;
    }
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(info.ScheduleToJsonStr(info));
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("CreateJob"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "createJob err ," << reply;
        return  -1;
    }
    QDBusReply<qint64> id =  reply;
    return  id.value();
}

/**
 * @brief CScheduleDBus::GetJobs        获取日程信息
 * @param startDate                     开始时间
 * @param endDate                       结束时间
 * @return
 */
bool CScheduleDBus::GetJobs(const QDate &startDate, const QDate &endDate, QMap<QDate, QVector<ScheduleDataInfo> > &info)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(static_cast<quint32>(startDate.year()))
                 << QVariant::fromValue(static_cast<quint32>(startDate.month()))
                 << QVariant::fromValue(static_cast<quint32>(startDate.day()));
    argumentList << QVariant::fromValue(static_cast<quint32>(endDate.year()))
                 << QVariant::fromValue(static_cast<quint32>(endDate.month()))
                 << QVariant::fromValue(static_cast<quint32>(endDate.day()));
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("GetJobs"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "GetJobs err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;

    if (!jobs.isValid())
        return false;
    info = ScheduleDataInfo::StrJsonToRangeInfo(jobs.value());
    return  true;
}

/**
 * @brief CScheduleDBus::GetJob     根据日程id获取对应日程
 * @param jobId                     日程id
 * @param out                       获取到的日程信息
 * @return
 */
bool CScheduleDBus::GetJob(qint64 jobId, ScheduleDataInfo &out)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(jobId);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("GetJob"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "GetJob err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;
    out = ScheduleDataInfo::JsonStrToSchedule(jobs.value());
    return true;
}

/**
 * @brief CScheduleDBus::UpdateJob      更新日程
 * @param info                          需要更新的日程信息
 * @return
 */
bool CScheduleDBus::UpdateJob(const ScheduleDataInfo &info)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(ScheduleDataInfo::ScheduleToJsonStr(info));
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("UpdateJob"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "UpdateJob err ," << reply;
        return false;
    }
    return true;
}

/**
 * @brief CScheduleDBus::DeleteJob      根据日程id删除日程
 * @param jobId                         需要删除的日程id
 * @return
 */
bool CScheduleDBus::DeleteJob(qint64 jobId)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(jobId);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("DeleteJob"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "DeleteJob err ," << reply;
        return false;
    }
    return true;
}

/**
 * @brief CScheduleDBus::QueryJobs      查询一段时间内某日程
 * @param key                           需要查询日程的关键字
 * @param starttime                     开始时间
 * @param endtime                       结束时间
 * @param out                           查询到的日程
 * @return
 */
bool CScheduleDBus::QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QMap<QDate, QVector<ScheduleDataInfo> > &out)
{
    //若时间无效则退出查询
    if (starttime.isNull() || endtime.isNull()) {
        qWarning() << "starttime or endtime is not Valid";
        return false;
    }
    QJsonObject qjson;
    qjson.insert("Key", key);
    qjson.insert("Start", ScheduleDataInfo::DateTimeToStringDate(starttime));
    qjson.insert("End", ScheduleDataInfo::DateTimeToStringDate(endtime));
    // 构建 JSON 文档
    QJsonDocument qdocument;
    qdocument.setObject(qjson);
    QByteArray qbyteArray = qdocument.toJson(QJsonDocument::Compact);
    QString strJson(qbyteArray);
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(strJson);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("QueryJobs"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "QueryJobs err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;
    out = ScheduleDataInfo::StrJsonToRangeInfo(jobs.value());
    return  true;
}

/**
 * @brief CScheduleDBus::QueryJobs          查询日程
 * @param key
 * @param starttime
 * @param endtime
 * @param outStr
 * @return
 */
bool CScheduleDBus::QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QString &outStr)
{
    outStr = "";
    //若时间无效则退出查询
    if (starttime.isNull() || endtime.isNull()) {
        qWarning() << "starttime or endtime is not Valid";
        return false;
    }
    QJsonObject qjson;
    qjson.insert("Key", key);
    qjson.insert("Start", ScheduleDataInfo::DateTimeToStringDate(starttime));
    qjson.insert("End", ScheduleDataInfo::DateTimeToStringDate(endtime));
    // 构建 JSON 文档
    QJsonDocument qdocument;
    qdocument.setObject(qjson);
    QByteArray qbyteArray = qdocument.toJson(QJsonDocument::Compact);
    QString strJson(qbyteArray);
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(strJson);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("QueryJobs"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "QueryJobs err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;
    outStr = jobs.value();
    return  true;
}

/**
 * @brief CScheduleDBus::GetFestivalMonth       获取班休信息
 * @param year                                  年
 * @param month                                 月
 * @return
 */
bool CScheduleDBus::GetFestivalMonth(quint32 year, quint32 month, QVector<FestivalInfo> &out)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(year) << QVariant::fromValue(month) ;
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("GetFestivalMonth"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "GetFestivalMonth err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;

    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jobs.value().toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonArray rootarry = jsonDoc.array();

    for (int i = 0; i < rootarry.size(); i++) {

        QJsonObject subObj = rootarry.at(i).toObject();
        FestivalInfo festivalInfoday;

        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (subObj.contains("id")) {
            festivalInfoday.ID = subObj.value("id").toString();
        }
        if (subObj.contains("name")) {
            festivalInfoday.FestivalName = subObj.value("name").toString();
        }
        if (subObj.contains("description")) {
            festivalInfoday.description = subObj.value("description").toString();
        }
        if (subObj.contains("rest")) {
            festivalInfoday.Rest = subObj.value("rest").toString();
        }
        if (subObj.contains("month")) {
            festivalInfoday.month = subObj.value("month").toInt();
        }
        if (subObj.contains("list")) {
            QJsonArray sublistArray = subObj.value("list").toArray();
            for (int j = 0; j < sublistArray.size(); j++) {
                QJsonObject hsubObj = sublistArray.at(j).toObject();
                HolidayInfo dayinfo;
                if (hsubObj.contains("status")) {
                    dayinfo.status = static_cast<char>(hsubObj.value("status").toInt());
                }
                if (hsubObj.contains("date")) {
                    dayinfo.date = QDate::fromString(hsubObj.value("date").toString(), "yyyy-M-d");
                }
                festivalInfoday.listHoliday.append(dayinfo);
            }
        }
        festivalInfoday.year = static_cast<int>(year);
        out.append(festivalInfoday);
    }
    return true;
}

/**
 * @brief CScheduleDBus::GetHuangLiDay          获取某天的农历信息
 * @param getDay
 * @param out
 * @return
 */
bool CScheduleDBus::GetHuangLiDay(const QDate &getDay, CaHuangLiDayInfo &out)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(static_cast<quint32>(getDay.year()))
                 << QVariant::fromValue(static_cast<quint32>(getDay.month()))
                 << QVariant::fromValue(static_cast<quint32>(getDay.day()));
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("GetHuangLiDay"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "GetHuangLiDay err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;
    bool _isVoild;
    out.strJsonToInfo(jobs.value(), _isVoild);
    return _isVoild;
}

/**
 * @brief CScheduleDBus::GetHuangLiMonth        获取某月的农历信息
 * @param year
 * @param month
 * @param fill
 * @return
 */
bool CScheduleDBus::GetHuangLiMonth(const quint32 year, const quint32 month, bool &fill, CaHuangLiMonthInfo &out)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(year) << QVariant::fromValue(month) << QVariant::fromValue(fill);
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("GetHuangLiMonth"), argumentList);
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "GetHuangLiMonth err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;
    bool _infoIsVaild;
    out.strJsonToInfo(jobs.value(), _infoIsVaild);
    return _infoIsVaild;
}

QString CScheduleDBus::getHuangLiShortName(const QDate &date)
{
    CaHuangLiDayInfo info;
    if (GetHuangLiDay(date, info)) {
        return info.mLunarMonthName + info.mLunarDayName;
    }
    return "";
}


void CScheduleDBus::propertyChanged(const QDBusMessage &msg)
{
    if (msg.type() == QDBusMessage::SignalMessage && msg.path() == this->path() && msg.interface() == this->interface()) {
        if (msg.member() == "JobsUpdated") {
            //日程更新信号
            emit jobsUpdate();
        }

        if (msg.member() == "JobTypeOrColorUpdated") {
            //日程类型或颜色更新
            emit jobsTypeOrColorUpdate();
        }
    }
}

//获取日程类型信息列表
bool CScheduleDBus::GetJobTypeList(QString &strJson)
{
    QDBusPendingCall pCall = asyncCall(QStringLiteral("GetJobTypeList"));
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "GetJobTypeList err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;
    strJson = jobs.value();
    return  true;
}
//新增日程类型信息
bool CScheduleDBus::AddJobType(QString strJson)
{
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("CreateJobType"), {QVariant(strJson)});
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "AddJobType err ," << reply << " strJson:" << strJson;
        return false;
    }
    QDBusReply<bool> jobs =  reply;
    if (!jobs.isValid())
        return false;
    return jobs.value();
}
//更新日程类型信息
bool CScheduleDBus::UpdateJobType(QString strJson)
{
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("UpdateJobType"), {QVariant(strJson)});
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "UpdateJobType err ," << reply << " strJson:" << strJson;
        return  false;
    }
    QDBusReply<bool> jobs =  reply;
    if (!jobs.isValid())
        return false;
    return  jobs.value();
}
//删除日程类型信息
bool CScheduleDBus::DeleteJobType(int jobTypeNo)
{
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("DeleteJobType"), {QVariant(jobTypeNo)});
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "DeleteJobType err ," << reply << " jobTypeNo:" << jobTypeNo;
        return false;
    }
    QDBusReply<bool> jobs =  reply;
    if (!jobs.isValid())
        return false;
    return jobs.value();
}
/**
 * @brief isJobTypeUsed    获取日程类型是否被使用
 * return bool             返回是否被使用
 */
bool CScheduleDBus::isJobTypeUsed(int jobTypeNo)
{
    QDBusPendingCall pCall = asyncCallWithArgumentList(QStringLiteral("isJobTypeUsed"), {QVariant(jobTypeNo)});
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "isJobTypeUsed err ," << reply << " jobTypeNo:" << jobTypeNo;
        return false;
    }
    QDBusReply<bool> jobs =  reply;
    if (!jobs.isValid())
        return false;
    return jobs.value();
}

//获取颜色信息列表
bool CScheduleDBus::GetJobTypeColorList(QString &strJson)
{
    QDBusPendingCall pCall = asyncCall(QStringLiteral("GetColorTypeList"));
    pCall.waitForFinished();
    QDBusMessage reply = pCall.reply();
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "GetColorTypeList err ," << reply;
        return false;
    }
    QDBusReply<QString> jobs =  reply;
    if (!jobs.isValid())
        return false;
    strJson = jobs.value();
    return  true;
}
