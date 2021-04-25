/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     uniontech  <uniontech@uniontech.com>
*
* Maintainer: uniontech  <chenhaifeng@uniontech.com>
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
#include "exportedinterface.h"
#include "scheduledatamanage.h"
#include "calendarmainwindow.h"

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
    ScheduleDtailInfo info;
    Exportpara para;
    QString tstr = parameters;

    if (!analysispara(tstr, info, para)) {
        return QVariant(false);
    }

    if (action == "CREATE") {
        qint64 tindex = CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->addSchedule(info);

        if (tindex < 0) {
            return QVariant(false);
        }
    } else if (action == "VIEW") {
        dynamic_cast<Calendarmainwindow *>(m_object)->viewWindow(para.viewType, para.viewTime);
    } else if (action == "QUERY") {
        QString qstr;
        bool flag =  CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->queryScheduleInfo(para.ADTitleName, para.ADStartTime, para.ADEndTime, qstr);
        Q_UNUSED(flag);

        return QVariant(qstr);
    } else if (action == "CANCEL") {
        QVector<ScheduleDateRangeInfo> out;
        bool flag =  CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->queryScheduleInfo(para.ADTitleName, para.ADStartTime, para.ADEndTime, out);

        if (!flag) return QVariant(false);

        for (int j = 0; j < out.size(); j++) {
            QVector<ScheduleDtailInfo> scheduleInfolist = out.at(j).vData;
            for (int m = 0; m  < scheduleInfolist.count(); m++) {
                ScheduleDtailInfo newschedule;
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->getScheduleInfoById(scheduleInfolist.at(m).id, newschedule);
                newschedule.ignore.append(scheduleInfolist.at(m).beginDateTime);
                CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->updateScheduleInfo(newschedule);
            }
        }
    }
    dynamic_cast<Calendarmainwindow *>(m_object)->UpdateJob();
    return QVariant(true);
}

bool ExportedInterface::analysispara( QString &parameters, ScheduleDtailInfo &info, Exportpara &para) const
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(parameters.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
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
        CScheduleDataManage::getScheduleDataManage()->getscheduleDataCtrl()->GetType(rootObj.value("Type").toInt(), info.type);
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
void ExportedInterface::parsingScheduleRemind(QString str, ScheduleDtailInfo &info) const
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
void ExportedInterface::parsingScheduleRRule(QString str, ScheduleDtailInfo &info) const
{
    if (str.isEmpty()) {
        info.rpeat = 0;
        return;
    }
    QString rrulestrs = str;
    QStringList rruleslist = rrulestrs.split(";", QString::SkipEmptyParts);

    if (rruleslist.count() > 0) {
        if (rruleslist.contains("FREQ=DAILY") && rruleslist.contains("BYDAY=MO,TU,WE,TH,FR")) info.rpeat = 2;
        else if (rruleslist.contains("FREQ=DAILY") ) {
            info.rpeat = 1;
        } else if (rruleslist.contains("FREQ=WEEKLY") ) {
            info.rpeat = 3;
        } else if (rruleslist.contains("FREQ=MONTHLY") ) {
            info.rpeat = 4;
        } else if (rruleslist.contains("FREQ=YEARLY") ) {
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
                info.enddata.date = info.enddata.date.addDays(1);
            }
        }
    }
}
QDateTime ExportedInterface::fromconvertData(QString str) const
{
    QStringList liststr = str.split("+", QString::SkipEmptyParts);
    return QDateTime::fromString(liststr.at(0), "yyyy-MM-ddThh:mm:ss");
}
