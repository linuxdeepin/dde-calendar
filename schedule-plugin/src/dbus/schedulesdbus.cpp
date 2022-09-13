// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulesdbus.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
/*
 * Implementation of interface class DCalendarDBus
 */

CSchedulesDBus::CSchedulesDBus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    QDBusConnection::sessionBus().connect(this->service(), this->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", "sa{sv}as", this, SLOT(__propertyChanged__(QDBusMessage)));
}

CSchedulesDBus::~CSchedulesDBus()
{
    QDBusConnection::sessionBus().disconnect(service(), path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", "sa{sv}as", this, SLOT(propertyChanged(QDBusMessage)));
}

QString CSchedulesDBus::createScheduleDtailInfojson(const ScheduleDtailInfo &info)
{
    QJsonObject json;
    json.insert("ID", info.id);
    json.insert("AllDay", info.allday);
    json.insert("Remind", createScheduleRemind(info));
    json.insert("RRule", createScheduleRRule(info));
    json.insert("Title", info.titleName);
    json.insert("Description", info.description);
    json.insert("Type", info.type.ID);
    json.insert("Start", toconvertData(info.beginDateTime));
    json.insert("End", toconvertData(info.endDateTime));
    //json.insert("Start", "2006-01-02T15:04:05+07:00");
    //json.insert("End", "2006-01-02T17:04:05+07:00");
    json.insert("RecurID", info.RecurID);
    QJsonArray jsonarry;
    for (int i = 0; i < info.ignore.count(); i++) {
        jsonarry.append(toconvertData(info.ignore.at(i)));
    }
    json.insert("Ignore", jsonarry);
    // 构建 JSON 文档
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    return strJson;
}

QString CSchedulesDBus::createScheduleTypejson(const ScheduleType &info)
{
    QJsonObject json;
    json.insert("ID", info.ID);
    json.insert("Name", info.typeName);
    QString colorName = QString("#%1%2%3").arg(info.color.red(), 2, 16, QChar('0')).arg(info.color.green(), 2, 16, QChar('0')).arg(info.color.blue(), 2, 16, QChar('0'));
    json.insert("Color", info.color.name());
    // 构建 JSON 文档
    QJsonDocument document;
    document.setObject(json);
    QByteArray byteArray = document.toJson(QJsonDocument::Compact);
    QString strJson(byteArray);
    return strJson;
}

ScheduleType CSchedulesDBus::parsingScheduleTypejson(QJsonObject &object)
{
    ScheduleType type;
    QJsonObject &rootObj = object;

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if (rootObj.contains("ID")) {
        type.ID = rootObj.value("ID").toInt();
    }
    if (rootObj.contains("Name")) {
        type.typeName = rootObj.value("Name").toString();
    }
    if (rootObj.contains("Color")) {
        QString str = rootObj.value("Color").toString();
        type.color = QColor(rootObj.value("Color").toString());
    }
    return type;
}

ScheduleDtailInfo CSchedulesDBus::parsingScheduleDtailInfojsonID(QJsonObject &obj)
{
    ScheduleDtailInfo info;

    QJsonObject &rootObj = obj;
    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if (rootObj.contains("ID")) {
        info.id = rootObj.value("ID").toInt();
    }
    if (rootObj.contains("AllDay")) {
        info.allday = rootObj.value("AllDay").toBool();
    }
    if (rootObj.contains("Remind")) {
        parsingScheduleRemind(rootObj.value("Remind").toString(), info);
    }
    if (rootObj.contains("Title")) {
        info.titleName = rootObj.value("Title").toString();
    }
    if (rootObj.contains("Description")) {
        info.description = rootObj.value("Description").toString();
    }
    if (rootObj.contains("Type")) {
        GetType(rootObj.value("Type").toInt(), info.type);
    }
    if (rootObj.contains("Start")) {
        info.beginDateTime = fromconvertData(rootObj.value("Start").toString());
    }
    if (rootObj.contains("End")) {
        info.endDateTime = fromconvertData(rootObj.value("End").toString());
    }
    if (rootObj.contains("RecurID")) {
        info.RecurID = rootObj.value("RecurID").toInt();
    }
    if (rootObj.contains("RRule")) {
        parsingScheduleRRule(rootObj.value("RRule").toString(), info);
    }
    if (rootObj.contains("Ignore")) {
        QJsonArray subArray = rootObj.value("Ignore").toArray();
        for (int i = 0; i < subArray.size(); i++) {
            QString subObj = subArray.at(i).toString();
            info.ignore.append(fromconvertData(subObj));
        }
    }
    return info;
}

QString CSchedulesDBus::createScheduleRRule(const ScheduleDtailInfo &info)
{
    if (info.rpeat == 0)
        return QString();
    // QString str = "'";
    QString str;
    switch (info.rpeat) {
    case 1: {
        str += "FREQ=DAILY";
    } break;
    case 2: {
        str += "FREQ=DAILY;BYDAY=MO,TU,WE,TH,FR";
    } break;
    case 3: {
        str += "FREQ=WEEKLY";
    } break;
    case 4: {
        str += "FREQ=MONTHLY";
    } break;
    case 5: {
        str += "FREQ=YEARLY";
    } break;
    }
    switch (info.enddata.type) {
    case 1: {
        str += QString(";COUNT=%1").arg(info.enddata.tcount + 1);
    } break;
    case 2: {
        QDateTime datetime = info.enddata.date;
        //datetime.setDate(datetime);
        str += ";UNTIL=" + datetime.toString("yyyyMMddThhmmss") + "Z";
        // str += ";UNTIL=" + toconvertData(datetime);
    } break;
    }
    //str += "'";
    return str;
}

void CSchedulesDBus::parsingScheduleRRule(QString str, ScheduleDtailInfo &info)
{
    if (str.isEmpty()) {
        info.rpeat = 0;
        return;
    }
    QString rrulestrs = str;
    QStringList rruleslist = rrulestrs.split(";", QString::SkipEmptyParts);
    if (rruleslist.count() > 0) {
        if (rruleslist.contains("FREQ=DAILY") && rruleslist.contains("BYDAY=MO,TU,WE,TH,FR"))
            info.rpeat = 2;
        else if (rruleslist.contains("FREQ=DAILY")) {
            info.rpeat = 1;
        } else if (rruleslist.contains("FREQ=WEEKLY")) {
            info.rpeat = 3;
        } else if (rruleslist.contains("FREQ=MONTHLY")) {
            info.rpeat = 4;
        } else if (rruleslist.contains("FREQ=YEARLY")) {
            info.rpeat = 5;
        }
        info.enddata.type = 0;
        for (int i = 0; i < rruleslist.count(); i++) {
            if (rruleslist.at(i).contains("COUNT=")) {
                QStringList liststr = rruleslist.at(i).split("=", QString::SkipEmptyParts);
                info.enddata.type = 1;
                info.enddata.tcount = liststr.at(1).toInt() - 1;
            }

            if (rruleslist.at(i).contains("UNTIL=")) {
                QStringList liststr = rruleslist.at(i).split("=", QString::SkipEmptyParts);
                info.enddata.type = 2;
                info.enddata.date = QDateTime::fromString(liststr.at(1).left(liststr.at(1).count() - 1), "yyyyMMddThhmmss");
                //info.enddata.date = fromconvertData(liststr.at(1));
                info.enddata.date = info.enddata.date;
            }
        }
    }
}

QString CSchedulesDBus::createScheduleRemind(const ScheduleDtailInfo &info)
{
    if (!info.remind)
        return QString();
    QString str;
    if (info.allday) {
        str = QString::number(info.remindData.n) + ";" + info.remindData.time.toString("hh:mm");
    } else {
        str = QString::number(info.remindData.n);
    }
    return str;
}

void CSchedulesDBus::parsingScheduleRemind(QString str, ScheduleDtailInfo &info)
{
    if (str.isEmpty()) {
        info.remind = false;
        return;
    }
    info.remind = true;
    if (info.allday) {
        QStringList liststr = str.split(";", QString::SkipEmptyParts);
        info.remindData.n = liststr.at(0).toInt();
        info.remindData.time = QTime::fromString(liststr.at(1), "hh:mm");
    } else {
        info.remindData.n = str.toInt();
    }
}

QString CSchedulesDBus::toconvertData(QDateTime date)
{
    QDateTime datetimeutc = QDateTime::fromTime_t(0);
    QString str = date.toString("yyyy-MM-ddThh:mm:ss") + "+" + datetimeutc.toString("hh:mm");
    //QString str = date.toString("yyyy-MM-ddThh:mm:ss") + "Z07:00";
    return str;
}

QDateTime CSchedulesDBus::fromconvertData(QString str)
{
    QStringList liststr = str.split("+", QString::SkipEmptyParts);
    return QDateTime::fromString(liststr.at(0), "yyyy-MM-ddThh:mm:ss");
}

QString CSchedulesDBus::toconvertIGData(QDateTime date)
{
    QDateTime datetimeutc = QDateTime::fromTime_t(0);
    QString str = date.toString("yyyy-MM-ddThh:mm:ss") + "Z" + datetimeutc.toString("hh:mm");
    //QString str = date.toString("yyyy-MM-ddThh:mm:ss") + "Z07:00";
    return str;
}

QDateTime CSchedulesDBus::fromconvertiIGData(QString str)
{
    QStringList liststr = str.split("Z", QString::SkipEmptyParts);
    return QDateTime::fromString(liststr.at(0), "yyyy-MM-ddThh:mm:ss");
}

qint64 CSchedulesDBus::CreateJob(const ScheduleDtailInfo &info)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(createScheduleDtailInfojson(info));
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("CreateJob"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qDebug() << reply;
        return -1;
    }
    QDBusReply<qint64> id = reply;

    return id.value();
}

