// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef JOBREMINDMANAGER_H
#define JOBREMINDMANAGER_H
#include "src/commondatastruct.h"

#include <QObject>
#include <QMap>

class QTimer;
class DbusUIOpenSchedule;
class DBusNotify;

class JobRemindManager : public QObject
{
    Q_OBJECT
public:
    explicit JobRemindManager(QObject *parent = nullptr);

private:
    void CallUiOpenSchedule(const Job &job);

    int GetRemindAdvanceDays(const QString &remind);
    bool GetRemindLaterDuration(int count, qint64 &duration);
    QString GetRemindBody(const Job &job, const QDateTime &tm);
    void RemindJobLater(const Job &job, const int operationNum);
    QString GetBodyTimePart(const QDateTime &nowtime, const QDateTime &jobtime, bool allday, bool isstart);

signals:
    void ModifyJobRemind(const Job &job, const QString &remind);
    void saveNotifyID(const Job &job, int notifyid);
public slots:
    void UpdateRemindJobs(const QList<Job> &jobs);
    void NotifyJobsChanged(const QList<Job> &jobs);
    void RemindJob(const Job &job);
    /**
     * @brief notifyMsgHanding      通知提示框交互处理
     * @param job                   日程信息
     * @param operationNum          操作编号
     *                              1：打开日历，
     *                              2：稍后提醒 21：15min后提醒 22：一个小时后提醒 23：四个小时后提醒
     *                              3：明天提醒 4： 提前1天提醒
     */
    void notifyMsgHanding(const Job &job, const int operationNum);

    /**
     * @brief closeNotification     关闭桌面顶部通知
     * @param notifyID              通知ID
     */
    void closeNotification(quint32 notifyID);
private:
    DbusUIOpenSchedule *m_dbusuiopen; //打开日历前端dbus操作相关
    DBusNotify *m_dbusnotify; //日程提醒dbus操作相关
};

#endif // JOBREMINDMANAGER_H
