/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chenhaifeng  <chenhaifeng@uniontech.com>
*
* Maintainer: chenhaifeng  <chenhaifeng@uniontech.com>
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
#include "units.h"

#include <QTimeZone>
#include <QStandardPaths>
#include <QLocale>

QString dtToString(const QDateTime &dt)
{
    QTime _offsetTime = QTime(0, 0).addSecs(dt.timeZone().offsetFromUtc(dt));
    return QString("%1+%2").arg(dt.toString("yyyy-MM-ddThh:mm:ss")).arg(_offsetTime.toString("hh:mm"));
}

QDateTime dtConvert(const QDateTime &datetime)
{
    QDateTime dt = datetime;
    dt.setOffsetFromUtc(dt.offsetFromUtc());
    return dt;
}

QDateTime dtFromString(const QString &st)
{
    return QDateTime::fromString(st, Qt::ISODate);
}

QString getDBPath()
{
    return getHomeConfigPath().append("/deepin/dde-calendar-service");
}

QDate dateFromString(const QString &date)
{
    return QDate::fromString(date, Qt::ISODate);
}

QString dateToString(const QDate &date)
{
    return date.toString("yyyy-MM-dd");
}

bool isChineseEnv()
{
    return QLocale::system().name().startsWith("zh_");
}

QString getHomeConfigPath()
{
    //根据环境变量获取config目录
    return QString(qgetenv("XDG_CONFIG_HOME"));
}
