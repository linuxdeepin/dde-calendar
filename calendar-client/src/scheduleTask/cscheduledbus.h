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
#ifndef CSCHEDULEDBUS_H
#define CSCHEDULEDBUS_H

#include "dschedule.h"
#include "huangliData/lunardatastruct.h"
#include "huangliData/dbusdatastruct.h"

#include <QDBusAbstractInterface>
#include <QDBusPendingReply>
#include <QDateTime>


#define DBUS_INTEERFACENAME "com.deepin.dataserver.Calendar"
#define DBUS_NAME "com.deepin.dataserver.Calendar"
#define DBUS_PATH "/com/deepin/dataserver/Calendar"

//TODO:使用新类获取农历信息和日程信息
class CScheduleDBus : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static CScheduleDBus *getInstance();
    static void releaseInstance();
    ~CScheduleDBus();
public:
    static inline const char *staticInterfaceName()
    {
        return DBUS_INTEERFACENAME;
    }
public:
    //获取节假日班休信息
    bool GetFestivalMonth(quint32 year, quint32 month, QVector<FestivalInfo> &out);
    //按天获取农历信息
    bool GetHuangLiDay(const QDate &getDay, CaHuangLiDayInfo &out);
    //按月获取农历信息
    bool GetHuangLiMonth(const quint32 year, const quint32 month, bool &fill, CaHuangLiMonthInfo &out);
    //获取当天的农历月日期和日日期名
    QString getHuangLiShortName(const QDate &date);

private:
    explicit CScheduleDBus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    static CScheduleDBus *m_scheduleDBus;

};
#endif // CSCHEDULEDBUS_H