bool CSchedulesDBus::GetJobs(int startYear, int startMonth, int startDay, int endYear, int endMonth, int endDay, QVector<ScheduleDateRangeInfo> &out)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(startYear) << QVariant::fromValue(startMonth) << QVariant::fromValue(startDay);
    argumentList << QVariant::fromValue(endYear) << QVariant::fromValue(endMonth) << QVariant::fromValue(endDay);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("GetJobs"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

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

        ScheduleDateRangeInfo info;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (subObj.contains("Date")) {
            info.date = QDate::fromString(subObj.value("Date").toString(), "yyyy-MM-dd");
        }
        if (subObj.contains("Jobs")) {
            QJsonArray subarry = subObj.value("Jobs").toArray();
            for (int j = 0; j < subarry.size(); j++) {
                QJsonObject ssubObj = subarry.at(j).toObject();
                info.vData.append(parsingScheduleDtailInfojsonID(ssubObj));
            }
        }
        out.append(info);
    }

    return true;
}

bool CSchedulesDBus::GetJob(qint64 jobId, ScheduleDtailInfo &out)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(jobId);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("GetJob"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

    if (!jobs.isValid())
        return false;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jobs.value().toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject ssubObj = jsonDoc.object();
    out = parsingScheduleDtailInfojsonID(ssubObj);

    return true;
}

