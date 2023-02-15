/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     kongyunzhen <kongyunzhen@uniontech.com>
*
* Maintainer: kongyunzhen <kongyunzhen@uniontech.com>
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
#ifndef DBUSUIOPENSCHEDULE_H
#define DBUSUIOPENSCHEDULE_H
#include <QtDBus/QtDBus>

class DbusUIOpenSchedule : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.Calendar";
    }
    explicit DbusUIOpenSchedule(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);
    inline void OpenSchedule(const QString strjson)
    {
        QList<QVariant> argumentList;
        argumentList << strjson;
        callWithArgumentList(QDBus::Block, QStringLiteral("OpenSchedule"), argumentList);
    }
};

#endif // DBUSUIOPENSCHEDULE_H
