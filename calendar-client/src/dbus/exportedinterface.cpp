// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "exportedinterface.h"
#include "scheduledatamanage.h"
#include "calendarmainwindow.h"
#include "cscheduleoperation.h"

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
    ScheduleDataInfo info;
    Exportpara para;
    QString tstr = parameters;
    CScheduleOperation _scheduleOperation;

    if (!analysispara(tstr, info, para)) {
        return QVariant(false);
    }

    if (action == "CREATE") {
        // 创建日程
        bool _createSucc = _scheduleOperation.createSchedule(info);
        //如果创建失败
        if (!_createSucc) {
            return QVariant(false);
        }
    } else if (action == "VIEW") {
        dynamic_cast<Calendarmainwindow *>(m_object)->viewWindow(para.viewType);
    } else if (action == "QUERY") {
        // 对外接口查询日程
        QString qstr = _scheduleOperation.queryScheduleStr(para.ADTitleName, para.ADStartTime, para.ADEndTime);
        return QVariant(qstr);
    } else if (action == "CANCEL") {
        //对外接口删除日程
        QMap<QDate, QVector<ScheduleDataInfo> > out;
//        //口查询日程
        if (_scheduleOperation.queryScheduleInfo(para.ADTitleName, para.ADStartTime, para.ADEndTime, out)) {
            //删除查询到的日程
            QMap<QDate, QVector<ScheduleDataInfo> >::const_iterator _iterator = nullptr;
            for (_iterator = out.constBegin(); _iterator != out.constEnd(); ++_iterator) {
                for (int i = 0 ; i < _iterator.value().size(); ++i) {
                    _scheduleOperation.deleteOnlyInfo(_iterator.value().at(i));
                }
            }
        } else {
            return QVariant(false);
        }
    }
    return QVariant(true);
}

bool ExportedInterface::analysispara(QString &parameters, ScheduleDataInfo &info, Exportpara &para) const
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(parameters.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
    info = ScheduleDataInfo::JsonToSchedule(rootObj);

    if (rootObj.contains("ViewName")) {
        para.viewType = rootObj.value("ViewName").toInt();
    }
    if (rootObj.contains("ViewTime")) {
        para.viewTime = QDateTime::fromString(rootObj.value("ViewTime").toString(), "yyyy-MM-ddThh:mm:ss");
    }
    if (rootObj.contains("ADTitleName")) {
        para.ADTitleName = rootObj.value("ADTitleName").toString();
    }
    if (rootObj.contains("ADStartTime")) {
        para.ADStartTime = QDateTime::fromString(rootObj.value("ADStartTime").toString(), "yyyy-MM-ddThh:mm:ss");
    }
    if (rootObj.contains("ADEndTime")) {
        para.ADEndTime = QDateTime::fromString(rootObj.value("ADEndTime").toString(), "yyyy-MM-ddThh:mm:ss");
    }
    return true;
}