bool CSchedulesDBus::UpdateJob(const ScheduleDtailInfo &info)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(createScheduleDtailInfojson(info));
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("UpdateJob"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        //dbus UpdateJob 错误提醒
        qDebug() << "UpdateJob Err";
        qDebug() << argumentList;
        return false;
    }
    //QDBusReply<QString> jobs =  reply;

    //if (!jobs.isValid()) return false;

    return true;
}

bool CSchedulesDBus::DeleteJob(qint64 jobId)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(jobId);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("DeleteJob"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }

    return true;
}

bool CSchedulesDBus::QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QVector<ScheduleDateRangeInfo> &out)
{
    QJsonObject qjson;
    qjson.insert("Key", key);
    qjson.insert("Start", toconvertData(starttime));
    qjson.insert("End", toconvertData(endtime));
    // 构建 JSON 文档
    QJsonDocument qdocument;
    qdocument.setObject(qjson);
    QByteArray qbyteArray = qdocument.toJson(QJsonDocument::Compact);
    QString strJson(qbyteArray);

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(strJson);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("QueryJobs"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

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

        ScheduleDateRangeInfo info;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (subObj.contains("Date")) {
            info.date = QDate::fromString(subObj.value("Date").toString(), "yyyy-MM-dd");
        }
        if (subObj.contains("Jobs")) {
            QJsonArray subarry = subObj.value("Jobs").toArray();
            for (int j = 0; j < subarry.size(); j++) {
                QJsonObject ssubObj = subarry.at(j).toObject();
                info.vData.append(parsingScheduleDtailInfojsonID(ssubObj));
            }
        }
        out.append(info);
    }
    return true;
}

