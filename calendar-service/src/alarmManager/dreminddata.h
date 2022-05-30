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
#ifndef DREMINDDATA_H
#define DREMINDDATA_H

#include <QString>
#include <QDateTime>
#include <QSharedPointer>

class DRemindData
{
public:
    typedef QSharedPointer<DRemindData> Ptr;
    typedef QVector<Ptr> List;

    DRemindData();

    QString accountID() const;
    void setAccountID(const QString &accountID);

    QString scheduleID() const;
    void setScheduleID(const QString &scheduleID);

    QDateTime recurrenceId() const;
    void setRecurrenceId(const QDateTime &recurrenceId);

    int remindCount() const;
    void setRemindCount(int remindCount);

    int notifyid() const;
    void setNotifyid(int notifyid);

    QDateTime dtRemind() const;
    void setDtRemind(const QDateTime &dtRemind);

    QDateTime dtStart() const;
    void setDtStart(const QDateTime &dtStart);

    QDateTime dtEnd() const;
    void setDtEnd(const QDateTime &dtEnd);

    QString alarmID() const;
    void setAlarmID(const QString &alarmID);

    void updateRemindTimeByCount();

    void updateRemindTimeByMesc(qint64 duration);

private:
    QDateTime getRemindTimeByMesc(qint64 duration);

private:
    QString m_alarmID;
    QString m_accountID;
    QString m_scheduleID;
    QDateTime m_recurrenceId;
    int m_remindCount;
    int m_notifyid;
    QDateTime m_dtRemind;
    QDateTime m_dtStart;
    QDateTime m_dtEnd;
};

#endif // DREMINDDATA_H
