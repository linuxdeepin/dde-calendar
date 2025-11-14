// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhuanglidatabase.h"
#include "units.h"
#include "ddlogService.h"

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QFile>
#include <QProcess>
#include <QTimer>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

const QString HolidayDir = ":/holiday-cn";
const QString HolidayUpdateURLPrefix = "https://cdn-nu-common.uniontech.com/deepin-calendar";
const QString HolidayUpdateDateSetKey = "festivalUpdateDate";

using namespace DDLOG_SERVICE;

DHuangLiDataBase::DHuangLiDataBase(QObject *parent)
    : DDataBase(parent)
    , m_settings(getAppConfigDir().filePath("config.ini"), QSettings::IniFormat)
{
    qDebug() << "Config file:" << getAppConfigDir().filePath("config.ini");
    QString huangliPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                 QString("dde-calendar/data/huangli.db"),
                                                 QStandardPaths::LocateFile);
    setDBPath(huangliPath);
    setConnectionName("HuangLi");
    dbOpen();

    // 延迟1-10秒后从网络更新节假日数据
    QTimer::singleShot(QRandomGenerator::global()->bounded(1000, 9999),
                       this,
                       &DHuangLiDataBase::updateFestivalList);
}

// readJSON 会读取一个JSON文件，如果 cache 为 true，则会缓存对象，以供下次使用
QJsonDocument DHuangLiDataBase::readJSON(QString filename, bool cache)
{
    if (cache && readJSONCache.contains(filename)) {
        return readJSONCache.value(filename);
    }
    qDebug() << "Reading JSON file:" << filename;
    QJsonDocument doc;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open JSON file:" << filename;
    } else {
        auto data = file.readAll();
        doc = QJsonDocument::fromJson(data);
    }
    if (cache) {
        readJSONCache.insert(filename, doc);
    }
    return doc;
}

void DHuangLiDataBase::updateFestivalList()
{
    auto now = QDateTime::currentDateTime();
    auto festivalUpdateDate = now.toString("yyyy-MM-dd");
    // 每天更新一次
    if (m_settings.value(HolidayUpdateDateSetKey, "") == festivalUpdateDate) {
        qDebug() << "Festival list already updated today";
        return;
    }
    m_settings.setValue(HolidayUpdateDateSetKey, festivalUpdateDate);
    m_settings.sync();

    // 获取今年和明年的节假日数据
    for (auto i = 0; i < 2; i++) {
        auto year = now.date().year() + i;
        auto filename = getAppCacheDir().filePath(QString("%1.json").arg(year));
        auto url = QString("%1/%2.json").arg(HolidayUpdateURLPrefix).arg(year);
        auto process = DownloadFile(url, filename);
        qDebug() << "Downloading festival data:" << url << "to" << filename;
        connect(process.get(), &QProcess::readyReadStandardError, [process]() {
            qDebug() << "Download error:" << process->readAllStandardError();
        });
        connect(process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                [process, filename](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                qDebug() << "Successfully downloaded festival data to" << filename;
            } else {
                qWarning() << "Failed to download festival data, exit code:" << exitCode;
            }
        });
    }
}

QString DHuangLiDataBase::queryFestivalList(quint32 year, quint8 month)
{
    qDebug() << "Query festival list for year:" << year << "month:" << month;
    QJsonArray resultArray;

    // 优先使用缓存目录中的网络下载数据
    QString filename = getAppCacheDir().filePath(QString("%1.json").arg(year));
    bool useCached = false;

    if (QFile(filename).exists()) {
        // 验证缓存文件的有效性
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();
            // 检查文件不为空且包含有效的JSON
            if (!data.isEmpty()) {
                QJsonParseError parseError;
                QJsonDocument testDoc = QJsonDocument::fromJson(data, &parseError);
                if (parseError.error == QJsonParseError::NoError &&
                    !testDoc.isNull() &&
                    testDoc.object().contains("days")) {
                    useCached = true;
                    qDebug() << "Using cached festival data:" << filename;
                } else {
                    qWarning() << "Cached file is invalid, parse error:" << parseError.errorString();
                }
            } else {
                qWarning() << "Cached file is empty:" << filename;
            }
        }
    }

    if (!useCached) {
        // 如果缓存不存在或无效,使用内置资源
        filename = QString("%1/%2.json").arg(HolidayDir).arg(year);
        qDebug() << "Using built-in festival data:" << filename;
    }

    auto doc = readJSON(filename, true);
    QMap<QString, QJsonObject> festivalMap;

    for (auto val : doc.object().value("days").toArray()) {
        auto day = val.toObject();
        auto name = day.value("name").toString("");
        auto dateStr = day.value("date").toString("");
        QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (quint32(date.year()) == year && quint32(date.month()) == month) {
            // 分组处理同一节假日的多个日期
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
