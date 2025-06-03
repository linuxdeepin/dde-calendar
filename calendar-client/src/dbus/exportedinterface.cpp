// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exportedinterface.h"
#include "scheduledatamanage.h"
#include "calendarmainwindow.h"
#include "cscheduleoperation.h"
#include "accountmanager.h"
#include "units.h"
#include "compatibledata.h"
#include "commondef.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

ExportedInterface::ExportedInterface(QObject *parent)
    : Dtk::Core::DUtil::DExportedInterface(parent)
{
    m_object = parent;
}

QVariant ExportedInterface::invoke(const QString &action, const QString &parameters) const
{
    //对外接口数据设置
    DSchedule::Ptr info;
    Exportpara para;
    QString tstr = parameters;
    CScheduleOperation _scheduleOperation;

    if (!analysispara(tstr, info, para)) {
        qCWarning(ClientLogger) << "Failed to parse parameters for action:" << action;
        return QVariant(false);
    }

    if (action == "CREATE") {
        // 创建日程
        if (info.isNull()) {
            qCWarning(ClientLogger) << "Invalid schedule info for CREATE action";
            return QVariant(false);
        }
        qCDebug(ClientLogger) << "Creating schedule with title:" << info->summary();
        bool _createSucc = _scheduleOperation.createSchedule(info);
        //如果创建失败
        if (!_createSucc) {
            qCWarning(ClientLogger) << "Failed to create schedule:" << info->summary();
            return QVariant(false);
        }
        qCDebug(ClientLogger) << "Successfully created schedule:" << info->summary();
    } else if (action == "VIEW") {
        qCDebug(ClientLogger) << "Viewing calendar with type:" << para.viewType;
        dynamic_cast<Calendarmainwindow *>(m_object)->viewWindow(para.viewType);
    } else if (action == "QUERY") {
        if (gLocalAccountItem) {
            qCDebug(ClientLogger) << "Querying schedules with title:" << para.ADTitleName 
                                  << "from" << para.ADStartTime.toString() 
                                  << "to" << para.ADEndTime.toString();
            DSchedule::Map scheduleMap = DSchedule::fromMapString(gLocalAccountItem->querySchedulesByExternal(para.ADTitleName, para.ADStartTime, para.ADEndTime));
            QString qstr = DDE_Calendar::getExternalSchedule(scheduleMap);
            qCDebug(ClientLogger) << "Found" << scheduleMap.size() << "schedules";
            return QVariant(qstr);
        } else {
            qCWarning(ClientLogger) << "Local account not available for QUERY action";
            return "";
        }
    } else if (action == "CANCEL") {
        qCDebug(ClientLogger) << "Canceling schedules with title:" << para.ADTitleName 
                              << "from" << para.ADStartTime.toString() 
                              << "to" << para.ADEndTime.toString();
        //对外接口删除日程
        QMap<QDate, DSchedule::List> out;
        //口查询日程
        if (gLocalAccountItem && gLocalAccountItem->querySchedulesByExternal(para.ADTitleName, para.ADStartTime, para.ADEndTime, out)) {
            //删除查询到的日程
            QMap<QDate, DSchedule::List>::const_iterator _iterator;
            for (_iterator = out.constBegin(); _iterator != out.constEnd(); ++_iterator) {
                for (int i = 0 ; i < _iterator.value().size(); ++i) {
                    _scheduleOperation.deleteOnlyInfo(_iterator.value().at(i));
                }
            }
        } else {
            qCWarning(ClientLogger) << "No schedules found to cancel or local account not available";
            return QVariant(false);
        }
    }
    return QVariant(true);
}

bool ExportedInterface::analysispara(QString &parameters, DSchedule::Ptr &info, Exportpara &para) const
{
    //如果是创建则info有效
    //如果是其他则para有效
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(parameters.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        qCWarning(ClientLogger) << "Failed to parse JSON parameters:" << json_error.errorString();
        return false;
    }

    QJsonObject rootObj = jsonDoc.object();
    //数据反序列化
    info = DDE_Calendar::getScheduleByExported(parameters);

    if (rootObj.contains("ViewName")) {
        para.viewType = rootObj.value("ViewName").toInt();
        qCDebug(ClientLogger) << "View type:" << para.viewType;
    }
    if (rootObj.contains("ViewTime")) {
        para.viewTime = QDateTime::fromString(rootObj.value("ViewTime").toString(), "yyyy-MM-ddThh:mm:ss");
        qCDebug(ClientLogger) << "View time:" << para.viewTime.toString();
    }
    if (rootObj.contains("ADTitleName")) {
        para.ADTitleName = rootObj.value("ADTitleName").toString();
        qCDebug(ClientLogger) << "Title:" << para.ADTitleName;
    }
    if (rootObj.contains("ADStartTime")) {
        para.ADStartTime = QDateTime::fromString(rootObj.value("ADStartTime").toString(), "yyyy-MM-ddThh:mm:ss");
        qCDebug(ClientLogger) << "Start time:" << para.ADStartTime.toString();
    }
    if (rootObj.contains("ADEndTime")) {
        para.ADEndTime = QDateTime::fromString(rootObj.value("ADEndTime").toString(), "yyyy-MM-ddThh:mm:ss");
        qCDebug(ClientLogger) << "End time:" << para.ADEndTime.toString();
    }
    return true;
}

