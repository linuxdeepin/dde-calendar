// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "units.h"
#include "commondef.h"
#include <QDir>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QSaveFile>
#include <QUrl>

#include <QTimeZone>
#include <QStandardPaths>
#include <QLocale>
#include <QSharedPointer>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>

QString dtToString(const QDateTime &dt)
{
    // qCDebug(CommonLogger) << "Converting QDateTime to string:" << dt;
    const int offsetSeconds = dt.timeZone().offsetFromUtc(dt);
    const QChar sign = offsetSeconds < 0 ? QLatin1Char('-') : QLatin1Char('+');
    const int absoluteSeconds = qAbs(offsetSeconds);
    const int hours = absoluteSeconds / 3600;
    const int minutes = (absoluteSeconds % 3600) / 60;
    return QStringLiteral("%1%2%3:%4")
        .arg(dt.toString(QStringLiteral("yyyy-MM-ddThh:mm:ss")))
        .arg(sign)
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'));
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
    // 保留字符串中的时区/UTC 偏移，显示层再根据当前系统时区转换。
    // 不能在这里转成本地时间，否则用户修改系统时区后会丢失原始时刻。
    return QDateTime::fromString(st, Qt::ISODate);
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

bool DownloadFile(const QString &url, const QString &filename)
{
    qCDebug(CommonLogger) << "Downloading file from" << url << "to" << filename;

    QNetworkAccessManager manager;
    QNetworkRequest request{QUrl(url)};
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::NoLessSafeRedirectPolicy);
#endif

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&loop, reply]() {
        reply->abort();
        loop.quit();
    });
    timer.start(15000);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(CommonLogger) << "Download failed:" << url << reply->errorString();
        reply->deleteLater();
        return false;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();

    // QSaveFile 提交时原子替换，失败时保留原有缓存
    QSaveFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(CommonLogger) << "Cannot open download file:" << filename;
        return false;
    }
    if (file.write(data) != data.size() || !file.commit()) {
        qCWarning(CommonLogger) << "Failed to write downloaded data to" << filename;
        return false;
    }
    return true;
}

bool withinTimeFrame(const QDate &date)
{
    // qCDebug(CommonLogger) << "Checking if date" << date << "is within time frame.";
    return date.isValid() && (date.year() >= 1900 && date.year() <=2100);
}

bool isCommunityEdition()
{
    // Add static cache, query only once    
    static bool cachedResult = false;
    static bool hasQueried = false;
    qCDebug(CommonLogger) << "isCommunityEdition";

    // If already queried, return cached result
    if (hasQueried) {
        qCDebug(CommonLogger) << "isCommunityEdition cached";
        return cachedResult;
    }

    QDBusInterface interface("org.deepin.dde.SystemInfo1",
                            "/org/deepin/dde/SystemInfo1",
                            "org.deepin.dde.SystemInfo1",
                            QDBusConnection::sessionBus());

    if(!interface.isValid()) {
        qCDebug(CommonLogger) << "SystemInfo DBus interface invalid";
        hasQueried = true;
        return cachedResult;
    }

    QVariant distroID = interface.property("DistroID");
    if(!distroID.isValid()) {
        qCDebug(CommonLogger) << "Failed to get DistroID property";
        hasQueried = true;
        return cachedResult;
    }

    QString distroIDStr = distroID.toString();
    if(distroIDStr.isEmpty()) {
        qCDebug(CommonLogger) << "DistroID property is empty";
        hasQueried = true;
        return cachedResult;
    }

    // Check if DistroID is "Deepin" (case insensitive)
    qCDebug(CommonLogger) << "DistroID:" << distroIDStr;
    cachedResult = (distroIDStr.toLower() == "deepin");
    hasQueried = true;
    qCDebug(CommonLogger) << "Is community edition (Deepin):" << cachedResult;
    return cachedResult;
}