bool CSchedulesDBus::QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QString &out)
{
    QJsonObject qjson;
    qjson.insert("Key", key);
    qjson.insert("Start", toconvertData(starttime));
    qjson.insert("End", toconvertData(endtime));
    // 构建 JSON 文档
    QJsonDocument qdocument;
    qdocument.setObject(qjson);
    QByteArray qbyteArray = qdocument.toJson(QJsonDocument::Compact);
    QString strJson(qbyteArray);

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(strJson);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("QueryJobs"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

    if (!jobs.isValid())
        return false;
    out = jobs.value().toLocal8Bit();
    return true;
}

bool CSchedulesDBus::QueryJobsWithLimit(QDateTime starttime, QDateTime endtime, qint32 maxNum, QVector<ScheduleDateRangeInfo> &out)
{
    QJsonObject qjson;
    qjson.insert("Start", toconvertData(starttime));
    qjson.insert("End", toconvertData(endtime));
    qjson.insert("key", "");
    // 构建 JSON 文档
    QJsonDocument qdocument;
    qdocument.setObject(qjson);
    QByteArray qbyteArray = qdocument.toJson(QJsonDocument::Compact);
    QString strJson(qbyteArray);

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(strJson);
    argumentList << maxNum;
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("QueryJobsWithLimit"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

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

        ScheduleDateRangeInfo info;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (subObj.contains("Date")) {
            info.date = QDate::fromString(subObj.value("Date").toString(), "yyyy-MM-dd");
        }
        if (subObj.contains("Jobs")) {
            QJsonArray subarry = subObj.value("Jobs").toArray();
            for (int j = 0; j < subarry.size(); j++) {
                QJsonObject ssubObj = subarry.at(j).toObject();
                info.vData.append(parsingScheduleDtailInfojsonID(ssubObj));
            }
        }
        out.append(info);
    }
    return true;
}

bool CSchedulesDBus::QueryJobsWithRule(QDateTime starttime, QDateTime endtime, const QString &rule, QVector<ScheduleDateRangeInfo> &out)
{

    QJsonObject qjson;
    qjson.insert("Start", toconvertData(starttime));
    qjson.insert("End", toconvertData(endtime));
    qjson.insert("key", "");
    // 构建 JSON 文档
    QJsonDocument qdocument;
    qdocument.setObject(qjson);
    QByteArray qbyteArray = qdocument.toJson(QJsonDocument::Compact);
    QString strJson(qbyteArray);

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(strJson);
    argumentList << rule;
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("QueryJobsWithRule"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

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

        ScheduleDateRangeInfo info;
        //因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (subObj.contains("Date")) {
            info.date = QDate::fromString(subObj.value("Date").toString(), "yyyy-MM-dd");
        }
        if (subObj.contains("Jobs")) {
            QJsonArray subarry = subObj.value("Jobs").toArray();
            for (int j = 0; j < subarry.size(); j++) {
                QJsonObject ssubObj = subarry.at(j).toObject();
                info.vData.append(parsingScheduleDtailInfojsonID(ssubObj));
            }
        }
        out.append(info);
    }
    return true;
}

bool CSchedulesDBus::GetTypes(QVector<ScheduleType> &out)
{
    QList<QVariant> argumentList;
    //argumentList << QVariant::fromValue(jobId);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("GetTypes"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

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
        out.append(parsingScheduleTypejson(subObj));
    }
    return true;
}

bool CSchedulesDBus::GetType(qint64 jobId, ScheduleType &out)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(jobId);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("GetType"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    QDBusReply<QString> jobs = reply;

    if (!jobs.isValid())
        return false;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jobs.value().toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject subObj = jsonDoc.object();
    out = parsingScheduleTypejson(subObj);
    return true;
}

qint64 CSchedulesDBus::CreateType(const ScheduleType &info)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(createScheduleTypejson(info));
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("CreateType"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return -1;
    }
    QDBusReply<qint64> id = reply;

    return id.value();
}

bool CSchedulesDBus::DeleteType(qint64 jobId)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(jobId);
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("DeleteType"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    return true;
}

bool CSchedulesDBus::UpdateType(const ScheduleType &info)
{
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(createScheduleTypejson(info));
    QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("UpdateType"), argumentList);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        return false;
    }
    return true;
}
