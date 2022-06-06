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
#include "dservicebase.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QFile>

DServiceBase::DServiceBase(const QString &path, const QString &interface, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_interface(interface)
{
}

QString DServiceBase::getPath() const
{
    return m_path;
}

QString DServiceBase::getInterface() const
{
    return m_interface;
}

QString DServiceBase::getClientName()
{
    uint pid = QDBusConnection::sessionBus().interface()->servicePid(message().service());
    QString name;
    QFile file(QString("/proc/%1/status").arg(pid));
    if (file.open(QFile::ReadOnly)) {
        name = QString(file.readLine()).section(QRegExp("([\\t ]*:[\\t ]*|\\n)"), 1, 1);
        file.close();
    }
    return name;
}

bool DServiceBase::clientWhite(const int index)
{
    //TODO:根据编号,获取不同到白名单
    static QStringList whiteList_0 {"dde-calendar"};
    static QStringList whiteList_1 {"dde-calendar", ""};

    return true; //whiteList_0.contains(getClientName());
}
