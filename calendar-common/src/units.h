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
#ifndef UNITS_H
#define UNITS_H

#include <QString>
#include <QDateTime>

const QString serviceBaseName = "com.deepin.dataserver.Calendar";
const QString serviceBasePath = "/com/deepin/dataserver/Calendar";

QString dtToString(const QDateTime &dt);
QDateTime dtFromString(const QString &st);

//获取新数据库路径
QString getDBPath();

//时间转换，
QDateTime dtConvert(const QDateTime &datetime);

#endif // UNITS_H
