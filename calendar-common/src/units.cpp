// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "units.h"
#include "commondef.h"
#include <QDir>
#include <QProcess>

#include <QTimeZone>
#include <QStandardPaths>
#include <QLocale>
#include <QSharedPointer>

QString dtToString(const QDateTime &dt)
{
    // qCDebug(CommonLogger) << "Converting QDateTime to string:" << dt;
    QTime _offsetTime = QTime(0, 0).addSecs(dt.timeZone().offsetFromUtc(dt));
    return QString("%1+%2").arg(dt.toString("yyyy-MM-ddThh:mm:ss")).arg(_offsetTime.toString("hh:mm"));
}

QDateTime dtConvert(const QDateTime &datetime)
{
    // qCDebug(CommonLogger) << "Converting QDateTime:" << datetime;
    QDateTime dt = datetime;
    dt.setOffsetFromUtc(dt.offsetFromUtc());
    return dt;
}

QDateTime dtFromString(const QString &st)
{
    // qCDebug(CommonLogger) << "Converting string to QDateTime:" << st;
    QDateTime &&dtSt = QDateTime::fromString(st, Qt::ISODate);
    //转换为本地时区
    return QDateTime(dtSt.date(),dtSt.time());
}

QString getDBPath()
{
    // qCDebug(CommonLogger) << "Getting DB path.";
    return getHomeConfigPath().append("/deepin/dde-calendar-service");
}

QDate dateFromString(const QString &date)
{
    // qCDebug(CommonLogger) << "Converting string to QDate:" << date;
    return QDate::fromString(date, Qt::ISODate);
}

QString dateToString(const QDate &date)
{
    // qCDebug(CommonLogger) << "Converting QDate to string:" << date;
    return date.toString("yyyy-MM-dd");
}

bool isChineseEnv()
{
    // qCDebug(CommonLogger) << "Checking for Chinese environment.";
    return QLocale::system().name().startsWith("zh_");
}

QString getHomeConfigPath()
{
    qCDebug(CommonLogger) << "Getting home config path.";
    //根据环境变量获取config目录
    QString configPath = QString(qgetenv("XDG_CONFIG_HOME"));
    if(configPath.trimmed().isEmpty()) {
        qCDebug(CommonLogger) << "XDG_CONFIG_HOME is empty, using QStandardPaths.";
        configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    }
    return configPath;
}

QDir getAppConfigDir()
{
    // qCDebug(CommonLogger) << "Getting app config directory.";
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QDir getAppCacheDir()
{
    // qCDebug(CommonLogger) << "Getting app cache directory.";
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

QSharedPointer<QProcess> DownloadFile(QString url, QString filename)
{
    qCDebug(CommonLogger) << "Downloading file from" << url << "to" << filename;
    auto process = QSharedPointer<QProcess>::create();
    process->setEnvironment({"LANGUAGE=en"});
    process->start("wget", { "-c", "-N", "-O", filename, url });
    return process;
}

bool withinTimeFrame(const QDate &date)
{
    // qCDebug(CommonLogger) << "Checking if date" << date << "is within time frame.";
    return date.isValid() && (date.year() >= 1900 && date.year() <=2100);
}

