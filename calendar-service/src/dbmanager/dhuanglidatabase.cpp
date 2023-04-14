// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhuanglidatabase.h"
#include "units.h"

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

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
    QString strtable = QString("festival_%1").arg(year);
    QString strsql = QString("SELECT id,month,name,description,rest,list FROM %1 WHERE month = %2").arg(strtable).arg(month);
    SqliteQuery query(strsql, m_database);
    QString strjson;
    if (query.exec()) {
        QJsonDocument doc;
        QJsonArray arr;
        while (query.next()) {
            QJsonObject obj;
            obj.insert("id", query.value("id").toString());
            obj.insert("month", query.value("month").toInt());
            obj.insert("name", query.value("name").toString());
            obj.insert("rest", query.value("rest").toString());
            obj.insert("description", query.value("description").toString());
            QString strlist = query.value("list").toString();
            QJsonParseError error;
            QJsonArray listarr;
            QJsonDocument doctmp = QJsonDocument::fromJson(strlist.toLocal8Bit(), &error);
            if (!doctmp.isNull()) {
                listarr = doctmp.array();
            } else {
                qDebug() << __FUNCTION__ << error.errorString();
            }
            obj.insert("list", listarr);
            arr.append(obj);
        }
        doc.setArray(arr);
        strjson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    } else {
        qWarning() << Q_FUNC_INFO << query.lastError();
    }
    if (query.isActive()) {
        query.finish();
    }
    return strjson;
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
