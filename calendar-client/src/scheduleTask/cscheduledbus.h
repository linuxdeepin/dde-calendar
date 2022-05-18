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
    //创建日程
    qint64 CreateJob(const DSchedule &info);
    //根据开始结束日期获取日程
    bool GetJobs(const QDate &startDate, const QDate &endDate, QMap<QDate, QVector<DSchedule>> &info);
    //根据日程id 获取日程
    bool GetJob(qint64 jobId, DSchedule &out);
    //更新日程信息
    bool UpdateJob(const DSchedule &info);
    //删除日程
    bool DeleteJob(qint64 jobId);
    //查询日程
    bool QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QMap<QDate, QVector<DSchedule>> &out);
    //查询日程
    bool QueryJobs(QString key, QDateTime starttime, QDateTime endtime, QString &outStr);
    //获取节假日班休信息
    bool GetFestivalMonth(quint32 year, quint32 month, QVector<FestivalInfo> &out);
    //按天获取农历信息
    bool GetHuangLiDay(const QDate &getDay, CaHuangLiDayInfo &out);
    //按月获取农历信息
    bool GetHuangLiMonth(const quint32 year, const quint32 month, bool &fill, CaHuangLiMonthInfo &out);
    //获取当天的农历月日期和日日期名
    QString getHuangLiShortName(const QDate &date);

    //获取日程类型信息列表
    bool GetJobTypeList(QString &strJson);
    //新增日程类型信息
    bool AddJobType(QString strJson);
    //更新日程类型信息
    bool UpdateJobType(QString strJson);
    //删除日程类型信息
    bool DeleteJobType(int jobTypeNo);
    //获取日程类型是否被使用
    bool isJobTypeUsed(int jobTypeNo);
    //获取颜色信息列表
    bool GetJobTypeColorList(QString &strJson);
signals:
    void jobsUpdate();
    //日程类型或颜色更新
    void jobsTypeOrColorUpdate();
public slots:
    void propertyChanged(const QDBusMessage &msg);
private:
    explicit CScheduleDBus(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    static CScheduleDBus *m_scheduleDBus;

};
#endif // CSCHEDULEDBUS_H
