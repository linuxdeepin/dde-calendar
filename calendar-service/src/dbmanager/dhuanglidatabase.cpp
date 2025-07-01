// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhuanglidatabase.h"
#include "units.h"
#include "ddlogService.h"

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

const QString HolidayDir = ":/holiday-cn";

using namespace DDLOG_SERVICE;

DHuangLiDataBase::DHuangLiDataBase(QObject *parent)
    : DDataBase(parent)
{
    QString huangliPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                 QString("dde-calendar/data/huangli.db"),
                                                 QStandardPaths::LocateFile);
    setDBPath(huangliPath);
    setConnectionName("HuangLi");
    dbOpen();
}

QString DHuangLiDataBase::queryFestivalList(quint32 year, quint8 month)
{
    QJsonArray resultArray;
    QFile file(QString("%1/%2.json").arg(HolidayDir).arg(year));
    if (file.open(QIODevice::ReadOnly)) {
        auto data = file.readAll();
        file.close();
        auto doc = QJsonDocument::fromJson(data);
        QMap<QString, QJsonObject> festivalMap;
        for (auto val : doc.object().value("days").toArray()) {
            auto day = val.toObject();
            auto name = day.value("name").toString("");
            auto dateStr = day.value("date").toString("");
            QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
            if (quint32(date.year()) == year && quint32(date.month()) == month) {
                // 分组
                if (!festivalMap.contains(name)) {
                    QJsonObject festObj;
                    festObj["id"] = "";
                    festObj["month"] = static_cast<int>(month);
                    festObj["year"] = static_cast<int>(year);
                    festObj["name"] = name;
                    festObj["rest"] = "";
                    festObj["description"] = "";
                    festObj["list"] = QJsonArray();
                    festivalMap.insert(name, festObj);
                }
                QJsonObject itemObj;
                itemObj["status"] = day.value("isOffDay").toBool() ? 1 : 2;
                itemObj["date"] = dateStr;
                QJsonArray listArr = festivalMap[name]["list"].toArray();
                listArr.append(itemObj);
                festivalMap[name]["list"] = listArr;
            }
        }
        for (auto it = festivalMap.begin(); it != festivalMap.end(); ++it) {
            resultArray.append(it.value());
        }
    }
    return QString(QJsonDocument(resultArray).toJson(QJsonDocument::Compact));
}

QList<stHuangLi> DHuangLiDataBase::queryHuangLiByDays(const QList<stDay> &days)
{
    QList<stHuangLi> infos;
    SqliteQuery query(m_database);
    foreach (stDay d, days) {
        //查询的id
        qint64 id = QString().sprintf("%d%02d%02d", d.Year, d.Month, d.Day).toInt();
        QString strsql("SELECT id, avoid, suit FROM huangli WHERE id = %1");
        strsql = strsql.arg(id);
        //数据库中的宜忌信息是从2008年开始的
        stHuangLi sthuangli;
        //因此这里先将sthuangli内容初始化
        sthuangli.ID = id;
        //如果数据库中有查询到数据，则进行赋值，如果没有，则使用初始值
        if (query.exec(strsql) && query.next()) {
            sthuangli.ID = query.value("id").toInt();
            sthuangli.Avoid = query.value("avoid").toString();
            sthuangli.Suit = query.value("suit").toString();
        }
        //将黄历数据放到list中
        infos.append(sthuangli);
    }
    if (query.isActive()) {
        query.finish();
    }
    return infos;
}

void DHuangLiDataBase::initDBData()
{
}

void DHuangLiDataBase::createDB()
{
}
