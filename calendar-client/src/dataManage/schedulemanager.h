/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
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
#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H

#include "accountmanager.h"
#include <QObject>

class ScheduleManager : public QObject
{
    Q_OBJECT
public:
    explicit ScheduleManager(QObject *parent = nullptr);

    static ScheduleManager* getInstace();

    QMap<QDate, DSchedule::List> getAllSchedule();

    QMap<QDate, bool> getAllScheduleDate();

    void resetSchedule(int year);

signals:
    void signalScheduleUpdate();

public slots:
    void slotScheduleUpdate();

private:
    void initconnect();

    void updateDate();

private:
    QMap<QDate, DSchedule::List> m_scheduleMap;
};
#define gScheduleManager ScheduleManager::getInstace()
#endif // SCHEDULEMANAGER_H
